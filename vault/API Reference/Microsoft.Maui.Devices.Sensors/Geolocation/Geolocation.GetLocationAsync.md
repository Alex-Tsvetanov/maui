---
title: "Geolocation.GetLocationAsync"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Devices-Sensors
aliases:
  - "Microsoft.Maui.Devices.Sensors.Geolocation.GetLocationAsync"
declaring_type: "Geolocation"
member_kind: method
---

# Geolocation.GetLocationAsync

> [!abstract] Method of [[Geolocation|Geolocation]]
> Namespace: `Microsoft.Maui.Devices.Sensors`

Returns the current location of the device.

## Signatures

```csharp
System.Threading.Tasks.Task<Microsoft.Maui.Devices.Sensors.Location?>! static GetLocationAsync()
System.Threading.Tasks.Task<Microsoft.Maui.Devices.Sensors.Location?>! static GetLocationAsync(Microsoft.Maui.Devices.Sensors.GeolocationRequest! request, System.Threading.CancellationToken cancelToken)
System.Threading.Tasks.Task<Microsoft.Maui.Devices.Sensors.Location?>! static GetLocationAsync(Microsoft.Maui.Devices.Sensors.GeolocationRequest! request)
```

## Remarks

The location permissions will be requested at runtime if needed. You might still need to declare something in your app manifest.

## Returns

A `Location` object containing current location information or `null` if no location could be determined.

## See also

- Declaring type: [[Geolocation|Geolocation]]
- [[_Microsoft.Maui.Devices.Sensors|Microsoft.Maui.Devices.Sensors namespace]]
