///|
/// Capture a screenshot through the host platform API.
#borrow(output_path, output_path_wide)
extern "c" fn capture_native_ffi(
  kind : Int,
  display_index : Int,
  x : Int,
  y : Int,
  width : Int,
  height : Int,
  window : UInt64,
  output_path : Bytes,
  output_path_wide : Bytes,
) -> Int = "moonbit_screenshots_capture_to_file"

///|
/// Return a non-negative display index.
fn normalize_display_index(index : Int) -> Int {
  if index < 0 {
    0
  } else {
    index
  }
}

///|
/// Convert a target into the native FFI shape.
fn target_native_fields(
  target : CaptureTarget,
) -> (Int, Int, Int, Int, Int, Int, UInt64) {
  match target {
    AllDisplays => (0, 0, 0, 0, 1, 1, 0UL)
    Display(index) => (1, normalize_display_index(index), 0, 0, 1, 1, 0UL)
    Window(handle) => (2, 0, 0, 0, 1, 1, handle)
    Area(area) => {
      let value = clamp_area(area)
      (3, 0, value.x, value.y, value.width, value.height, 0UL)
    }
  }
}

///|
/// Capture through the host native API.
fn capture_native_to_file(target : CaptureTarget, output_path : String) -> Bool {
  let (kind, display_index, x, y, width, height, window) = target_native_fields(
    target,
  )
  capture_native_ffi(
    kind,
    display_index,
    x,
    y,
    width,
    height,
    window,
    @ffi.to_cstr(output_path),
    @ffi.to_wstr(output_path),
  ) ==
  0
}
