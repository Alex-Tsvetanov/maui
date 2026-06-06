---
title: "ShellNavigatingEventArgs"
tags:
  - api
  - kind/class
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.ShellNavigatingEventArgs"
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

# ShellNavigatingEventArgs

> [!abstract] Class in `Microsoft.Maui.Controls`
> Full name: `Microsoft.Maui.Controls.ShellNavigatingEventArgs`

Provides data for the `Navigating` event.

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
| [[ShellNavigatingEventArgs.ShellNavigatingEventArgs\|ShellNavigatingEventArgs]] | Initializes a new instance of the `ShellNavigatingEventArgs` class. |

## Properties

| Name | Summary |
|---|---|
| [[ShellNavigatingEventArgs.CanCancel\|CanCancel]] | Gets a value indicating whether the navigation can be cancelled. |
| [[ShellNavigatingEventArgs.Cancelled\|Cancelled]] | Gets a value indicating whether the navigation has been cancelled. |
| [[ShellNavigatingEventArgs.Current\|Current]] | Gets the current navigation state. |
| [[ShellNavigatingEventArgs.Source\|Source]] | Gets the source of the navigation. |
| [[ShellNavigatingEventArgs.Target\|Target]] | Gets the target navigation state. |

## Methods

| Name | Summary |
|---|---|
| [[ShellNavigatingEventArgs.Cancel\|Cancel]] | Cancels the navigation if `CanCancel` is `true`. |
| [[ShellNavigatingEventArgs.GetDeferral\|GetDeferral]] | Gets a deferral to delay navigation until the deferral is completed. |

## See also

- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
- [Online API docs](https://learn.microsoft.com/dotnet/api/microsoft.maui.controls.shellnavigatingeventargs)
