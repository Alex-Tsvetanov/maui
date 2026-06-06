---
title: "SkiaImageLoadingService (Skia).FromStream"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Graphics-Skia
aliases:
  - "Microsoft.Maui.Graphics.Skia.SkiaImageLoadingService.FromStream"
declaring_type: "SkiaImageLoadingService (Skia)"
member_kind: method
---

# SkiaImageLoadingService (Skia).FromStream

> [!abstract] Method of [[SkiaImageLoadingService (Skia)|SkiaImageLoadingService (Skia)]]
> Namespace: `Microsoft.Maui.Graphics.Skia`

Creates a new image from a stream.

## Signature

```csharp
Microsoft.Maui.Graphics.IImage FromStream(System.IO.Stream stream, Microsoft.Maui.Graphics.ImageFormat formatHint = Microsoft.Maui.Graphics.ImageFormat.Png)
```

## Parameters

| Parameter | Description |
|---|---|
| `stream` | The stream containing the image data. |
| `formatHint` | Optional hint about the image format. |

## Returns

A new `IImage` instance containing the image from the stream.

## See also

- Declaring type: [[SkiaImageLoadingService (Skia)|SkiaImageLoadingService (Skia)]]
- [[_Microsoft.Maui.Graphics.Skia|Microsoft.Maui.Graphics.Skia namespace]]
