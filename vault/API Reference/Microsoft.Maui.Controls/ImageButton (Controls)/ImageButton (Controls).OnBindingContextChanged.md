---
title: "ImageButton (Controls).OnBindingContextChanged"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.ImageButton.OnBindingContextChanged"
declaring_type: "ImageButton (Controls)"
member_kind: method
---

# ImageButton (Controls).OnBindingContextChanged

> [!abstract] Method of [[ImageButton (Controls)|ImageButton (Controls)]]
> Namespace: `Microsoft.Maui.Controls`

Gets or sets a value indicating whether the image should be rendered as opaque. This is a bindable property.

## Signature

```csharp
void override OnBindingContextChanged()
```

## Remarks

This property is typically used in MVVM patterns to bind the button to a command in the view model. The button's `IsEnabled` property is controlled by `CanExecute`.

## See also

- Declaring type: [[ImageButton (Controls)|ImageButton (Controls)]]
- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
