---
title: "FileImageSourceService.GetImageAsync"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui
aliases:
  - "Microsoft.Maui.FileImageSourceService.GetImageAsync"
declaring_type: "FileImageSourceService"
member_kind: method
---

# FileImageSourceService.GetImageAsync

> [!abstract] Method of [[FileImageSourceService|FileImageSourceService]]
> Namespace: `Microsoft.Maui`

Asynchronously loads the file image source as a native platform image.

## Signatures

```csharp
System.Threading.Tasks.Task<Microsoft.Maui.IImageSourceServiceResult<UIKit.UIImage!>?>! GetImageAsync(Microsoft.Maui.IFileImageSource! imageSource, float scale = 1, System.Threading.CancellationToken cancellationToken = default(System.Threading.CancellationToken))
System.Threading.Tasks.Task<Microsoft.Maui.IImageSourceServiceResult<UIKit.UIImage!>?>! override GetImageAsync(Microsoft.Maui.IImageSource! imageSource, float scale = 1, System.Threading.CancellationToken cancellationToken = default(System.Threading.CancellationToken))
System.Threading.Tasks.Task<Microsoft.Maui.IImageSourceServiceResult<Microsoft.Maui.Platform.MauiImageSource!>?>! GetImageAsync(Microsoft.Maui.IFileImageSource! imageSource, System.Threading.CancellationToken cancellationToken = default(System.Threading.CancellationToken))
System.Threading.Tasks.Task<Microsoft.Maui.IImageSourceServiceResult<Microsoft.Maui.Platform.MauiImageSource!>?>! override GetImageAsync(Microsoft.Maui.IImageSource! imageSource, System.Threading.CancellationToken cancellationToken = default(System.Threading.CancellationToken))
```

## See also

- Declaring type: [[FileImageSourceService|FileImageSourceService]]
- [[_Microsoft.Maui|Microsoft.Maui namespace]]
