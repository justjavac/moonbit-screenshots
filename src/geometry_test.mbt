///|
/// Verify clamp_area repairs negative coordinates and empty dimensions.
test "capture areas are clamped into valid bounds" {
  let area = @screenshots.clamp_area({ x: -10, y: -5, width: 0, height: -20 })
  inspect(area, content="{ x: 0, y: 0, width: 1, height: 1 }")
}

///|
/// Verify clamp_area leaves already valid rectangles unchanged.
test "valid capture areas are preserved by clamp" {
  let area = @screenshots.clamp_area({ x: 10, y: 5, width: 20, height: 30 })
  inspect(area, content="{ x: 10, y: 5, width: 20, height: 30 }")
}

///|
/// Verify pixel_count applies the same normalization as capture.
test "pixel counts use clamped geometry" {
  inspect(
    @screenshots.pixel_count({ x: 0, y: 0, width: -20, height: 10 }),
    content="10",
  )
}

///|
/// Verify intersect returns the overlap between two rectangles.
test "intersection returns overlapping area" {
  debug_inspect(
    @screenshots.intersect({ x: 0, y: 0, width: 100, height: 100 }, {
      x: 50,
      y: 20,
      width: 60,
      height: 40,
    }),
    content="Some({ x: 50, y: 20, width: 50, height: 40 })",
  )
}

///|
/// Verify touching rectangles do not count as overlapping.
test "intersection returns none when rectangles do not overlap" {
  debug_inspect(
    @screenshots.intersect({ x: 0, y: 0, width: 10, height: 10 }, {
      x: 10,
      y: 10,
      width: 5,
      height: 5,
    }),
    content="None",
  )
}
