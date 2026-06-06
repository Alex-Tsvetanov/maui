---
title: "GeolocationRequest.RequestFullAccuracy"
tags:
  - api
  - member/property
  - ns/Microsoft-Maui-Devices-Sensors
aliases:
  - "Microsoft.Maui.Devices.Sensors.GeolocationRequest.RequestFullAccuracy"
declaring_type: "GeolocationRequest"
member_kind: property
---

# GeolocationRequest.RequestFullAccuracy

> [!abstract] Property of [[GeolocationRequest|GeolocationRequest]]
> Namespace: `Microsoft.Maui.Devices.Sensors`

Gets or sets whether to request full permission to temporarily use location services with full accuracy.

## Signature

```csharp
bool RequestFullAccuracy { get; set; }
```

## Remarks

This value is only used on iOS 14+. Using this functionality requires the NSLocationTemporaryUsageDescriptionDictionary key to be present in the info.plist file.

## See also

- Declaring type: [[GeolocationRequest|GeolocationRequest]]
- [[_Microsoft.Maui.Devices.Sensors|Microsoft.Maui.Devices.Sensors namespace]]
