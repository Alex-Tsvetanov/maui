---
title: "BlazorWebView (Wpf)"
tags:
  - api
  - kind/class
  - ns/Microsoft-AspNetCore-Components-WebView-Wpf
aliases:
  - "Microsoft.AspNetCore.Components.WebView.Wpf.BlazorWebView"
namespace: "Microsoft.AspNetCore.Components.WebView.Wpf"
kind: class
platforms:
  - All platforms (.NET)
assemblies:
  - BlazorWebView
---

# BlazorWebView (Wpf)

> [!abstract] Class in `Microsoft.AspNetCore.Components.WebView.Wpf`
> Full name: `Microsoft.AspNetCore.Components.WebView.Wpf.BlazorWebView`

A Windows Presentation Foundation (WPF) control for hosting Razor components locally in Windows desktop applications.

## Platforms

| Platform | Available |
|---|---|
| All platforms (.NET) | ✅ |


## Constructors

| Name | Summary |
|---|---|
| [[BlazorWebView (Wpf).BlazorWebView\|BlazorWebView]] | Creates a new instance of `BlazorWebView`. |

## Properties

| Name | Summary |
|---|---|
| [[BlazorWebView (Wpf).BlazorWebViewInitialized\|BlazorWebViewInitialized]] |  |
| [[BlazorWebView (Wpf).BlazorWebViewInitializing\|BlazorWebViewInitializing]] |  |
| [[BlazorWebView (Wpf).HostPage\|HostPage]] |  |
| [[BlazorWebView (Wpf).RootComponents\|RootComponents]] | Path to the host page within the application's static files. For example, wwwroot\index.html . This property must be set to a valid value for the Razor compo… |
| [[BlazorWebView (Wpf).Services\|Services]] |  |
| [[BlazorWebView (Wpf).StartPath\|StartPath]] |  |
| [[BlazorWebView (Wpf).UrlLoading\|UrlLoading]] |  |
| [[BlazorWebView (Wpf).WebView\|WebView]] | Returns the inner `WebView2Control` used by this control. |

## Methods

| Name | Summary |
|---|---|
| [[BlazorWebView (Wpf).CreateFileProvider\|CreateFileProvider]] | Creates a file provider for static assets used in the `BlazorWebView`. The default implementation serves files from disk. Override this method to return a cu… |
| [[BlazorWebView (Wpf).DisposeAsync\|DisposeAsync]] |  |
| [[BlazorWebView (Wpf).DisposeAsyncCore\|DisposeAsyncCore]] | Allows asynchronous disposal of the `BlazorWebView`. |
| [[BlazorWebView (Wpf).OnApplyTemplate\|OnApplyTemplate]] | Gets or sets an `IServiceProvider` containing services to be used by this control and also by application code. This property must be set to a valid value fo… |
| [[BlazorWebView (Wpf).OnInitialized\|OnInitialized]] |  |
| [[BlazorWebView (Wpf).TryDispatchAsync\|TryDispatchAsync]] |  |

## Fields

| Name | Summary |
|---|---|
| [[BlazorWebView (Wpf).BlazorWebViewInitializedProperty\|BlazorWebViewInitializedProperty]] | The backing store for the `BlazorWebViewInitialized` event. |
| [[BlazorWebView (Wpf).BlazorWebViewInitializingProperty\|BlazorWebViewInitializingProperty]] | The backing store for the `BlazorWebViewInitializing` event. |
| [[BlazorWebView (Wpf).HostPageProperty\|HostPageProperty]] | The backing store for the `HostPage` property. |
| [[BlazorWebView (Wpf).RootComponentsProperty\|RootComponentsProperty]] | The backing store for the `RootComponent` property. |
| [[BlazorWebView (Wpf).ServicesProperty\|ServicesProperty]] | The backing store for the `Services` property. |
| [[BlazorWebView (Wpf).StartPathProperty\|StartPathProperty]] | The backing store for the `StartPath` property. |
| [[BlazorWebView (Wpf).UrlLoadingProperty\|UrlLoadingProperty]] | The backing store for the `UrlLoading` property. |

## Guide

- 📖 Conceptual: [[blazorwebview]]

## See also

- [[_Microsoft.AspNetCore.Components.WebView.Wpf|Microsoft.AspNetCore.Components.WebView.Wpf namespace]]
- [Online API docs](https://learn.microsoft.com/dotnet/api/microsoft.aspnetcore.components.webview.wpf.blazorwebview)
