---
title: "GeometryHelper.FlattenGeometry"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Controls-Shapes
aliases:
  - "Microsoft.Maui.Controls.Shapes.GeometryHelper.FlattenGeometry"
declaring_type: "GeometryHelper"
member_kind: method
---

# GeometryHelper.FlattenGeometry

> [!abstract] Method of [[GeometryHelper|GeometryHelper]]
> Namespace: `Microsoft.Maui.Controls.Shapes`

Flattens a `Geometry` into a `PathGeometry` containing only polyline segments.

## Signatures

```csharp
Microsoft.Maui.Controls.Shapes.PathGeometry static FlattenGeometry(Microsoft.Maui.Controls.Shapes.Geometry geoSrc, double tolerance)
void static FlattenGeometry(Microsoft.Maui.Controls.Shapes.PathGeometry pathGeoDst, Microsoft.Maui.Controls.Shapes.Geometry geoSrc, double tolerance, Microsoft.Maui.Controls.Shapes.Matrix matxPrevious)
```

## Parameters

| Parameter | Description |
|---|---|
| `geoSrc` | The source geometry to flatten. |
| `tolerance` | The maximum distance between the curve and the polyline approximation. |

## Returns

A new `PathGeometry` with all curves converted to line segments.

## See also

- Declaring type: [[GeometryHelper|GeometryHelper]]
- [[_Microsoft.Maui.Controls.Shapes|Microsoft.Maui.Controls.Shapes namespace]]
