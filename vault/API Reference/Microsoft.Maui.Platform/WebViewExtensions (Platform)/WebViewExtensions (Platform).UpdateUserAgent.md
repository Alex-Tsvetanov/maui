---
title: "WebViewExtensions (Platform).UpdateUserAgent"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Platform
aliases:
  - "Microsoft.Maui.Platform.WebViewExtensions.UpdateUserAgent"
declaring_type: "WebViewExtensions (Platform)"
member_kind: method
---

# WebViewExtensions (Platform).UpdateUserAgent

> [!abstract] Method of [[WebViewExtensions (Platform)|WebViewExtensions (Platform)]]
> Namespace: `Microsoft.Maui.Platform`

Updates the user agent string of the native web view to match the specified cross-platform web view.

## Signatures

```csharp
void static UpdateUserAgent(this Android.Webkit.WebView! platformWebView, Microsoft.Maui.IWebView! webView)
void static UpdateUserAgent(this WebKit.WKWebView! platformWebView, Microsoft.Maui.IWebView! webView)
void static UpdateUserAgent(this Microsoft.Maui.Platform.MauiWebView! platformWebView, Microsoft.Maui.IWebView! webView)
void static UpdateUserAgent(this Microsoft.UI.Xaml.Controls.WebView2! platformWebView, Microsoft.Maui.IWebView! webView)
```

## See also

- Declaring type: [[WebViewExtensions (Platform)|WebViewExtensions (Platform)]]
- [[_Microsoft.Maui.Platform|Microsoft.Maui.Platform namespace]]
