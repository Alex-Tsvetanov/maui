---
title: "TextCell.Command"
tags:
  - api
  - member/property
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.TextCell.Command"
declaring_type: "TextCell"
member_kind: property
---

# TextCell.Command

> [!abstract] Property of [[TextCell|TextCell]]
> Namespace: `Microsoft.Maui.Controls`

Gets or sets the ICommand to be executed when the TextCell is tapped. This is a bindable property.

## Signature

```csharp
System.Windows.Input.ICommand Command { get; set; }
```

## Remarks

Setting the Command property has a side effect of changing the Enabled property depending on ICommand.CanExecute.

## See also

- Declaring type: [[TextCell|TextCell]]
- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
