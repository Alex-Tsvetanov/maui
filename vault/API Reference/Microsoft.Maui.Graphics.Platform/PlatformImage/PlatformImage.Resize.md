---
title: "PlatformImage.Resize"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Graphics-Platform
aliases:
  - "Microsoft.Maui.Graphics.Platform.PlatformImage.Resize"
declaring_type: "PlatformImage"
member_kind: method
---

# PlatformImage.Resize

> [!abstract] Method of [[PlatformImage|PlatformImage]]
> Namespace: `Microsoft.Maui.Graphics.Platform`

Resizes the image to the specified dimensions using the specified resize mode.

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

A new `IImage` with the resized dimensions.

## See also

- Declaring type: [[PlatformImage|PlatformImage]]
- [[_Microsoft.Maui.Graphics.Platform|Microsoft.Maui.Graphics.Platform namespace]]
