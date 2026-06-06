---
title: "LocationExtensions.OpenMapsAsync"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Devices-Sensors
aliases:
  - "Microsoft.Maui.Devices.Sensors.LocationExtensions.OpenMapsAsync"
declaring_type: "LocationExtensions"
member_kind: method
---

# LocationExtensions.OpenMapsAsync

> [!abstract] Method of [[LocationExtensions|LocationExtensions]]
> Namespace: `Microsoft.Maui.Devices.Sensors`

Open the browser to specified URI.

## Signatures

```csharp
System.Threading.Tasks.Task static OpenMapsAsync(this Microsoft.Maui.Devices.Sensors.Location location, Microsoft.Maui.ApplicationModel.MapLaunchOptions options)
System.Threading.Tasks.Task static OpenMapsAsync(this Microsoft.Maui.Devices.Sensors.Location location)
```

## Returns

Completed task when browser is launched, but not necessarily closed. Result indicates if launching was successful or not.

## Parameters

| Parameter | Description |
|---|---|
| `uri` | URI to open. |
| `options` | Launch options for the browser. |

## See also

- Declaring type: [[LocationExtensions|LocationExtensions]]
- [[_Microsoft.Maui.Devices.Sensors|Microsoft.Maui.Devices.Sensors namespace]]
