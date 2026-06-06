---
title: "ImageSourceExtensions.GetPlatformImageAsync"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui
aliases:
  - "Microsoft.Maui.ImageSourceExtensions.GetPlatformImageAsync"
declaring_type: "ImageSourceExtensions"
member_kind: method
---

# ImageSourceExtensions.GetPlatformImageAsync

> [!abstract] Method of [[ImageSourceExtensions|ImageSourceExtensions]]
> Namespace: `Microsoft.Maui`

Asynchronously loads the image source and returns the platform-specific image result.

## Signatures

```csharp
System.Threading.Tasks.Task<Microsoft.Maui.IImageSourceServiceResult<Android.Graphics.Drawables.Drawable!>?>! static GetPlatformImageAsync(this Microsoft.Maui.IImageSource? imageSource, Microsoft.Maui.IMauiContext! mauiContext)
System.Threading.Tasks.Task<Microsoft.Maui.IImageSourceServiceResult<Android.Graphics.Drawables.Drawable!>?>! static GetPlatformImageAsync(this Microsoft.Maui.IImageSourceService! imageSourceService, Microsoft.Maui.IImageSource? imageSource, Microsoft.Maui.IMauiContext! mauiContext)
```

## See also

- Declaring type: [[ImageSourceExtensions|ImageSourceExtensions]]
- [[_Microsoft.Maui|Microsoft.Maui namespace]]
