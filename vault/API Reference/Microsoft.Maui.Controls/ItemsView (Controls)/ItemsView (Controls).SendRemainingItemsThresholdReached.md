---
title: "ItemsView (Controls).SendRemainingItemsThresholdReached"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.ItemsView.SendRemainingItemsThresholdReached"
declaring_type: "ItemsView (Controls)"
member_kind: method
---

# ItemsView (Controls).SendRemainingItemsThresholdReached

> [!abstract] Method of [[ItemsView (Controls)|ItemsView (Controls)]]
> Namespace: `Microsoft.Maui.Controls`

Manually triggers the remaining items threshold behavior.

## Signature

```csharp
void SendRemainingItemsThresholdReached()
```

## Remarks

This method raises the `RemainingItemsThresholdReached` event and executes the `RemainingItemsThresholdReachedCommand`. It's typically called by platform-specific renderers when scrolling reaches the threshold, but can be called manually if needed.

## See also

- Declaring type: [[ItemsView (Controls)|ItemsView (Controls)]]
- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
