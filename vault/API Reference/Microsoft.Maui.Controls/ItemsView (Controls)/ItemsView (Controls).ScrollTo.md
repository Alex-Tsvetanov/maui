---
title: "ItemsView (Controls).ScrollTo"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.ItemsView.ScrollTo"
declaring_type: "ItemsView (Controls)"
member_kind: method
---

# ItemsView (Controls).ScrollTo

> [!abstract] Method of [[ItemsView (Controls)|ItemsView (Controls)]]
> Namespace: `Microsoft.Maui.Controls`

Gets or sets the scroll behavior when items are added, removed, or updated in the collection.

## Signatures

```csharp
void ScrollTo(object item, object group = null, Microsoft.Maui.Controls.ScrollToPosition position = Microsoft.Maui.Controls.ScrollToPosition.MakeVisible, bool animate = true)
void ScrollTo(int index, int groupIndex = -1, Microsoft.Maui.Controls.ScrollToPosition position = Microsoft.Maui.Controls.ScrollToPosition.MakeVisible, bool animate = true)
```

## Parameters

| Parameter | Description |
|---|---|
| `index` | The zero-based index of the item to scroll to. |
| `groupIndex` | The zero-based index of the group containing the item, or -1 if not grouped. |
| `position` | The position where the item should appear in the visible area after scrolling. |
| `animate` | `true` to animate the scroll; `false` to jump immediately. |

## Remarks

This property controls how the view maintains its scroll position when the `ItemsSource` changes. Use `KeepLastItemInView` for chat-like interfaces where new items are added at the end.

## See also

- Declaring type: [[ItemsView (Controls)|ItemsView (Controls)]]
- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
