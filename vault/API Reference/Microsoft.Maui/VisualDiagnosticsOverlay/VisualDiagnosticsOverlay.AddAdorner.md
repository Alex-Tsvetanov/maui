---
title: "VisualDiagnosticsOverlay.AddAdorner"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui
aliases:
  - "Microsoft.Maui.VisualDiagnosticsOverlay.AddAdorner"
declaring_type: "VisualDiagnosticsOverlay"
member_kind: method
---

# VisualDiagnosticsOverlay.AddAdorner

> [!abstract] Method of [[VisualDiagnosticsOverlay|VisualDiagnosticsOverlay]]
> Namespace: `Microsoft.Maui`

Adds a new adorner to the Visual Diagnostics Overlay.

## Signatures

```csharp
bool AddAdorner(Microsoft.Maui.IAdorner! adorner, bool scrollToView = false)
bool AddAdorner(Microsoft.Maui.IVisualTreeElement! visualElement, bool scrollToView = false)
```

## Parameters

| Parameter | Description |
|---|---|
| `adorner` | `IAdorner`. |
| `scrollToElement` | When adding the adorner, scroll to the element. Only applies if the element is contained in an `IScrollView`. |

## See also

- Declaring type: [[VisualDiagnosticsOverlay|VisualDiagnosticsOverlay]]
- [[_Microsoft.Maui|Microsoft.Maui namespace]]
