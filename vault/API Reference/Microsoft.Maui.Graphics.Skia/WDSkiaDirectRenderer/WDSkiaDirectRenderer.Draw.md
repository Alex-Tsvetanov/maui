---
title: "WDSkiaDirectRenderer.Draw"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Graphics-Skia
aliases:
  - "Microsoft.Maui.Graphics.Skia.WDSkiaDirectRenderer.Draw"
declaring_type: "WDSkiaDirectRenderer"
member_kind: method
---

# WDSkiaDirectRenderer.Draw

> [!abstract] Method of [[WDSkiaDirectRenderer|WDSkiaDirectRenderer]]
> Namespace: `Microsoft.Maui.Graphics.Skia`

Gets or sets the drawable that provides the content to be rendered.

## Signature

```csharp
void Draw(SkiaSharp.SKCanvas skiaCanvas, Microsoft.Maui.Graphics.RectF dirtyRect)
```

## Parameters

| Parameter | Description |
|---|---|
| `skiaCanvas` | The SkiaSharp canvas to draw on. |
| `dirtyRect` | The rectangle that needs to be redrawn. |

## Remarks

When this property is set, the renderer is invalidated to reflect the new drawable.

## See also

- Declaring type: [[WDSkiaDirectRenderer|WDSkiaDirectRenderer]]
- [[_Microsoft.Maui.Graphics.Skia|Microsoft.Maui.Graphics.Skia namespace]]
