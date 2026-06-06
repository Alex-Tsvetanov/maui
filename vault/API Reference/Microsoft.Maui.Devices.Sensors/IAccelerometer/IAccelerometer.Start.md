---
title: "IAccelerometer.Start"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Devices-Sensors
aliases:
  - "Microsoft.Maui.Devices.Sensors.IAccelerometer.Start"
declaring_type: "IAccelerometer"
member_kind: method
---

# IAccelerometer.Start

> [!abstract] Method of [[IAccelerometer|IAccelerometer]]
> Namespace: `Microsoft.Maui.Devices.Sensors`

Start monitoring for changes to accelerometer.

## Signature

```csharp
void Start(Microsoft.Maui.Devices.Sensors.SensorSpeed sensorSpeed)
```

## Remarks

Will throw `FeatureNotSupportedException` if `IsSupported` is `false`. Will throw `InvalidOperationException` if `IsMonitoring` is `true`.

## Parameters

| Parameter | Description |
|---|---|
| `sensorSpeed` | Speed to monitor the sensor. |

## See also

- Declaring type: [[IAccelerometer|IAccelerometer]]
- [[_Microsoft.Maui.Devices.Sensors|Microsoft.Maui.Devices.Sensors namespace]]
