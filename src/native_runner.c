#include "native_stub.h"

MOONBIT_FFI_EXPORT int32_t moonbit_screenshots_capture_to_file(
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
  if (output_path == NULL || output_path_wide == NULL) {
    return -1;
  }
#if defined(_WIN32) || defined(_WIN64)
  return moonbit_screenshots_capture_windows(
    kind,
    display_index,
    x,
    y,
    width,
    height,
    window_id,
    output_path,
    output_path_wide
  );
#elif defined(__APPLE__) && defined(__MACH__)
  return moonbit_screenshots_capture_macos(
    kind,
    display_index,
    x,
    y,
    width,
    height,
    window_id,
    output_path,
    output_path_wide
  );
#elif defined(__linux__)
  return moonbit_screenshots_capture_linux(
    kind,
    display_index,
    x,
    y,
    width,
    height,
    window_id,
    output_path,
    output_path_wide
  );
#else
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
#endif
}
