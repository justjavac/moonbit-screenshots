# Examples

Runnable examples live under `src/examples` so all MoonBit source stays inside
the module's `src` tree.

Print a stable label and output name:

```sh
moon run src/examples/target_labels --target native
```

Capture a 640x360 desktop area into `example-area-0.png`:

```sh
moon run src/examples/capture_area --target native
```

Linux captures use X11 through `libX11`. Wayland sessions without XWayland or
an accessible X display return failure instead of falling back to external
commands.
