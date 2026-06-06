---
title: "MapExtensions (ApplicationModel).OpenAsync"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-ApplicationModel
aliases:
  - "Microsoft.Maui.ApplicationModel.MapExtensions.OpenAsync"
declaring_type: "MapExtensions (ApplicationModel)"
member_kind: method
---

# MapExtensions (ApplicationModel).OpenAsync

> [!abstract] Method of [[MapExtensions (ApplicationModel)|MapExtensions (ApplicationModel)]]
> Namespace: `Microsoft.Maui.ApplicationModel`

Open the installed application to a specific location.

## Signatures

```csharp
System.Threading.Tasks.Task! static OpenAsync(this Microsoft.Maui.ApplicationModel.IMap! map, double latitude, double longitude)
System.Threading.Tasks.Task! static OpenAsync(this Microsoft.Maui.ApplicationModel.IMap! map, Microsoft.Maui.Devices.Sensors.Location! location, Microsoft.Maui.ApplicationModel.MapLaunchOptions! options)
System.Threading.Tasks.Task! static OpenAsync(this Microsoft.Maui.ApplicationModel.IMap! map, Microsoft.Maui.Devices.Sensors.Location! location)
System.Threading.Tasks.Task! static OpenAsync(this Microsoft.Maui.ApplicationModel.IMap! map, Microsoft.Maui.Devices.Sensors.Placemark! placemark)
```

## Parameters

| Parameter | Description |
|---|---|
| `map` | The object this method is invoked on. |
| `location` | Location to open in the map application. |

## Returns

A `Task` object with the current status of the asynchronous operation.

## See also

- Declaring type: [[MapExtensions (ApplicationModel)|MapExtensions (ApplicationModel)]]
- [[_Microsoft.Maui.ApplicationModel|Microsoft.Maui.ApplicationModel namespace]]
