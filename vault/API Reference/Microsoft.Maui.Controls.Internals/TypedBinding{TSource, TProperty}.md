---
title: "TypedBinding<TSource, TProperty>"
tags:
  - api
  - kind/class
  - ns/Microsoft-Maui-Controls-Internals
aliases:
  - "Microsoft.Maui.Controls.Internals.TypedBinding<TSource, TProperty>"
namespace: "Microsoft.Maui.Controls.Internals"
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

# TypedBinding<TSource, TProperty>

> [!abstract] Class in `Microsoft.Maui.Controls.Internals`
> Full name: `Microsoft.Maui.Controls.Internals.TypedBinding<TSource, TProperty>`

This factory method was added to simplify creating typed bindings for a property that isn't nested which is the most common scenario. This factory method must be used carefully. As the name implies, it is only applicable when the getter and setter access a property directly on the source object. Whenever the property is nested two or more levels deep, create the binding manually and construct the handlers array for that usecase.

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
| [[TypedBinding{TSource, TProperty}.TypedBinding\|TypedBinding]] |  |

## See also

- [[_Microsoft.Maui.Controls.Internals|Microsoft.Maui.Controls.Internals namespace]]
- [Online API docs](https://learn.microsoft.com/dotnet/api/microsoft.maui.controls.internals.typedbinding)
