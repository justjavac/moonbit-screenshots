# justjavac/screenshots

Native API-backed screenshot helpers.

```mbt check
test {
  let area = @screenshots.clamp_area({ x: -4, y: 8, width: 0, height: 120 })
  inspect(area, content="{ x: 0, y: 8, width: 1, height: 120 }")
}
```

```mbt check
test {
  let target = @screenshots.Area({ x: 10, y: 20, width: 300, height: 200 })
  inspect(@screenshots.target_label(target), content="area-10-20-300x200")
}
```
