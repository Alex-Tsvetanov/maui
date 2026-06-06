---
title: "GeometryHelper.FlattenQuadraticBezier"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Controls-Shapes
aliases:
  - "Microsoft.Maui.Controls.Shapes.GeometryHelper.FlattenQuadraticBezier"
declaring_type: "GeometryHelper"
member_kind: method
---

# GeometryHelper.FlattenQuadraticBezier

> [!abstract] Method of [[GeometryHelper|GeometryHelper]]
> Namespace: `Microsoft.Maui.Controls.Shapes`

Flattens a quadratic Bezier curve into a series of line segments.

## Signature

```csharp
void static FlattenQuadraticBezier(System.Collections.Generic.List<Microsoft.Maui.Graphics.Point> points, Microsoft.Maui.Graphics.Point ptStart, Microsoft.Maui.Graphics.Point ptCtrl, Microsoft.Maui.Graphics.Point ptEnd, double tolerance)
```

## Parameters

| Parameter | Description |
|---|---|
| `points` | The list to add the resulting points to. |
| `ptStart` | The start point of the curve. |
| `ptCtrl` | The control point. |
| `ptEnd` | The end point of the curve. |
| `tolerance` | The maximum distance between the curve and the polyline approximation. |

## See also

- Declaring type: [[GeometryHelper|GeometryHelper]]
- [[_Microsoft.Maui.Controls.Shapes|Microsoft.Maui.Controls.Shapes namespace]]
