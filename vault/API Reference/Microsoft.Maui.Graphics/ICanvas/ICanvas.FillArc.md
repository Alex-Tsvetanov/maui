---
title: "ICanvas.FillArc"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Graphics
aliases:
  - "Microsoft.Maui.Graphics.ICanvas.FillArc"
declaring_type: "ICanvas"
member_kind: method
---

# ICanvas.FillArc

> [!abstract] Method of [[ICanvas|ICanvas]]
> Namespace: `Microsoft.Maui.Graphics`

Draws a filled arc onto the canvas.

## Signature

```csharp
void FillArc(float x, float y, float width, float height, float startAngle, float endAngle, bool clockwise)
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

## See also

- Declaring type: [[ICanvas|ICanvas]]
- [[_Microsoft.Maui.Graphics|Microsoft.Maui.Graphics namespace]]
