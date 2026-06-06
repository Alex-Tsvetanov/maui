---
title: "BlazorWebView (WindowsForms)"
tags:
  - api
  - kind/class
  - ns/Microsoft-AspNetCore-Components-WebView-WindowsForms
aliases:
  - "Microsoft.AspNetCore.Components.WebView.WindowsForms.BlazorWebView"
namespace: "Microsoft.AspNetCore.Components.WebView.WindowsForms"
kind: class
platforms:
  - All platforms (.NET)
assemblies:
  - BlazorWebView
---

# BlazorWebView (WindowsForms)

> [!abstract] Class in `Microsoft.AspNetCore.Components.WebView.WindowsForms`
> Full name: `Microsoft.AspNetCore.Components.WebView.WindowsForms.BlazorWebView`

A Windows Forms control for hosting Razor components locally in Windows desktop applications.

## Platforms

| Platform | Available |
|---|---|
| All platforms (.NET) | ✅ |


## Constructors

| Name | Summary |
|---|---|
| [[BlazorWebView (WindowsForms).BlazorWebView\|BlazorWebView]] | Creates a new instance of `BlazorWebView`. |

## Properties

| Name | Summary |
|---|---|
| [[BlazorWebView (WindowsForms).HostPage\|HostPage]] |  |
| [[BlazorWebView (WindowsForms).RootComponents\|RootComponents]] | A collection of `RootComponent` instances that specify the Blazor `IComponent` types to be used directly in the specified `HostPage`. |
| [[BlazorWebView (WindowsForms).Services\|Services]] |  |
| [[BlazorWebView (WindowsForms).StartPath\|StartPath]] | Path to the host page within the application's static files. For example, wwwroot\index.html . This property must be set to a valid value for the Razor compo… |
| [[BlazorWebView (WindowsForms).WebView\|WebView]] | Returns the inner `WebView2Control` used by this control. |

## Methods

| Name | Summary |
|---|---|
| [[BlazorWebView (WindowsForms).CreateControlsInstance\|CreateControlsInstance]] |  |
| [[BlazorWebView (WindowsForms).CreateFileProvider\|CreateFileProvider]] | Creates a file provider for static assets used in the `BlazorWebView`. The default implementation serves files from disk. Override this method to return a cu… |
| [[BlazorWebView (WindowsForms).Dispose\|Dispose]] |  |
| [[BlazorWebView (WindowsForms).OnCreateControl\|OnCreateControl]] |  |
| [[BlazorWebView (WindowsForms).TryDispatchAsync\|TryDispatchAsync]] |  |

## Events

| Name | Summary |
|---|---|
| [[BlazorWebView (WindowsForms).BlazorWebViewInitialized\|BlazorWebViewInitialized]] |  |
| [[BlazorWebView (WindowsForms).BlazorWebViewInitializing\|BlazorWebViewInitializing]] |  |
| [[BlazorWebView (WindowsForms).UrlLoading\|UrlLoading]] |  |

## Guide

- 📖 Conceptual: [[blazorwebview]]

## See also

- [[_Microsoft.AspNetCore.Components.WebView.WindowsForms|Microsoft.AspNetCore.Components.WebView.WindowsForms namespace]]
- [Online API docs](https://learn.microsoft.com/dotnet/api/microsoft.aspnetcore.components.webview.windowsforms.blazorwebview)
