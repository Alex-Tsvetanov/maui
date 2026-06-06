---
title: "BindingPropertyChangingDelegate"
tags:
  - api
  - kind/class
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.BindableProperty.BindingPropertyChangingDelegate"
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

# BindingPropertyChangingDelegate

> [!abstract] Class in `Microsoft.Maui.Controls`
> Full name: `Microsoft.Maui.Controls.BindableProperty.BindingPropertyChangingDelegate`

Represents a delegate that is called when a bindable property value is about to change.

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
| [[BindingPropertyChangingDelegate.Invoke\|Invoke]] |  |

## Remarks

This delegate is invoked before a property value is changed on a `BindableObject`. Like `BindingPropertyChangedDelegate`, this delegate does not include information about which specific `BindableProperty` is changing when multiple properties share the same callback.

## Parameters

| Parameter | Description |
|---|---|
| `bindable` | The `BindableObject` instance that owns the property. |
| `oldValue` | The current value of the property before the change. |
| `newValue` | The new value that the property will be set to. |

## See also

- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
- [Online API docs](https://learn.microsoft.com/dotnet/api/microsoft.maui.controls.bindableproperty.bindingpropertychangingdelegate)
