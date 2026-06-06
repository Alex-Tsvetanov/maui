---
title: "PathF.GetSegmentType"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Graphics
aliases:
  - "Microsoft.Maui.Graphics.PathF.GetSegmentType"
declaring_type: "PathF"
member_kind: method
---

# PathF.GetSegmentType

> [!abstract] Method of [[PathF|PathF]]
> Namespace: `Microsoft.Maui.Graphics`

Gets the count of segment operations excluding a leading `Move` and a trailing `Close`, if present.

## Signature

```csharp
Microsoft.Maui.Graphics.PathOperation GetSegmentType(int aIndex)
```

## Parameters

| Parameter | Description |
|---|---|
| `aIndex` | Segment index. |

## Returns

The `PathOperation` value.

## See also

- Declaring type: [[PathF|PathF]]
- [[_Microsoft.Maui.Graphics|Microsoft.Maui.Graphics namespace]]
