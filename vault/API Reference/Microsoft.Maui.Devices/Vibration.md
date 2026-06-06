---
title: "Vibration"
tags:
  - api
  - kind/class
  - ns/Microsoft-Maui-Devices
aliases:
  - "Microsoft.Maui.Devices.Vibration"
namespace: "Microsoft.Maui.Devices"
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

# Vibration

> [!abstract] Class in `Microsoft.Maui.Devices`
> Full name: `Microsoft.Maui.Devices.Vibration`

Gets a value indicating whether vibration is supported on this device.

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
| [[Vibration.Default\|Default]] | Gets a value indicating whether vibration is supported on this device. |
| [[Vibration.IsSupported\|IsSupported]] |  |

## Methods

| Name | Summary |
|---|---|
| [[Vibration.Cancel\|Cancel]] | Cancel any current vibrations. |
| [[Vibration.Vibrate\|Vibrate]] | Vibrates the device for 500ms. |

## Remarks

On iOS, the device will only vibrate for 500ms, regardless of the value specified.

## See also

- [[_Microsoft.Maui.Devices|Microsoft.Maui.Devices namespace]]
- [Online API docs](https://learn.microsoft.com/dotnet/api/microsoft.maui.devices.vibration)
