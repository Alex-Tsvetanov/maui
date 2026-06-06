---
title: "MapElementHandler.MapStroke"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Maps-Handlers
aliases:
  - "Microsoft.Maui.Maps.Handlers.MapElementHandler.MapStroke"
declaring_type: "MapElementHandler"
member_kind: method
---

# MapElementHandler.MapStroke

> [!abstract] Method of [[MapElementHandler|MapElementHandler]]
> Namespace: `Microsoft.Maui.Maps.Handlers`

Maps the `Stroke` property to the platform element.

## Signature

```csharp
void static MapStroke(Microsoft.Maui.Maps.Handlers.IMapElementHandler! handler, Microsoft.Maui.Maps.IMapElement! mapElement)
```

## Remarks

Windows Limitation: Stroke is not supported on MapIcon. This method is a no-op on Windows. For shape rendering, use Azure Maps Web SDK.

## See also

- Declaring type: [[MapElementHandler|MapElementHandler]]
- [[_Microsoft.Maui.Maps.Handlers|Microsoft.Maui.Maps.Handlers namespace]]
