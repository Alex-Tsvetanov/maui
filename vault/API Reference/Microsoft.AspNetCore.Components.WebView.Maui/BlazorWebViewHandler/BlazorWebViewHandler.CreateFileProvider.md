---
title: "BlazorWebViewHandler.CreateFileProvider"
tags:
  - api
  - member/method
  - ns/Microsoft-AspNetCore-Components-WebView-Maui
aliases:
  - "Microsoft.AspNetCore.Components.WebView.Maui.BlazorWebViewHandler.CreateFileProvider"
declaring_type: "BlazorWebViewHandler"
member_kind: method
---

# BlazorWebViewHandler.CreateFileProvider

> [!abstract] Method of [[BlazorWebViewHandler|BlazorWebViewHandler]]
> Namespace: `Microsoft.AspNetCore.Components.WebView.Maui`

Creates a file provider for static assets used in the `BlazorWebView`. The default implementation serves files from a platform-specific location. Override this method to return a custom `IFileProvider` to serve assets such as wwwroot/index.html . Call the base method and combine its return value with a `CompositeFileProvider` to use both custom assets and default assets.

## Signature

```csharp
Microsoft.Extensions.FileProviders.IFileProvider! virtual CreateFileProvider(string! contentRootDir)
```

## Returns

Returns a `IFileProvider` for static assets.

## Parameters

| Parameter | Description |
|---|---|
| `contentRootDir` | The base directory to use for all requested assets, such as wwwroot . |

## See also

- Declaring type: [[BlazorWebViewHandler|BlazorWebViewHandler]]
- [[_Microsoft.AspNetCore.Components.WebView.Maui|Microsoft.AspNetCore.Components.WebView.Maui namespace]]
