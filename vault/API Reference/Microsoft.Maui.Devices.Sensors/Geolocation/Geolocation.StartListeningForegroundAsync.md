---
title: "Geolocation.StartListeningForegroundAsync"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Devices-Sensors
aliases:
  - "Microsoft.Maui.Devices.Sensors.Geolocation.StartListeningForegroundAsync"
declaring_type: "Geolocation"
member_kind: method
---

# Geolocation.StartListeningForegroundAsync

> [!abstract] Method of [[Geolocation|Geolocation]]
> Namespace: `Microsoft.Maui.Devices.Sensors`

Starts listening to location updates using the `LocationChanged` event or the `ListeningFailed` event. Events may only sent when the app is in the foreground. Requests `LocationWhenInUse` from the user.

## Signature

```csharp
System.Threading.Tasks.Task<bool>! static StartListeningForegroundAsync(Microsoft.Maui.Devices.Sensors.GeolocationListeningRequest! request)
```

## Returns

`true` when listening was started, or `false` when listening couldn't be started.

## Parameters

| Parameter | Description |
|---|---|
| `request` | The listening request parameters to use. |

## See also

- Declaring type: [[Geolocation|Geolocation]]
- [[_Microsoft.Maui.Devices.Sensors|Microsoft.Maui.Devices.Sensors namespace]]
