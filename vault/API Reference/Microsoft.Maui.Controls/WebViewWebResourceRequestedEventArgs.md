---
title: "WebViewWebResourceRequestedEventArgs"
tags:
  - api
  - kind/class
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.WebViewWebResourceRequestedEventArgs"
namespace: "Microsoft.Maui.Controls"
kind: class
platforms:
  - All platforms (.NET)
  - Android
  - iOS
  - Mac Catalyst
  - Windows
  - .NET Standard
assemblies:
  - Controls
---

# WebViewWebResourceRequestedEventArgs

> [!abstract] Class in `Microsoft.Maui.Controls`
> Full name: `Microsoft.Maui.Controls.WebViewWebResourceRequestedEventArgs`

Event arguments for the `WebResourceRequested` event.

## Platforms

| Platform | Available |
|---|---|
| All platforms (.NET) | ✅ |
| Android | ✅ |
| iOS | ✅ |
| Mac Catalyst | ✅ |
| Windows | ✅ |
| .NET Standard | ✅ |


## Constructors

| Name | Summary |
|---|---|
| [[WebViewWebResourceRequestedEventArgs.WebViewWebResourceRequestedEventArgs\|WebViewWebResourceRequestedEventArgs]] | Initializes a new instance of the `WebViewWebResourceRequestedEventArgs` class with the specified URI and method. |

## Properties

| Name | Summary |
|---|---|
| [[WebViewWebResourceRequestedEventArgs.Handled\|Handled]] | Gets or sets a value indicating whether the request has been handled. If set to true, the web view will not process the request further and a response must b… |
| [[WebViewWebResourceRequestedEventArgs.Headers\|Headers]] |  |
| [[WebViewWebResourceRequestedEventArgs.Method\|Method]] | Gets the HTTP method used for the request (e.g., GET, POST). |
| [[WebViewWebResourceRequestedEventArgs.PlatformArgs\|PlatformArgs]] | Gets the platform-specific event arguments. |
| [[WebViewWebResourceRequestedEventArgs.QueryParameters\|QueryParameters]] |  |
| [[WebViewWebResourceRequestedEventArgs.Uri\|Uri]] | Gets the URI of the requested resource. |

## Methods

| Name | Summary |
|---|---|
| [[WebViewWebResourceRequestedEventArgs.SetResponse\|SetResponse]] | Sets the response for the web resource request. This method must be called if the `Handled` property is set to true. |

## See also

- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
- [Online API docs](https://learn.microsoft.com/dotnet/api/microsoft.maui.controls.webviewwebresourcerequestedeventargs)
