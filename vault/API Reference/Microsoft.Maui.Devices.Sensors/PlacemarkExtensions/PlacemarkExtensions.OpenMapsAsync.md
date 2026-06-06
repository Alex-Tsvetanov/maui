---
title: "PlacemarkExtensions.OpenMapsAsync"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Devices-Sensors
aliases:
  - "Microsoft.Maui.Devices.Sensors.PlacemarkExtensions.OpenMapsAsync"
declaring_type: "PlacemarkExtensions"
member_kind: method
---

# PlacemarkExtensions.OpenMapsAsync

> [!abstract] Method of [[PlacemarkExtensions|PlacemarkExtensions]]
> Namespace: `Microsoft.Maui.Devices.Sensors`

Open the browser to specified URI.

## Signatures

```csharp
System.Threading.Tasks.Task static OpenMapsAsync(this Microsoft.Maui.Devices.Sensors.Placemark placemark, Microsoft.Maui.ApplicationModel.MapLaunchOptions options)
System.Threading.Tasks.Task static OpenMapsAsync(this Microsoft.Maui.Devices.Sensors.Placemark placemark)
```

## Returns

Completed task when browser is launched, but not necessarily closed. Result indicates if launching was successful or not.

## Parameters

| Parameter | Description |
|---|---|
| `uri` | URI to open. |
| `options` | Launch options for the browser. |

## See also

- Declaring type: [[PlacemarkExtensions|PlacemarkExtensions]]
- [[_Microsoft.Maui.Devices.Sensors|Microsoft.Maui.Devices.Sensors namespace]]
