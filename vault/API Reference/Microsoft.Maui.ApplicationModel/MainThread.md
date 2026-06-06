---
title: "MainThread"
tags:
  - api
  - kind/class
  - ns/Microsoft-Maui-ApplicationModel
aliases:
  - "Microsoft.Maui.ApplicationModel.MainThread"
namespace: "Microsoft.Maui.ApplicationModel"
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
  - src
---

# MainThread

> [!abstract] Class in `Microsoft.Maui.ApplicationModel`
> Full name: `Microsoft.Maui.ApplicationModel.MainThread`

The MainThread class allows applications to run code on the main thread of execution, and to determine if a particular block of code is currently running on the main thread.

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
| [[MainThread.IsMainThread\|IsMainThread]] | True if the current thread is the UI thread. |

## Methods

| Name | Summary |
|---|---|
| [[MainThread.BeginInvokeOnMainThread\|BeginInvokeOnMainThread]] | Invokes an action on the main thread of the application. |
| [[MainThread.GetMainThreadSynchronizationContextAsync\|GetMainThreadSynchronizationContextAsync]] |  |
| [[MainThread.InvokeOnMainThreadAsync\|InvokeOnMainThreadAsync]] | Invokes an action on the main thread of the application asynchronously. |
| [[MainThread.InvokeOnMainThreadAsync{T}\|InvokeOnMainThreadAsync<T>]] |  |

## Guide

- 📖 Conceptual: [[main-thread]]

## See also

- [[_Microsoft.Maui.ApplicationModel|Microsoft.Maui.ApplicationModel namespace]]
- [Online API docs](https://learn.microsoft.com/dotnet/api/microsoft.maui.applicationmodel.mainthread)
