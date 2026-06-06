---
title: "WebViewInitializationStartedEventArgs.EnvironmentOptions"
tags:
  - api
  - member/property
  - ns/Microsoft-Maui
aliases:
  - "Microsoft.Maui.WebViewInitializationStartedEventArgs.EnvironmentOptions"
declaring_type: "WebViewInitializationStartedEventArgs"
member_kind: property
---

# WebViewInitializationStartedEventArgs.EnvironmentOptions

> [!abstract] Property of [[WebViewInitializationStartedEventArgs|WebViewInitializationStartedEventArgs]]
> Namespace: `Microsoft.Maui`

Gets or sets the options used to create WebView2 Environment.

## Signature

```csharp
Microsoft.Web.WebView2.Core.CoreWebView2EnvironmentOptions? EnvironmentOptions { get; set; }
```

## Remarks

As a browser process may be shared among WebViews, WebView creation fails if the specified options does not match the options of the WebViews that are currently running in the shared browser process.

## See also

- Declaring type: [[WebViewInitializationStartedEventArgs|WebViewInitializationStartedEventArgs]]
- [[_Microsoft.Maui|Microsoft.Maui namespace]]
