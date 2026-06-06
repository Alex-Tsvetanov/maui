---
title: "ImageSourceServiceCollectionExtensions.AddService<TImageSource>"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Hosting
aliases:
  - "Microsoft.Maui.Hosting.ImageSourceServiceCollectionExtensions.AddService<TImageSource>"
declaring_type: "ImageSourceServiceCollectionExtensions"
member_kind: method
---

# ImageSourceServiceCollectionExtensions.AddService<TImageSource>

> [!abstract] Method of [[ImageSourceServiceCollectionExtensions|ImageSourceServiceCollectionExtensions]]
> Namespace: `Microsoft.Maui.Hosting`

Registers an image service with the underlying service container via AddSingleton.

## Signature

```csharp
Microsoft.Maui.Hosting.IImageSourceServiceCollection! static AddService<TImageSource>(this Microsoft.Maui.Hosting.IImageSourceServiceCollection! services, System.Func<System.IServiceProvider!, Microsoft.Maui.IImageSourceService<TImageSource>!>! implementationFactory)
```

## Returns

The service collection

## Parameters

| Parameter | Description |
|---|---|
| `services` | The service collection |
| `implementationFactory` | A factory method to create the service |

## See also

- Declaring type: [[ImageSourceServiceCollectionExtensions|ImageSourceServiceCollectionExtensions]]
- [[_Microsoft.Maui.Hosting|Microsoft.Maui.Hosting namespace]]
