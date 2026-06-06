---
title: "IGeolocation.StartListeningForegroundAsync"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Devices-Sensors
aliases:
  - "Microsoft.Maui.Devices.Sensors.IGeolocation.StartListeningForegroundAsync"
declaring_type: "IGeolocation"
member_kind: method
---

# IGeolocation.StartListeningForegroundAsync

> [!abstract] Method of [[IGeolocation|IGeolocation]]
> Namespace: `Microsoft.Maui.Devices.Sensors`

Starts listening to location updates using the `LocationChanged` event. Events may only sent when the app is in the foreground. Requests `LocationWhenInUse` from the user.

## Signature

```csharp
System.Threading.Tasks.Task<bool>! StartListeningForegroundAsync(Microsoft.Maui.Devices.Sensors.GeolocationListeningRequest! request)
```

## Returns

`true` when listening was started, or `false` when listening couldn't be started.

## Parameters

| Parameter | Description |
|---|---|
| `request` | The listening request parameters to use. |

## See also

- Declaring type: [[IGeolocation|IGeolocation]]
- [[_Microsoft.Maui.Devices.Sensors|Microsoft.Maui.Devices.Sensors namespace]]
