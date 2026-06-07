///|
/// Capture a small desktop area into a PNG file.
fn main {
  let target = @screenshots.Area({ x: 20, y: 20, width: 640, height: 360 })
  let output = @screenshots.output_name("example-area", 0)
  if @screenshots.capture_to_file(target, output) {
    println("saved \{output}")
  } else {
    println("screenshot command failed")
  }
}
