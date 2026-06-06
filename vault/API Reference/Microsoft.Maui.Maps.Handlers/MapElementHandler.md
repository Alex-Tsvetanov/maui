---
title: "MapElementHandler"
tags:
  - api
  - kind/class
  - ns/Microsoft-Maui-Maps-Handlers
aliases:
  - "Microsoft.Maui.Maps.Handlers.MapElementHandler"
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

# MapElementHandler

> [!abstract] Class in `Microsoft.Maui.Maps.Handlers`
> Full name: `Microsoft.Maui.Maps.Handlers.MapElementHandler`

Handler for map elements (polylines, polygons, circles) on Windows.

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
| [[MapElementHandler.MapElementHandler\|MapElementHandler]] | Initializes a new instance of the `MapElementHandler` class with the default mapper. |

## Methods

| Name | Summary |
|---|---|
| [[MapElementHandler.CreatePlatformElement\|CreatePlatformElement]] |  |
| [[MapElementHandler.MapCenter\|MapCenter]] |  |
| [[MapElementHandler.MapFill\|MapFill]] | Maps the `Fill` property to the platform element. |
| [[MapElementHandler.MapGeopath\|MapGeopath]] |  |
| [[MapElementHandler.MapRadius\|MapRadius]] |  |
| [[MapElementHandler.MapStroke\|MapStroke]] | Maps the `Stroke` property to the platform element. |
| [[MapElementHandler.MapStrokeThickness\|MapStrokeThickness]] | Maps the `StrokeThickness` property to the platform element. |

## Fields

| Name | Summary |
|---|---|
| [[MapElementHandler.Mapper\|Mapper]] |  |

## Remarks

Platform Limitations (Windows/WinUI 3): Polylines: The WinUI 3 MapElementsLayer does not support polylines directly. To render polylines, consider using Azure Maps REST API, Web SDK in a WebView, or custom XAML overlays. Polygons: The WinUI 3 MapElementsLayer does not support polygons directly. To render polygons, consider using Azure Maps REST API, Web SDK in a WebView, or custom XAML overlays. Circles: The WinUI 3 MapElementsLayer does not support circles directly. To render circles, approximate with a polygon or use Azure Maps REST API. Stroke/Fill: MapIcon (the only fully supported element type) does not support stroke or fill properties. These properties are no-ops on Windows. The current implementation returns a MapIcon as a placeholder. For full shape support, consider integrating the Azure Maps Web SDK via a WebView control.

## See also

- [[_Microsoft.Maui.Maps.Handlers|Microsoft.Maui.Maps.Handlers namespace]]
- [Online API docs](https://learn.microsoft.com/dotnet/api/microsoft.maui.maps.handlers.mapelementhandler)
