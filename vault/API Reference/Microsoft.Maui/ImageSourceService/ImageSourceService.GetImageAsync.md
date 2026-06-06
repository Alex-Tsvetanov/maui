---
title: "ImageSourceService.GetImageAsync"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui
aliases:
  - "Microsoft.Maui.ImageSourceService.GetImageAsync"
declaring_type: "ImageSourceService"
member_kind: method
---

# ImageSourceService.GetImageAsync

> [!abstract] Method of [[ImageSourceService|ImageSourceService]]
> Namespace: `Microsoft.Maui`

Asynchronously produces a native image for the specified image source at the given scale.

## Signatures

```csharp
System.Threading.Tasks.Task<Microsoft.Maui.IImageSourceServiceResult<UIKit.UIImage!>?>! abstract GetImageAsync(Microsoft.Maui.IImageSource! imageSource, float scale = 1, System.Threading.CancellationToken cancellationToken = default(System.Threading.CancellationToken))
System.Threading.Tasks.Task<Microsoft.Maui.IImageSourceServiceResult<Microsoft.Maui.Platform.MauiImageSource!>?>! abstract GetImageAsync(Microsoft.Maui.IImageSource! imageSource, System.Threading.CancellationToken cancellationToken = default(System.Threading.CancellationToken))
```

## See also

- Declaring type: [[ImageSourceService|ImageSourceService]]
- [[_Microsoft.Maui|Microsoft.Maui namespace]]
