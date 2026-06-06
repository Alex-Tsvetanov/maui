---
title: "Accelerometer"
tags:
  - api
  - kind/class
  - ns/Microsoft-Maui-Devices-Sensors
aliases:
  - "Microsoft.Maui.Devices.Sensors.Accelerometer"
namespace: "Microsoft.Maui.Devices.Sensors"
kind: class
platforms:
  - All platforms (.NET)
  - Android
  - iOS
  - Mac Catalyst
  - Windows
  - Tizen
  - .NET Standard
assemblies:
  - src
---

# Accelerometer

> [!abstract] Class in `Microsoft.Maui.Devices.Sensors`
> Full name: `Microsoft.Maui.Devices.Sensors.Accelerometer`

Occurs when the sensor reading changes.

## Platforms

| Platform | Available |
|---|---|
| All platforms (.NET) | ✅ |
| Android | ✅ |
| iOS | ✅ |
| Mac Catalyst | ✅ |
| Windows | ✅ |
| Tizen | ✅ |
| .NET Standard | ✅ |


## Properties

| Name | Summary |
|---|---|
| [[Accelerometer.Default\|Default]] | Provides the default implementation for static usage of this API. |
| [[Accelerometer.IsMonitoring\|IsMonitoring]] | Occurs when the accelerometer detects that the device has been shaken. |
| [[Accelerometer.IsSupported\|IsSupported]] |  |

## Methods

| Name | Summary |
|---|---|
| [[Accelerometer.Start\|Start]] | Start monitoring for changes to accelerometer. |
| [[Accelerometer.Stop\|Stop]] | Stop monitoring for changes to accelerometer. |

## Events

| Name | Summary |
|---|---|
| [[Accelerometer.ReadingChanged\|ReadingChanged]] |  |
| [[Accelerometer.ShakeDetected\|ShakeDetected]] |  |

## Remarks

Will throw `FeatureNotSupportedException` if `IsSupported` is `false`. Will throw `InvalidOperationException` if `IsMonitoring` is `true`.

## See also

- [[_Microsoft.Maui.Devices.Sensors|Microsoft.Maui.Devices.Sensors namespace]]
- [Online API docs](https://learn.microsoft.com/dotnet/api/microsoft.maui.devices.sensors.accelerometer)
