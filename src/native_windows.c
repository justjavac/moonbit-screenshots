#include "native_stub.h"

#if defined(_WIN32) || defined(_WIN64)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "user32.lib")

typedef struct {
  int wanted;
  int seen;
  int found;
  RECT rect;
} MoonBitScreenshotsMonitorContext;

typedef struct {
  HMODULE module;
  HDC(WINAPI *create_compatible_dc)(HDC);
  HBITMAP(WINAPI *create_compatible_bitmap)(HDC, int, int);
  HGDIOBJ(WINAPI *select_object)(HDC, HGDIOBJ);
  BOOL(WINAPI *bit_blt)(HDC, int, int, int, int, HDC, int, int, DWORD);
  int(WINAPI *get_di_bits)(HDC, HBITMAP, UINT, UINT, LPVOID, LPBITMAPINFO, UINT);
  BOOL(WINAPI *delete_object)(HGDIOBJ);
  BOOL(WINAPI *delete_dc)(HDC);
} MoonBitScreenshotsGdi;

static FARPROC moonbit_screenshots_gdi_proc(HMODULE module, const char *name) {
  return module == NULL ? NULL : GetProcAddress(module, name);
}

static int moonbit_screenshots_load_gdi(MoonBitScreenshotsGdi *gdi) {
  if (gdi == NULL) {
    return -1;
  }
  HMODULE module = LoadLibraryA("gdi32.dll");
  if (module == NULL) {
    return -1;
  }
  gdi->module = module;
  gdi->create_compatible_dc =
    (HDC(WINAPI *)(HDC))moonbit_screenshots_gdi_proc(module, "CreateCompatibleDC");
  gdi->create_compatible_bitmap = (HBITMAP(WINAPI *)(HDC, int, int))
    moonbit_screenshots_gdi_proc(module, "CreateCompatibleBitmap");
  gdi->select_object =
    (HGDIOBJ(WINAPI *)(HDC, HGDIOBJ))moonbit_screenshots_gdi_proc(module, "SelectObject");
  gdi->bit_blt = (BOOL(WINAPI *)(
    HDC,
    int,
    int,
    int,
    int,
    HDC,
    int,
    int,
    DWORD
  ))moonbit_screenshots_gdi_proc(module, "BitBlt");
  gdi->get_di_bits = (int(WINAPI *)(
    HDC,
    HBITMAP,
    UINT,
    UINT,
    LPVOID,
    LPBITMAPINFO,
    UINT
  ))moonbit_screenshots_gdi_proc(module, "GetDIBits");
  gdi->delete_object =
    (BOOL(WINAPI *)(HGDIOBJ))moonbit_screenshots_gdi_proc(module, "DeleteObject");
  gdi->delete_dc = (BOOL(WINAPI *)(HDC))moonbit_screenshots_gdi_proc(module, "DeleteDC");
  if (
    gdi->create_compatible_dc == NULL ||
    gdi->create_compatible_bitmap == NULL ||
    gdi->select_object == NULL ||
    gdi->bit_blt == NULL ||
    gdi->get_di_bits == NULL ||
    gdi->delete_object == NULL ||
    gdi->delete_dc == NULL
  ) {
    FreeLibrary(module);
    gdi->module = NULL;
    return -1;
  }
  return 0;
}

static void moonbit_screenshots_unload_gdi(MoonBitScreenshotsGdi *gdi) {
  if (gdi != NULL && gdi->module != NULL) {
    FreeLibrary(gdi->module);
    gdi->module = NULL;
  }
}

static BOOL CALLBACK moonbit_screenshots_monitor_proc(
  HMONITOR monitor,
  HDC hdc,
  LPRECT rect,
  LPARAM data
) {
  (void)monitor;
  (void)hdc;
  MoonBitScreenshotsMonitorContext *context =
    (MoonBitScreenshotsMonitorContext *)data;
  if (context->seen == context->wanted) {
    context->rect = *rect;
    context->found = 1;
    return FALSE;
  }
  context->seen += 1;
  return TRUE;
}

