---
title: "Behavior"
tags:
  - api
  - kind/class
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.Behavior"
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

# Behavior

> [!abstract] Class in `Microsoft.Maui.Controls`
> Full name: `Microsoft.Maui.Controls.Behavior`

Base class for generalized user-defined behaviors that can respond to arbitrary conditions and events.

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
| [[Behavior.Behavior\|Behavior]] | Creates a new `Behavior` with default values. |

## Properties

| Name | Summary |
|---|---|
| [[Behavior.AssociatedType\|AssociatedType]] | Gets the type of the objects with which this `Behavior` can be associated. |

## Methods

| Name | Summary |
|---|---|
| [[Behavior.OnAttachedTo\|OnAttachedTo]] | Application developers override this method to implement the behaviors that will be associated with `bindable`. |
| [[Behavior.OnDetachingFrom\|OnDetachingFrom]] | Application developers override this method to remove the behaviors from `bindable` that were implemented in a previous call to the `OnAttachedTo` method. |

## Remarks

Application developers should specialize the `Behavior{T}` generic class, instead of directly using `Behavior`.

## See also

- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
- [Online API docs](https://learn.microsoft.com/dotnet/api/microsoft.maui.controls.behavior)
