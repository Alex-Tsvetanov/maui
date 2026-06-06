---
title: "MapHandler.MapMoveToRegion"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Maps-Handlers
aliases:
  - "Microsoft.Maui.Maps.Handlers.MapHandler.MapMoveToRegion"
declaring_type: "MapHandler"
member_kind: method
---

# MapHandler.MapMoveToRegion

> [!abstract] Method of [[MapHandler|MapHandler]]
> Namespace: `Microsoft.Maui.Maps.Handlers`

Handles the `MoveToRegion` command by navigating via the Azure Maps JS camera API.

## Signature

```csharp
void static MapMoveToRegion(Microsoft.Maui.Maps.Handlers.IMapHandler! handler, Microsoft.Maui.Maps.IMap! map, object? arg)
```

## Remarks

The WinUI 3 MapControl wraps Azure Maps in a WebView2. Setting the Center dependency property does not reliably navigate the map view. Instead, we call map.setCamera() via JavaScript. The zoom level is calculated from the `MapSpan` using the Spherical Mercator formula: zoom = log2(360 / degrees) , clamped to the Azure Maps range of 0–24. Navigation uses an ease animation (300ms) for smooth transitions.

## See also

- Declaring type: [[MapHandler|MapHandler]]
- [[_Microsoft.Maui.Maps.Handlers|Microsoft.Maui.Maps.Handlers namespace]]
