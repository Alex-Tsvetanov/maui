---
title: "PlatformImage.Save"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Graphics-Platform
aliases:
  - "Microsoft.Maui.Graphics.Platform.PlatformImage.Save"
declaring_type: "PlatformImage"
member_kind: method
---

# PlatformImage.Save

> [!abstract] Method of [[PlatformImage|PlatformImage]]
> Namespace: `Microsoft.Maui.Graphics.Platform`

Saves the contents of this image to the provided `Stream` object.

## Signature

```csharp
void Save(System.IO.Stream stream, Microsoft.Maui.Graphics.ImageFormat format = Microsoft.Maui.Graphics.ImageFormat.Png, float quality = 1)
```

## Parameters

| Parameter | Description |
|---|---|
| `stream` | The destination stream the bytes of this image will be saved to. |
| `format` | The destination format of the image. |
| `quality` | The destination quality of the image. |

## Remarks

The `quality` value is currently unused.

## See also

- Declaring type: [[PlatformImage|PlatformImage]]
- [[_Microsoft.Maui.Graphics.Platform|Microsoft.Maui.Graphics.Platform namespace]]
