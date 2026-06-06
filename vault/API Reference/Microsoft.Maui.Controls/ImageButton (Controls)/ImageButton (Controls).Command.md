---
title: "ImageButton (Controls).Command"
tags:
  - api
  - member/property
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.ImageButton.Command"
declaring_type: "ImageButton (Controls)"
member_kind: property
---

# ImageButton (Controls).Command

> [!abstract] Property of [[ImageButton (Controls)|ImageButton (Controls)]]
> Namespace: `Microsoft.Maui.Controls`

Gets or sets the command to invoke when the image button is clicked. This is a bindable property.

## Signature

```csharp
System.Windows.Input.ICommand Command { get; set; }
```

## Remarks

This property is typically used in MVVM patterns to bind the button to a command in the view model. The button's `IsEnabled` property is controlled by `CanExecute`.

## See also

- Declaring type: [[ImageButton (Controls)|ImageButton (Controls)]]
- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
