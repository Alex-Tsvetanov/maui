---
title: "GeographyUtils.ToCircumferencePositions"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Maps
aliases:
  - "Microsoft.Maui.Maps.GeographyUtils.ToCircumferencePositions"
declaring_type: "GeographyUtils"
member_kind: method
---

# GeographyUtils.ToCircumferencePositions

> [!abstract] Method of [[GeographyUtils|GeographyUtils]]
> Namespace: `Microsoft.Maui.Maps`

Calculates the circumference positions that form the boundary of a circle on the map.

## Signature

```csharp
System.Collections.Generic.List<Microsoft.Maui.Devices.Sensors.Location!>! static ToCircumferencePositions(this Microsoft.Maui.Maps.ICircleMapElement! circle)
```

## Returns

A list of positions that approximate the circle's circumference.

## Parameters

| Parameter | Description |
|---|---|
| `circle` | The circle element to calculate positions for. |

## See also

- Declaring type: [[GeographyUtils|GeographyUtils]]
- [[_Microsoft.Maui.Maps|Microsoft.Maui.Maps namespace]]
