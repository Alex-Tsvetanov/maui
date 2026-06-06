---
title: "PlatformImage.SaveAsync"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Graphics-Platform
aliases:
  - "Microsoft.Maui.Graphics.Platform.PlatformImage.SaveAsync"
declaring_type: "PlatformImage"
member_kind: method
---

# PlatformImage.SaveAsync

> [!abstract] Method of [[PlatformImage|PlatformImage]]
> Namespace: `Microsoft.Maui.Graphics.Platform`

Saves the image to a stream in the specified format.

## Signature

```csharp
System.Threading.Tasks.Task SaveAsync(System.IO.Stream stream, Microsoft.Maui.Graphics.ImageFormat format = Microsoft.Maui.Graphics.ImageFormat.Png, float quality = 1)
```

## Parameters

| Parameter | Description |
|---|---|
| `stream` | The stream to save the image to. |
| `format` | The format to save the image in (default is PNG). |
| `quality` | The quality level (0.0 to 1.0) when using lossy formats like JPEG (default is 1.0). |

## See also

- Declaring type: [[PlatformImage|PlatformImage]]
- [[_Microsoft.Maui.Graphics.Platform|Microsoft.Maui.Graphics.Platform namespace]]
