---
title: "AbsoluteLayout (Controls).GetLayoutBounds"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.AbsoluteLayout.GetLayoutBounds"
declaring_type: "AbsoluteLayout (Controls)"
member_kind: method
---

# AbsoluteLayout (Controls).GetLayoutBounds

> [!abstract] Method of [[AbsoluteLayout (Controls)|AbsoluteLayout (Controls)]]
> Namespace: `Microsoft.Maui.Controls`

Gets the layout bounds of a view that will be used to interpret the layout bounds set on it when it is added to the layout.

## Signatures

```csharp
Microsoft.Maui.Graphics.Rect GetLayoutBounds(Microsoft.Maui.IView view)
Microsoft.Maui.Graphics.Rect static GetLayoutBounds(Microsoft.Maui.Controls.BindableObject bindable)
```

## Parameters

| Parameter | Description |
|---|---|
| `bindable` | The bindable object to determine the layout bounds for. |

## Returns

A `Rect` with the layout bounds for the given bindable object.

## See also

- Declaring type: [[AbsoluteLayout (Controls)|AbsoluteLayout (Controls)]]
- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
