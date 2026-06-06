---
title: "Barometer.Start"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Devices-Sensors
aliases:
  - "Microsoft.Maui.Devices.Sensors.Barometer.Start"
declaring_type: "Barometer"
member_kind: method
---

# Barometer.Start

> [!abstract] Method of [[Barometer|Barometer]]
> Namespace: `Microsoft.Maui.Devices.Sensors`

Gets a value indicating whether the barometer is actively being monitored.

## Signature

```csharp
void static Start(Microsoft.Maui.Devices.Sensors.SensorSpeed sensorSpeed)
```

## Parameters

| Parameter | Description |
|---|---|
| `sensorSpeed` | The speed to listen for changes. |

## Remarks

Will throw `FeatureNotSupportedException` if not supported on device. Will throw `InvalidOperationException` if `IsMonitoring` is `true`.

## See also

- Declaring type: [[Barometer|Barometer]]
- [[_Microsoft.Maui.Devices.Sensors|Microsoft.Maui.Devices.Sensors namespace]]
