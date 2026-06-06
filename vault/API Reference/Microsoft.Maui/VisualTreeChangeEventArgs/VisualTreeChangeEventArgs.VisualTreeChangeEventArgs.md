---
title: "VisualTreeChangeEventArgs.VisualTreeChangeEventArgs"
tags:
  - api
  - member/constructor
  - ns/Microsoft-Maui
aliases:
  - "Microsoft.Maui.VisualTreeChangeEventArgs.VisualTreeChangeEventArgs"
declaring_type: "VisualTreeChangeEventArgs"
member_kind: constructor
---

# VisualTreeChangeEventArgs.VisualTreeChangeEventArgs

> [!abstract] Constructor of [[VisualTreeChangeEventArgs|VisualTreeChangeEventArgs]]
> Namespace: `Microsoft.Maui`

Initializes a new instance of `VisualTreeChangeEventArgs`.

## Signature

```csharp
void VisualTreeChangeEventArgs(object? parent, object! child, int childIndex, Microsoft.Maui.VisualTreeChangeType changeType)
```

## Parameters

| Parameter | Description |
|---|---|
| `parent` | The parent visual element, or null for the root. |
| `child` | The child visual element involved in the change. |
| `childIndex` | The logical index of the child within its parent at the time of change. |
| `changeType` | The type of change that occurred. |

## See also

- Declaring type: [[VisualTreeChangeEventArgs|VisualTreeChangeEventArgs]]
- [[_Microsoft.Maui|Microsoft.Maui namespace]]
