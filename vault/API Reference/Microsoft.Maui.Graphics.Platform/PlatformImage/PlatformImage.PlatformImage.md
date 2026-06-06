---
title: "PlatformImage.PlatformImage"
tags:
  - api
  - member/constructor
  - ns/Microsoft-Maui-Graphics-Platform
aliases:
  - "Microsoft.Maui.Graphics.Platform.PlatformImage.PlatformImage"
declaring_type: "PlatformImage"
member_kind: constructor
---

# PlatformImage.PlatformImage

> [!abstract] Constructor of [[PlatformImage|PlatformImage]]
> Namespace: `Microsoft.Maui.Graphics.Platform`

Initializes a new instance of the `PlatformImage` class with the specified bytes and format.

## Signatures

```csharp
void PlatformImage(Android.Graphics.Bitmap bitmap)
void PlatformImage(UIKit.UIImage image)
void PlatformImage(AppKit.NSImage image)
void PlatformImage(byte[] bytes, Microsoft.Maui.Graphics.ImageFormat originalFormat = Microsoft.Maui.Graphics.ImageFormat.Png)
void PlatformImage(Microsoft.Graphics.Canvas.ICanvasResourceCreator creator, Microsoft.Graphics.Canvas.CanvasBitmap bitmap)
```

## Parameters

| Parameter | Description |
|---|---|
| `bytes` | The raw image data. |
| `originalFormat` | The format of the image data (default is PNG). |

## See also

- Declaring type: [[PlatformImage|PlatformImage]]
- [[_Microsoft.Maui.Graphics.Platform|Microsoft.Maui.Graphics.Platform namespace]]
