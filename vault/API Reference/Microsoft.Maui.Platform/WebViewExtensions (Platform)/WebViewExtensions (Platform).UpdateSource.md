---
title: "WebViewExtensions (Platform).UpdateSource"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Platform
aliases:
  - "Microsoft.Maui.Platform.WebViewExtensions.UpdateSource"
declaring_type: "WebViewExtensions (Platform)"
member_kind: method
---

# WebViewExtensions (Platform).UpdateSource

> [!abstract] Method of [[WebViewExtensions (Platform)|WebViewExtensions (Platform)]]
> Namespace: `Microsoft.Maui.Platform`

Updates the source loaded by the native web view to match the specified cross-platform web view.

## Signatures

```csharp
void static UpdateSource(this Android.Webkit.WebView! platformWebView, Microsoft.Maui.IWebView! webView, Microsoft.Maui.IWebViewDelegate? webViewDelegate)
void static UpdateSource(this Android.Webkit.WebView! platformWebView, Microsoft.Maui.IWebView! webView)
void static UpdateSource(this WebKit.WKWebView! platformWebView, Microsoft.Maui.IWebView! webView, Microsoft.Maui.IWebViewDelegate? webViewDelegate)
void static UpdateSource(this WebKit.WKWebView! platformWebView, Microsoft.Maui.IWebView! webView)
void static UpdateSource(this Microsoft.Maui.Platform.MauiWebView! platformWebView, Microsoft.Maui.IWebView! webView, Microsoft.Maui.IWebViewDelegate? webViewDelegate)
void static UpdateSource(this Microsoft.Maui.Platform.MauiWebView! platformWebView, Microsoft.Maui.IWebView! webView)
void static UpdateSource(this Microsoft.UI.Xaml.Controls.WebView2! platformWebView, Microsoft.Maui.IWebView! webView, Microsoft.Maui.IWebViewDelegate? webViewDelegate)
void static UpdateSource(this Microsoft.UI.Xaml.Controls.WebView2! platformWebView, Microsoft.Maui.IWebView! webView)
```

## See also

- Declaring type: [[WebViewExtensions (Platform)|WebViewExtensions (Platform)]]
- [[_Microsoft.Maui.Platform|Microsoft.Maui.Platform namespace]]
