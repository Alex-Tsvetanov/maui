---
title: "Microsoft.Maui.Devices"
tags:
  - api
  - namespace
  - ns/Microsoft-Maui-Devices
---

# Microsoft.Maui.Devices

> [!info] Namespace
> `Microsoft.Maui.Devices` — 26 public types.

- [Online namespace docs](https://learn.microsoft.com/dotnet/api/microsoft.maui.devices)

## Overview

`Microsoft.Maui.Devices` is the cross-platform device-features layer of .NET MAUI. It exposes a uniform API surface for querying the hardware and the running device, and for driving simple device capabilities, so that a single codebase can work consistently across Android, iOS, Windows, and other supported targets. Rather than writing platform-specific code to inspect the screen or trigger hardware, applications consume the abstractions and static helpers in this namespace.

The namespace groups into a few clear concerns. Device and platform identity is provided through [[DeviceInfo|DeviceInfo]] and [[IDeviceInfo|IDeviceInfo]], together with the descriptive types [[DeviceIdiom|DeviceIdiom]] (form factor), [[DevicePlatform|DevicePlatform]], and [[DeviceType|DeviceType]]. Screen information is exposed by [[DeviceDisplay|DeviceDisplay]] and [[IDeviceDisplay|IDeviceDisplay]], carrying a [[DisplayInfo|DisplayInfo]] value with [[DisplayOrientation|DisplayOrientation]] and [[DisplayRotation|DisplayRotation]], and raising [[DisplayInfoChangedEventArgs|DisplayInfoChangedEventArgs]] when the display changes.

Power state is covered by [[Battery (Devices)|Battery]] and [[IBattery|IBattery]], which report charge level, [[BatteryState|BatteryState]], [[BatteryPowerSource|BatteryPowerSource]], and the device [[EnergySaverStatus|EnergySaverStatus]], with [[BatteryInfoChangedEventArgs|BatteryInfoChangedEventArgs]] and [[EnergySaverStatusChangedEventArgs|EnergySaverStatusChangedEventArgs]] for change notifications. Finally, the namespace drives simple hardware output: [[Flashlight (Devices)|Flashlight]] / [[IFlashlight|IFlashlight]] toggle the camera flash, [[Vibration|Vibration]] / [[IVibration|IVibration]] make the device vibrate, and [[HapticFeedback|HapticFeedback]] / [[IHapticFeedback|IHapticFeedback]] produce haptic responses described by [[HapticFeedbackType|HapticFeedbackType]]. Each capability follows the same pattern: an interface for testability and dependency injection alongside a static convenience class.

## Key types

- [[DeviceInfo|DeviceInfo]] — Reports identity details of the running device, such as its model.
- [[DeviceDisplay|DeviceDisplay]] — Provides information about the device screen and keeps-on state.
- [[DisplayInfo|DisplayInfo]] — Represents the screen's metrics, orientation, and rotation.
- [[IBattery|IBattery]] — Methods and properties for battery and charging information of the device.
- [[Battery (Devices)|Battery]] — Static access to the current charge level (0.0 to 1.0) and power state.
- [[IFlashlight|IFlashlight]] — Turns the device's camera flash on or off to use it as a flashlight.
- [[Flashlight (Devices)|Flashlight]] — Checks availability and toggles the device flashlight.
- [[IVibration|IVibration]] — Provides an easy way to make the device vibrate.
- [[IHapticFeedback|IHapticFeedback]] — Lets you control haptic feedback on the device.
- [[DeviceIdiom|DeviceIdiom]] — Represents the idiom (form factor) of the device.
- [[DevicePlatform|DevicePlatform]] — Represents the platform the application is running on.
- [[EnergySaverStatus|EnergySaverStatus]] — Describes the device's power/energy-saver state.


## Classes

| Type | Summary |
|---|---|
| [[Battery (Devices)\|Battery (Devices)]] | Gets the current charge level of the device from 0.0 to 1.0. |
| [[BatteryInfoChangedEventArgs\|BatteryInfoChangedEventArgs]] | Status of energy saving is unknown. |
| [[DeviceDisplay\|DeviceDisplay]] | Represents information about the device screen. |
| [[DeviceInfo\|DeviceInfo]] | Gets the model of the device. |
| [[DisplayInfoChangedEventArgs\|DisplayInfoChangedEventArgs]] | Gets or sets if the screen should be kept on. |
| [[EnergySaverStatusChangedEventArgs\|EnergySaverStatusChangedEventArgs]] | Event arguments when the energy saver status changes. |
| [[Flashlight (Devices)\|Flashlight (Devices)]] | Checks if the flashlight is available and can be turned on or off. |
| [[HapticFeedback\|HapticFeedback]] | Gets a value indicating whether haptic feedback is supported on this device. |
| [[Vibration\|Vibration]] | Gets a value indicating whether vibration is supported on this device. |
| [[VibrationExtensions\|VibrationExtensions]] | Static class with extension methods for the `IVibration` APIs. |

## Interfaces

| Type | Summary |
|---|---|
| [[IBattery\|IBattery]] | Methods and properties for battery and charging information of the device. |
| [[IDeviceDisplay\|IDeviceDisplay]] | Represents information about the device screen. |
| [[IDeviceInfo\|IDeviceInfo]] | An unknown device type. |
| [[IFlashlight\|IFlashlight]] | The Flashlight API has the ability to turn on or off the device's camera flash to turn it into a flashlight. |
| [[IHapticFeedback\|IHapticFeedback]] | The HapticFeedback API lets you control haptic feedback on the device. |
| [[IVibration\|IVibration]] | The Vibration API provides an easy way to make the device vibrate. |

## Structs

| Type | Summary |
|---|---|
| [[DeviceIdiom\|DeviceIdiom]] | Represents the idiom (form factor) of the device. |
| [[DevicePlatform\|DevicePlatform]] | Represents the device platform that the application is running on. |
| [[DisplayInfo\|DisplayInfo]] | Represents information about the device's screen. |

## Enums

| Type | Summary |
|---|---|
| [[BatteryPowerSource\|BatteryPowerSource]] | Battery state could not be determined. |
| [[BatteryState\|BatteryState]] | Describes possible states of the battery. |
| [[DeviceType\|DeviceType]] | Types of devices. |
| [[DisplayOrientation\|DisplayOrientation]] | Represents the orientation a device display can have. |
| [[DisplayRotation\|DisplayRotation]] | Represents the rotation a device display can have. |
| [[EnergySaverStatus\|EnergySaverStatus]] | Power source cannot be determined. |
| [[HapticFeedbackType\|HapticFeedbackType]] | Enumerates the possible types of `IHapticFeedback` response. |

## See also

- [[_API Reference]]
