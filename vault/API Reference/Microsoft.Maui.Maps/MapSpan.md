---
title: "MapSpan"
tags:
  - api
  - kind/class
  - ns/Microsoft-Maui-Maps
aliases:
  - "Microsoft.Maui.Maps.MapSpan"
namespace: "Microsoft.Maui.Maps"
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

# MapSpan

> [!abstract] Class in `Microsoft.Maui.Maps`
> Full name: `Microsoft.Maui.Maps.MapSpan`

Represents a rectangular region on the map, defined by a center point and span.

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
| [[MapSpan.MapSpan\|MapSpan]] | Initializes a new instance of the `MapSpan` class with the specified center and span in degrees. |

## Properties

| Name | Summary |
|---|---|
| [[MapSpan.Center\|Center]] | Gets the center location of this span. |
| [[MapSpan.LatitudeDegrees\|LatitudeDegrees]] | Gets the latitude span in degrees. |
| [[MapSpan.LongitudeDegrees\|LongitudeDegrees]] | Gets the longitude span in degrees. |
| [[MapSpan.Radius\|Radius]] |  |

## Methods

| Name | Summary |
|---|---|
| [[MapSpan.ClampLatitude\|ClampLatitude]] | Gets the approximate radius of the span. |
| [[MapSpan.Equals\|Equals]] |  |
| [[MapSpan.FromCenterAndRadius\|FromCenterAndRadius]] | Creates a new `MapSpan` from a center location and radius. |
| [[MapSpan.GetHashCode\|GetHashCode]] |  |
| [[MapSpan.ToString\|ToString]] |  |
| [[MapSpan.WithZoom\|WithZoom]] | Determines whether two `MapSpan` instances are not equal. |

## Operators

| Name | Summary |
|---|---|
| [[MapSpan.operator !=\|operator !=]] |  |
| [[MapSpan.operator ==\|operator ==]] |  |

## See also

- [[_Microsoft.Maui.Maps|Microsoft.Maui.Maps namespace]]
- [Online API docs](https://learn.microsoft.com/dotnet/api/microsoft.maui.maps.mapspan)
