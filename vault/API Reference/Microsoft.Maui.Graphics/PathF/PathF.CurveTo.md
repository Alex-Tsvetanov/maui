---
title: "PathF.CurveTo"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Graphics
aliases:
  - "Microsoft.Maui.Graphics.PathF.CurveTo"
declaring_type: "PathF"
member_kind: method
---

# PathF.CurveTo

> [!abstract] Method of [[PathF|PathF]]
> Namespace: `Microsoft.Maui.Graphics`

Adds a cubic Bézier curve segment using coordinate values.

## Signatures

```csharp
Microsoft.Maui.Graphics.PathF CurveTo(float c1X, float c1Y, float c2X, float c2Y, float x, float y)
Microsoft.Maui.Graphics.PathF CurveTo(Microsoft.Maui.Graphics.PointF controlPoint1, Microsoft.Maui.Graphics.PointF controlPoint2, Microsoft.Maui.Graphics.PointF point)
```

## Parameters

| Parameter | Description |
|---|---|
| `c1X` | X-coordinate of the first control point. |
| `c1Y` | Y-coordinate of the first control point. |
| `c2X` | X-coordinate of the second control point. |
| `c2Y` | Y-coordinate of the second control point. |
| `x` | X-coordinate of the end point. |
| `y` | Y-coordinate of the end point. |

## Returns

The current path.

## See also

- Declaring type: [[PathF|PathF]]
- [[_Microsoft.Maui.Graphics|Microsoft.Maui.Graphics namespace]]
