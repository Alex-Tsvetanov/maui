---
title: "PathF.GetSegmentInfo"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Graphics
aliases:
  - "Microsoft.Maui.Graphics.PathF.GetSegmentInfo"
declaring_type: "PathF"
member_kind: method
---

# PathF.GetSegmentInfo

> [!abstract] Method of [[PathF|PathF]]
> Namespace: `Microsoft.Maui.Graphics`

Retrieves segment metadata, returning the segment type and output indices pointing into internal collections.

## Signature

```csharp
Microsoft.Maui.Graphics.PathOperation GetSegmentInfo(int segmentIndex, out int pointIndex, out int arcAngleIndex, out int arcClockwiseIndex)
```

## Parameters

| Parameter | Description |
|---|---|
| `segmentIndex` | Segment index. |
| `pointIndex` | Receives the starting point index. |
| `arcAngleIndex` | Receives the starting arc angle index. |
| `arcClockwiseIndex` | Receives the arc clockwise flag index. |

## Returns

The segment operation type, or `Close` if invalid.

## See also

- Declaring type: [[PathF|PathF]]
- [[_Microsoft.Maui.Graphics|Microsoft.Maui.Graphics namespace]]
