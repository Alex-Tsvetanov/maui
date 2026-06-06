---
title: "BindableObjectExtensions.SetBinding<TSource, TProperty>"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.BindableObjectExtensions.SetBinding<TSource, TProperty>"
declaring_type: "BindableObjectExtensions"
member_kind: method
---

# BindableObjectExtensions.SetBinding<TSource, TProperty>

> [!abstract] Method of [[BindableObjectExtensions|BindableObjectExtensions]]
> Namespace: `Microsoft.Maui.Controls`

Creates and applies a binding to a property.

## Signature

```csharp
void static SetBinding<TSource, TProperty>(this Microsoft.Maui.Controls.BindableObject! self, Microsoft.Maui.Controls.BindableProperty! targetProperty, System.Func<TSource, TProperty>! getter, Microsoft.Maui.Controls.BindingMode mode = Microsoft.Maui.Controls.BindingMode.Default, Microsoft.Maui.Controls.IValueConverter? converter = null, object? converterParameter = null, string? stringFormat = null, object? source = null, object? fallbackValue = null, object? targetNullValue = null)
```

## Remarks

The following example shows how to use the extension method to set a binding.

## Parameters

| Parameter | Description |
|---|---|
| `self` | The `BindableObject`. |
| `targetProperty` | The BindableProperty on which to set a binding. |
| `path` | A `String` indicating the property path to bind to. |
| `mode` | The `BindingMode` for the binding. This parameter is optional. Default is `Default`. |
| `converter` | An `IValueConverter` for the binding. This parameter is optional. Default is `null`. |
| `stringFormat` | A string used as stringFormat for the binding. This parameter is optional. Default is `null`. |

## See also

- Declaring type: [[BindableObjectExtensions|BindableObjectExtensions]]
- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
