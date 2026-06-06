---
title: "FontImageSourceService.GetImageSourceAsync"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui
aliases:
  - "Microsoft.Maui.FontImageSourceService.GetImageSourceAsync"
declaring_type: "FontImageSourceService"
member_kind: method
---

# FontImageSourceService.GetImageSourceAsync

> [!abstract] Method of [[FontImageSourceService|FontImageSourceService]]
> Namespace: `Microsoft.Maui`

Asynchronously produces a native image source for the specified font image source at the given scale.

## Signatures

```csharp
System.Threading.Tasks.Task<Microsoft.Maui.IImageSourceServiceResult<Microsoft.UI.Xaml.Media.ImageSource!>?>! GetImageSourceAsync(Microsoft.Maui.IFontImageSource! imageSource, float scale = 1, System.Threading.CancellationToken cancellationToken = default(System.Threading.CancellationToken))
System.Threading.Tasks.Task<Microsoft.Maui.IImageSourceServiceResult<Microsoft.UI.Xaml.Media.ImageSource!>?>! override GetImageSourceAsync(Microsoft.Maui.IImageSource! imageSource, float scale = 1, System.Threading.CancellationToken cancellationToken = default(System.Threading.CancellationToken))
```

## See also

- Declaring type: [[FontImageSourceService|FontImageSourceService]]
- [[_Microsoft.Maui|Microsoft.Maui namespace]]
