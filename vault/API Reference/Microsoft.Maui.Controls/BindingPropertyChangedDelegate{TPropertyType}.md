---
title: "BindingPropertyChangedDelegate<TPropertyType>"
tags:
  - api
  - kind/class
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.BindableProperty.BindingPropertyChangedDelegate<TPropertyType>"
namespace: "Microsoft.Maui.Controls"
kind: class
platforms:
  - All platforms (.NET)
  - Android
  - iOS
  - Mac Catalyst
  - Windows
  - Tizen
  - .NET Standard
assemblies:
  - Controls
---

# BindingPropertyChangedDelegate<TPropertyType>

> [!abstract] Class in `Microsoft.Maui.Controls`
> Full name: `Microsoft.Maui.Controls.BindableProperty.BindingPropertyChangedDelegate<TPropertyType>`

Represents a delegate that is called when a bindable property value has changed.

## Platforms

| Platform | Available |
|---|---|
| All platforms (.NET) | ✅ |
| Android | ✅ |
| iOS | ✅ |
| Mac Catalyst | ✅ |
| Windows | ✅ |
| Tizen | ✅ |
| .NET Standard | ✅ |


## Methods

| Name | Summary |
|---|---|
| [[BindingPropertyChangedDelegate{TPropertyType}.Invoke\|Invoke]] |  |

## Remarks

This delegate does not provide information about which specific `BindableProperty` triggered the change. If multiple properties share the same callback and need to be distinguished, consider using separate callbacks or the `PropertyChanged` event.

## Parameters

| Parameter | Description |
|---|---|
| `bindable` | The `BindableObject` instance that owns the property. |
| `oldValue` | The previous value of the property. |
| `newValue` | The new value of the property. |

## See also

- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
- [Online API docs](https://learn.microsoft.com/dotnet/api/microsoft.maui.controls.bindableproperty.bindingpropertychangeddelegate)
