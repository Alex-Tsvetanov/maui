---
title: "ViewHandler<TVirtualView, TPlatformView>.ContainerView"
tags:
  - api
  - member/property
  - ns/Microsoft-Maui-Handlers
aliases:
  - "Microsoft.Maui.Handlers.ViewHandler<TVirtualView, TPlatformView>.ContainerView"
declaring_type: "ViewHandler<TVirtualView, TPlatformView>"
member_kind: property
---

# ViewHandler<TVirtualView, TPlatformView>.ContainerView

> [!abstract] Property of [[ViewHandler{TVirtualView, TPlatformView}|ViewHandler<TVirtualView, TPlatformView>]]
> Namespace: `Microsoft.Maui.Handlers`

Gets the view that acts as a container for the `PlatformView`.

## Signature

```csharp
Microsoft.Maui.Platform.WrapperView? ContainerView { get; set; }
```

## Remarks

Note that this can be `null`. Especially when `HasContainer` is set to `false` this value might not be set.

## See also

- Declaring type: [[ViewHandler{TVirtualView, TPlatformView}|ViewHandler<TVirtualView, TPlatformView>]]
- [[_Microsoft.Maui.Handlers|Microsoft.Maui.Handlers namespace]]
