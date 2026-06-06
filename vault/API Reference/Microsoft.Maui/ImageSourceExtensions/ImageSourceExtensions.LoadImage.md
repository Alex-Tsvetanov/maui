---
title: "ImageSourceExtensions.LoadImage"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui
aliases:
  - "Microsoft.Maui.ImageSourceExtensions.LoadImage"
declaring_type: "ImageSourceExtensions"
member_kind: method
---

# ImageSourceExtensions.LoadImage

> [!abstract] Method of [[ImageSourceExtensions|ImageSourceExtensions]]
> Namespace: `Microsoft.Maui`

Loads the image source using the given MAUI context and invokes the callback with the result.

## Signatures

```csharp
void static LoadImage(this Microsoft.Maui.IImageSource? source, Microsoft.Maui.IMauiContext! mauiContext, System.Action<Microsoft.Maui.IImageSourceServiceResult<Android.Graphics.Drawables.Drawable!>?>? finished = null)
void static LoadImage(this Microsoft.Maui.IImageSource? source, Microsoft.Maui.IMauiContext! mauiContext, System.Action<Microsoft.Maui.IImageSourceServiceResult<UIKit.UIImage!>?>? finished = null)
void static LoadImage(this Microsoft.Maui.IImageSource? source, Microsoft.Maui.IMauiContext! mauiContext, System.Action<Microsoft.Maui.IImageSourceServiceResult<Microsoft.Maui.Platform.MauiImageSource!>?>? finished = null)
void static LoadImage(this Microsoft.Maui.IImageSource? source, Microsoft.Maui.IMauiContext! mauiContext, System.Action<Microsoft.Maui.IImageSourceServiceResult<Microsoft.UI.Xaml.Media.ImageSource!>?>? finished = null)
void static LoadImage(this Microsoft.Maui.IImageSource? source, Microsoft.Maui.IMauiContext! mauiContext, System.Action<Microsoft.Maui.IImageSourceServiceResult<object!>?>? finished = null)
```

## See also

- Declaring type: [[ImageSourceExtensions|ImageSourceExtensions]]
- [[_Microsoft.Maui|Microsoft.Maui namespace]]
