---
title: "BindableProperty.Create"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.BindableProperty.Create"
declaring_type: "BindableProperty"
member_kind: method
---

# BindableProperty.Create

> [!abstract] Method of [[BindableProperty|BindableProperty]]
> Namespace: `Microsoft.Maui.Controls`

Registers a dependency on another BindableProperty. When this property's value is retrieved, if the dependency has a binding that hasn't resolved yet (value is null), return null.

## Signature

```csharp
Microsoft.Maui.Controls.BindableProperty static Create(string propertyName, System.Type returnType, System.Type declaringType, object defaultValue = null, Microsoft.Maui.Controls.BindingMode defaultBindingMode = Microsoft.Maui.Controls.BindingMode.OneWay, Microsoft.Maui.Controls.BindableProperty.ValidateValueDelegate validateValue = null, Microsoft.Maui.Controls.BindableProperty.BindingPropertyChangedDelegate propertyChanged = null, Microsoft.Maui.Controls.BindableProperty.BindingPropertyChangingDelegate propertyChanging = null, Microsoft.Maui.Controls.BindableProperty.CoerceValueDelegate coerceValue = null, Microsoft.Maui.Controls.BindableProperty.CreateDefaultValueDelegate defaultValueCreator = null)
```

## Parameters

| Parameter | Description |
|---|---|
| `propertyName` | The name of the BindableProperty. |
| `returnType` | The type of the property. |
| `declaringType` | The type of the declaring object. |
| `defaultValue` | The default value for the property. |
| `defaultBindingMode` | The BindingMode to use on SetBinding() if no BindingMode is given. This parameter is optional. Default is BindingMode.OneWay. |
| `validateValue` | A delegate to be run when a value is set. This parameter is optional. Default is null. |
| `propertyChanged` | A delegate to be run when the value has changed. This parameter is optional. Default is null. |
| `propertyChanging` | A delegate to be run when the value will change. This parameter is optional. Default is null. |
| `coerceValue` | A delegate used to coerce the range of a value. This parameter is optional. Default is null. |
| `defaultValueCreator` | A Func used to initialize default value for reference types. |

## Returns

A newly created BindableProperty.

## Remarks

When using the `propertyChanged` callback, note that if multiple `BindableProperty` instances share the same `BindingPropertyChangedDelegate`, the callback cannot determine which specific property triggered the change. Consider using separate callback methods for properties that require different handling, or use alternative approaches such as monitoring the `PropertyChanged` event which includes the property name.

## See also

- Declaring type: [[BindableProperty|BindableProperty]]
- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
