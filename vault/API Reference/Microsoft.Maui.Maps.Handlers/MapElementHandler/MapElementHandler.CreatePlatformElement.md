---
title: "MapElementHandler.CreatePlatformElement"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Maps-Handlers
aliases:
  - "Microsoft.Maui.Maps.Handlers.MapElementHandler.CreatePlatformElement"
declaring_type: "MapElementHandler"
member_kind: method
---

# MapElementHandler.CreatePlatformElement

> [!abstract] Method of [[MapElementHandler|MapElementHandler]]
> Namespace: `Microsoft.Maui.Maps.Handlers`

Creates and returns the native map element backing the cross-platform map element.

## Signature

```csharp
Java.Lang.Object! override CreatePlatformElement()
```

## Remarks

Windows Limitation: Returns a MapIcon as a placeholder since the WinUI 3 MapControl does not support polylines, polygons, or circles in MapElementsLayer.

## See also

- Declaring type: [[MapElementHandler|MapElementHandler]]
- [[_Microsoft.Maui.Maps.Handlers|Microsoft.Maui.Maps.Handlers namespace]]
