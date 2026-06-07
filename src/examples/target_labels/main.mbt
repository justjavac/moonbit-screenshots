///|
/// Print a stable label and output name for a capture target.
fn main {
  let target = @screenshots.Area({ x: 20, y: 20, width: 640, height: 360 })
  let label = @screenshots.target_label(target)
  println(label)
  println(@screenshots.output_name(label, 0))
}
