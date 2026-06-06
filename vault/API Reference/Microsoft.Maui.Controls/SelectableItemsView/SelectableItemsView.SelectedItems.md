---
title: "SelectableItemsView.SelectedItems"
tags:
  - api
  - member/property
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.SelectableItemsView.SelectedItems"
declaring_type: "SelectableItemsView"
member_kind: property
---

# SelectableItemsView.SelectedItems

> [!abstract] Property of [[SelectableItemsView|SelectableItemsView]]
> Namespace: `Microsoft.Maui.Controls`

Gets or sets the collection of currently selected items when `SelectionMode` is `Multiple`.

## Signature

```csharp
System.Collections.Generic.IList<object> SelectedItems { get; set; }
```

## Remarks

This property tracks all selected items when `SelectionMode` is `Multiple`. For single selection, use the `SelectedItem` property instead for simpler binding.

## See also

- Declaring type: [[SelectableItemsView|SelectableItemsView]]
- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
