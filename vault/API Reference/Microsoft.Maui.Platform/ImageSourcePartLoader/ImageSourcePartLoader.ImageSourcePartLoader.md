---
title: "ImageSourcePartLoader.ImageSourcePartLoader"
tags:
  - api
  - member/constructor
  - ns/Microsoft-Maui-Platform
aliases:
  - "Microsoft.Maui.Platform.ImageSourcePartLoader.ImageSourcePartLoader"
declaring_type: "ImageSourcePartLoader"
member_kind: constructor
---

# ImageSourcePartLoader.ImageSourcePartLoader

> [!abstract] Constructor of [[ImageSourcePartLoader|ImageSourcePartLoader]]
> Namespace: `Microsoft.Maui.Platform`

Initializes a new instance of the image source part loader for the specified handler or setter.

## Signatures

```csharp
void ImageSourcePartLoader(Microsoft.Maui.IElementHandler! handler, System.Func<Microsoft.Maui.IImageSourcePart?>! imageSourcePart, System.Action<Android.Graphics.Drawables.Drawable?>! setImage)
void ImageSourcePartLoader(Microsoft.Maui.Platform.IImageSourcePartSetter! setter)
void ImageSourcePartLoader(Microsoft.Maui.IElementHandler! handler, System.Func<Microsoft.Maui.IImageSourcePart?>! imageSourcePart, System.Action<UIKit.UIImage?>! setImage)
void ImageSourcePartLoader(Microsoft.Maui.IElementHandler! handler, System.Func<Microsoft.Maui.IImageSourcePart?>! imageSourcePart, System.Action<Microsoft.Maui.Platform.MauiImageSource?>! setImage)
void ImageSourcePartLoader(Microsoft.Maui.IElementHandler! handler, System.Func<Microsoft.Maui.IImageSourcePart?>! imageSourcePart, System.Action<Microsoft.UI.Xaml.Media.ImageSource?>! setImage)
void ImageSourcePartLoader(Microsoft.Maui.IElementHandler! handler, System.Func<Microsoft.Maui.IImageSourcePart?>! imageSourcePart, System.Action<object?>! setImage)
```

## See also

- Declaring type: [[ImageSourcePartLoader|ImageSourcePartLoader]]
- [[_Microsoft.Maui.Platform|Microsoft.Maui.Platform namespace]]
