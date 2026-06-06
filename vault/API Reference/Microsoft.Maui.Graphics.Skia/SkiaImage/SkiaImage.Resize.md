---
title: "SkiaImage.Resize"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Graphics-Skia
aliases:
  - "Microsoft.Maui.Graphics.Skia.SkiaImage.Resize"
declaring_type: "SkiaImage"
member_kind: method
---

# SkiaImage.Resize

> [!abstract] Method of [[SkiaImage|SkiaImage]]
> Namespace: `Microsoft.Maui.Graphics.Skia`

Creates a new image by resizing this image to the specified dimensions.

## Signature

```csharp
Microsoft.Maui.Graphics.IImage Resize(float width, float height, Microsoft.Maui.Graphics.ResizeMode resizeMode = Microsoft.Maui.Graphics.ResizeMode.Fit, bool disposeOriginal = false)
```

## Parameters

| Parameter | Description |
|---|---|
| `width` | The width of the resulting image. |
| `height` | The height of the resulting image. |
| `resizeMode` | The mode to use when resizing the image. |
| `disposeOriginal` | Whether to dispose the original image after resizing. |

## Returns

A new image instance with the specified dimensions.

## See also

- Declaring type: [[SkiaImage|SkiaImage]]
- [[_Microsoft.Maui.Graphics.Skia|Microsoft.Maui.Graphics.Skia namespace]]
