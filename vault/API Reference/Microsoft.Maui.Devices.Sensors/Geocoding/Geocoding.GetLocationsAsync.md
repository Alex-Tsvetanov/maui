---
title: "Geocoding.GetLocationsAsync"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Devices-Sensors
aliases:
  - "Microsoft.Maui.Devices.Sensors.Geocoding.GetLocationsAsync"
declaring_type: "Geocoding"
member_kind: method
---

# Geocoding.GetLocationsAsync

> [!abstract] Method of [[Geocoding|Geocoding]]
> Namespace: `Microsoft.Maui.Devices.Sensors`

Retrieve potential locations for a given address.

## Signature

```csharp
System.Threading.Tasks.Task<System.Collections.Generic.IEnumerable<Microsoft.Maui.Devices.Sensors.Location!>!>! static GetLocationsAsync(string! address)
```

## Returns

List of `Location` that best match the address or `null` if no locations are found.

## Parameters

| Parameter | Description |
|---|---|
| `address` | Address to retrieve the location for. |

## See also

- Declaring type: [[Geocoding|Geocoding]]
- [[_Microsoft.Maui.Devices.Sensors|Microsoft.Maui.Devices.Sensors namespace]]
