---
title: "FilePicker"
tags:
  - api
  - kind/class
  - ns/Microsoft-Maui-Storage
aliases:
  - "Microsoft.Maui.Storage.FilePicker"
namespace: "Microsoft.Maui.Storage"
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

# FilePicker

> [!abstract] Class in `Microsoft.Maui.Storage`
> Full name: `Microsoft.Maui.Storage.FilePicker`

Opens the default file picker to allow the user to pick a single file.

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
| [[FilePicker.Default\|Default]] | Provides the default implementation for static usage of this API. |

## Methods

| Name | Summary |
|---|---|
| [[FilePicker.PickAsync\|PickAsync]] |  |
| [[FilePicker.PickMultipleAsync\|PickMultipleAsync]] |  |

## Remarks

File types can be specified in order to limit files that can be selected, using a `PickOptions` object. Note that this method may re-throw platform specific exceptions that occurred during file picking. When calling `PickAsync` again while showing a file picker, the `Task` object that was returned from the first call is cancelled. Be sure to also handle the `TaskCanceledException` in this case.

## Guide

- 📖 Conceptual: [[file-picker]]

## See also

- [[_Microsoft.Maui.Storage|Microsoft.Maui.Storage namespace]]
- [Online API docs](https://learn.microsoft.com/dotnet/api/microsoft.maui.storage.filepicker)
