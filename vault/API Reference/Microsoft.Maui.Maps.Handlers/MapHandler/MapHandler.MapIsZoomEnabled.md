---
title: "MapHandler.MapIsZoomEnabled"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Maps-Handlers
aliases:
  - "Microsoft.Maui.Maps.Handlers.MapHandler.MapIsZoomEnabled"
declaring_type: "MapHandler"
member_kind: method
---

# MapHandler.MapIsZoomEnabled

> [!abstract] Method of [[MapHandler|MapHandler]]
> Namespace: `Microsoft.Maui.Maps.Handlers`

Maps `IsZoomEnabled` via the Azure Maps JS map.setUserInteraction() API.

## Signature

```csharp
void static MapIsZoomEnabled(Microsoft.Maui.Maps.Handlers.IMapHandler! handler, Microsoft.Maui.Maps.IMap! map)
```

## Remarks

Controls scrollZoomInteraction and dblClickZoomInteraction independently from scroll/drag. Also keeps InteractiveControlsVisible in sync so the built-in UI controls remain accessible.

## See also

- Declaring type: [[MapHandler|MapHandler]]
- [[_Microsoft.Maui.Maps.Handlers|Microsoft.Maui.Maps.Handlers namespace]]
