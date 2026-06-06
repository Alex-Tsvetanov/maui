---
title: "ImageExtensions (Graphics).AsStream"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Graphics
aliases:
  - "Microsoft.Maui.Graphics.ImageExtensions.AsStream"
declaring_type: "ImageExtensions (Graphics)"
member_kind: method
---

# ImageExtensions (Graphics).AsStream

> [!abstract] Method of [[ImageExtensions (Graphics)|ImageExtensions (Graphics)]]
> Namespace: `Microsoft.Maui.Graphics`

Converts an image to a stream in the specified format.

## Signature

```csharp
System.IO.Stream static AsStream(this Microsoft.Maui.Graphics.IImage target, Microsoft.Maui.Graphics.ImageFormat format = Microsoft.Maui.Graphics.ImageFormat.Png, float quality = 1)
```

## Parameters

| Parameter | Description |
|---|---|
| `target` | The image to convert. |
| `format` | The format to encode the image in (default is PNG). |
| `quality` | The quality setting for lossy formats like JPEG, ranging from 0 to 1 (default is 1 for maximum quality). |

## Returns

A memory stream containing the encoded image data, or null if the target image is null.

## See also

- Declaring type: [[ImageExtensions (Graphics)|ImageExtensions (Graphics)]]
- [[_Microsoft.Maui.Graphics|Microsoft.Maui.Graphics namespace]]
