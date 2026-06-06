---
title: "Location.Speed"
tags:
  - api
  - member/property
  - ns/Microsoft-Maui-Devices-Sensors
aliases:
  - "Microsoft.Maui.Devices.Sensors.Location.Speed"
declaring_type: "Location"
member_kind: property
---

# Location.Speed

> [!abstract] Property of [[Location|Location]]
> Namespace: `Microsoft.Maui.Devices.Sensors`

Gets or sets the current speed in meters per second at the time when this location was determined.

## Signature

```csharp
double? Speed { get; set; }
```

## Remarks

Returns 0 or `null` if not available. Otherwise the value will range between 0-360. Requires `Accuracy` to be `High` or better and may not be returned when calling `GetLastKnownLocationAsync`.

## See also

- Declaring type: [[Location|Location]]
- [[_Microsoft.Maui.Devices.Sensors|Microsoft.Maui.Devices.Sensors namespace]]
