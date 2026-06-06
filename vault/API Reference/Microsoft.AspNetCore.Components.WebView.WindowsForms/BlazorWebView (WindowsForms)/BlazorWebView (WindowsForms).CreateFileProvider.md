---
title: "BlazorWebView (WindowsForms).CreateFileProvider"
tags:
  - api
  - member/method
  - ns/Microsoft-AspNetCore-Components-WebView-WindowsForms
aliases:
  - "Microsoft.AspNetCore.Components.WebView.WindowsForms.BlazorWebView.CreateFileProvider"
declaring_type: "BlazorWebView (WindowsForms)"
member_kind: method
---

# BlazorWebView (WindowsForms).CreateFileProvider

> [!abstract] Method of [[BlazorWebView (WindowsForms)|BlazorWebView (WindowsForms)]]
> Namespace: `Microsoft.AspNetCore.Components.WebView.WindowsForms`

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

- Declaring type: [[BlazorWebView (WindowsForms)|BlazorWebView (WindowsForms)]]
- [[_Microsoft.AspNetCore.Components.WebView.WindowsForms|Microsoft.AspNetCore.Components.WebView.WindowsForms namespace]]
