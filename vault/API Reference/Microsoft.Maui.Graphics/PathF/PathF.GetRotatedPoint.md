---
title: "PathF.GetRotatedPoint"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Graphics
aliases:
  - "Microsoft.Maui.Graphics.PathF.GetRotatedPoint"
declaring_type: "PathF"
member_kind: method
---

# PathF.GetRotatedPoint

> [!abstract] Method of [[PathF|PathF]]
> Namespace: `Microsoft.Maui.Graphics`

Computes the position of a point in the path after rotation about a pivot.

## Signature

```csharp
Microsoft.Maui.Graphics.PointF GetRotatedPoint(int pointIndex, Microsoft.Maui.Graphics.PointF pivotPoint, float angle)
```

## Parameters

| Parameter | Description |
|---|---|
| `pointIndex` | Index into the internal point list. |
| `pivotPoint` | The pivot point for rotation. |
| `angle` | Rotation angle in degrees (counter-clockwise positive). |

## Returns

The rotated point.

## Remarks

This helper applies the same rotation semantics used by `Rotate` and does not cache results.

## See also

- Declaring type: [[PathF|PathF]]
- [[_Microsoft.Maui.Graphics|Microsoft.Maui.Graphics namespace]]
