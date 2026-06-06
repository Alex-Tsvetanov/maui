---
title: "SkiaCanvas.SetFillPaint"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Graphics-Skia
aliases:
  - "Microsoft.Maui.Graphics.Skia.SkiaCanvas.SetFillPaint"
declaring_type: "SkiaCanvas"
member_kind: method
---

# SkiaCanvas.SetFillPaint

> [!abstract] Method of [[SkiaCanvas|SkiaCanvas]]
> Namespace: `Microsoft.Maui.Graphics.Skia`

Sets the fill paint for the canvas.

## Signature

```csharp
void override SetFillPaint(Microsoft.Maui.Graphics.Paint paint, Microsoft.Maui.Graphics.RectF rectangle)
```

## Parameters

| Parameter | Description |
|---|---|
| `paint` | The paint to use for filling. If null, white color will be used. |
| `rectangle` | The rectangle that defines the coordinate space for the paint. |

## Remarks

This method handles different types of paints including SolidPaint, LinearGradientPaint, RadialGradientPaint, PatternPaint, and ImagePaint.

## See also

- Declaring type: [[SkiaCanvas|SkiaCanvas]]
- [[_Microsoft.Maui.Graphics.Skia|Microsoft.Maui.Graphics.Skia namespace]]
