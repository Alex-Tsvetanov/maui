---
title: "Location.CalculateDistance"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Devices-Sensors
aliases:
  - "Microsoft.Maui.Devices.Sensors.Location.CalculateDistance"
declaring_type: "Location"
member_kind: method
---

# Location.CalculateDistance

> [!abstract] Method of [[Location|Location]]
> Namespace: `Microsoft.Maui.Devices.Sensors`

Calculate distance between two locations.

## Signatures

```csharp
double static CalculateDistance(double latitudeStart, double longitudeStart, Microsoft.Maui.Devices.Sensors.Location locationEnd, Microsoft.Maui.Devices.Sensors.DistanceUnits units)
double static CalculateDistance(Microsoft.Maui.Devices.Sensors.Location locationStart, double latitudeEnd, double longitudeEnd, Microsoft.Maui.Devices.Sensors.DistanceUnits units)
double static CalculateDistance(Microsoft.Maui.Devices.Sensors.Location locationStart, Microsoft.Maui.Devices.Sensors.Location locationEnd, Microsoft.Maui.Devices.Sensors.DistanceUnits units)
double static CalculateDistance(double latitudeStart, double longitudeStart, double latitudeEnd, double longitudeEnd, Microsoft.Maui.Devices.Sensors.DistanceUnits units)
```

## Parameters

| Parameter | Description |
|---|---|
| `latitudeStart` | Latitude coordinate of the starting location. |
| `longitudeStart` | Longitude coordinate of the starting location. |
| `locationEnd` | The end location. |
| `units` | The unit in which the result distance is returned. |

## Returns

Distance between two locations in the unit selected.

## See also

- Declaring type: [[Location|Location]]
- [[_Microsoft.Maui.Devices.Sensors|Microsoft.Maui.Devices.Sensors namespace]]
