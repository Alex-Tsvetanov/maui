---
title: "ReorderableItemsView.SendReorderCompleted"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.ReorderableItemsView.SendReorderCompleted"
declaring_type: "ReorderableItemsView"
member_kind: method
---

# ReorderableItemsView.SendReorderCompleted

> [!abstract] Method of [[ReorderableItemsView|ReorderableItemsView]]
> Namespace: `Microsoft.Maui.Controls`

Gets or sets a value indicating whether items in the collection can be reordered by the user.

## Signature

```csharp
void SendReorderCompleted()
```

## Remarks

When enabled, users can typically drag and drop items to reorder them within the collection. The specific interaction method (drag and drop, etc.) depends on the platform implementation. When an item is successfully reordered, the `ReorderCompleted` event is raised.

## See also

- Declaring type: [[ReorderableItemsView|ReorderableItemsView]]
- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
