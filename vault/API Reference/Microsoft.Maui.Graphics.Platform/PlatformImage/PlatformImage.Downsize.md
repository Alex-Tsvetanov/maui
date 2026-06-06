---
title: "PlatformImage.Downsize"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Graphics-Platform
aliases:
  - "Microsoft.Maui.Graphics.Platform.PlatformImage.Downsize"
declaring_type: "PlatformImage"
member_kind: method
---

# PlatformImage.Downsize

> [!abstract] Method of [[PlatformImage|PlatformImage]]
> Namespace: `Microsoft.Maui.Graphics.Platform`

Downsizes the image to fit within the specified maximum dimension while maintaining aspect ratio.

## Signatures

```csharp
Microsoft.Maui.Graphics.IImage Downsize(float maxWidth, float maxHeight, bool disposeOriginal = false)
Microsoft.Maui.Graphics.IImage Downsize(float maxWidthOrHeight, bool disposeOriginal = false)
```

## Parameters

| Parameter | Description |
|---|---|
| `maxWidthOrHeight` | The maximum width or height of the resulting image. |
| `disposeOriginal` | Whether to dispose the original image after downsizing. |

## Returns

A new `IImage` with the downsized dimensions.

## See also

- Declaring type: [[PlatformImage|PlatformImage]]
- [[_Microsoft.Maui.Graphics.Platform|Microsoft.Maui.Graphics.Platform namespace]]
