---
title: "Geolocation"
tags:
  - api
  - kind/class
  - ns/Microsoft-Maui-Devices-Sensors
aliases:
  - "Microsoft.Maui.Devices.Sensors.Geolocation"
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

# Geolocation

> [!abstract] Class in `Microsoft.Maui.Devices.Sensors`
> Full name: `Microsoft.Maui.Devices.Sensors.Geolocation`

Returns the last known location of the device.

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
| [[Geolocation.Default\|Default]] | Provides the default implementation for static usage of this API. |
| [[Geolocation.IsEnabled\|IsEnabled]] | Returns true when the device's location services are enabled |
| [[Geolocation.IsListeningForeground\|IsListeningForeground]] | Indicates if currently listening to location updates while the app is in foreground. |

## Methods

| Name | Summary |
|---|---|
| [[Geolocation.GetLastKnownLocationAsync\|GetLastKnownLocationAsync]] |  |
| [[Geolocation.GetLocationAsync\|GetLocationAsync]] |  |
| [[Geolocation.StartListeningForegroundAsync\|StartListeningForegroundAsync]] |  |
| [[Geolocation.StopListeningForeground\|StopListeningForeground]] | Stop listening for location updates when the app is in the foreground. Has no effect when not listening and `IsListeningForeground` is currently `false`. |

## Events

| Name | Summary |
|---|---|
| [[Geolocation.ListeningFailed\|ListeningFailed]] |  |
| [[Geolocation.LocationChanged\|LocationChanged]] |  |

## Remarks

The location permissions will be requested at runtime if needed. You might still need to declare something in your app manifest. This location may be a recently cached location.

## Guide

- 📖 Conceptual: [[geolocation]]

## See also

- [[_Microsoft.Maui.Devices.Sensors|Microsoft.Maui.Devices.Sensors namespace]]
- [Online API docs](https://learn.microsoft.com/dotnet/api/microsoft.maui.devices.sensors.geolocation)
