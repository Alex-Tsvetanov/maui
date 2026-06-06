---
title: "IGeolocation.GetLastKnownLocationAsync"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Devices-Sensors
aliases:
  - "Microsoft.Maui.Devices.Sensors.IGeolocation.GetLastKnownLocationAsync"
declaring_type: "IGeolocation"
member_kind: method
---

# IGeolocation.GetLastKnownLocationAsync

> [!abstract] Method of [[IGeolocation|IGeolocation]]
> Namespace: `Microsoft.Maui.Devices.Sensors`

Returns the last known location of the device.

## Signature

```csharp
System.Threading.Tasks.Task<Microsoft.Maui.Devices.Sensors.Location?>! GetLastKnownLocationAsync()
```

## Remarks

The location permissions will be requested at runtime if needed. You might still need to declare something in your app manifest. This location may be a recently cached location.

## Returns

A `Location` object containing recent location information or `null` if no location is known.

## See also

- Declaring type: [[IGeolocation|IGeolocation]]
- [[_Microsoft.Maui.Devices.Sensors|Microsoft.Maui.Devices.Sensors namespace]]
