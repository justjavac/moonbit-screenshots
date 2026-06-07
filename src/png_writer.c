#include "native_stub.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32) || defined(_WIN64)
#include <wchar.h>
#endif

static uint32_t moonbit_screenshots_crc_table[256];
static int moonbit_screenshots_crc_ready = 0;

static void moonbit_screenshots_make_crc_table(void) {
  if (moonbit_screenshots_crc_ready) {
    return;
  }
  for (uint32_t n = 0; n < 256; n++) {
    uint32_t c = n;
    for (int k = 0; k < 8; k++) {
      c = (c & 1) ? (0xEDB88320u ^ (c >> 1)) : (c >> 1);
    }
    moonbit_screenshots_crc_table[n] = c;
  }
  moonbit_screenshots_crc_ready = 1;
}

static uint32_t moonbit_screenshots_crc32_update(
  uint32_t crc,
  const uint8_t *data,
  size_t len
) {
  moonbit_screenshots_make_crc_table();
  uint32_t c = crc ^ 0xFFFFFFFFu;
  for (size_t i = 0; i < len; i++) {
    c = moonbit_screenshots_crc_table[(c ^ data[i]) & 0xFFu] ^ (c >> 8);
  }
  return c ^ 0xFFFFFFFFu;
}

static uint32_t moonbit_screenshots_adler32(const uint8_t *data, size_t len) {
  uint32_t a = 1;
  uint32_t b = 0;
  for (size_t i = 0; i < len; i++) {
    a = (a + data[i]) % 65521u;
    b = (b + a) % 65521u;
  }
  return (b << 16) | a;
}

static int moonbit_screenshots_write_be32(FILE *file, uint32_t value) {
  uint8_t bytes[4] = {
    (uint8_t)((value >> 24) & 0xFFu),
    (uint8_t)((value >> 16) & 0xFFu),
    (uint8_t)((value >> 8) & 0xFFu),
    (uint8_t)(value & 0xFFu),
  };
  return fwrite(bytes, 1, 4, file) == 4 ? 0 : -1;
}

static int moonbit_screenshots_write_chunk(
  FILE *file,
  const char type[4],
  const uint8_t *data,
  size_t len
) {
  if (len > 0xFFFFFFFFu) {
    return -1;
  }
  if (moonbit_screenshots_write_be32(file, (uint32_t)len) != 0) {
    return -1;
  }
  if (fwrite(type, 1, 4, file) != 4) {
    return -1;
  }
  if (len > 0 && fwrite(data, 1, len, file) != len) {
    return -1;
  }
  uint32_t crc = moonbit_screenshots_crc32_update(0, (const uint8_t *)type, 4);
  crc = moonbit_screenshots_crc32_update(crc, data, len);
  return moonbit_screenshots_write_be32(file, crc);
}

static FILE *moonbit_screenshots_open_write(
  const uint8_t *path,
  const uint8_t *wide_path
) {
#if defined(_WIN32) || defined(_WIN64)
  (void)path;
  return _wfopen((const wchar_t *)wide_path, L"wb");
#else
  (void)wide_path;
  return fopen((const char *)path, "wb");
#endif
}

int moonbit_screenshots_write_png_rgba(
  const uint8_t *path,
  const uint8_t *wide_path,
  int width,
  int height,
  const uint8_t *rgba
) {
  if (path == NULL || width <= 0 || height <= 0 || rgba == NULL) {
    return -1;
  }
  size_t row_bytes = (size_t)width * 4u;
  size_t raw_stride = row_bytes + 1u;
  size_t raw_len = raw_stride * (size_t)height;
  if (row_bytes / 4u != (size_t)width || raw_len / raw_stride != (size_t)height) {
    return -1;
  }
  uint8_t *raw = (uint8_t *)malloc(raw_len);
  if (raw == NULL) {
    return -1;
  }
  for (int y = 0; y < height; y++) {
    size_t raw_offset = (size_t)y * raw_stride;
    raw[raw_offset] = 0;
    memcpy(raw + raw_offset + 1u, rgba + (size_t)y * row_bytes, row_bytes);
  }

  size_t blocks = (raw_len + 65534u) / 65535u;
  size_t zlib_len = 2u + raw_len + blocks * 5u + 4u;
  uint8_t *zlib = (uint8_t *)malloc(zlib_len);
  if (zlib == NULL) {
    free(raw);
    return -1;
  }
  size_t pos = 0;
  zlib[pos++] = 0x78;
  zlib[pos++] = 0x01;
  size_t remaining = raw_len;
  size_t raw_pos = 0;
  while (remaining > 0) {
    uint16_t block_len = remaining > 65535u ? 65535u : (uint16_t)remaining;
    int final_block = remaining <= 65535u;
    zlib[pos++] = (uint8_t)(final_block ? 1 : 0);
    zlib[pos++] = (uint8_t)(block_len & 0xFFu);
    zlib[pos++] = (uint8_t)((block_len >> 8) & 0xFFu);
    zlib[pos++] = (uint8_t)((~block_len) & 0xFFu);
    zlib[pos++] = (uint8_t)(((~block_len) >> 8) & 0xFFu);
    memcpy(zlib + pos, raw + raw_pos, block_len);
    pos += block_len;
    raw_pos += block_len;
    remaining -= block_len;
  }
  uint32_t adler = moonbit_screenshots_adler32(raw, raw_len);
  zlib[pos++] = (uint8_t)((adler >> 24) & 0xFFu);
  zlib[pos++] = (uint8_t)((adler >> 16) & 0xFFu);
  zlib[pos++] = (uint8_t)((adler >> 8) & 0xFFu);
  zlib[pos++] = (uint8_t)(adler & 0xFFu);
  free(raw);

  FILE *file = moonbit_screenshots_open_write(path, wide_path);
  if (file == NULL) {
    free(zlib);
    return -1;
  }
  static const uint8_t signature[8] = { 137, 80, 78, 71, 13, 10, 26, 10 };
  uint8_t ihdr[13] = {
    (uint8_t)((width >> 24) & 0xFF),
    (uint8_t)((width >> 16) & 0xFF),
    (uint8_t)((width >> 8) & 0xFF),
    (uint8_t)(width & 0xFF),
    (uint8_t)((height >> 24) & 0xFF),
    (uint8_t)((height >> 16) & 0xFF),
    (uint8_t)((height >> 8) & 0xFF),
    (uint8_t)(height & 0xFF),
    8,
    6,
    0,
    0,
    0,
  };
  int ok = 0;
  ok |= fwrite(signature, 1, sizeof(signature), file) == sizeof(signature) ? 0 : -1;
  ok |= moonbit_screenshots_write_chunk(file, "IHDR", ihdr, sizeof(ihdr));
  ok |= moonbit_screenshots_write_chunk(file, "IDAT", zlib, pos);
  ok |= moonbit_screenshots_write_chunk(file, "IEND", NULL, 0);
  free(zlib);
  if (fclose(file) != 0) {
    ok = -1;
  }
  return ok == 0 ? 0 : -1;
}
