---
title: "Map (ApplicationModel).OpenAsync"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-ApplicationModel
aliases:
  - "Microsoft.Maui.ApplicationModel.Map.OpenAsync"
declaring_type: "Map (ApplicationModel)"
member_kind: method
---

# Map (ApplicationModel).OpenAsync

> [!abstract] Method of [[Map (ApplicationModel)|Map (ApplicationModel)]]
> Namespace: `Microsoft.Maui.ApplicationModel`

Open the installed application to a specific location.

## Signatures

```csharp
System.Threading.Tasks.Task! static OpenAsync(double latitude, double longitude, Microsoft.Maui.ApplicationModel.MapLaunchOptions! options)
System.Threading.Tasks.Task! static OpenAsync(double latitude, double longitude)
System.Threading.Tasks.Task! static OpenAsync(Microsoft.Maui.Devices.Sensors.Location! location, Microsoft.Maui.ApplicationModel.MapLaunchOptions! options)
System.Threading.Tasks.Task! static OpenAsync(Microsoft.Maui.Devices.Sensors.Location! location)
System.Threading.Tasks.Task! static OpenAsync(Microsoft.Maui.Devices.Sensors.Placemark! placemark, Microsoft.Maui.ApplicationModel.MapLaunchOptions! options)
System.Threading.Tasks.Task! static OpenAsync(Microsoft.Maui.Devices.Sensors.Placemark! placemark)
```

## Parameters

| Parameter | Description |
|---|---|
| `location` | Location to open in the map application. |

## Returns

A `Task` object with the current status of the asynchronous operation.

## See also

- Declaring type: [[Map (ApplicationModel)|Map (ApplicationModel)]]
- [[_Microsoft.Maui.ApplicationModel|Microsoft.Maui.ApplicationModel namespace]]
