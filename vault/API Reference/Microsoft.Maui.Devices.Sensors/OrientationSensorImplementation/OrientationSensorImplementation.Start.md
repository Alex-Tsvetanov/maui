---
title: "OrientationSensorImplementation.Start"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Devices-Sensors
aliases:
  - "Microsoft.Maui.Devices.Sensors.OrientationSensorImplementation.Start"
declaring_type: "OrientationSensorImplementation"
member_kind: method
---

# OrientationSensorImplementation.Start

> [!abstract] Method of [[OrientationSensorImplementation|OrientationSensorImplementation]]
> Namespace: `Microsoft.Maui.Devices.Sensors`

Start monitoring for changes to the orientation.

## Signature

```csharp
void Start(Microsoft.Maui.Devices.Sensors.SensorSpeed sensorSpeed)
```

## Parameters

| Parameter | Description |
|---|---|
| `sensorSpeed` | The speed to listen for changes. |

## Remarks

Will throw `FeatureNotSupportedException` if not supported on device. Will throw `InvalidOperationException` if `IsMonitoring` is `true`.

## See also

- Declaring type: [[OrientationSensorImplementation|OrientationSensorImplementation]]
- [[_Microsoft.Maui.Devices.Sensors|Microsoft.Maui.Devices.Sensors namespace]]
