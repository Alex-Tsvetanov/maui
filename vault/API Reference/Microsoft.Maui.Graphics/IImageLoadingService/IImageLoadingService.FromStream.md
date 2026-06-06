---
title: "IImageLoadingService.FromStream"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Graphics
aliases:
  - "Microsoft.Maui.Graphics.IImageLoadingService.FromStream"
declaring_type: "IImageLoadingService"
member_kind: method
---

# IImageLoadingService.FromStream

> [!abstract] Method of [[IImageLoadingService|IImageLoadingService]]
> Namespace: `Microsoft.Maui.Graphics`

Creates an image from the specified stream.

## Signature

```csharp
Microsoft.Maui.Graphics.IImage FromStream(System.IO.Stream stream, Microsoft.Maui.Graphics.ImageFormat format = Microsoft.Maui.Graphics.ImageFormat.Png)
```

## Returns

An `IImage` created from the stream.

## Parameters

| Parameter | Description |
|---|---|
| `stream` | The stream containing the image data. |
| `format` | The format of the image data (default is PNG). |

## See also

- Declaring type: [[IImageLoadingService|IImageLoadingService]]
- [[_Microsoft.Maui.Graphics|Microsoft.Maui.Graphics namespace]]
