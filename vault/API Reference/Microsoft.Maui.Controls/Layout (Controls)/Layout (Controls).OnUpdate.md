---
title: "Layout (Controls).OnUpdate"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.Layout.OnUpdate"
declaring_type: "Layout (Controls)"
member_kind: method
---

# Layout (Controls).OnUpdate

> [!abstract] Method of [[Layout (Controls)|Layout (Controls)]]
> Namespace: `Microsoft.Maui.Controls`

Invoked when `this[int]` is called and notifies the handler associated to this layout.

## Signature

```csharp
void virtual OnUpdate(int index, Microsoft.Maui.IView view, Microsoft.Maui.IView oldView)
```

## Parameters

| Parameter | Description |
|---|---|
| `index` | The index at which the child view was updated. |
| `view` | The new child view which was added at `index`. |
| `oldView` | The previous child view which was at `index`. |

## See also

- Declaring type: [[Layout (Controls)|Layout (Controls)]]
- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
