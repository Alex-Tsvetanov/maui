---
title: "IGeolocation.ListeningFailed"
tags:
  - api
  - member/event
  - ns/Microsoft-Maui-Devices-Sensors
aliases:
  - "Microsoft.Maui.Devices.Sensors.IGeolocation.ListeningFailed"
declaring_type: "IGeolocation"
member_kind: event
---

# IGeolocation.ListeningFailed

> [!abstract] Event of [[IGeolocation|IGeolocation]]
> Namespace: `Microsoft.Maui.Devices.Sensors`

Occurs when an error during listening for location updates arises. When the event is fired, listening for further location updates has been stopped and no further `LocationChanged` events are sent.

## Signature

```csharp
System.EventHandler<Microsoft.Maui.Devices.Sensors.GeolocationListeningFailedEventArgs!>? ListeningFailed
```

## See also

- Declaring type: [[IGeolocation|IGeolocation]]
- [[_Microsoft.Maui.Devices.Sensors|Microsoft.Maui.Devices.Sensors namespace]]
