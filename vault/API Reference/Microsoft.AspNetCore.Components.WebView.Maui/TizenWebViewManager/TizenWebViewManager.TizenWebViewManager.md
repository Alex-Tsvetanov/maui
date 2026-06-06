---
title: "TizenWebViewManager.TizenWebViewManager"
tags:
  - api
  - member/constructor
  - ns/Microsoft-AspNetCore-Components-WebView-Maui
aliases:
  - "Microsoft.AspNetCore.Components.WebView.Maui.TizenWebViewManager.TizenWebViewManager"
declaring_type: "TizenWebViewManager"
member_kind: constructor
---

# TizenWebViewManager.TizenWebViewManager

> [!abstract] Constructor of [[TizenWebViewManager|TizenWebViewManager]]
> Namespace: `Microsoft.AspNetCore.Components.WebView.Maui`

Initializes a new instance of `TizenWebViewManager`

## Signature

```csharp
void TizenWebViewManager(Microsoft.AspNetCore.Components.WebView.Maui.BlazorWebViewHandler! blazorMauiWebViewHandler, Tizen.NUI.BaseComponents.WebView! webview, System.IServiceProvider! provider, Microsoft.AspNetCore.Components.Dispatcher! dispatcher, Microsoft.Extensions.FileProviders.IFileProvider! fileProvider, Microsoft.AspNetCore.Components.Web.JSComponentConfigurationStore! jsComponents, string! contentRootRelativeToAppRoot, string! hostPageRelativePath)
```

## Parameters

| Parameter | Description |
|---|---|
| `blazorMauiWebViewHandler` | The `BlazorWebViewHandler`. |
| `webview` | A wrapper to access platform-specific WebView APIs. |
| `provider` | The `IServiceProvider` for the application. |
| `dispatcher` | A `Dispatcher` instance instance that can marshal calls to the required thread or sync context. |
| `fileProvider` | Provides static content to the webview. |
| `jsComponents` | Describes configuration for adding, removing, and updating root components from JavaScript code. |
| `contentRootRelativeToAppRoot` | Path to the directory containing application content files. |
| `hostPageRelativePath` | Path to the host page within the fileProvider. |

## See also

- Declaring type: [[TizenWebViewManager|TizenWebViewManager]]
- [[_Microsoft.AspNetCore.Components.WebView.Maui|Microsoft.AspNetCore.Components.WebView.Maui namespace]]
