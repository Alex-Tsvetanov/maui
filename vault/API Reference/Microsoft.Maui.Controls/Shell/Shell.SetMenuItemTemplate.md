---
title: "Shell.SetMenuItemTemplate"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.Shell.SetMenuItemTemplate"
declaring_type: "Shell"
member_kind: method
---

# Shell.SetMenuItemTemplate

> [!abstract] Method of [[Shell|Shell]]
> Namespace: `Microsoft.Maui.Controls`

Sets the applied to objects in the MenuItems collection. Shell provides the Text and IconImageSource properties to the BindingContext of the .

## Signature

```csharp
void static SetMenuItemTemplate(Microsoft.Maui.Controls.BindableObject obj, Microsoft.Maui.Controls.DataTemplate menuItemTemplate)
```

## Parameters

| Parameter | Description |
|---|---|
| `obj` | The object that sets the applied to objects. |
| `menuItemTemplate` | The applied to objects. |

## Remarks

Title can be used instead of Text, and Icon instead of IconImageSource. This allows reuse of the same template for menu items and flyout items.

## See also

- Declaring type: [[Shell|Shell]]
- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
