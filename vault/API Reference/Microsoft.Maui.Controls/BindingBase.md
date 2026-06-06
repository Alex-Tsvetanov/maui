---
title: "BindingBase"
tags:
  - api
  - kind/class
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.BindingBase"
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

# BindingBase

> [!abstract] Class in `Microsoft.Maui.Controls`
> Full name: `Microsoft.Maui.Controls.BindingBase`

An abstract base class for all bindings providing `BindingMode` selection, fallback/target null values, and formatting support.

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
| [[BindingBase.FallbackValue\|FallbackValue]] |  |
| [[BindingBase.Mode\|Mode]] |  |
| [[BindingBase.StringFormat\|StringFormat]] |  |
| [[BindingBase.TargetNullValue\|TargetNullValue]] |  |

## Methods

| Name | Summary |
|---|---|
| [[BindingBase.Create{TSource, TProperty}\|Create<TSource, TProperty>]] |  |
| [[BindingBase.DisableCollectionSynchronization\|DisableCollectionSynchronization]] | Gets or sets the mode for this binding. |
| [[BindingBase.EnableCollectionSynchronization\|EnableCollectionSynchronization]] | Enables synchronized (thread-safe) access to `collection` using the supplied callback. |

## Remarks

This class underlies concrete binding implementations (e.g., `Binding`, `MultiBinding`) and supplies common features such as binding mode control, string formatting and thread-safe collection synchronization helpers.

## See also

- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
- [Online API docs](https://learn.microsoft.com/dotnet/api/microsoft.maui.controls.bindingbase)
