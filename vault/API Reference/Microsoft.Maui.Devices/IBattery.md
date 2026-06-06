---
title: "IBattery"
tags:
  - api
  - kind/interface
  - ns/Microsoft-Maui-Devices
aliases:
  - "Microsoft.Maui.Devices.IBattery"
namespace: "Microsoft.Maui.Devices"
kind: interface
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

# IBattery

> [!abstract] Interface in `Microsoft.Maui.Devices`
> Full name: `Microsoft.Maui.Devices.IBattery`

Methods and properties for battery and charging information of the device.

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
| [[IBattery.ChargeLevel\|ChargeLevel]] |  |
| [[IBattery.EnergySaverStatus\|EnergySaverStatus]] |  |
| [[IBattery.PowerSource\|PowerSource]] |  |
| [[IBattery.State\|State]] |  |

## Events

| Name | Summary |
|---|---|
| [[IBattery.BatteryInfoChanged\|BatteryInfoChanged]] |  |
| [[IBattery.EnergySaverStatusChanged\|EnergySaverStatusChanged]] |  |

## Remarks

Platform specific remarks: - Android: Battery_Stats permission must be set in manifest. - iOS: Simulator will not return battery information, must be run on device. - Windows: None.

## See also

- [[_Microsoft.Maui.Devices|Microsoft.Maui.Devices namespace]]
- [Online API docs](https://learn.microsoft.com/dotnet/api/microsoft.maui.devices.ibattery)
