---
title: "PathF.GetPointsForSegment"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Graphics
aliases:
  - "Microsoft.Maui.Graphics.PathF.GetPointsForSegment"
declaring_type: "PathF"
member_kind: method
---

# PathF.GetPointsForSegment

> [!abstract] Method of [[PathF|PathF]]
> Namespace: `Microsoft.Maui.Graphics`

Gets the points defining the segment at the specified index.

## Signature

```csharp
Microsoft.Maui.Graphics.PointF[] GetPointsForSegment(int segmentIndex)
```

## Parameters

| Parameter | Description |
|---|---|
| `segmentIndex` | Segment index. |

## Returns

An array of points (length varies by segment type), an empty array for close segments, or null if invalid.

## See also

- Declaring type: [[PathF|PathF]]
- [[_Microsoft.Maui.Graphics|Microsoft.Maui.Graphics namespace]]
