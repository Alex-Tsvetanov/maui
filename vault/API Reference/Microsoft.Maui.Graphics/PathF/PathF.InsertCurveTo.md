---
title: "PathF.InsertCurveTo"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Graphics
aliases:
  - "Microsoft.Maui.Graphics.PathF.InsertCurveTo"
declaring_type: "PathF"
member_kind: method
---

# PathF.InsertCurveTo

> [!abstract] Method of [[PathF|PathF]]
> Namespace: `Microsoft.Maui.Graphics`

Inserts a cubic Bézier segment at a specific segment index.

## Signature

```csharp
Microsoft.Maui.Graphics.PathF InsertCurveTo(Microsoft.Maui.Graphics.PointF controlPoint1, Microsoft.Maui.Graphics.PointF controlPoint2, Microsoft.Maui.Graphics.PointF point, int index)
```

## Parameters

| Parameter | Description |
|---|---|
| `controlPoint1` | First control point. |
| `controlPoint2` | Second control point. |
| `point` | End point. |
| `index` | Insertion segment index. |

## Returns

The current path.

## See also

- Declaring type: [[PathF|PathF]]
- [[_Microsoft.Maui.Graphics|Microsoft.Maui.Graphics namespace]]
