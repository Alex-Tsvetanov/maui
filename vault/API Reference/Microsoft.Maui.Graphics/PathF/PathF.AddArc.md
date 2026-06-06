---
title: "PathF.AddArc"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Graphics
aliases:
  - "Microsoft.Maui.Graphics.PathF.AddArc"
declaring_type: "PathF"
member_kind: method
---

# PathF.AddArc

> [!abstract] Method of [[PathF|PathF]]
> Namespace: `Microsoft.Maui.Graphics`

Adds an elliptical arc segment using coordinate values instead of points.

## Signatures

```csharp
Microsoft.Maui.Graphics.PathF AddArc(float x1, float y1, float x2, float y2, float startAngle, float endAngle, bool clockwise)
Microsoft.Maui.Graphics.PathF AddArc(Microsoft.Maui.Graphics.PointF topLeft, Microsoft.Maui.Graphics.PointF bottomRight, float startAngle, float endAngle, bool clockwise)
```

## Parameters

| Parameter | Description |
|---|---|
| `x1` | The X coordinate of the top-left corner of the bounding rectangle of the ellipse. |
| `y1` | The Y coordinate of the top-left corner of the bounding rectangle of the ellipse. |
| `x2` | The X coordinate of the bottom-right corner of the bounding rectangle of the ellipse. |
| `y2` | The Y coordinate of the bottom-right corner of the bounding rectangle of the ellipse. |
| `startAngle` | Starting angle of the arc in degrees. 0° points to the right (along the positive X axis). Angles increase counter-clockwise. |
| `endAngle` | Ending angle of the arc in degrees, measured with the same convention as `startAngle`. |
| `clockwise` | If true , the arc is drawn in the clockwise direction from `startAngle` to `endAngle`; otherwise it is drawn counter-clockwise (the positive angle direction). |

## Returns

The current path for chaining.

## See also

- Declaring type: [[PathF|PathF]]
- [[_Microsoft.Maui.Graphics|Microsoft.Maui.Graphics namespace]]
