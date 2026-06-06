---
title: "MultiTrigger"
tags:
  - api
  - kind/class
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.MultiTrigger"
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

# MultiTrigger

> [!abstract] Class in `Microsoft.Maui.Controls`
> Full name: `Microsoft.Maui.Controls.MultiTrigger`

Class that represents a list of property and binding conditions, and a list of setters that are applied when all of the conditions in the list are met.

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


## Constructors

| Name | Summary |
|---|---|
| [[MultiTrigger.MultiTrigger\|MultiTrigger]] | Initializes a new `MultiTrigger` instance. |

## Properties

| Name | Summary |
|---|---|
| [[MultiTrigger.Conditions\|Conditions]] |  |
| [[MultiTrigger.Setters\|Setters]] |  |

## Remarks

Developers can use a `MultiTrigger` to compare against property values on the control that contains it by using `Trigger` objects, or on any bound property (including those on the enclosing control) by using `BindingCondition` objects. These can be mixed in the same `Conditions` list. `PropertyCondition` `BindingCondition`

## See also

- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
- [Online API docs](https://learn.microsoft.com/dotnet/api/microsoft.maui.controls.multitrigger)
