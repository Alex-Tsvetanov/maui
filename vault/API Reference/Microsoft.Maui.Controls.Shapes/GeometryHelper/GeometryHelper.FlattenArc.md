---
title: "GeometryHelper.FlattenArc"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Controls-Shapes
aliases:
  - "Microsoft.Maui.Controls.Shapes.GeometryHelper.FlattenArc"
declaring_type: "GeometryHelper"
member_kind: method
---

# GeometryHelper.FlattenArc

> [!abstract] Method of [[GeometryHelper|GeometryHelper]]
> Namespace: `Microsoft.Maui.Controls.Shapes`

Flattens an elliptical arc into a series of line segments.

## Signature

```csharp
void static FlattenArc(System.Collections.Generic.List<Microsoft.Maui.Graphics.Point> points, Microsoft.Maui.Graphics.Point pt1, Microsoft.Maui.Graphics.Point pt2, double radiusX, double radiusY, double angleRotation, bool isLargeArc, bool isCounterclockwise, double tolerance)
```

## Parameters

| Parameter | Description |
|---|---|
| `points` | The list to add the resulting points to. |
| `pt1` | The start point of the arc. |
| `pt2` | The end point of the arc. |
| `radiusX` | The x-radius of the ellipse. |
| `radiusY` | The y-radius of the ellipse. |
| `angleRotation` | The rotation angle of the ellipse in degrees. |
| `isLargeArc` | Whether to use the larger of the two possible arcs. |
| `isCounterclockwise` | Whether the arc sweeps counterclockwise. |
| `tolerance` | The maximum distance between the arc and the polyline approximation. |

## Remarks

See http://www.charlespetzold.com/blog/2008/01/Mathematics-of-ArcSegment.html for more information.

## See also

- Declaring type: [[GeometryHelper|GeometryHelper]]
- [[_Microsoft.Maui.Controls.Shapes|Microsoft.Maui.Controls.Shapes namespace]]
