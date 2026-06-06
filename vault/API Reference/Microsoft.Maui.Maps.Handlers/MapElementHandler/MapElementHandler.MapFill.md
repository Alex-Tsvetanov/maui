---
title: "MapElementHandler.MapFill"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Maps-Handlers
aliases:
  - "Microsoft.Maui.Maps.Handlers.MapElementHandler.MapFill"
declaring_type: "MapElementHandler"
member_kind: method
---

# MapElementHandler.MapFill

> [!abstract] Method of [[MapElementHandler|MapElementHandler]]
> Namespace: `Microsoft.Maui.Maps.Handlers`

Maps the `Fill` property to the platform element.

## Signature

```csharp
void static MapFill(Microsoft.Maui.Maps.Handlers.IMapElementHandler! handler, Microsoft.Maui.Maps.IMapElement! mapElement)
```

## Remarks

Windows Limitation: Fill is not supported on MapIcon. This method is a no-op on Windows. For shape rendering, use Azure Maps Web SDK.

## See also

- Declaring type: [[MapElementHandler|MapElementHandler]]
- [[_Microsoft.Maui.Maps.Handlers|Microsoft.Maui.Maps.Handlers namespace]]
