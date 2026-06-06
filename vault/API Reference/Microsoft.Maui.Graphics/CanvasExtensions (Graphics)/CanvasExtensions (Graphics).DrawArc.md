---
title: "CanvasExtensions (Graphics).DrawArc"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Graphics
aliases:
  - "Microsoft.Maui.Graphics.CanvasExtensions.DrawArc"
declaring_type: "CanvasExtensions (Graphics)"
member_kind: method
---

# CanvasExtensions (Graphics).DrawArc

> [!abstract] Method of [[CanvasExtensions (Graphics)|CanvasExtensions (Graphics)]]
> Namespace: `Microsoft.Maui.Graphics`

Draws the arc. This is a helper method to draw an arc when you have a rectangle already defined for the ellipse bounds.

## Signatures

```csharp
void static DrawArc(this Microsoft.Maui.Graphics.ICanvas canvas, Microsoft.Maui.Graphics.Rect bounds, float startAngle, float endAngle, bool clockwise, bool closed)
void static DrawArc(this Microsoft.Maui.Graphics.ICanvas canvas, Microsoft.Maui.Graphics.RectF bounds, float startAngle, float endAngle, bool clockwise, bool closed)
```

## Parameters

| Parameter | Description |
|---|---|
| `canvas` | canvas |
| `bounds` | The ellipse bounds. |
| `startAngle` | The start angle |
| `endAngle` | The end angle |
| `clockwise` | The direction to draw the arc |
| `closed` | If the arc is closed or not |

## See also

- Declaring type: [[CanvasExtensions (Graphics)|CanvasExtensions (Graphics)]]
- [[_Microsoft.Maui.Graphics|Microsoft.Maui.Graphics namespace]]
