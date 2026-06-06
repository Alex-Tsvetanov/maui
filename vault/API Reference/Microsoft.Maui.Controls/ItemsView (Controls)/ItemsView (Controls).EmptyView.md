---
title: "ItemsView (Controls).EmptyView"
tags:
  - api
  - member/property
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.ItemsView.EmptyView"
declaring_type: "ItemsView (Controls)"
member_kind: property
---

# ItemsView (Controls).EmptyView

> [!abstract] Property of [[ItemsView (Controls)|ItemsView (Controls)]]
> Namespace: `Microsoft.Maui.Controls`

Gets or sets the view or object to display when the `ItemsSource` is empty or `null`.

## Signature

```csharp
object EmptyView { get; set; }
```

## Remarks

The empty view provides user feedback when there are no items to display. If `EmptyViewTemplate` is set, it is used to render the empty view; otherwise, the object's string representation or the view itself is displayed.

## See also

- Declaring type: [[ItemsView (Controls)|ItemsView (Controls)]]
- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
