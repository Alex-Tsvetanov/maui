---
title: "ItemsView (Controls).RemainingItemsThreshold"
tags:
  - api
  - member/property
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.ItemsView.RemainingItemsThreshold"
declaring_type: "ItemsView (Controls)"
member_kind: property
---

# ItemsView (Controls).RemainingItemsThreshold

> [!abstract] Property of [[ItemsView (Controls)|ItemsView (Controls)]]
> Namespace: `Microsoft.Maui.Controls`

Gets or sets the number of remaining items in the view that trigger the `RemainingItemsThresholdReached` event.

## Signature

```csharp
int RemainingItemsThreshold { get; set; }
```

## Remarks

When scrolling reaches a point where only this many items remain to be displayed, the `RemainingItemsThresholdReached` event fires. Set to -1 to disable the threshold behavior. This is commonly used for implementing infinite scroll or incremental loading.

## See also

- Declaring type: [[ItemsView (Controls)|ItemsView (Controls)]]
- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
