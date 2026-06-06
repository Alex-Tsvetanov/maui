---
title: "HybridWebView"
tags:
  - api
  - kind/class
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.HybridWebView"
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

# HybridWebView

> [!abstract] Class in `Microsoft.Maui.Controls`
> Full name: `Microsoft.Maui.Controls.HybridWebView`

A `View` that presents local HTML content in a web view and allows JavaScript and C# code to communicate by using messages and by invoking methods.

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
| [[HybridWebView.HybridWebView\|HybridWebView]] |  |

## Properties

| Name | Summary |
|---|---|
| [[HybridWebView.DefaultFile\|DefaultFile]] |  |
| [[HybridWebView.HybridRoot\|HybridRoot]] |  |

## Methods

| Name | Summary |
|---|---|
| [[HybridWebView.EvaluateJavaScriptAsync\|EvaluateJavaScriptAsync]] |  |
| [[HybridWebView.InvokeJavaScriptAsync\|InvokeJavaScriptAsync]] | Invokes a JavaScript method named `methodName` and optionally passes in the parameter values specified by `paramValues` by JSON-encoding each one. |
| [[HybridWebView.InvokeJavaScriptAsync{TReturnType}\|InvokeJavaScriptAsync<TReturnType>]] |  |
| [[HybridWebView.SendRawMessage\|SendRawMessage]] | Sends a raw message to the code running in the web view. Raw messages have no additional processing. |
| [[HybridWebView.SetInvokeJavaScriptTarget{T}\|SetInvokeJavaScriptTarget<T>]] |  |

## Events

| Name | Summary |
|---|---|
| [[HybridWebView.RawMessageReceived\|RawMessageReceived]] |  |
| [[HybridWebView.WebResourceRequested\|WebResourceRequested]] |  |
| [[HybridWebView.WebViewInitialized\|WebViewInitialized]] |  |
| [[HybridWebView.WebViewInitializing\|WebViewInitializing]] |  |

## Fields

| Name | Summary |
|---|---|
| [[HybridWebView.DefaultFileProperty\|DefaultFileProperty]] | Bindable property for `DefaultFile`. |
| [[HybridWebView.HybridRootProperty\|HybridRootProperty]] | Bindable property for `HybridRoot`. |

## Guide

- 📖 Conceptual: [[hybridwebview]]

## See also

- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
- [Online API docs](https://learn.microsoft.com/dotnet/api/microsoft.maui.controls.hybridwebview)
