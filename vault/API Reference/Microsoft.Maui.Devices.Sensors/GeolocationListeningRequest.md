---
title: "GeolocationListeningRequest"
tags:
  - api
  - kind/class
  - ns/Microsoft-Maui-Devices-Sensors
aliases:
  - "Microsoft.Maui.Devices.Sensors.GeolocationListeningRequest"
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

# GeolocationListeningRequest

> [!abstract] Class in `Microsoft.Maui.Devices.Sensors`
> Full name: `Microsoft.Maui.Devices.Sensors.GeolocationListeningRequest`

Request options for listening to geolocations

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


## Constructors

| Name | Summary |
|---|---|
| [[GeolocationListeningRequest.GeolocationListeningRequest\|GeolocationListeningRequest]] | Creates a new request object with default values |

## Properties

| Name | Summary |
|---|---|
| [[GeolocationListeningRequest.DesiredAccuracy\|DesiredAccuracy]] | The desired minimum accuracy for the location updates being sent. Locations that don't satisfy this accuracy are not sent using the event handler. |
| [[GeolocationListeningRequest.MinimumTime\|MinimumTime]] | Minimum time between location updates being sent. This value must positive. Most location sensors may not return locations in intervals shorter than 1 second. |

## See also

- [[_Microsoft.Maui.Devices.Sensors|Microsoft.Maui.Devices.Sensors namespace]]
- [Online API docs](https://learn.microsoft.com/dotnet/api/microsoft.maui.devices.sensors.geolocationlisteningrequest)
