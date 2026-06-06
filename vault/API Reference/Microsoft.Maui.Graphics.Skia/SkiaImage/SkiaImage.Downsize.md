---
title: "SkiaImage.Downsize"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Graphics-Skia
aliases:
  - "Microsoft.Maui.Graphics.Skia.SkiaImage.Downsize"
declaring_type: "SkiaImage"
member_kind: method
---

# SkiaImage.Downsize

> [!abstract] Method of [[SkiaImage|SkiaImage]]
> Namespace: `Microsoft.Maui.Graphics.Skia`

Creates a new image by downsizing this image to fit within the specified maximum dimension.

## Signatures

```csharp
Microsoft.Maui.Graphics.IImage Downsize(float maxWidth, float maxHeight, bool disposeOriginal = false)
Microsoft.Maui.Graphics.IImage Downsize(float maxWidthOrHeight, bool disposeOriginal = false)
```

## Parameters

| Parameter | Description |
|---|---|
| `maxWidthOrHeight` | The maximum width or height the resulting image should have. |
| `disposeOriginal` | Whether to dispose the original image after downsizing. |

## Returns

A new image instance with the downsized dimensions.

## See also

- Declaring type: [[SkiaImage|SkiaImage]]
- [[_Microsoft.Maui.Graphics.Skia|Microsoft.Maui.Graphics.Skia namespace]]
