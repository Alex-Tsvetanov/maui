---
title: "BlazorWebView (Wpf).CreateFileProvider"
tags:
  - api
  - member/method
  - ns/Microsoft-AspNetCore-Components-WebView-Wpf
aliases:
  - "Microsoft.AspNetCore.Components.WebView.Wpf.BlazorWebView.CreateFileProvider"
declaring_type: "BlazorWebView (Wpf)"
member_kind: method
---

# BlazorWebView (Wpf).CreateFileProvider

> [!abstract] Method of [[BlazorWebView (Wpf)|BlazorWebView (Wpf)]]
> Namespace: `Microsoft.AspNetCore.Components.WebView.Wpf`

Creates a file provider for static assets used in the `BlazorWebView`. The default implementation serves files from disk. Override this method to return a custom `IFileProvider` to serve assets such as wwwroot/index.html . Call the base method and combine its return value with a `CompositeFileProvider` to use both custom assets and default assets.

## Signature

```csharp
Microsoft.Extensions.FileProviders.IFileProvider! virtual CreateFileProvider(string! contentRootDir)
```

## Parameters

| Parameter | Description |
|---|---|
| `contentRootDir` | The base directory to use for all requested assets, such as wwwroot . |

## Returns

Returns a `IFileProvider` for static assets.

## See also

- Declaring type: [[BlazorWebView (Wpf)|BlazorWebView (Wpf)]]
- [[_Microsoft.AspNetCore.Components.WebView.Wpf|Microsoft.AspNetCore.Components.WebView.Wpf namespace]]
