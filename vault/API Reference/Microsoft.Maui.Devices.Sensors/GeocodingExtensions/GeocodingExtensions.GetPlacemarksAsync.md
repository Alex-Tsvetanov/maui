---
title: "GeocodingExtensions.GetPlacemarksAsync"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Devices-Sensors
aliases:
  - "Microsoft.Maui.Devices.Sensors.GeocodingExtensions.GetPlacemarksAsync"
declaring_type: "GeocodingExtensions"
member_kind: method
---

# GeocodingExtensions.GetPlacemarksAsync

> [!abstract] Method of [[GeocodingExtensions|GeocodingExtensions]]
> Namespace: `Microsoft.Maui.Devices.Sensors`

Retrieve potential placemarks for a given location specified by `Location`.

## Signature

```csharp
System.Threading.Tasks.Task<System.Collections.Generic.IEnumerable<Microsoft.Maui.Devices.Sensors.Placemark!>!>! static GetPlacemarksAsync(this Microsoft.Maui.Devices.Sensors.IGeocoding! geocoding, Microsoft.Maui.Devices.Sensors.Location! location)
```

## Returns

List of `Placemark` that best match the coordinates or `null` if no placemarks are found.

## Parameters

| Parameter | Description |
|---|---|
| `geocoding` | The object this method is invoked on. |
| `location` | A `Location` instance to find placemarks near. |

## See also

- Declaring type: [[GeocodingExtensions|GeocodingExtensions]]
- [[_Microsoft.Maui.Devices.Sensors|Microsoft.Maui.Devices.Sensors namespace]]
