---
title: "SkiaImage.SaveAsync"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Graphics-Skia
aliases:
  - "Microsoft.Maui.Graphics.Skia.SkiaImage.SaveAsync"
declaring_type: "SkiaImage"
member_kind: method
---

# SkiaImage.SaveAsync

> [!abstract] Method of [[SkiaImage|SkiaImage]]
> Namespace: `Microsoft.Maui.Graphics.Skia`

Asynchronously saves the image to a stream in the specified format.

## Signature

```csharp
System.Threading.Tasks.Task SaveAsync(System.IO.Stream stream, Microsoft.Maui.Graphics.ImageFormat format = Microsoft.Maui.Graphics.ImageFormat.Png, float quality = 1)
```

## Parameters

| Parameter | Description |
|---|---|
| `stream` | The stream to save the image to. |
| `format` | The format to save the image in. |
| `quality` | The quality level to use when saving the image (0.0 to 1.0). |

## Returns

A task that represents the asynchronous save operation.

## See also

- Declaring type: [[SkiaImage|SkiaImage]]
- [[_Microsoft.Maui.Graphics.Skia|Microsoft.Maui.Graphics.Skia namespace]]
