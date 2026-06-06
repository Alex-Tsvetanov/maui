---
title: "ViewHandler.PlatformView"
tags:
  - api
  - member/property
  - ns/Microsoft-Maui-Handlers
aliases:
  - "Microsoft.Maui.Handlers.ViewHandler.PlatformView"
declaring_type: "ViewHandler"
member_kind: property
---

# ViewHandler.PlatformView

> [!abstract] Property of [[ViewHandler|ViewHandler]]
> Namespace: `Microsoft.Maui.Handlers`

Gets or sets the platform representation of the view associated to this handler.

## Signature

```csharp
Android.Views.View? PlatformView { get; }
```

## Remarks

This property holds the reference to platform layer view, e.g. the iOS/macOS, Android or Windows view. The abstract (.NET MAUI) view is found in `VirtualView`.

## See also

- Declaring type: [[ViewHandler|ViewHandler]]
- [[_Microsoft.Maui.Handlers|Microsoft.Maui.Handlers namespace]]
