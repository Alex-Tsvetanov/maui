---
title: "GtkSkiaDirectRenderer.Draw"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Graphics-Skia
aliases:
  - "Microsoft.Maui.Graphics.Skia.GtkSkiaDirectRenderer.Draw"
declaring_type: "GtkSkiaDirectRenderer"
member_kind: method
---

# GtkSkiaDirectRenderer.Draw

> [!abstract] Method of [[GtkSkiaDirectRenderer|GtkSkiaDirectRenderer]]
> Namespace: `Microsoft.Maui.Graphics.Skia`

Gets or sets the drawable object that will be rendered.

## Signature

```csharp
void Draw(SkiaSharp.SKCanvas skiaCanvas, Microsoft.Maui.Graphics.RectF dirtyRect)
```

## Parameters

| Parameter | Description |
|---|---|
| `skiaCanvas` | The Skia canvas to draw on. |
| `dirtyRect` | The rectangle region that needs to be redrawn. |

## Remarks

When the drawable is changed, the view is automatically invalidated to trigger a redraw.

## See also

- Declaring type: [[GtkSkiaDirectRenderer|GtkSkiaDirectRenderer]]
- [[_Microsoft.Maui.Graphics.Skia|Microsoft.Maui.Graphics.Skia namespace]]
