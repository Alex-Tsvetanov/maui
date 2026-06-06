---
title: "CanvasExtensions (Graphics).FillArc"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Graphics
aliases:
  - "Microsoft.Maui.Graphics.CanvasExtensions.FillArc"
declaring_type: "CanvasExtensions (Graphics)"
member_kind: method
---

# CanvasExtensions (Graphics).FillArc

> [!abstract] Method of [[CanvasExtensions (Graphics)|CanvasExtensions (Graphics)]]
> Namespace: `Microsoft.Maui.Graphics`

Fills the arc with the specified paint. This is a helper method for when filling an arc with a gradient, so that you don't need to worry about calculating the gradient handle locations based on the rectangle size and location.

## Signatures

```csharp
void static FillArc(this Microsoft.Maui.Graphics.ICanvas canvas, float x, float y, float width, float height, float startAngle, float endAngle, Microsoft.Maui.Graphics.Paint paint, bool clockwise)
void static FillArc(this Microsoft.Maui.Graphics.ICanvas canvas, Microsoft.Maui.Graphics.Rect bounds, float startAngle, float endAngle, bool clockwise)
void static FillArc(this Microsoft.Maui.Graphics.ICanvas canvas, Microsoft.Maui.Graphics.RectF bounds, float startAngle, float endAngle, bool clockwise)
```

## Parameters

| Parameter | Description |
|---|---|
| `canvas` | canvas |
| `x` | The x coordinate. |
| `y` | The y coordinate. |
| `width` | The rectangle width. |
| `height` | The rectangle height |
| `startAngle` | The start angle |
| `endAngle` | The end angle |
| `paint` | The paint |
| `clockwise` | The direction to draw the arc |

## See also

- Declaring type: [[CanvasExtensions (Graphics)|CanvasExtensions (Graphics)]]
- [[_Microsoft.Maui.Graphics|Microsoft.Maui.Graphics namespace]]
