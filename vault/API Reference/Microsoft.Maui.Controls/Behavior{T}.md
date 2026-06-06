---
title: "Behavior<T>"
tags:
  - api
  - kind/class
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.Behavior<T>"
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

# Behavior<T>

> [!abstract] Class in `Microsoft.Maui.Controls`
> Full name: `Microsoft.Maui.Controls.Behavior<T>`

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
| [[Behavior{T}.Behavior\|Behavior]] | Creates a new `Behavior` with default values. |

## Methods

| Name | Summary |
|---|---|
| [[Behavior{T}.OnAttachedTo\|OnAttachedTo]] | Application developers override this method to implement the behaviors that will be associated with `bindable`. |
| [[Behavior{T}.OnDetachingFrom\|OnDetachingFrom]] | Application developers override this method to remove the behaviors from `bindable` that were implemented in a previous call to the `OnAttachedTo` method. |

## Remarks

Application developers should specialize the `Behavior{T}` generic class, instead of directly using `Behavior`.

## See also

- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
- [Online API docs](https://learn.microsoft.com/dotnet/api/microsoft.maui.controls.behavior)
