---
title: "ICanvas.DrawArc"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Graphics
aliases:
  - "Microsoft.Maui.Graphics.ICanvas.DrawArc"
declaring_type: "ICanvas"
member_kind: method
---

# ICanvas.DrawArc

> [!abstract] Method of [[ICanvas|ICanvas]]
> Namespace: `Microsoft.Maui.Graphics`

Draws an arc onto the canvas.

## Signature

```csharp
void DrawArc(float x, float y, float width, float height, float startAngle, float endAngle, bool clockwise, bool closed)
```

## Parameters

| Parameter | Description |
|---|---|
| `x` | Starting x coordinate. |
| `y` | Starting y coordinate. |
| `width` | Width of the arc. |
| `height` | Height of the arc. |
| `startAngle` | The angle from the x-axis to the start point of the arc. |
| `endAngle` | The angle from the x-axis to the end point of the arc. |
| `clockwise` | `true` to draw the arc in a clockwise direction; `false` to draw the arc counterclockwise. |
| `closed` | `true` to specify whether the end point of the arc will be connected to the start point; `false` otherwise. |

## See also

- Declaring type: [[ICanvas|ICanvas]]
- [[_Microsoft.Maui.Graphics|Microsoft.Maui.Graphics namespace]]
