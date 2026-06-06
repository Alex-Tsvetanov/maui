---
title: "DataTemplateExtensions.SelectDataTemplate"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Controls-Internals
aliases:
  - "Microsoft.Maui.Controls.Internals.DataTemplateExtensions.SelectDataTemplate"
declaring_type: "DataTemplateExtensions"
member_kind: method
---

# DataTemplateExtensions.SelectDataTemplate

> [!abstract] Method of [[DataTemplateExtensions|DataTemplateExtensions]]
> Namespace: `Microsoft.Maui.Controls.Internals`

Returns the appropriate template, invoking selector logic if the template is a `DataTemplateSelector`.

## Signature

```csharp
Microsoft.Maui.Controls.DataTemplate static SelectDataTemplate(this Microsoft.Maui.Controls.DataTemplate self, object item, Microsoft.Maui.Controls.BindableObject container)
```

## Parameters

| Parameter | Description |
|---|---|
| `self` | The template or selector. |
| `item` | The data item. |
| `container` | The container that will display the item. |

## Returns

The selected `DataTemplate`.

## See also

- Declaring type: [[DataTemplateExtensions|DataTemplateExtensions]]
- [[_Microsoft.Maui.Controls.Internals|Microsoft.Maui.Controls.Internals namespace]]
