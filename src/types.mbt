///|
/// A rectangular region in desktop coordinates.
///
/// `x` and `y` describe the top-left corner. `width` and `height` describe the
/// size in pixels. Use `clamp_area` before handing user input to platform
/// screenshot APIs so negative coordinates become zero and empty dimensions
/// become one pixel.
///
/// # Example
///
/// ```mbt check
/// test {
///   let area = @screenshots.clamp_area({ x: -8, y: 4, width: 0, height: 12 })
///   inspect(area, content="{ x: 0, y: 4, width: 1, height: 12 }")
/// }
/// ```
pub(all) struct CaptureArea {
  x : Int
  y : Int
  width : Int
  height : Int
} derive(Debug, Eq)

///|
/// The desktop object to capture.
///
/// `AllDisplays` captures the whole desktop, `Display(index)` targets a display
/// when the platform API supports it, `Window(handle)` targets an operating
/// system window handle or id, and `Area(area)` captures a rectangular region.
/// `Window` stores the handle as `UInt64` so 64-bit native handles can be passed
/// without truncation.
///
/// # Example
///
/// ```mbt check
/// test {
///   let target = @screenshots.Area({ x: 10, y: 20, width: 300, height: 200 })
///   inspect(@screenshots.target_label(target), content="area-10-20-300x200")
/// }
/// ```
pub(all) enum CaptureTarget {
  AllDisplays
  Display(Int)
  Window(UInt64)
  Area(CaptureArea)
} derive(Debug, Eq)

///|
/// Write a debug representation for a public type.
fn[T : Debug] write_debug_show(value : T, logger : &Logger) -> Unit {
  logger.write_string(value.to_repr().to_string())
}

///|
/// Render a capture area in the same compact shape used by Debug output.
pub impl Show for CaptureArea with fn output(self, logger : &Logger) -> Unit {
  write_debug_show(self, logger)
}

///|
/// Render a capture target in the same compact shape used by Debug output.
pub impl Show for CaptureTarget with fn output(self, logger : &Logger) -> Unit {
  write_debug_show(self, logger)
}
