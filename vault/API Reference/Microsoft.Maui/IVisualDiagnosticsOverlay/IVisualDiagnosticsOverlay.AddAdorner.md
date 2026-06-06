---
title: "IVisualDiagnosticsOverlay.AddAdorner"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui
aliases:
  - "Microsoft.Maui.IVisualDiagnosticsOverlay.AddAdorner"
declaring_type: "IVisualDiagnosticsOverlay"
member_kind: method
---

# IVisualDiagnosticsOverlay.AddAdorner

> [!abstract] Method of [[IVisualDiagnosticsOverlay|IVisualDiagnosticsOverlay]]
> Namespace: `Microsoft.Maui`

Adds a new adorner to the Visual Diagnostics Overlay.

## Signatures

```csharp
bool AddAdorner(Microsoft.Maui.IAdorner! adorner, bool scrollToElement)
bool AddAdorner(Microsoft.Maui.IVisualTreeElement! visualElement, bool scrollToElement)
```

## Parameters

| Parameter | Description |
|---|---|
| `adorner` | `IAdorner`. |
| `scrollToElement` | When adding the adorner, scroll to the element. Only applies if the element is contained in an `IScrollView`. |

## See also

- Declaring type: [[IVisualDiagnosticsOverlay|IVisualDiagnosticsOverlay]]
- [[_Microsoft.Maui|Microsoft.Maui namespace]]
