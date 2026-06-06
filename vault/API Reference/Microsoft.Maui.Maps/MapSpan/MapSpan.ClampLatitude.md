---
title: "MapSpan.ClampLatitude"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Maps
aliases:
  - "Microsoft.Maui.Maps.MapSpan.ClampLatitude"
declaring_type: "MapSpan"
member_kind: method
---

# MapSpan.ClampLatitude

> [!abstract] Method of [[MapSpan|MapSpan]]
> Namespace: `Microsoft.Maui.Maps`

Gets the approximate radius of the span.

## Signature

```csharp
Microsoft.Maui.Maps.MapSpan! ClampLatitude(double north, double south)
```

## Parameters

| Parameter | Description |
|---|---|
| `north` | The northern boundary (will be clamped to 0 to 90). |
| `south` | The southern boundary (will be clamped to -90 to 0). |

## Returns

A new `MapSpan` with the clamped latitude.

## See also

- Declaring type: [[MapSpan|MapSpan]]
- [[_Microsoft.Maui.Maps|Microsoft.Maui.Maps namespace]]
