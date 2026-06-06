---
title: "Clipboard"
tags:
  - api
  - kind/class
  - ns/Microsoft-Maui-ApplicationModel-DataTransfer
aliases:
  - "Microsoft.Maui.ApplicationModel.DataTransfer.Clipboard"
namespace: "Microsoft.Maui.ApplicationModel.DataTransfer"
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

# Clipboard

> [!abstract] Class in `Microsoft.Maui.ApplicationModel.DataTransfer`
> Full name: `Microsoft.Maui.ApplicationModel.DataTransfer.Clipboard`

Gets a value indicating whether there is any text on the clipboard.

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
| [[Clipboard.Default\|Default]] | Provides the default implementation for static usage of this API. |
| [[Clipboard.HasText\|HasText]] |  |

## Methods

| Name | Summary |
|---|---|
| [[Clipboard.GetTextAsync\|GetTextAsync]] |  |
| [[Clipboard.SetTextAsync\|SetTextAsync]] | Sets the contents of the clipboard to be the specified text. |

## Events

| Name | Summary |
|---|---|
| [[Clipboard.ClipboardContentChanged\|ClipboardContentChanged]] |  |

## Remarks

This method returns immediately and does not guarentee that the text is on the clipboard by the time this method returns.

## Guide

- 📖 Conceptual: [[clipboard]]

## See also

- [[_Microsoft.Maui.ApplicationModel.DataTransfer|Microsoft.Maui.ApplicationModel.DataTransfer namespace]]
- [Online API docs](https://learn.microsoft.com/dotnet/api/microsoft.maui.applicationmodel.datatransfer.clipboard)
