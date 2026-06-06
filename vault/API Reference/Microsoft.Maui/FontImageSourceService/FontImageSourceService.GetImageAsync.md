---
title: "FontImageSourceService.GetImageAsync"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui
aliases:
  - "Microsoft.Maui.FontImageSourceService.GetImageAsync"
declaring_type: "FontImageSourceService"
member_kind: method
---

# FontImageSourceService.GetImageAsync

> [!abstract] Method of [[FontImageSourceService|FontImageSourceService]]
> Namespace: `Microsoft.Maui`

Asynchronously produces a native image for the specified font image source at the given scale.

## Signatures

```csharp
System.Threading.Tasks.Task<Microsoft.Maui.IImageSourceServiceResult<UIKit.UIImage!>?>! GetImageAsync(Microsoft.Maui.IFontImageSource! imageSource, float scale = 1, System.Threading.CancellationToken cancellationToken = default(System.Threading.CancellationToken))
System.Threading.Tasks.Task<Microsoft.Maui.IImageSourceServiceResult<UIKit.UIImage!>?>! override GetImageAsync(Microsoft.Maui.IImageSource! imageSource, float scale = 1, System.Threading.CancellationToken cancellationToken = default(System.Threading.CancellationToken))
System.Threading.Tasks.Task<Microsoft.Maui.IImageSourceServiceResult<Microsoft.Maui.Platform.MauiImageSource!>?>! GetImageAsync(Microsoft.Maui.IFontImageSource! imageSource, System.Threading.CancellationToken cancellationToken = default(System.Threading.CancellationToken))
System.Threading.Tasks.Task<Microsoft.Maui.IImageSourceServiceResult<Microsoft.Maui.Platform.MauiImageSource!>?>! override GetImageAsync(Microsoft.Maui.IImageSource! imageSource, System.Threading.CancellationToken cancellationToken = default(System.Threading.CancellationToken))
```

## See also

- Declaring type: [[FontImageSourceService|FontImageSourceService]]
- [[_Microsoft.Maui|Microsoft.Maui namespace]]
