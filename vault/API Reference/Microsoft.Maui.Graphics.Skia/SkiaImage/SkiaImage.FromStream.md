---
title: "SkiaImage.FromStream"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Graphics-Skia
aliases:
  - "Microsoft.Maui.Graphics.Skia.SkiaImage.FromStream"
declaring_type: "SkiaImage"
member_kind: method
---

# SkiaImage.FromStream

> [!abstract] Method of [[SkiaImage|SkiaImage]]
> Namespace: `Microsoft.Maui.Graphics.Skia`

Creates a new image from a stream.

## Signature

```csharp
Microsoft.Maui.Graphics.IImage static FromStream(System.IO.Stream stream, Microsoft.Maui.Graphics.ImageFormat formatHint = Microsoft.Maui.Graphics.ImageFormat.Png)
```

## Parameters

| Parameter | Description |
|---|---|
| `stream` | The stream containing image data. |
| `formatHint` | Optional hint about the image format. |

## Returns

A new `IImage` instance containing the image from the stream.

## See also

- Declaring type: [[SkiaImage|SkiaImage]]
- [[_Microsoft.Maui.Graphics.Skia|Microsoft.Maui.Graphics.Skia namespace]]
