---
title: "MarshalingObservableCollection"
tags:
  - api
  - kind/class
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.MarshalingObservableCollection"
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

# MarshalingObservableCollection

> [!abstract] Class in `Microsoft.Maui.Controls`
> Full name: `Microsoft.Maui.Controls.MarshalingObservableCollection`

A thread-safe observable collection that marshals all collection changes to the main thread.

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
| [[MarshalingObservableCollection.MarshalingObservableCollection\|MarshalingObservableCollection]] |  |

## Events

| Name | Summary |
|---|---|
| [[MarshalingObservableCollection.CollectionChanged\|CollectionChanged]] |  |

## Remarks

This collection wraps an `INotifyCollectionChanged` collection and ensures that all collection change notifications are processed on the main UI thread, making it safe to bind to UI controls even when the underlying collection is modified from background threads.

## See also

- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
- [Online API docs](https://learn.microsoft.com/dotnet/api/microsoft.maui.controls.marshalingobservablecollection)
