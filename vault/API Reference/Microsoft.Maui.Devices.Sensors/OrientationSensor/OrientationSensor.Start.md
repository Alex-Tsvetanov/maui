---
title: "OrientationSensor.Start"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Devices-Sensors
aliases:
  - "Microsoft.Maui.Devices.Sensors.OrientationSensor.Start"
declaring_type: "OrientationSensor"
member_kind: method
---

# OrientationSensor.Start

> [!abstract] Method of [[OrientationSensor|OrientationSensor]]
> Namespace: `Microsoft.Maui.Devices.Sensors`

Start monitoring for changes to the orientation.

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

- Declaring type: [[OrientationSensor|OrientationSensor]]
- [[_Microsoft.Maui.Devices.Sensors|Microsoft.Maui.Devices.Sensors namespace]]
