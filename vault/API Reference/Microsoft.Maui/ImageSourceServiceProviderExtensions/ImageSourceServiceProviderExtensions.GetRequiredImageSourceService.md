---
title: "ImageSourceServiceProviderExtensions.GetRequiredImageSourceService"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui
aliases:
  - "Microsoft.Maui.ImageSourceServiceProviderExtensions.GetRequiredImageSourceService"
declaring_type: "ImageSourceServiceProviderExtensions"
member_kind: method
---

# ImageSourceServiceProviderExtensions.GetRequiredImageSourceService

> [!abstract] Method of [[ImageSourceServiceProviderExtensions|ImageSourceServiceProviderExtensions]]
> Namespace: `Microsoft.Maui`

Returns the image source service that can handle the specified image source, throwing if none is registered.

## Signatures

```csharp
Microsoft.Maui.IImageSourceService! static GetRequiredImageSourceService(this Microsoft.Maui.IImageSourceServiceProvider! provider, Microsoft.Maui.IImageSource! imageSource)
Microsoft.Maui.IImageSourceService! static GetRequiredImageSourceService(this Microsoft.Maui.IImageSourceServiceProvider! provider, System.Type! imageSourceType)
```

## See also

- Declaring type: [[ImageSourceServiceProviderExtensions|ImageSourceServiceProviderExtensions]]
- [[_Microsoft.Maui|Microsoft.Maui namespace]]
