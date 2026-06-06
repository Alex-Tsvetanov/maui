---
title: "ImageSourcePartExtensions.UpdateSourceAsync"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Platform
aliases:
  - "Microsoft.Maui.Platform.ImageSourcePartExtensions.UpdateSourceAsync"
declaring_type: "ImageSourcePartExtensions"
member_kind: method
---

# ImageSourcePartExtensions.UpdateSourceAsync

> [!abstract] Method of [[ImageSourcePartExtensions|ImageSourcePartExtensions]]
> Namespace: `Microsoft.Maui.Platform`

Asynchronously updates the platform image from the image source part and returns the load result.

## Signature

```csharp
System.Threading.Tasks.Task<Microsoft.Maui.IImageSourceServiceResult<Microsoft.Maui.Platform.MauiImageSource!>?>! static UpdateSourceAsync(this Microsoft.Maui.IImageSourcePart! image, Tizen.NUI.BaseComponents.View! destinationContext, Microsoft.Maui.IImageSourceServiceProvider! services, System.Action<Microsoft.Maui.Platform.MauiImageSource?>! setImage, System.Threading.CancellationToken cancellationToken = default(System.Threading.CancellationToken))
```

## See also

- Declaring type: [[ImageSourcePartExtensions|ImageSourcePartExtensions]]
- [[_Microsoft.Maui.Platform|Microsoft.Maui.Platform namespace]]