static int moonbit_screenshots_windows_rect(
  int32_t kind,
  int32_t display_index,
  int32_t x,
  int32_t y,
  int32_t width,
  int32_t height,
  uint64_t window_id,
  RECT *rect
) {
  if (rect == NULL) {
    return -1;
  }
  if (kind == MOONBIT_SCREENSHOTS_TARGET_AREA) {
    rect->left = x;
    rect->top = y;
    rect->right = x + (width < 1 ? 1 : width);
    rect->bottom = y + (height < 1 ? 1 : height);
    return 0;
  }
  if (kind == MOONBIT_SCREENSHOTS_TARGET_WINDOW) {
    HWND hwnd = (HWND)(uintptr_t)window_id;
    return hwnd != NULL && GetWindowRect(hwnd, rect) ? 0 : -1;
  }
  if (kind == MOONBIT_SCREENSHOTS_TARGET_DISPLAY) {
    MoonBitScreenshotsMonitorContext context;
    context.wanted = display_index < 0 ? 0 : display_index;
    context.seen = 0;
    context.found = 0;
    SetRectEmpty(&context.rect);
    EnumDisplayMonitors(NULL, NULL, moonbit_screenshots_monitor_proc, (LPARAM)&context);
    if (context.found) {
      *rect = context.rect;
      return 0;
    }
  }
  rect->left = GetSystemMetrics(SM_XVIRTUALSCREEN);
  rect->top = GetSystemMetrics(SM_YVIRTUALSCREEN);
  rect->right = rect->left + GetSystemMetrics(SM_CXVIRTUALSCREEN);
  rect->bottom = rect->top + GetSystemMetrics(SM_CYVIRTUALSCREEN);
  return 0;
}

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
) {
  RECT rect;
  if (moonbit_screenshots_windows_rect(
        kind,
        display_index,
        x,
        y,
        width,
        height,
        window_id,
        &rect
      ) != 0) {
    return -1;
  }
  int capture_width = rect.right - rect.left;
  int capture_height = rect.bottom - rect.top;
  if (capture_width <= 0 || capture_height <= 0) {
    return -1;
  }

  MoonBitScreenshotsGdi gdi;
  if (moonbit_screenshots_load_gdi(&gdi) != 0) {
    return -1;
  }
  HDC screen_dc = GetDC(NULL);
  if (screen_dc == NULL) {
    moonbit_screenshots_unload_gdi(&gdi);
    return -1;
  }
  HDC memory_dc = gdi.create_compatible_dc(screen_dc);
  HBITMAP bitmap = gdi.create_compatible_bitmap(
    screen_dc,
    capture_width,
    capture_height
  );
  if (memory_dc == NULL || bitmap == NULL) {
    if (bitmap != NULL) {
      gdi.delete_object(bitmap);
    }
    if (memory_dc != NULL) {
      gdi.delete_dc(memory_dc);
    }
    ReleaseDC(NULL, screen_dc);
    moonbit_screenshots_unload_gdi(&gdi);
    return -1;
  }
  HGDIOBJ old_bitmap = gdi.select_object(memory_dc, bitmap);
  BOOL copied = gdi.bit_blt(
    memory_dc,
    0,
    0,
    capture_width,
    capture_height,
    screen_dc,
    rect.left,
    rect.top,
    SRCCOPY | CAPTUREBLT
  );
  if (!copied) {
    gdi.select_object(memory_dc, old_bitmap);
    gdi.delete_object(bitmap);
    gdi.delete_dc(memory_dc);
    ReleaseDC(NULL, screen_dc);
    moonbit_screenshots_unload_gdi(&gdi);
    return -1;
  }

  BITMAPINFO info;
  memset(&info, 0, sizeof(info));
  info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  info.bmiHeader.biWidth = capture_width;
  info.bmiHeader.biHeight = -capture_height;
  info.bmiHeader.biPlanes = 1;
  info.bmiHeader.biBitCount = 32;
  info.bmiHeader.biCompression = BI_RGB;

  size_t pixel_count = (size_t)capture_width * (size_t)capture_height;
  uint8_t *bgra = (uint8_t *)malloc(pixel_count * 4u);
  uint8_t *rgba = (uint8_t *)malloc(pixel_count * 4u);
  if (bgra == NULL || rgba == NULL) {
    free(bgra);
    free(rgba);
    gdi.select_object(memory_dc, old_bitmap);
    gdi.delete_object(bitmap);
    gdi.delete_dc(memory_dc);
    ReleaseDC(NULL, screen_dc);
    moonbit_screenshots_unload_gdi(&gdi);
    return -1;
  }
  int got = gdi.get_di_bits(
    memory_dc,
    bitmap,
    0,
    (UINT)capture_height,
    bgra,
    &info,
    DIB_RGB_COLORS
  );
  gdi.select_object(memory_dc, old_bitmap);
  gdi.delete_object(bitmap);
  gdi.delete_dc(memory_dc);
  ReleaseDC(NULL, screen_dc);
  moonbit_screenshots_unload_gdi(&gdi);
  if (got == 0) {
    free(bgra);
    free(rgba);
    return -1;
  }
  for (size_t i = 0; i < pixel_count; i++) {
    rgba[i * 4u + 0u] = bgra[i * 4u + 2u];
    rgba[i * 4u + 1u] = bgra[i * 4u + 1u];
    rgba[i * 4u + 2u] = bgra[i * 4u + 0u];
    rgba[i * 4u + 3u] = 255;
  }
  free(bgra);
  int result = moonbit_screenshots_write_png_rgba(
    output_path,
    output_path_wide,
    capture_width,
    capture_height,
    rgba
  );
  free(rgba);
  return result;
}

#else
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
) {
  (void)kind;
  (void)display_index;
  (void)x;
  (void)y;
  (void)width;
  (void)height;
  (void)window_id;
  (void)output_path;
  (void)output_path_wide;
  return -1;
}
#endif
