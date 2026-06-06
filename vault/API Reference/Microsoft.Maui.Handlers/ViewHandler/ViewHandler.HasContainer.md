---
title: "ViewHandler.HasContainer"
tags:
  - api
  - member/property
  - ns/Microsoft-Maui-Handlers
aliases:
  - "Microsoft.Maui.Handlers.ViewHandler.HasContainer"
declaring_type: "ViewHandler"
member_kind: property
---

# ViewHandler.HasContainer

> [!abstract] Property of [[ViewHandler|ViewHandler]]
> Namespace: `Microsoft.Maui.Handlers`

Gets or sets a value that indicates whether the `PlatformView` is contained within a view.

## Signature

```csharp
bool HasContainer { get; set; }
```

## Remarks

When set to `true`, `SetupContainer` is called to setup the container view. When set to `false`, `RemoveContainer` is called to remove the current container view.

## See also

- Declaring type: [[ViewHandler|ViewHandler]]
- [[_Microsoft.Maui.Handlers|Microsoft.Maui.Handlers namespace]]
