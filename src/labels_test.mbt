///|
/// Verify output_name preserves prefixes and normalizes negative indexes.
test "output names are deterministic" {
  inspect(@screenshots.output_name("capture", 3), content="capture-3.png")
  inspect(@screenshots.output_name("capture", -1), content="capture-0.png")
}

///|
/// Verify target labels are stable across the supported target variants.
test "target labels normalize area geometry" {
  inspect(
    @screenshots.target_label(Area({ x: -1, y: 2, width: 0, height: 5 })),
    content="area-0-2-1x5",
  )
  inspect(@screenshots.target_label(Display(2)), content="display-2")
  inspect(@screenshots.target_label(AllDisplays), content="all-displays")
}
