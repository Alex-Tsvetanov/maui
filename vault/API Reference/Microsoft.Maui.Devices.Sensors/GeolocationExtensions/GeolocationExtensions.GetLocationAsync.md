---
title: "GeolocationExtensions.GetLocationAsync"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Devices-Sensors
aliases:
  - "Microsoft.Maui.Devices.Sensors.GeolocationExtensions.GetLocationAsync"
declaring_type: "GeolocationExtensions"
member_kind: method
---

# GeolocationExtensions.GetLocationAsync

> [!abstract] Method of [[GeolocationExtensions|GeolocationExtensions]]
> Namespace: `Microsoft.Maui.Devices.Sensors`

Returns the current location of the device.

## Signatures

```csharp
System.Threading.Tasks.Task<Microsoft.Maui.Devices.Sensors.Location?>! static GetLocationAsync(this Microsoft.Maui.Devices.Sensors.IGeolocation! geolocation, Microsoft.Maui.Devices.Sensors.GeolocationRequest! request)
System.Threading.Tasks.Task<Microsoft.Maui.Devices.Sensors.Location?>! static GetLocationAsync(this Microsoft.Maui.Devices.Sensors.IGeolocation! geolocation)
```

## Remarks

The location permissions will be requested at runtime if needed. You might still need to declare something in your app manifest.

## Returns

A `Location` object containing current location information or `null` if no location could be determined.

## Parameters

| Parameter | Description |
|---|---|
| `geolocation` | The object this method is invoked on. |

## See also

- Declaring type: [[GeolocationExtensions|GeolocationExtensions]]
- [[_Microsoft.Maui.Devices.Sensors|Microsoft.Maui.Devices.Sensors namespace]]
