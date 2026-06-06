---
title: "VisualDiagnostics.OnChildRemoved"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui
aliases:
  - "Microsoft.Maui.VisualDiagnostics.OnChildRemoved"
declaring_type: "VisualDiagnostics"
member_kind: method
---

# VisualDiagnostics.OnChildRemoved

> [!abstract] Method of [[VisualDiagnostics|VisualDiagnostics]]
> Namespace: `Microsoft.Maui`

Called when a child element is removed from the visual tree; raises `VisualTreeChanged` event.

## Signature

```csharp
void static OnChildRemoved(Microsoft.Maui.IVisualTreeElement! parent, Microsoft.Maui.IVisualTreeElement! child, int oldLogicalIndex)
```

## Parameters

| Parameter | Description |
|---|---|
| `parent` | The parent visual element. |
| `child` | The child visual element that was removed. |
| `oldLogicalIndex` | The previous logical index of the child. |

## See also

- Declaring type: [[VisualDiagnostics|VisualDiagnostics]]
- [[_Microsoft.Maui|Microsoft.Maui namespace]]
