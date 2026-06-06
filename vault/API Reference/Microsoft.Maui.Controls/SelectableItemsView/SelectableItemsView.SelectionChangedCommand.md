---
title: "SelectableItemsView.SelectionChangedCommand"
tags:
  - api
  - member/property
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.SelectableItemsView.SelectionChangedCommand"
declaring_type: "SelectableItemsView"
member_kind: property
---

# SelectableItemsView.SelectionChangedCommand

> [!abstract] Property of [[SelectableItemsView|SelectableItemsView]]
> Namespace: `Microsoft.Maui.Controls`

Gets or sets the command to execute when the selection changes.

## Signature

```csharp
System.Windows.Input.ICommand SelectionChangedCommand { get; set; }
```

## Remarks

The command's parameter is set to the `SelectionChangedCommandParameter` value. The command executes in addition to the `SelectionChanged` event firing.

## See also

- Declaring type: [[SelectableItemsView|SelectableItemsView]]
- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
