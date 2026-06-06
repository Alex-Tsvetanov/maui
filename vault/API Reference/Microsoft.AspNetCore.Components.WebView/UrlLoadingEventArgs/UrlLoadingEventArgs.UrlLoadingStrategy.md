---
title: "UrlLoadingEventArgs.UrlLoadingStrategy"
tags:
  - api
  - member/property
  - ns/Microsoft-AspNetCore-Components-WebView
aliases:
  - "Microsoft.AspNetCore.Components.WebView.UrlLoadingEventArgs.UrlLoadingStrategy"
declaring_type: "UrlLoadingEventArgs"
member_kind: property
---

# UrlLoadingEventArgs.UrlLoadingStrategy

> [!abstract] Property of [[UrlLoadingEventArgs|UrlLoadingEventArgs]]
> Namespace: `Microsoft.AspNetCore.Components.WebView`

The policy to use when loading links from the webview. Defaults to `OpenExternally` unless `Url` has a host matching the app origin, in which case the default becomes `OpenInWebView`. This value should not be changed to `OpenInWebView` for external links unless you can ensure they are fully trusted.

## Signature

```csharp
Microsoft.AspNetCore.Components.WebView.UrlLoadingStrategy UrlLoadingStrategy { get; set; }
```

## See also

- Declaring type: [[UrlLoadingEventArgs|UrlLoadingEventArgs]]
- [[_Microsoft.AspNetCore.Components.WebView|Microsoft.AspNetCore.Components.WebView namespace]]
