---
title: "UrlLoadingEventArgs"
tags:
  - api
  - kind/class
  - ns/Microsoft-AspNetCore-Components-WebView
aliases:
  - "Microsoft.AspNetCore.Components.WebView.UrlLoadingEventArgs"
namespace: "Microsoft.AspNetCore.Components.WebView"
kind: class
platforms:
  - All platforms (.NET)
  - Android
  - iOS
  - Mac Catalyst
  - Windows
  - Tizen
assemblies:
  - BlazorWebView
---

# UrlLoadingEventArgs

> [!abstract] Class in `Microsoft.AspNetCore.Components.WebView`
> Full name: `Microsoft.AspNetCore.Components.WebView.UrlLoadingEventArgs`

Used to provide information about a link (]]>) clicked within a Blazor WebView. Anchor tags with target="_blank" will always open in the default browser and the UrlLoading event won't be called.

## Platforms

| Platform | Available |
|---|---|
| All platforms (.NET) | ✅ |
| Android | ✅ |
| iOS | ✅ |
| Mac Catalyst | ✅ |
| Windows | ✅ |
| Tizen | ✅ |


## Properties

| Name | Summary |
|---|---|
| [[UrlLoadingEventArgs.Url\|Url]] | Gets the `Url`URL to be loaded. |
| [[UrlLoadingEventArgs.UrlLoadingStrategy\|UrlLoadingStrategy]] | The policy to use when loading links from the webview. Defaults to `OpenExternally` unless `Url` has a host matching the app origin, in which case the defaul… |

## See also

- [[_Microsoft.AspNetCore.Components.WebView|Microsoft.AspNetCore.Components.WebView namespace]]
- [Online API docs](https://learn.microsoft.com/dotnet/api/microsoft.aspnetcore.components.webview.urlloadingeventargs)
