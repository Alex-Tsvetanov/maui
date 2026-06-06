---
title: "StreamImageSourceService.GetImageAsync"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui
aliases:
  - "Microsoft.Maui.StreamImageSourceService.GetImageAsync"
declaring_type: "StreamImageSourceService"
member_kind: method
---

# StreamImageSourceService.GetImageAsync

> [!abstract] Method of [[StreamImageSourceService|StreamImageSourceService]]
> Namespace: `Microsoft.Maui`

Asynchronously loads the stream image source as a native platform image.

## Signatures

```csharp
System.Threading.Tasks.Task<Microsoft.Maui.IImageSourceServiceResult<UIKit.UIImage!>?>! GetImageAsync(Microsoft.Maui.IStreamImageSource! imageSource, float scale = 1, System.Threading.CancellationToken cancellationToken = default(System.Threading.CancellationToken))
System.Threading.Tasks.Task<Microsoft.Maui.IImageSourceServiceResult<UIKit.UIImage!>?>! override GetImageAsync(Microsoft.Maui.IImageSource! imageSource, float scale = 1, System.Threading.CancellationToken cancellationToken = default(System.Threading.CancellationToken))
System.Threading.Tasks.Task<Microsoft.Maui.IImageSourceServiceResult<Microsoft.Maui.Platform.MauiImageSource!>?>! GetImageAsync(Microsoft.Maui.IStreamImageSource! imageSource, System.Threading.CancellationToken cancellationToken = default(System.Threading.CancellationToken))
System.Threading.Tasks.Task<Microsoft.Maui.IImageSourceServiceResult<Microsoft.Maui.Platform.MauiImageSource!>?>! override GetImageAsync(Microsoft.Maui.IImageSource! imageSource, System.Threading.CancellationToken cancellationToken = default(System.Threading.CancellationToken))
```

## See also

- Declaring type: [[StreamImageSourceService|StreamImageSourceService]]
- [[_Microsoft.Maui|Microsoft.Maui namespace]]
