---
title: "IGeocoding.GetLocationsAsync"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Devices-Sensors
aliases:
  - "Microsoft.Maui.Devices.Sensors.IGeocoding.GetLocationsAsync"
declaring_type: "IGeocoding"
member_kind: method
---

# IGeocoding.GetLocationsAsync

> [!abstract] Method of [[IGeocoding|IGeocoding]]
> Namespace: `Microsoft.Maui.Devices.Sensors`

Retrieve potential locations for a given address.

## Signature

```csharp
System.Threading.Tasks.Task<System.Collections.Generic.IEnumerable<Microsoft.Maui.Devices.Sensors.Location!>!>! GetLocationsAsync(string! address)
```

## Returns

List of `Location` that best match the address or `null` if no locations are found.

## Parameters

| Parameter | Description |
|---|---|
| `address` | Address to retrieve the location for. |

## See also

- Declaring type: [[IGeocoding|IGeocoding]]
- [[_Microsoft.Maui.Devices.Sensors|Microsoft.Maui.Devices.Sensors namespace]]
