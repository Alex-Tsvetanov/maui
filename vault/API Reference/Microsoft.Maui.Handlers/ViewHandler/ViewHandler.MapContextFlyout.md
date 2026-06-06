---
title: "ViewHandler.MapContextFlyout"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Handlers
aliases:
  - "Microsoft.Maui.Handlers.ViewHandler.MapContextFlyout"
declaring_type: "ViewHandler"
member_kind: method
---

# ViewHandler.MapContextFlyout

> [!abstract] Method of [[ViewHandler|ViewHandler]]
> Namespace: `Microsoft.Maui.Handlers`

Maps the abstract `IView` to the platform-specific implementations of a `IContextFlyoutElement`.

## Signature

```csharp
void static MapContextFlyout(Microsoft.Maui.IViewHandler! handler, Microsoft.Maui.IView! view)
```

## Parameters

| Parameter | Description |
|---|---|
| `handler` | The associated handler. |
| `view` | The associated `IView` instance. |

## Remarks

If the `view` can't be cast to a `IContextFlyoutElement`, this method does nothing.

## See also

- Declaring type: [[ViewHandler|ViewHandler]]
- [[_Microsoft.Maui.Handlers|Microsoft.Maui.Handlers namespace]]
