---
title: "Map (ApplicationModel).TryOpenAsync"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-ApplicationModel
aliases:
  - "Microsoft.Maui.ApplicationModel.Map.TryOpenAsync"
declaring_type: "Map (ApplicationModel)"
member_kind: method
---

# Map (ApplicationModel).TryOpenAsync

> [!abstract] Method of [[Map (ApplicationModel)|Map (ApplicationModel)]]
> Namespace: `Microsoft.Maui.ApplicationModel`

First checks if the installed map application can be opened, then opens the installed application to a specific location with launch options.

## Signatures

```csharp
System.Threading.Tasks.Task<bool>! static TryOpenAsync(double latitude, double longitude, Microsoft.Maui.ApplicationModel.MapLaunchOptions! options)
System.Threading.Tasks.Task<bool>! static TryOpenAsync(double latitude, double longitude)
System.Threading.Tasks.Task<bool>! static TryOpenAsync(Microsoft.Maui.Devices.Sensors.Location! location, Microsoft.Maui.ApplicationModel.MapLaunchOptions! options)
System.Threading.Tasks.Task<bool>! static TryOpenAsync(Microsoft.Maui.Devices.Sensors.Location! location)
System.Threading.Tasks.Task<bool>! static TryOpenAsync(Microsoft.Maui.Devices.Sensors.Placemark! placemark, Microsoft.Maui.ApplicationModel.MapLaunchOptions! options)
System.Threading.Tasks.Task<bool>! static TryOpenAsync(Microsoft.Maui.Devices.Sensors.Placemark! placemark)
```

## Returns

`true` if the map application is opened, otherwise `false`.

## Parameters

| Parameter | Description |
|---|---|
| `location` | Location to open in the map application. |

## See also

- Declaring type: [[Map (ApplicationModel)|Map (ApplicationModel)]]
- [[_Microsoft.Maui.ApplicationModel|Microsoft.Maui.ApplicationModel namespace]]
