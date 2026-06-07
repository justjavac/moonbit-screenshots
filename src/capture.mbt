///|
/// Capture a screenshot into a PNG file through the host operating-system API.
///
/// `target` selects the desktop object or rectangle to capture. `output_path`
/// is converted with `justjavac/ffi` into null-terminated native buffers:
/// Windows consumes the UTF-16 wide path, while macOS and Linux consume the
/// UTF-8 path. The function returns `true` only when the platform API succeeds
/// and the PNG file is written completely.
///
/// On Windows the backend uses Win32/GDI, on macOS it uses CoreGraphics and
/// ImageIO, and on Linux it uses X11. Wayland sessions without an accessible
/// X11 display return `false` because there is no single stable desktop-wide
/// screenshot API to call directly.
pub fn capture_to_file(target : CaptureTarget, output_path : String) -> Bool {
  capture_native_to_file(target, output_path)
}
