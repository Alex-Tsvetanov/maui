---
title: "MapPinHandler"
tags:
  - api
  - kind/class
  - ns/Microsoft-Maui-Maps-Handlers
aliases:
  - "Microsoft.Maui.Maps.Handlers.MapPinHandler"
namespace: "Microsoft.Maui.Maps.Handlers"
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

# MapPinHandler

> [!abstract] Class in `Microsoft.Maui.Maps.Handlers`
> Full name: `Microsoft.Maui.Maps.Handlers.MapPinHandler`

Handler for map pins on Windows using the WinUI 3 MapIcon.

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
| [[MapPinHandler.MapPinHandler\|MapPinHandler]] | Initializes a new instance of the `MapPinHandler` class with the default mapper. |

## Methods

| Name | Summary |
|---|---|
| [[MapPinHandler.CreatePlatformElement\|CreatePlatformElement]] |  |
| [[MapPinHandler.MapAddress\|MapAddress]] | Maps the `Address` property to the platform element. |
| [[MapPinHandler.MapLabel\|MapLabel]] | Maps the `Label` property to the platform element. |
| [[MapPinHandler.MapLocation\|MapLocation]] | Maps the `Location` property to the platform element. |

## Fields

| Name | Summary |
|---|---|
| [[MapPinHandler.Mapper\|Mapper]] |  |

## Remarks

Platform Limitations (Windows/WinUI 3): Label: MapIcon does not have a built-in label property. To display pin labels, use a custom XAML overlay or tooltip. Address: MapIcon does not display address information. Address display requires custom UI implementation. Custom Icons: MapIcon has limited customization. Custom pin images require additional platform-specific code. Info Windows: Tap-to-show-info-window pattern is not built-in. Implement custom flyouts or popups for pin information display.

## See also

- [[_Microsoft.Maui.Maps.Handlers|Microsoft.Maui.Maps.Handlers namespace]]
- [Online API docs](https://learn.microsoft.com/dotnet/api/microsoft.maui.maps.handlers.mappinhandler)
