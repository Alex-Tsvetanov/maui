---
title: "WebViewInitializationStartedEventArgs.ScriptLocale"
tags:
  - api
  - member/property
  - ns/Microsoft-Maui
aliases:
  - "Microsoft.Maui.WebViewInitializationStartedEventArgs.ScriptLocale"
declaring_type: "WebViewInitializationStartedEventArgs"
member_kind: property
---

# WebViewInitializationStartedEventArgs.ScriptLocale

> [!abstract] Property of [[WebViewInitializationStartedEventArgs|WebViewInitializationStartedEventArgs]]
> Namespace: `Microsoft.Maui`

Gets or sets the controller's default script locale.

## Signature

```csharp
string? ScriptLocale { get; set; }
```

## Remarks

This property sets the default locale for all Intl JavaScript APIs and other JavaScript APIs that depend on it, namely Intl.DateTimeFormat() which affects string formatting like in the time/date formats. The intended locale value is in the format of BCP 47 Language Tags. More information can be found from https://www.ietf.org/rfc/bcp/bcp47.html. The default value for ScriptLocale will be depend on the WebView2 language and OS region. If the language portions of the WebView2 language and OS region match, then it will use the OS region. Otherwise, it will use the WebView2 language.

## See also

- Declaring type: [[WebViewInitializationStartedEventArgs|WebViewInitializationStartedEventArgs]]
- [[_Microsoft.Maui|Microsoft.Maui namespace]]
