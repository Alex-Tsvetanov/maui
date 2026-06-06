---
title: "VisualDiagnostics.OnChildAdded"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui
aliases:
  - "Microsoft.Maui.VisualDiagnostics.OnChildAdded"
declaring_type: "VisualDiagnostics"
member_kind: method
---

# VisualDiagnostics.OnChildAdded

> [!abstract] Method of [[VisualDiagnostics|VisualDiagnostics]]
> Namespace: `Microsoft.Maui`

Called when a child element is added to the visual tree; raises `VisualTreeChanged` event.

## Signatures

```csharp
void static OnChildAdded(Microsoft.Maui.IVisualTreeElement! parent, Microsoft.Maui.IVisualTreeElement! child)
void static OnChildAdded(Microsoft.Maui.IVisualTreeElement? parent, Microsoft.Maui.IVisualTreeElement! child, int newLogicalIndex)
```

## Parameters

| Parameter | Description |
|---|---|
| `parent` | The parent visual element. |
| `child` | The child visual element that was added. |

## See also

- Declaring type: [[VisualDiagnostics|VisualDiagnostics]]
- [[_Microsoft.Maui|Microsoft.Maui namespace]]
