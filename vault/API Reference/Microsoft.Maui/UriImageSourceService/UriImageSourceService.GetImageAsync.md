---
title: "UriImageSourceService.GetImageAsync"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui
aliases:
  - "Microsoft.Maui.UriImageSourceService.GetImageAsync"
declaring_type: "UriImageSourceService"
member_kind: method
---

# UriImageSourceService.GetImageAsync

> [!abstract] Method of [[UriImageSourceService|UriImageSourceService]]
> Namespace: `Microsoft.Maui`

Asynchronously loads a native image from the specified URI image source at the given scale.

## Signatures

```csharp
System.Threading.Tasks.Task<Microsoft.Maui.IImageSourceServiceResult<UIKit.UIImage!>?>! GetImageAsync(Microsoft.Maui.IUriImageSource! imageSource, float scale = 1, System.Threading.CancellationToken cancellationToken = default(System.Threading.CancellationToken))
System.Threading.Tasks.Task<Microsoft.Maui.IImageSourceServiceResult<UIKit.UIImage!>?>! override GetImageAsync(Microsoft.Maui.IImageSource! imageSource, float scale = 1, System.Threading.CancellationToken cancellationToken = default(System.Threading.CancellationToken))
System.Threading.Tasks.Task<Microsoft.Maui.IImageSourceServiceResult<Microsoft.Maui.Platform.MauiImageSource!>?>! GetImageAsync(Microsoft.Maui.IUriImageSource! imageSource, System.Threading.CancellationToken cancellationToken = default(System.Threading.CancellationToken))
System.Threading.Tasks.Task<Microsoft.Maui.IImageSourceServiceResult<Microsoft.Maui.Platform.MauiImageSource!>?>! override GetImageAsync(Microsoft.Maui.IImageSource! imageSource, System.Threading.CancellationToken cancellationToken = default(System.Threading.CancellationToken))
```

## See also

- Declaring type: [[UriImageSourceService|UriImageSourceService]]
- [[_Microsoft.Maui|Microsoft.Maui namespace]]
