---
title: "ScrollToRequestEventArgs.ScrollToRequestEventArgs"
tags:
  - api
  - member/constructor
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.ScrollToRequestEventArgs.ScrollToRequestEventArgs"
declaring_type: "ScrollToRequestEventArgs"
member_kind: constructor
---

# ScrollToRequestEventArgs.ScrollToRequestEventArgs

> [!abstract] Constructor of [[ScrollToRequestEventArgs|ScrollToRequestEventArgs]]
> Namespace: `Microsoft.Maui.Controls`

Initializes a new instance of the `ScrollToRequestEventArgs` class for scrolling by position index.

## Signatures

```csharp
void ScrollToRequestEventArgs(object item, object group, Microsoft.Maui.Controls.ScrollToPosition scrollToPosition, bool isAnimated)
void ScrollToRequestEventArgs(int index, int groupIndex, Microsoft.Maui.Controls.ScrollToPosition scrollToPosition, bool isAnimated)
```

## Parameters

| Parameter | Description |
|---|---|
| `index` | The zero-based index of the item to scroll to. |
| `groupIndex` | The zero-based index of the group containing the item, or -1 if not grouped. |
| `scrollToPosition` | The position where the item should appear in the visible area. |
| `isAnimated` | `true` to animate the scroll; `false` to jump immediately. |

## See also

- Declaring type: [[ScrollToRequestEventArgs|ScrollToRequestEventArgs]]
- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
