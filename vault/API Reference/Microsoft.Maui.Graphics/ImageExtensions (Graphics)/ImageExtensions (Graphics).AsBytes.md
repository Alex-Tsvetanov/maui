---
title: "ImageExtensions (Graphics).AsBytes"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Graphics
aliases:
  - "Microsoft.Maui.Graphics.ImageExtensions.AsBytes"
declaring_type: "ImageExtensions (Graphics)"
member_kind: method
---

# ImageExtensions (Graphics).AsBytes

> [!abstract] Method of [[ImageExtensions (Graphics)|ImageExtensions (Graphics)]]
> Namespace: `Microsoft.Maui.Graphics`

Converts an image to a byte array in the specified format.

## Signature

```csharp
byte[] static AsBytes(this Microsoft.Maui.Graphics.IImage target, Microsoft.Maui.Graphics.ImageFormat format = Microsoft.Maui.Graphics.ImageFormat.Png, float quality = 1)
```

## Parameters

| Parameter | Description |
|---|---|
| `target` | The image to convert. |
| `format` | The format to encode the image in (default is PNG). |
| `quality` | The quality setting for lossy formats like JPEG, ranging from 0 to 1 (default is 1 for maximum quality). |

## Returns

A byte array containing the encoded image data, or null if the target image is null.

## See also

- Declaring type: [[ImageExtensions (Graphics)|ImageExtensions (Graphics)]]
- [[_Microsoft.Maui.Graphics|Microsoft.Maui.Graphics namespace]]
