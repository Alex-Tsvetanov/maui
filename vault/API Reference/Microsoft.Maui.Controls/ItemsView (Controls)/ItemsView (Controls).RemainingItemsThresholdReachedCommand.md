---
title: "ItemsView (Controls).RemainingItemsThresholdReachedCommand"
tags:
  - api
  - member/property
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.ItemsView.RemainingItemsThresholdReachedCommand"
declaring_type: "ItemsView (Controls)"
member_kind: property
---

# ItemsView (Controls).RemainingItemsThresholdReachedCommand

> [!abstract] Property of [[ItemsView (Controls)|ItemsView (Controls)]]
> Namespace: `Microsoft.Maui.Controls`

Gets or sets the command to execute when the remaining items threshold is reached during scrolling.

## Signature

```csharp
System.Windows.Input.ICommand RemainingItemsThresholdReachedCommand { get; set; }
```

## Remarks

This command is commonly used to implement incremental loading (infinite scroll). When scrolling reaches the threshold specified by `RemainingItemsThreshold`, this command is executed, allowing you to load more items asynchronously.

## See also

- Declaring type: [[ItemsView (Controls)|ItemsView (Controls)]]
- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
