---
title: "Compass.Start"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Devices-Sensors
aliases:
  - "Microsoft.Maui.Devices.Sensors.Compass.Start"
declaring_type: "Compass"
member_kind: method
---

# Compass.Start

> [!abstract] Method of [[Compass|Compass]]
> Namespace: `Microsoft.Maui.Devices.Sensors`

Gets a value indicating whether reading the compass is supported on this device.

## Signatures

```csharp
void static Start(Microsoft.Maui.Devices.Sensors.SensorSpeed sensorSpeed, bool applyLowPassFilter)
void static Start(Microsoft.Maui.Devices.Sensors.SensorSpeed sensorSpeed)
```

## Parameters

| Parameter | Description |
|---|---|
| `sensorSpeed` | The speed to monitor for changes. |

## Remarks

Will throw `FeatureNotSupportedException` if not supported on device. Will throw `InvalidOperationException` if `IsMonitoring` is `true`.

## See also

- Declaring type: [[Compass|Compass]]
- [[_Microsoft.Maui.Devices.Sensors|Microsoft.Maui.Devices.Sensors namespace]]
