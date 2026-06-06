---
title: "ItemsView (Controls).ItemsUpdatingScrollMode"
tags:
  - api
  - member/property
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.ItemsView.ItemsUpdatingScrollMode"
declaring_type: "ItemsView (Controls)"
member_kind: property
---

# ItemsView (Controls).ItemsUpdatingScrollMode

> [!abstract] Property of [[ItemsView (Controls)|ItemsView (Controls)]]
> Namespace: `Microsoft.Maui.Controls`

Gets or sets the scroll behavior when items are added, removed, or updated in the collection.

## Signature

```csharp
Microsoft.Maui.Controls.ItemsUpdatingScrollMode ItemsUpdatingScrollMode { get; set; }
```

## Remarks

This property controls how the view maintains its scroll position when the `ItemsSource` changes. Use `KeepLastItemInView` for chat-like interfaces where new items are added at the end.

## See also

- Declaring type: [[ItemsView (Controls)|ItemsView (Controls)]]
- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
