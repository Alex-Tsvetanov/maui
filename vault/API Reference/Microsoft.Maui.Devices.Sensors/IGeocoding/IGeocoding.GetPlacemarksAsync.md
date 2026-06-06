---
title: "IGeocoding.GetPlacemarksAsync"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Devices-Sensors
aliases:
  - "Microsoft.Maui.Devices.Sensors.IGeocoding.GetPlacemarksAsync"
declaring_type: "IGeocoding"
member_kind: method
---

# IGeocoding.GetPlacemarksAsync

> [!abstract] Method of [[IGeocoding|IGeocoding]]
> Namespace: `Microsoft.Maui.Devices.Sensors`

Retrieve potential placemarks for a given location specified by coordinates.

## Signature

```csharp
System.Threading.Tasks.Task<System.Collections.Generic.IEnumerable<Microsoft.Maui.Devices.Sensors.Placemark!>!>! GetPlacemarksAsync(double latitude, double longitude)
```

## Returns

List of `Placemark` that best match the coordinates or `null` if no placemarks are found.

## Parameters

| Parameter | Description |
|---|---|
| `latitude` | The latitude coordinate to find placemarks near. |
| `longitude` | The longitude coordinate to find placemarks near. |

## See also

- Declaring type: [[IGeocoding|IGeocoding]]
- [[_Microsoft.Maui.Devices.Sensors|Microsoft.Maui.Devices.Sensors namespace]]
