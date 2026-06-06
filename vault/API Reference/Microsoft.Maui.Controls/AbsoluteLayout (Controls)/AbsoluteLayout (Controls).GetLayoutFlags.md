---
title: "AbsoluteLayout (Controls).GetLayoutFlags"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.AbsoluteLayout.GetLayoutFlags"
declaring_type: "AbsoluteLayout (Controls)"
member_kind: method
---

# AbsoluteLayout (Controls).GetLayoutFlags

> [!abstract] Method of [[AbsoluteLayout (Controls)|AbsoluteLayout (Controls)]]
> Namespace: `Microsoft.Maui.Controls`

Gets the layout flags of a view that will be used to interpret the layout bounds set on it when it is added to the layout.

## Signatures

```csharp
Microsoft.Maui.Layouts.AbsoluteLayoutFlags GetLayoutFlags(Microsoft.Maui.IView view)
Microsoft.Maui.Layouts.AbsoluteLayoutFlags static GetLayoutFlags(Microsoft.Maui.Controls.BindableObject bindable)
```

## Parameters

| Parameter | Description |
|---|---|
| `bindable` | The bindable object to retrieve the layout flags for. |

## Returns

The layout flags applied to the given bindable object.

## See also

- Declaring type: [[AbsoluteLayout (Controls)|AbsoluteLayout (Controls)]]
- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
