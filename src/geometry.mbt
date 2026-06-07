///|
/// Return zero when a coordinate is negative.
fn max_zero(value : Int) -> Int {
  if value < 0 {
    0
  } else {
    value
  }
}

///|
/// Return one when a dimension is empty or negative.
fn max_one(value : Int) -> Int {
  if value < 1 {
    1
  } else {
    value
  }
}

///|
/// Return the smaller integer.
fn min_int(left : Int, right : Int) -> Int {
  if left < right {
    left
  } else {
    right
  }
}

///|
/// Return the larger integer.
fn max_int(left : Int, right : Int) -> Int {
  if left > right {
    left
  } else {
    right
  }
}

///|
/// Normalize a capture area into coordinates and dimensions accepted by native
/// screenshot APIs.
///
/// Coordinates are clamped to `0`. Width and height are clamped to `1`, which
/// keeps downstream API calls from producing empty rectangles. This is
/// intentionally conservative: it preserves valid values exactly and only fixes
/// values that desktop APIs commonly reject.
///
/// # Example
///
/// ```mbt check
/// test {
///   let area = @screenshots.clamp_area({ x: -10, y: 5, width: 0, height: -3 })
///   inspect(area, content="{ x: 0, y: 5, width: 1, height: 1 }")
/// }
/// ```
pub fn clamp_area(area : CaptureArea) -> CaptureArea {
  {
    x: max_zero(area.x),
    y: max_zero(area.y),
    width: max_one(area.width),
    height: max_one(area.height),
  }
}

///|
/// Return the number of pixels covered by a normalized capture area.
///
/// The input is first passed through `clamp_area`, so empty or negative
/// dimensions still produce a useful one-pixel minimum.
///
/// # Example
///
/// ```mbt check
/// test {
///   inspect(
///     @screenshots.pixel_count({ x: 0, y: 0, width: 4, height: 3 }),
///     content="12",
///   )
///   inspect(
///     @screenshots.pixel_count({ x: 0, y: 0, width: 0, height: 3 }),
///     content="3",
///   )
/// }
/// ```
pub fn pixel_count(area : CaptureArea) -> Int {
  let normalized = clamp_area(area)
  normalized.width * normalized.height
}

///|
/// Intersect two capture areas and return the overlapping rectangle.
///
/// The function uses the rectangles exactly as provided. Normalize user input
/// with `clamp_area` first when you want negative coordinates or empty sizes to
/// be corrected before computing the overlap.
///
/// # Example
///
/// ```mbt check
/// test {
///   debug_inspect(
///     @screenshots.intersect({ x: 0, y: 0, width: 10, height: 10 }, {
///       x: 4,
///       y: 3,
///       width: 10,
///       height: 2,
///     }),
///     content="Some({ x: 4, y: 3, width: 6, height: 2 })",
///   )
/// }
/// ```
pub fn intersect(a : CaptureArea, b : CaptureArea) -> CaptureArea? {
  let left = max_int(a.x, b.x)
  let top = max_int(a.y, b.y)
  let right = min_int(a.x + a.width, b.x + b.width)
  let bottom = min_int(a.y + a.height, b.y + b.height)
  if right <= left || bottom <= top {
    None
  } else {
    Some({ x: left, y: top, width: right - left, height: bottom - top })
  }
}
