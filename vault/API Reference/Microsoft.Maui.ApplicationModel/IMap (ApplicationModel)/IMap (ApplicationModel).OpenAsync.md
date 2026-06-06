---
title: "IMap (ApplicationModel).OpenAsync"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-ApplicationModel
aliases:
  - "Microsoft.Maui.ApplicationModel.IMap.OpenAsync"
declaring_type: "IMap (ApplicationModel)"
member_kind: method
---

# IMap (ApplicationModel).OpenAsync

> [!abstract] Method of [[IMap (ApplicationModel)|IMap (ApplicationModel)]]
> Namespace: `Microsoft.Maui.ApplicationModel`

Open the installed application to a specific location with launch options.

## Signatures

```csharp
System.Threading.Tasks.Task! OpenAsync(double latitude, double longitude, Microsoft.Maui.ApplicationModel.MapLaunchOptions! options)
System.Threading.Tasks.Task! OpenAsync(Microsoft.Maui.Devices.Sensors.Placemark! placemark, Microsoft.Maui.ApplicationModel.MapLaunchOptions! options)
```

## Returns

A `Task` object with the current status of the asynchronous operation.

## Parameters

| Parameter | Description |
|---|---|
| `latitude` | Target latitude. |
| `longitude` | Target longitude. |
| `options` | Launch options to use. |

## See also

- Declaring type: [[IMap (ApplicationModel)|IMap (ApplicationModel)]]
- [[_Microsoft.Maui.ApplicationModel|Microsoft.Maui.ApplicationModel namespace]]
