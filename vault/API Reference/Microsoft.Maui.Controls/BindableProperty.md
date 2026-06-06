---
title: "BindableProperty"
tags:
  - api
  - kind/class
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.BindableProperty"
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

# BindableProperty

> [!abstract] Class in `Microsoft.Maui.Controls`
> Full name: `Microsoft.Maui.Controls.BindableProperty`

A BindableProperty is a backing store for properties allowing bindings on `BindableObject`.

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


## Properties

| Name | Summary |
|---|---|
| [[BindableProperty.DeclaringType\|DeclaringType]] | Gets the type declaring the BindableProperty |
| [[BindableProperty.DefaultBindingMode\|DefaultBindingMode]] | Gets the default BindingMode. |
| [[BindableProperty.DefaultValue\|DefaultValue]] | Gets the default value for the BindableProperty. |
| [[BindableProperty.IsReadOnly\|IsReadOnly]] | Gets a value indicating if the BindableProperty is created form a BindablePropertyKey. |
| [[BindableProperty.PropertyName\|PropertyName]] | Gets the property name. |
| [[BindableProperty.ReturnType\|ReturnType]] | Gets the type of the BindableProperty. |

## Methods

| Name | Summary |
|---|---|
| [[BindableProperty.Create\|Create]] | Registers a dependency on another BindableProperty. When this property's value is retrieved, if the dependency has a binding that hasn't resolved yet (value … |
| [[BindableProperty.CreateAttached\|CreateAttached]] | Creates a new instance of the BindableProperty class for an attached property. |
| [[BindableProperty.CreateAttachedReadOnly\|CreateAttachedReadOnly]] | Creates a new instance of the BindableProperty class for attached read-only properties. |
| [[BindableProperty.CreateReadOnly\|CreateReadOnly]] | Creates a new instance of the BindablePropertyKey class. |

## Fields

| Name | Summary |
|---|---|
| [[BindableProperty.UnsetValue\|UnsetValue]] | A sentinel object used to indicate that a BindableProperty value has not been set. |

## See also

- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
- [Online API docs](https://learn.microsoft.com/dotnet/api/microsoft.maui.controls.bindableproperty)
