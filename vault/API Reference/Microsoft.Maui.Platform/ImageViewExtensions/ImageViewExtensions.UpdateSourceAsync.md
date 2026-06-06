---
title: "ImageViewExtensions.UpdateSourceAsync"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Platform
aliases:
  - "Microsoft.Maui.Platform.ImageViewExtensions.UpdateSourceAsync"
declaring_type: "ImageViewExtensions"
member_kind: method
---

# ImageViewExtensions.UpdateSourceAsync

> [!abstract] Method of [[ImageViewExtensions|ImageViewExtensions]]
> Namespace: `Microsoft.Maui.Platform`

Asynchronously updates the source of the specified native image view from the given image source part, using the provided service provider.

## Signature

```csharp
System.Threading.Tasks.Task<Microsoft.Maui.IImageSourceServiceResult<UIKit.UIImage!>?>! static UpdateSourceAsync(this UIKit.UIImageView! imageView, Microsoft.Maui.IImageSourcePart! image, Microsoft.Maui.IImageSourceServiceProvider! services, System.Threading.CancellationToken cancellationToken = default(System.Threading.CancellationToken))
```

## See also

- Declaring type: [[ImageViewExtensions|ImageViewExtensions]]
- [[_Microsoft.Maui.Platform|Microsoft.Maui.Platform namespace]]
