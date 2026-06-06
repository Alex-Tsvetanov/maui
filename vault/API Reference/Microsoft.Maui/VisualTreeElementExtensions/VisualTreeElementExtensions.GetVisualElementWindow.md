---
title: "VisualTreeElementExtensions.GetVisualElementWindow"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui
aliases:
  - "Microsoft.Maui.VisualTreeElementExtensions.GetVisualElementWindow"
declaring_type: "VisualTreeElementExtensions"
member_kind: method
---

# VisualTreeElementExtensions.GetVisualElementWindow

> [!abstract] Method of [[VisualTreeElementExtensions|VisualTreeElementExtensions]]
> Namespace: `Microsoft.Maui`

Gets the Window containing the Visual Tree Element, if the element is contained within one.

## Signature

```csharp
Microsoft.Maui.IWindow? static GetVisualElementWindow(this Microsoft.Maui.IVisualTreeElement! element)
```

## Parameters

| Parameter | Description |
|---|---|
| `element` | `IVisualTreeElement`. |

## Returns

`IWindow` if element is contained within a Window, else returns null.

## See also

- Declaring type: [[VisualTreeElementExtensions|VisualTreeElementExtensions]]
- [[_Microsoft.Maui|Microsoft.Maui namespace]]
