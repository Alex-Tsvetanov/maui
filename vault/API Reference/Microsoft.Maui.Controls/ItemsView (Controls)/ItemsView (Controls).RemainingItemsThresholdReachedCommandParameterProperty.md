---
title: "ItemsView (Controls).RemainingItemsThresholdReachedCommandParameterProperty"
tags:
  - api
  - member/field
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.ItemsView.RemainingItemsThresholdReachedCommandParameterProperty"
declaring_type: "ItemsView (Controls)"
member_kind: field
---

# ItemsView (Controls).RemainingItemsThresholdReachedCommandParameterProperty

> [!abstract] Field of [[ItemsView (Controls)|ItemsView (Controls)]]
> Namespace: `Microsoft.Maui.Controls`

Gets or sets the command to execute when the remaining items threshold is reached during scrolling.

## Signature

```csharp
Microsoft.Maui.Controls.BindableProperty static readonly RemainingItemsThresholdReachedCommandParameterProperty
```

## Remarks

This command is commonly used to implement incremental loading (infinite scroll). When scrolling reaches the threshold specified by `RemainingItemsThreshold`, this command is executed, allowing you to load more items asynchronously.

## See also

- Declaring type: [[ItemsView (Controls)|ItemsView (Controls)]]
- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
