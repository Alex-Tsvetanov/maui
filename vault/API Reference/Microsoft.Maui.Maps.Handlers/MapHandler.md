---
title: "MapHandler"
tags:
  - api
  - kind/class
  - ns/Microsoft-Maui-Maps-Handlers
aliases:
  - "Microsoft.Maui.Maps.Handlers.MapHandler"
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

# MapHandler

> [!abstract] Class in `Microsoft.Maui.Maps.Handlers`
> Full name: `Microsoft.Maui.Maps.Handlers.MapHandler`

Handler for the Map control on Windows using the WinUI 3 MapControl backed by Azure Maps.

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
| [[MapHandler.MapHandler\|MapHandler]] | Initializes a new instance of the `MapHandler` class with default mappers. |

## Properties

| Name | Summary |
|---|---|
| [[MapHandler.Bundle\|Bundle]] |  |
| [[MapHandler.Map\|Map]] |  |

## Methods

| Name | Summary |
|---|---|
| [[MapHandler.ConnectHandler\|ConnectHandler]] |  |
| [[MapHandler.CreatePlatformView\|CreatePlatformView]] |  |
| [[MapHandler.DisconnectHandler\|DisconnectHandler]] |  |
| [[MapHandler.GetNativeCircle\|GetNativeCircle]] |  |
| [[MapHandler.GetNativePolygon\|GetNativePolygon]] |  |
| [[MapHandler.GetNativePolyline\|GetNativePolyline]] |  |
| [[MapHandler.GetPinForMarker\|GetPinForMarker]] |  |
| [[MapHandler.MapElements\|MapElements]] | Maps the `Elements` collection. No-op on Windows: the WinUI 3 `MapElementsLayer` only supports `MapIcon`, not shapes. |
| [[MapHandler.MapIsScrollEnabled\|MapIsScrollEnabled]] | Maps `IsScrollEnabled` via the Azure Maps JS map.setUserInteraction() API. |
| [[MapHandler.MapIsShowingUser\|MapIsShowingUser]] | Maps `IsShowingUser`. No-op on Windows: the WinUI 3 MapControl has no built-in user location display. |
| [[MapHandler.MapIsTrafficEnabled\|MapIsTrafficEnabled]] | Maps `IsTrafficEnabled` via the Azure Maps JS map.setTraffic() API. |
| [[MapHandler.MapIsZoomEnabled\|MapIsZoomEnabled]] | Maps `IsZoomEnabled` via the Azure Maps JS map.setUserInteraction() API. |
| [[MapHandler.MapMapType\|MapMapType]] | Enables or disables the traffic overlay via the Azure Maps JS API. |
| [[MapHandler.MapMoveToRegion\|MapMoveToRegion]] | Handles the `MoveToRegion` command by navigating via the Azure Maps JS camera API. |
| [[MapHandler.MapPins\|MapPins]] | Maps the `Pins` collection and handles dynamic updates to the Pins collection when invoked by the handler. |
| [[MapHandler.MapUpdateMapElement\|MapUpdateMapElement]] | Maps the `UpdateMapElement` command to the platform-specific implementation. |
| [[MapHandler.UpdateMapElement\|UpdateMapElement]] |  |

## Fields

| Name | Summary |
|---|---|
| [[MapHandler.CommandMapper\|CommandMapper]] | The command mapper that maps cross-platform commands to platform-specific methods. |
| [[MapHandler.Mapper\|Mapper]] |  |

## Remarks

Authentication: Set your Azure Maps subscription key using: builder.ConfigureEssentials(e => e.UseMapServiceToken("YOUR_AZURE_MAPS_KEY")); Get a key from the Azure Portal: https://portal.azure.com → Azure Maps account → Authentication. Supported features (via Azure Maps JS API through internal WebView2): MoveToRegion: Navigates via map.setCamera() . MapType: Street/Satellite/Hybrid via map.setStyle() (road/satellite/satellite_road_labels). IsTrafficEnabled: Traffic flow and incidents via map.setTraffic() . IsZoomEnabled/IsScrollEnabled: Independent control via map.setUserInteraction() . Pins: Displays map pins using `MapIcon` on a `MapElementsLayer`. Pin click events are supported. Unsupported features (no-op on Windows): IsShowingUser: Not built-in. Use the Geolocation API and a custom `MapIcon` to display user location. Polylines/Polygons/Circles: `MapElementsLayer` only supports `MapIcon`. Shapes are not rendered. Pin Labels/InfoWindows: `MapIcon` does not support labels or info windows. Map.Clicked (background): The `MapElementClick` event only fires for `MapElement` clicks, not empty map area clicks.

## See also

- [[_Microsoft.Maui.Maps.Handlers|Microsoft.Maui.Maps.Handlers namespace]]
- [Online API docs](https://learn.microsoft.com/dotnet/api/microsoft.maui.maps.handlers.maphandler)
