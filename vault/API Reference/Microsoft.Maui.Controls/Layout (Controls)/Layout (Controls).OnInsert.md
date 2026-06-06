---
title: "Layout (Controls).OnInsert"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.Layout.OnInsert"
declaring_type: "Layout (Controls)"
member_kind: method
---

# Layout (Controls).OnInsert

> [!abstract] Method of [[Layout (Controls)|Layout (Controls)]]
> Namespace: `Microsoft.Maui.Controls`

Invoked when `RemoveAt` is called and notifies the handler associated to this layout.

## Signature

```csharp
void virtual OnInsert(int index, Microsoft.Maui.IView view)
```

## Parameters

| Parameter | Description |
|---|---|
| `index` | The index at which the child view was removed. |
| `view` | The child view which was removed. |

## See also

- Declaring type: [[Layout (Controls)|Layout (Controls)]]
- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
