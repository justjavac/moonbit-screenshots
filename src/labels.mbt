///|
/// Produce a stable label for a capture target.
///
/// `Area` targets are normalized before formatting so the label always matches
/// the API-ready rectangle used by `capture_to_file`.
///
/// # Example
///
/// ```mbt check
/// test {
///   inspect(
///     @screenshots.target_label(Area({ x: -1, y: 5, width: 0, height: 4 })),
///     content="area-0-5-1x4",
///   )
/// }
/// ```
pub fn target_label(target : CaptureTarget) -> String {
  match target {
    AllDisplays => "all-displays"
    Display(index) => "display-\{index}"
    Window(handle) => "window-\{handle}"
    Area(area) => {
      let value = clamp_area(area)
      "area-\{value.x}-\{value.y}-\{value.width}x\{value.height}"
    }
  }
}

///|
/// Generate a deterministic PNG filename for a captured frame.
///
/// The prefix is preserved exactly so callers can decide whether to include a
/// directory, timestamp, or target label. Negative indexes are normalized to
/// zero to avoid accidental filenames like `shot--1.png`.
///
/// # Example
///
/// ```mbt check
/// test {
///   inspect(@screenshots.output_name("capture", 3), content="capture-3.png")
///   inspect(@screenshots.output_name("capture", -1), content="capture-0.png")
/// }
/// ```
pub fn output_name(prefix : String, index : Int) -> String {
  let value = if index < 0 { 0 } else { index }
  "\{prefix}-\{value}.png"
}
