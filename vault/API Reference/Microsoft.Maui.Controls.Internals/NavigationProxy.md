---
title: "NavigationProxy"
tags:
  - api
  - kind/class
  - ns/Microsoft-Maui-Controls-Internals
aliases:
  - "Microsoft.Maui.Controls.Internals.NavigationProxy"
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

# NavigationProxy

> [!abstract] Class in `Microsoft.Maui.Controls.Internals`
> Full name: `Microsoft.Maui.Controls.Internals.NavigationProxy`

Represents an object capable of handling stack-based navigation via proxying.

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
| [[NavigationProxy.NavigationProxy\|NavigationProxy]] |  |

## Properties

| Name | Summary |
|---|---|
| [[NavigationProxy.Inner\|Inner]] |  |
| [[NavigationProxy.ModalStack\|ModalStack]] |  |
| [[NavigationProxy.NavigationStack\|NavigationStack]] |  |

## Methods

| Name | Summary |
|---|---|
| [[NavigationProxy.GetModalStack\|GetModalStack]] |  |
| [[NavigationProxy.GetNavigationStack\|GetNavigationStack]] |  |
| [[NavigationProxy.InsertPageBefore\|InsertPageBefore]] | Internal API for Microsoft.Maui.Controls platform use. |
| [[NavigationProxy.OnInsertPageBefore\|OnInsertPageBefore]] |  |
| [[NavigationProxy.OnPopAsync\|OnPopAsync]] |  |
| [[NavigationProxy.OnPopModal\|OnPopModal]] |  |
| [[NavigationProxy.OnPopToRootAsync\|OnPopToRootAsync]] |  |
| [[NavigationProxy.OnPushAsync\|OnPushAsync]] |  |
| [[NavigationProxy.OnPushModal\|OnPushModal]] |  |
| [[NavigationProxy.OnRemovePage\|OnRemovePage]] |  |
| [[NavigationProxy.PopAsync\|PopAsync]] |  |
| [[NavigationProxy.PopModalAsync\|PopModalAsync]] |  |
| [[NavigationProxy.PopToRootAsync\|PopToRootAsync]] | Pops all pages except the root page with animation. |
| [[NavigationProxy.PushAsync\|PushAsync]] | Pushes a page onto the navigation stack with animation. |
| [[NavigationProxy.PushModalAsync\|PushModalAsync]] | Presents a page modally with animation. |
| [[NavigationProxy.RemovePage\|RemovePage]] | Internal API for Microsoft.Maui.Controls platform use. |

## Remarks

Elements may use navigation proxies to delegate navigation capabilities to their parents if they themselves can't handle it. For internal use for .NET MAUI.

## See also

- [[_Microsoft.Maui.Controls.Internals|Microsoft.Maui.Controls.Internals namespace]]
- [Online API docs](https://learn.microsoft.com/dotnet/api/microsoft.maui.controls.internals.navigationproxy)
