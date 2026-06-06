---
title: "ReorderableItemsView.CanReorderItemsProperty"
tags:
  - api
  - member/field
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.ReorderableItemsView.CanReorderItemsProperty"
declaring_type: "ReorderableItemsView"
member_kind: field
---

# ReorderableItemsView.CanReorderItemsProperty

> [!abstract] Field of [[ReorderableItemsView|ReorderableItemsView]]
> Namespace: `Microsoft.Maui.Controls`

Gets or sets a value indicating whether items from different groups can be mixed together during reordering.

## Signature

```csharp
Microsoft.Maui.Controls.BindableProperty static readonly CanReorderItemsProperty
```

## Remarks

When `true`, items can be dragged and dropped between different groups during reordering operations. When `false`, items can only be reordered within their own group. This property is only meaningful when the items view is grouped and `CanReorderItems` is `true`.

## See also

- Declaring type: [[ReorderableItemsView|ReorderableItemsView]]
- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
