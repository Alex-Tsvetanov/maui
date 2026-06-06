---
title: "Location"
tags:
  - api
  - kind/class
  - ns/Microsoft-Maui-Devices-Sensors
aliases:
  - "Microsoft.Maui.Devices.Sensors.Location"
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

# Location

> [!abstract] Class in `Microsoft.Maui.Devices.Sensors`
> Full name: `Microsoft.Maui.Devices.Sensors.Location`

The altitude reference system was not specified.

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
| [[Location.Location\|Location]] | Initializes a new instance of the `Location` class. |

## Properties

| Name | Summary |
|---|---|
| [[Location.Accuracy\|Accuracy]] | Gets or sets the horizontal accuracy (in meters) of the location. |
| [[Location.Altitude\|Altitude]] | Gets the altitude in meters (if available) in a reference system which is specified by `AltitudeReferenceSystem`. |
| [[Location.AltitudeReferenceSystem\|AltitudeReferenceSystem]] | Specifies the reference system in which the `Altitude` value is expressed. |
| [[Location.Course\|Course]] | Gets or sets the current degrees relative to true north at the time when this location was determined. |
| [[Location.IsFromMockProvider\|IsFromMockProvider]] | Gets or sets whether this location originates from a mocked sensor and thus might not be the real location of the device. |
| [[Location.Latitude\|Latitude]] | Gets or sets the latitude coordinate of this location. |
| [[Location.Longitude\|Longitude]] | Gets or sets the longitude coordinate of this location. |
| [[Location.ReducedAccuracy\|ReducedAccuracy]] | Gets or sets whether this location has a reduced accuracy reading. |
| [[Location.Speed\|Speed]] | Gets or sets the current speed in meters per second at the time when this location was determined. |
| [[Location.Timestamp\|Timestamp]] | Gets or sets the timestamp of the location in UTC. |
| [[Location.VerticalAccuracy\|VerticalAccuracy]] | Gets or sets the vertical accuracy (in meters) of the location. |

## Methods

| Name | Summary |
|---|---|
| [[Location.CalculateDistance\|CalculateDistance]] | Calculate distance between two locations. |
| [[Location.Equals\|Equals]] |  |
| [[Location.GetHashCode\|GetHashCode]] |  |
| [[Location.ToString\|ToString]] | Returns a string representation of the current values of `Location`. |

## Operators

| Name | Summary |
|---|---|
| [[Location.operator !=\|operator !=]] |  |
| [[Location.operator ==\|operator ==]] |  |

## See also

- [[_Microsoft.Maui.Devices.Sensors|Microsoft.Maui.Devices.Sensors namespace]]
- [Online API docs](https://learn.microsoft.com/dotnet/api/microsoft.maui.devices.sensors.location)
