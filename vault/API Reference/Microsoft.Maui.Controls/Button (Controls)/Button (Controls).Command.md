---
title: "Button (Controls).Command"
tags:
  - api
  - member/property
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.Button.Command"
declaring_type: "Button (Controls)"
member_kind: property
---

# Button (Controls).Command

> [!abstract] Property of [[Button (Controls)|Button (Controls)]]
> Namespace: `Microsoft.Maui.Controls`

Gets or sets the command to invoke when the button is activated. This is a bindable property.

## Signature

```csharp
System.Windows.Input.ICommand Command { get; set; }
```

## Remarks

This property is used to associate a command with an instance of a button. This property is most often set in the MVVM pattern to bind callbacks back into the ViewModel. `IsEnabled` is controlled by the `CanExecute` if set.

## See also

- Declaring type: [[Button (Controls)|Button (Controls)]]
- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
