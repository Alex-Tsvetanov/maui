---
title: "VisualTreeChangeEventArgs"
tags:
  - api
  - kind/class
  - ns/Microsoft-Maui
aliases:
  - "Microsoft.Maui.VisualTreeChangeEventArgs"
namespace: "Microsoft.Maui"
kind: class
platforms:
  - All platforms (.NET)
  - Android
  - iOS
  - Mac Catalyst
  - Windows
  - Tizen
  - .NET Standard
  - .NET Standard 2.0
assemblies:
  - src
---

# VisualTreeChangeEventArgs

> [!abstract] Class in `Microsoft.Maui`
> Full name: `Microsoft.Maui.VisualTreeChangeEventArgs`

Provides data for changes in the visual tree, such as when a child is added or removed.

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
| .NET Standard 2.0 | ✅ |


## Constructors

| Name | Summary |
|---|---|
| [[VisualTreeChangeEventArgs.VisualTreeChangeEventArgs\|VisualTreeChangeEventArgs]] | Initializes a new instance of `VisualTreeChangeEventArgs`. |

## Properties

| Name | Summary |
|---|---|
| [[VisualTreeChangeEventArgs.ChangeType\|ChangeType]] | Gets the type of visual tree change that occurred (Add or Remove). |
| [[VisualTreeChangeEventArgs.Child\|Child]] | Gets the child visual element involved in the change. |
| [[VisualTreeChangeEventArgs.ChildIndex\|ChildIndex]] | Gets the logical index of the child within its parent at the time of change. |
| [[VisualTreeChangeEventArgs.Parent\|Parent]] | Gets the parent visual element involved in the change, or null if this is the root. |

## See also

- [[_Microsoft.Maui|Microsoft.Maui namespace]]
- [Online API docs](https://learn.microsoft.com/dotnet/api/microsoft.maui.visualtreechangeeventargs)
