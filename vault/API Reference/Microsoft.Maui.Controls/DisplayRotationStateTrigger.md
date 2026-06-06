---
title: "DisplayRotationStateTrigger"
tags:
  - api
  - kind/class
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.DisplayRotationStateTrigger"
namespace: "Microsoft.Maui.Controls"
kind: class
platforms:
  - All platforms (.NET)
  - Android
  - iOS
  - Mac Catalyst
  - Windows
  - .NET Standard
assemblies:
  - Controls
---

# DisplayRotationStateTrigger

> [!abstract] Class in `Microsoft.Maui.Controls`
> Full name: `Microsoft.Maui.Controls.DisplayRotationStateTrigger`

Trigger that activates when the device display rotation matches the specified `Rotation`.

## Platforms

| Platform | Available |
|---|---|
| All platforms (.NET) | ✅ |
| Android | ✅ |
| iOS | ✅ |
| Mac Catalyst | ✅ |
| Windows | ✅ |
| .NET Standard | ✅ |


## Constructors

| Name | Summary |
|---|---|
| [[DisplayRotationStateTrigger.DisplayRotationStateTrigger\|DisplayRotationStateTrigger]] | Initializes a new instance of the `DisplayRotationStateTrigger` class. |

## Properties

| Name | Summary |
|---|---|
| [[DisplayRotationStateTrigger.Rotation\|Rotation]] |  |

## Fields

| Name | Summary |
|---|---|
| [[DisplayRotationStateTrigger.RotationProperty\|RotationProperty]] | Gets or sets the display rotation that will activate this trigger. |

## Remarks

The `DisplayRotationStateTrigger` enables developers to create visual states that are triggered based on the device's display rotation. Unlike `OrientationStateTrigger` which only differentiates between portrait and landscape orientations, this trigger provides granular control over specific rotation angles (0°, 90°, 180°, 270°). This trigger is particularly useful for applications that need to respond to specific device orientations, such as games that have different layouts for each rotation state or apps that need to handle upside-down orientations differently.

## See also

- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
- [Online API docs](https://learn.microsoft.com/dotnet/api/microsoft.maui.controls.displayrotationstatetrigger)
