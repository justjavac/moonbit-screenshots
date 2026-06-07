///|
/// Verify MoonBit capture targets are encoded for the native FFI boundary.
test "native fields encode target variants" {
  let (kind, display, x, y, width, height, window) = target_native_fields(
    Area({ x: -2, y: 3, width: 0, height: 4 }),
  )
  inspect(kind, content="3")
  inspect(display, content="0")
  inspect(x, content="0")
  inspect(y, content="3")
  inspect(width, content="1")
  inspect(height, content="4")
  inspect(window, content="0")
  let (kind, display, _, _, _, _, _) = target_native_fields(Display(-1))
  inspect(kind, content="1")
  inspect(display, content="0")
  let (kind, _, _, _, _, _, window) = target_native_fields(Window(42UL))
  inspect(kind, content="2")
  inspect(window, content="42")
}
