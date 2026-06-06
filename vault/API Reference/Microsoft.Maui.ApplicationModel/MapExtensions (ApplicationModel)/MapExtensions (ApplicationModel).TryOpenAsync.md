---
title: "MapExtensions (ApplicationModel).TryOpenAsync"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-ApplicationModel
aliases:
  - "Microsoft.Maui.ApplicationModel.MapExtensions.TryOpenAsync"
declaring_type: "MapExtensions (ApplicationModel)"
member_kind: method
---

# MapExtensions (ApplicationModel).TryOpenAsync

> [!abstract] Method of [[MapExtensions (ApplicationModel)|MapExtensions (ApplicationModel)]]
> Namespace: `Microsoft.Maui.ApplicationModel`

First checks if the installed map application can be opened, then opens the installed application to a specific location with launch options.

## Signatures

```csharp
System.Threading.Tasks.Task<bool>! static TryOpenAsync(this Microsoft.Maui.ApplicationModel.IMap! map, double latitude, double longitude)
System.Threading.Tasks.Task<bool>! static TryOpenAsync(this Microsoft.Maui.ApplicationModel.IMap! map, Microsoft.Maui.Devices.Sensors.Location! location, Microsoft.Maui.ApplicationModel.MapLaunchOptions! options)
System.Threading.Tasks.Task<bool>! static TryOpenAsync(this Microsoft.Maui.ApplicationModel.IMap! map, Microsoft.Maui.Devices.Sensors.Location! location)
System.Threading.Tasks.Task<bool>! static TryOpenAsync(this Microsoft.Maui.ApplicationModel.IMap! map, Microsoft.Maui.Devices.Sensors.Placemark! placemark)
```

## Returns

`true` if the map application is opened, otherwise `false`.

## Parameters

| Parameter | Description |
|---|---|
| `map` | The object this method is invoked on. |
| `location` | Location to open in the map application. |

## See also

- Declaring type: [[MapExtensions (ApplicationModel)|MapExtensions (ApplicationModel)]]
- [[_Microsoft.Maui.ApplicationModel|Microsoft.Maui.ApplicationModel namespace]]
