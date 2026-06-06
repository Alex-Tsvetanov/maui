---
title: "IGeolocation.GetLocationAsync"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Devices-Sensors
aliases:
  - "Microsoft.Maui.Devices.Sensors.IGeolocation.GetLocationAsync"
declaring_type: "IGeolocation"
member_kind: method
---

# IGeolocation.GetLocationAsync

> [!abstract] Method of [[IGeolocation|IGeolocation]]
> Namespace: `Microsoft.Maui.Devices.Sensors`

Returns the current location of the device.

## Signature

```csharp
System.Threading.Tasks.Task<Microsoft.Maui.Devices.Sensors.Location?>! GetLocationAsync(Microsoft.Maui.Devices.Sensors.GeolocationRequest! request, System.Threading.CancellationToken cancelToken)
```

## Remarks

The location permissions will be requested at runtime if needed. You might still need to declare something in your app manifest.

## Returns

A `Location` object containing current location information or `null` if no location could be determined.

## Parameters

| Parameter | Description |
|---|---|
| `request` | The criteria to use when determining the location of the device. |
| `cancelToken` | A token that can be used for cancelling the operation. |

## See also

- Declaring type: [[IGeolocation|IGeolocation]]
- [[_Microsoft.Maui.Devices.Sensors|Microsoft.Maui.Devices.Sensors namespace]]
