# justjavac/screenshots

[![coverage](https://img.shields.io/codecov/c/github/justjavac/moonbit-screenshots/main?label=coverage)](https://codecov.io/gh/justjavac/moonbit-screenshots)
[![linux](https://img.shields.io/codecov/c/github/justjavac/moonbit-screenshots/main?flag=linux&label=linux)](https://codecov.io/gh/justjavac/moonbit-screenshots)
[![macos](https://img.shields.io/codecov/c/github/justjavac/moonbit-screenshots/main?flag=macos&label=macos)](https://codecov.io/gh/justjavac/moonbit-screenshots)
[![windows](https://img.shields.io/codecov/c/github/justjavac/moonbit-screenshots/main?flag=windows&label=windows)](https://codecov.io/gh/justjavac/moonbit-screenshots)

`justjavac/screenshots` is a native-only MoonBit package for capturing desktop
screenshots through platform APIs and saving the result as PNG. It models
capture targets in MoonBit, uses `justjavac/ffi` for null-terminated FFI string
buffers, keeps source under `src`, uses the MIT license, and sets
`preferred_target = "native"` in `moon.mod`.

## Install

Add the package after it is published:

```sh
moon add justjavac/screenshots
```

For local development in this repository:

```sh
moon update
moon check --target native
moon test --target native
```

`justjavac/ffi` is declared in `moon.mod`; `moon update` fetches it and
its transitive dependencies.

## Quick Start

Capture the whole desktop:

```mbt
let ok = @screenshots.capture_to_file(@screenshots.AllDisplays, "desktop.png")
if ok {
  println("saved desktop.png")
} else {
  println("capture failed")
}
```

Capture a rectangle:

```mbt
let area = @screenshots.Area({ x: 20, y: 20, width: 640, height: 360 })
let ok = @screenshots.capture_to_file(area, "area.png")
```

Build stable names without capturing:

```mbt
let target = @screenshots.Area({ x: 20, y: 20, width: 640, height: 360 })
let label = @screenshots.target_label(target)
let file = @screenshots.output_name(label, 0)
println(file)
```

## API

`CaptureArea` stores a desktop rectangle with `x`, `y`, `width`, and `height`.
Use `clamp_area` to normalize user input before it crosses the native boundary.
Coordinates below zero become `0`; widths and heights below one become `1`.

`CaptureTarget` describes what to capture:

- `AllDisplays` captures the whole desktop.
- `Display(index)` targets a display when the platform API can address one.
- `Window(handle)` targets a native window handle or id stored as `UInt64`.
- `Area(area)` captures a rectangular region.

Functions:

- `capture_to_file(target, output_path)` captures through the host platform API
  and writes a PNG file.
- `clamp_area(area)` returns an API-safe rectangle.
- `pixel_count(area)` returns the normalized area size in pixels.
- `intersect(a, b)` returns the overlapping rectangle, if one exists.
- `target_label(target)` returns a stable lowercase label.
- `output_name(prefix, index)` returns a deterministic PNG filename.

There is intentionally no public `current_platform` or `platform_name` API.
The package is native-only and dispatches inside the C stub at compile time, so
callers do not need to branch on the host platform before capturing.

## Platform Behavior

Windows uses Win32 and GDI:

- `EnumDisplayMonitors` resolves display indexes.
- `GetWindowRect` resolves `Window(handle)`.
- `BitBlt` copies screen pixels.

macOS dynamically calls CoreGraphics and ImageIO:

- `CGDisplayCreateImage` captures a display.
- `CGDisplayCreateImageForRect` captures a rectangle.
- `CGWindowListCreateImage` captures a window id.
- `CGImageDestination` writes PNG output.

Linux dynamically calls X11 through `libX11`:

- `XOpenDisplay` and `XDefaultRootWindow` locate the active X display.
- `XGetGeometry` resolves root or window dimensions.
- `XGetImage` reads pixels.

Wayland desktops do not expose one universal screenshot API. This package does
not silently fall back to external commands there; `capture_to_file` returns
`false` when the X11 API path is unavailable.

## Examples

The top-level `examples` directory documents runnable packages under
`src/examples`, keeping all MoonBit source inside `src`.

Print a target label and filename:

```sh
moon run src/examples/target_labels --target native
```

Capture a 640x360 area:

```sh
moon run src/examples/capture_area --target native
```

## Coverage

CI runs native tests on Linux, macOS, and Windows with coverage enabled:

```sh
moon test --target native --enable-coverage
moon coverage analyze -p justjavac/screenshots -- -f cobertura -o coverage.xml
```

Codecov uploads one report per operating system with `linux`, `macos`, and
`windows` flags. The badges update after the default branch uploads coverage
and Codecov finishes processing.

## Development

Run the normal maintenance loop before publishing:

```sh
moon fmt
moon check --target all --warn-list +73
moon test --target native
moon test --target native --enable-coverage
moon coverage analyze -p justjavac/screenshots -- -f cobertura -o coverage.xml
moon info --target native
```

The generated `src/pkg.generated.mbti` file should be committed when the public
API changes.

## License

MIT.
