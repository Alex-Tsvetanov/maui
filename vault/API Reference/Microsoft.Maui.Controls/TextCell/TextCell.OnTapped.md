---
title: "TextCell.OnTapped"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.TextCell.OnTapped"
declaring_type: "TextCell"
member_kind: method
---

# TextCell.OnTapped

> [!abstract] Method of [[TextCell|TextCell]]
> Namespace: `Microsoft.Maui.Controls`

Gets or sets the ICommand to be executed when the TextCell is tapped. This is a bindable property.

## Signature

```csharp
void override OnTapped()
```

## Remarks

Setting the Command property has a side effect of changing the Enabled property depending on ICommand.CanExecute.

## See also

- Declaring type: [[TextCell|TextCell]]
- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
