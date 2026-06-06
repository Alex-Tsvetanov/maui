---
title: "PlatformWebViewInitializingEventArgs.EnvironmentOptions"
tags:
  - api
  - member/property
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.PlatformWebViewInitializingEventArgs.EnvironmentOptions"
declaring_type: "PlatformWebViewInitializingEventArgs"
member_kind: property
---

# PlatformWebViewInitializingEventArgs.EnvironmentOptions

> [!abstract] Property of [[PlatformWebViewInitializingEventArgs|PlatformWebViewInitializingEventArgs]]
> Namespace: `Microsoft.Maui.Controls`

Gets or sets the options used to create WebView2 Environment.

## Signature

```csharp
Microsoft.Web.WebView2.Core.CoreWebView2EnvironmentOptions? EnvironmentOptions { get; set; }
```

## Remarks

As a browser process may be shared among WebViews, WebView creation fails if the specified options does not match the options of the WebViews that are currently running in the shared browser process.

## See also

- Declaring type: [[PlatformWebViewInitializingEventArgs|PlatformWebViewInitializingEventArgs]]
- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
