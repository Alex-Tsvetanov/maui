---
title: "PathF.QuadTo"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Graphics
aliases:
  - "Microsoft.Maui.Graphics.PathF.QuadTo"
declaring_type: "PathF"
member_kind: method
---

# PathF.QuadTo

> [!abstract] Method of [[PathF|PathF]]
> Namespace: `Microsoft.Maui.Graphics`

Adds a quadratic Bézier curve segment using coordinate values.

## Signatures

```csharp
Microsoft.Maui.Graphics.PathF QuadTo(float cx, float cy, float x, float y)
Microsoft.Maui.Graphics.PathF QuadTo(Microsoft.Maui.Graphics.PointF controlPoint, Microsoft.Maui.Graphics.PointF point)
```

## Parameters

| Parameter | Description |
|---|---|
| `cx` | X-coordinate of the control point. |
| `cy` | Y-coordinate of the control point. |
| `x` | X-coordinate of the end point. |
| `y` | Y-coordinate of the end point. |

## Returns

The current path.

## See also

- Declaring type: [[PathF|PathF]]
- [[_Microsoft.Maui.Graphics|Microsoft.Maui.Graphics namespace]]
