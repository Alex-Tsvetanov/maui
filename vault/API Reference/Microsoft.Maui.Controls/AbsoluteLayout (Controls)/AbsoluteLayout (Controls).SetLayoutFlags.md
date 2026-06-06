---
title: "AbsoluteLayout (Controls).SetLayoutFlags"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.AbsoluteLayout.SetLayoutFlags"
declaring_type: "AbsoluteLayout (Controls)"
member_kind: method
---

# AbsoluteLayout (Controls).SetLayoutFlags

> [!abstract] Method of [[AbsoluteLayout (Controls)|AbsoluteLayout (Controls)]]
> Namespace: `Microsoft.Maui.Controls`

Sets the layout flags of a view that will be used to interpret the layout bounds set on it when it is added to the layout.

## Signatures

```csharp
void SetLayoutFlags(Microsoft.Maui.IView view, Microsoft.Maui.Layouts.AbsoluteLayoutFlags flags)
void static SetLayoutFlags(Microsoft.Maui.Controls.BindableObject bindable, Microsoft.Maui.Layouts.AbsoluteLayoutFlags flags)
```

## Remarks

This method supports the AbsoluteLayout.LayoutFlags XAML attached property. In XAML, application developers can specify one or more of the `AbsoluteLayoutFlags` enumeration value names for the value of this property on the children of a `AbsoluteLayout`.

## See also

- Declaring type: [[AbsoluteLayout (Controls)|AbsoluteLayout (Controls)]]
- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
