---
title: "Geocoding.GetPlacemarksAsync"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Devices-Sensors
aliases:
  - "Microsoft.Maui.Devices.Sensors.Geocoding.GetPlacemarksAsync"
declaring_type: "Geocoding"
member_kind: method
---

# Geocoding.GetPlacemarksAsync

> [!abstract] Method of [[Geocoding|Geocoding]]
> Namespace: `Microsoft.Maui.Devices.Sensors`

Retrieve potential placemarks for a given location specified by `Location`.

## Signatures

```csharp
System.Threading.Tasks.Task<System.Collections.Generic.IEnumerable<Microsoft.Maui.Devices.Sensors.Placemark!>!>! static GetPlacemarksAsync(double latitude, double longitude)
System.Threading.Tasks.Task<System.Collections.Generic.IEnumerable<Microsoft.Maui.Devices.Sensors.Placemark!>!>! static GetPlacemarksAsync(Microsoft.Maui.Devices.Sensors.Location! location)
```

## Returns

List of `Placemark` that best match the coordinates or `null` if no placemarks are found.

## Parameters

| Parameter | Description |
|---|---|
| `location` | A `Location` instance to find placemarks near. |

## See also

- Declaring type: [[Geocoding|Geocoding]]
- [[_Microsoft.Maui.Devices.Sensors|Microsoft.Maui.Devices.Sensors namespace]]
