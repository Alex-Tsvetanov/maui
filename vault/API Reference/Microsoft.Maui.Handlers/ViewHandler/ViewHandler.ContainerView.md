---
title: "ViewHandler.ContainerView"
tags:
  - api
  - member/property
  - ns/Microsoft-Maui-Handlers
aliases:
  - "Microsoft.Maui.Handlers.ViewHandler.ContainerView"
declaring_type: "ViewHandler"
member_kind: property
---

# ViewHandler.ContainerView

> [!abstract] Property of [[ViewHandler|ViewHandler]]
> Namespace: `Microsoft.Maui.Handlers`

Gets the view that acts as a container for the `PlatformView`.

## Signature

```csharp
Android.Views.View? ContainerView { get; }
```

## Remarks

Note that this can be `null`. Especially when `HasContainer` is set to `false` this value might not be set.

## See also

- Declaring type: [[ViewHandler|ViewHandler]]
- [[_Microsoft.Maui.Handlers|Microsoft.Maui.Handlers namespace]]
