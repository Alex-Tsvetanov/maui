---
title: "CreateDefaultValueDelegate<TDeclarer, TPropertyType>"
tags:
  - api
  - kind/class
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.BindableProperty.CreateDefaultValueDelegate<TDeclarer, TPropertyType>"
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

# CreateDefaultValueDelegate<TDeclarer, TPropertyType>

> [!abstract] Class in `Microsoft.Maui.Controls`
> Full name: `Microsoft.Maui.Controls.BindableProperty.CreateDefaultValueDelegate<TDeclarer, TPropertyType>`

Represents a delegate that creates a default value for a bindable property.

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
| [[CreateDefaultValueDelegate{TDeclarer, TPropertyType}.Invoke\|Invoke]] |  |

## Remarks

This delegate is useful for creating unique default instances for reference types, avoiding shared references between different bindable object instances.

## Returns

The default value for the property.

## Parameters

| Parameter | Description |
|---|---|
| `bindable` | The `BindableObject` instance that owns the property. |

## See also

- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
- [Online API docs](https://learn.microsoft.com/dotnet/api/microsoft.maui.controls.bindableproperty.createdefaultvaluedelegate)
