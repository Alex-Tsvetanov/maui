---
title: "PathF.Rotate"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Graphics
aliases:
  - "Microsoft.Maui.Graphics.PathF.Rotate"
declaring_type: "PathF"
member_kind: method
---

# PathF.Rotate

> [!abstract] Method of [[PathF|PathF]]
> Namespace: `Microsoft.Maui.Graphics`

Creates a new `PathF` representing this path rotated by the specified angle about a pivot point.

## Signature

```csharp
Microsoft.Maui.Graphics.PathF Rotate(float angleAsDegrees, Microsoft.Maui.Graphics.PointF pivot)
```

## Parameters

| Parameter | Description |
|---|---|
| `angleAsDegrees` | The rotation angle in degrees. Positive angles rotate counter-clockwise; 0° keeps the path unchanged. |
| `pivot` | The pivot point about which all points (and ellipse bounding rectangles for arc segments) are rotated. |

## Returns

A new `PathF` containing the rotated geometry. The original path is not modified.

## Remarks

Rotation uses the same degree-based, counter-clockwise positive convention as arc angles (see `AddArc`). Arc segments preserve their original start and end angle values; only their bounding rectangle corner points are rotated.

## See also

- Declaring type: [[PathF|PathF]]
- [[_Microsoft.Maui.Graphics|Microsoft.Maui.Graphics namespace]]
