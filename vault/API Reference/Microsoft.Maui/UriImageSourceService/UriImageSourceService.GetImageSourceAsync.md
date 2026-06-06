---
title: "UriImageSourceService.GetImageSourceAsync"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui
aliases:
  - "Microsoft.Maui.UriImageSourceService.GetImageSourceAsync"
declaring_type: "UriImageSourceService"
member_kind: method
---

# UriImageSourceService.GetImageSourceAsync

> [!abstract] Method of [[UriImageSourceService|UriImageSourceService]]
> Namespace: `Microsoft.Maui`

Asynchronously loads a native image source from the specified URI image source at the given scale.

## Signatures

```csharp
System.Threading.Tasks.Task<Microsoft.Maui.IImageSourceServiceResult<Microsoft.UI.Xaml.Media.ImageSource!>?>! GetImageSourceAsync(Microsoft.Maui.IUriImageSource! imageSource, float scale = 1, System.Threading.CancellationToken cancellationToken = default(System.Threading.CancellationToken))
System.Threading.Tasks.Task<Microsoft.Maui.IImageSourceServiceResult<Microsoft.UI.Xaml.Media.ImageSource!>?>! override GetImageSourceAsync(Microsoft.Maui.IImageSource! imageSource, float scale = 1, System.Threading.CancellationToken cancellationToken = default(System.Threading.CancellationToken))
```

## See also

- Declaring type: [[UriImageSourceService|UriImageSourceService]]
- [[_Microsoft.Maui|Microsoft.Maui namespace]]
