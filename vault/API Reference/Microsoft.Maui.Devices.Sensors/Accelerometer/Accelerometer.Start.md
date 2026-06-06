---
title: "Accelerometer.Start"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Devices-Sensors
aliases:
  - "Microsoft.Maui.Devices.Sensors.Accelerometer.Start"
declaring_type: "Accelerometer"
member_kind: method
---

# Accelerometer.Start

> [!abstract] Method of [[Accelerometer|Accelerometer]]
> Namespace: `Microsoft.Maui.Devices.Sensors`

Start monitoring for changes to accelerometer.

## Signature

```csharp
void static Start(Microsoft.Maui.Devices.Sensors.SensorSpeed sensorSpeed)
```

## Parameters

| Parameter | Description |
|---|---|
| `sensorSpeed` | Speed to monitor the sensor. |

## Remarks

Will throw `FeatureNotSupportedException` if not supported on device. Will throw `ArgumentNullException` if handler is null.

## See also

- Declaring type: [[Accelerometer|Accelerometer]]
- [[_Microsoft.Maui.Devices.Sensors|Microsoft.Maui.Devices.Sensors namespace]]
