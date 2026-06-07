#ifndef MOONBIT_SCREENSHOTS_NATIVE_STUB_H
#define MOONBIT_SCREENSHOTS_NATIVE_STUB_H

#include <moonbit.h>
#include <stdint.h>

enum {
  MOONBIT_SCREENSHOTS_TARGET_ALL = 0,
  MOONBIT_SCREENSHOTS_TARGET_DISPLAY = 1,
  MOONBIT_SCREENSHOTS_TARGET_WINDOW = 2,
  MOONBIT_SCREENSHOTS_TARGET_AREA = 3
};

int moonbit_screenshots_write_png_rgba(
  const uint8_t *path,
  const uint8_t *wide_path,
  int width,
  int height,
  const uint8_t *rgba
);

int moonbit_screenshots_capture_windows(
  int32_t kind,
  int32_t display_index,
  int32_t x,
  int32_t y,
  int32_t width,
  int32_t height,
  uint64_t window_id,
  moonbit_bytes_t output_path,
  moonbit_bytes_t output_path_wide
);

int moonbit_screenshots_capture_macos(
  int32_t kind,
  int32_t display_index,
  int32_t x,
  int32_t y,
  int32_t width,
  int32_t height,
  uint64_t window_id,
  moonbit_bytes_t output_path,
  moonbit_bytes_t output_path_wide
);

int moonbit_screenshots_capture_linux(
  int32_t kind,
  int32_t display_index,
  int32_t x,
  int32_t y,
  int32_t width,
  int32_t height,
  uint64_t window_id,
  moonbit_bytes_t output_path,
  moonbit_bytes_t output_path_wide
);

#endif
