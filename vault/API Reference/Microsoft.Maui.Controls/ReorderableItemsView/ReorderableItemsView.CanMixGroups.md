---
title: "ReorderableItemsView.CanMixGroups"
tags:
  - api
  - member/property
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.ReorderableItemsView.CanMixGroups"
declaring_type: "ReorderableItemsView"
member_kind: property
---

# ReorderableItemsView.CanMixGroups

> [!abstract] Property of [[ReorderableItemsView|ReorderableItemsView]]
> Namespace: `Microsoft.Maui.Controls`

Gets or sets a value indicating whether items from different groups can be mixed together during reordering.

## Signature

```csharp
bool CanMixGroups { get; set; }
```

## Remarks

When `true`, items can be dragged and dropped between different groups during reordering operations. When `false`, items can only be reordered within their own group. This property is only meaningful when the items view is grouped and `CanReorderItems` is `true`.

## See also

- Declaring type: [[ReorderableItemsView|ReorderableItemsView]]
- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
