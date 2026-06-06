---
title: "GeocodingExtensions.GetMapServiceToken"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Devices-Sensors
aliases:
  - "Microsoft.Maui.Devices.Sensors.GeocodingExtensions.GetMapServiceToken"
declaring_type: "GeocodingExtensions"
member_kind: method
---

# GeocodingExtensions.GetMapServiceToken

> [!abstract] Method of [[GeocodingExtensions|GeocodingExtensions]]
> Namespace: `Microsoft.Maui.Devices.Sensors`

Gets the map service API key for this platform.

## Signature

```csharp
string? static GetMapServiceToken(this Microsoft.Maui.Devices.Sensors.IGeocoding! geocoding)
```

## Parameters

| Parameter | Description |
|---|---|
| `geocoding` | The object this method is invoked on. |

## Returns

The currently configured map service API key, or `null` if it's not set.

## Remarks

Only needed for Windows and Tizen.

## See also

- Declaring type: [[GeocodingExtensions|GeocodingExtensions]]
- [[_Microsoft.Maui.Devices.Sensors|Microsoft.Maui.Devices.Sensors namespace]]
