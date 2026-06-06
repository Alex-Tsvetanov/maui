---
title: "VisualDiagnostics"
tags:
  - api
  - kind/class
  - ns/Microsoft-Maui
aliases:
  - "Microsoft.Maui.VisualDiagnostics"
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

# VisualDiagnostics

> [!abstract] Class in `Microsoft.Maui`
> Full name: `Microsoft.Maui.VisualDiagnostics`

Provides APIs for capturing source information, monitoring visual tree changes, and capturing screenshots for XAML and UI diagnostics.

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


## Methods

| Name | Summary |
|---|---|
| [[VisualDiagnostics.CaptureAsJpegAsync\|CaptureAsJpegAsync]] |  |
| [[VisualDiagnostics.CaptureAsPngAsync\|CaptureAsPngAsync]] |  |
| [[VisualDiagnostics.GetSourceInfo\|GetSourceInfo]] | Gets the previously registered source information for a specified object. |
| [[VisualDiagnostics.OnChildAdded\|OnChildAdded]] | Called when a child element is added to the visual tree; raises `VisualTreeChanged` event. |
| [[VisualDiagnostics.OnChildRemoved\|OnChildRemoved]] | Called when a child element is removed from the visual tree; raises `VisualTreeChanged` event. |
| [[VisualDiagnostics.RegisterSourceInfo\|RegisterSourceInfo]] | Registers source file information (URI, line number, and position) for the specified target object when XAML diagnostics are enabled. |

## Events

| Name | Summary |
|---|---|
| [[VisualDiagnostics.VisualTreeChanged\|VisualTreeChanged]] |  |

## See also

- [[_Microsoft.Maui|Microsoft.Maui namespace]]
- [Online API docs](https://learn.microsoft.com/dotnet/api/microsoft.maui.visualdiagnostics)
