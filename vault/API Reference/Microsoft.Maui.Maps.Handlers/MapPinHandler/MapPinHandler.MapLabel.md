---
title: "MapPinHandler.MapLabel"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Maps-Handlers
aliases:
  - "Microsoft.Maui.Maps.Handlers.MapPinHandler.MapLabel"
declaring_type: "MapPinHandler"
member_kind: method
---

# MapPinHandler.MapLabel

> [!abstract] Method of [[MapPinHandler|MapPinHandler]]
> Namespace: `Microsoft.Maui.Maps.Handlers`

Maps the `Label` property to the platform element.

## Signature

```csharp
void static MapLabel(Microsoft.Maui.Maps.Handlers.IMapPinHandler! handler, Microsoft.Maui.Maps.IMapPin! mapPin)
```

## Remarks

Windows Limitation: The WinUI 3 MapIcon does not support labels directly. This method is a no-op on Windows. To display labels, implement a custom overlay.

## See also

- Declaring type: [[MapPinHandler|MapPinHandler]]
- [[_Microsoft.Maui.Maps.Handlers|Microsoft.Maui.Maps.Handlers namespace]]
