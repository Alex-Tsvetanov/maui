---
title: "SelectableItemsView.SelectedItem"
tags:
  - api
  - member/property
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.SelectableItemsView.SelectedItem"
declaring_type: "SelectableItemsView"
member_kind: property
---

# SelectableItemsView.SelectedItem

> [!abstract] Property of [[SelectableItemsView|SelectableItemsView]]
> Namespace: `Microsoft.Maui.Controls`

Gets or sets the currently selected item when `SelectionMode` is `Single`.

## Signature

```csharp
object SelectedItem { get; set; }
```

## Remarks

This property is only meaningful when `SelectionMode` is `Single`. For multiple selection, use the `SelectedItems` property instead. The binding mode defaults to `TwoWay`.

## See also

- Declaring type: [[SelectableItemsView|SelectableItemsView]]
- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
