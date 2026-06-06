---
title: "MapPinHandler.MapAddress"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Maps-Handlers
aliases:
  - "Microsoft.Maui.Maps.Handlers.MapPinHandler.MapAddress"
declaring_type: "MapPinHandler"
member_kind: method
---

# MapPinHandler.MapAddress

> [!abstract] Method of [[MapPinHandler|MapPinHandler]]
> Namespace: `Microsoft.Maui.Maps.Handlers`

Maps the `Address` property to the platform element.

## Signature

```csharp
void static MapAddress(Microsoft.Maui.Maps.Handlers.IMapPinHandler! handler, Microsoft.Maui.Maps.IMapPin! mapPin)
```

## Remarks

Windows Limitation: The WinUI 3 MapIcon does not support address display. This method is a no-op on Windows. To display addresses, implement a custom overlay or info window.

## See also

- Declaring type: [[MapPinHandler|MapPinHandler]]
- [[_Microsoft.Maui.Maps.Handlers|Microsoft.Maui.Maps.Handlers namespace]]
