---
title: "LocationExtensions.CalculateDistance"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Devices-Sensors
aliases:
  - "Microsoft.Maui.Devices.Sensors.LocationExtensions.CalculateDistance"
declaring_type: "LocationExtensions"
member_kind: method
---

# LocationExtensions.CalculateDistance

> [!abstract] Method of [[LocationExtensions|LocationExtensions]]
> Namespace: `Microsoft.Maui.Devices.Sensors`

Calculate distance between two locations.

## Signatures

```csharp
double static CalculateDistance(this Microsoft.Maui.Devices.Sensors.Location locationStart, double latitudeEnd, double longitudeEnd, Microsoft.Maui.Devices.Sensors.DistanceUnits units)
double static CalculateDistance(this Microsoft.Maui.Devices.Sensors.Location locationStart, Microsoft.Maui.Devices.Sensors.Location locationEnd, Microsoft.Maui.Devices.Sensors.DistanceUnits units)
```

## Returns

Distance between two locations in the unit selected.

## Parameters

| Parameter | Description |
|---|---|
| `latitudeStart` | Latitude coordinate of the starting location. |
| `longitudeStart` | Longitude coordinate of the starting location. |
| `locationEnd` | The end location. |
| `units` | The unit in which the result distance is returned. |

## See also

- Declaring type: [[LocationExtensions|LocationExtensions]]
- [[_Microsoft.Maui.Devices.Sensors|Microsoft.Maui.Devices.Sensors namespace]]
