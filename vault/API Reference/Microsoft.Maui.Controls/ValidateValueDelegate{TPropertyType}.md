---
title: "ValidateValueDelegate<TPropertyType>"
tags:
  - api
  - kind/class
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.BindableProperty.ValidateValueDelegate<TPropertyType>"
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

# ValidateValueDelegate<TPropertyType>

> [!abstract] Class in `Microsoft.Maui.Controls`
> Full name: `Microsoft.Maui.Controls.BindableProperty.ValidateValueDelegate<TPropertyType>`

Represents a delegate that validates whether a value is acceptable for a bindable property.

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
| [[ValidateValueDelegate{TPropertyType}.Invoke\|Invoke]] |  |

## Remarks

If this delegate returns `false`, an `ArgumentException` will be thrown when attempting to set the property to the invalid value.

## Returns

`true` if the value is valid; otherwise, `false`.

## Parameters

| Parameter | Description |
|---|---|
| `bindable` | The `BindableObject` instance that owns the property. |
| `value` | The value to validate. |

## See also

- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
- [Online API docs](https://learn.microsoft.com/dotnet/api/microsoft.maui.controls.bindableproperty.validatevaluedelegate)
