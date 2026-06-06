---
title: "IImageSourceService.GetImageAsync"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui
aliases:
  - "Microsoft.Maui.IImageSourceService.GetImageAsync"
declaring_type: "IImageSourceService"
member_kind: method
---

# IImageSourceService.GetImageAsync

> [!abstract] Method of [[IImageSourceService|IImageSourceService]]
> Namespace: `Microsoft.Maui`

Asynchronously resolves the image source into a native platform image at the specified scale.

## Signatures

```csharp
System.Threading.Tasks.Task<Microsoft.Maui.IImageSourceServiceResult<UIKit.UIImage!>?>! GetImageAsync(Microsoft.Maui.IImageSource! imageSource, float scale = 1, System.Threading.CancellationToken cancellationToken = default(System.Threading.CancellationToken))
System.Threading.Tasks.Task<Microsoft.Maui.IImageSourceServiceResult<Microsoft.Maui.Platform.MauiImageSource!>?>! GetImageAsync(Microsoft.Maui.IImageSource! imageSource, System.Threading.CancellationToken cancellationToken = default(System.Threading.CancellationToken))
```

## See also

- Declaring type: [[IImageSourceService|IImageSourceService]]
- [[_Microsoft.Maui|Microsoft.Maui namespace]]
