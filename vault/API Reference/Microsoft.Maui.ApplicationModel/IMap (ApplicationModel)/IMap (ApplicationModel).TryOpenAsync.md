---
title: "IMap (ApplicationModel).TryOpenAsync"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-ApplicationModel
aliases:
  - "Microsoft.Maui.ApplicationModel.IMap.TryOpenAsync"
declaring_type: "IMap (ApplicationModel)"
member_kind: method
---

# IMap (ApplicationModel).TryOpenAsync

> [!abstract] Method of [[IMap (ApplicationModel)|IMap (ApplicationModel)]]
> Namespace: `Microsoft.Maui.ApplicationModel`

First checks if the installed map application can be opened, then opens the installed application to a specific location with launch options.

## Signatures

```csharp
System.Threading.Tasks.Task<bool>! TryOpenAsync(double latitude, double longitude, Microsoft.Maui.ApplicationModel.MapLaunchOptions! options)
System.Threading.Tasks.Task<bool>! TryOpenAsync(Microsoft.Maui.Devices.Sensors.Placemark! placemark, Microsoft.Maui.ApplicationModel.MapLaunchOptions! options)
```

## Returns

`true` if the map application is opened, otherwise `false`.

## Parameters

| Parameter | Description |
|---|---|
| `latitude` | Target latitude. |
| `longitude` | Target longitude. |
| `options` | Launch options to use. |

## See also

- Declaring type: [[IMap (ApplicationModel)|IMap (ApplicationModel)]]
- [[_Microsoft.Maui.ApplicationModel|Microsoft.Maui.ApplicationModel namespace]]
