---
title: "IImageSourcePartSetter.SetImageSource"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Platform
aliases:
  - "Microsoft.Maui.Platform.IImageSourcePartSetter.SetImageSource"
declaring_type: "IImageSourcePartSetter"
member_kind: method
---

# IImageSourcePartSetter.SetImageSource

> [!abstract] Method of [[IImageSourcePartSetter|IImageSourcePartSetter]]
> Namespace: `Microsoft.Maui.Platform`

The platform-specific implementation that knows how to set the loaded image into a platform view.

## Signatures

```csharp
void SetImageSource(Android.Graphics.Drawables.Drawable? platformImage)
void SetImageSource(UIKit.UIImage? platformImage)
void SetImageSource(Microsoft.Maui.Platform.MauiImageSource? platformImage)
void SetImageSource(Microsoft.UI.Xaml.Media.ImageSource? platformImage)
void SetImageSource(object? platformImage)
```

## Parameters

| Parameter | Description |
|---|---|
| `platformImage` | The platform image to set. |

## See also

- Declaring type: [[IImageSourcePartSetter|IImageSourcePartSetter]]
- [[_Microsoft.Maui.Platform|Microsoft.Maui.Platform namespace]]
