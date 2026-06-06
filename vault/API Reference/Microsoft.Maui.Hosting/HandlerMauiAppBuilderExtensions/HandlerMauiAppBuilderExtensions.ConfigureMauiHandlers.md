---
title: "HandlerMauiAppBuilderExtensions.ConfigureMauiHandlers"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Hosting
aliases:
  - "Microsoft.Maui.Hosting.HandlerMauiAppBuilderExtensions.ConfigureMauiHandlers"
declaring_type: "HandlerMauiAppBuilderExtensions"
member_kind: method
---

# HandlerMauiAppBuilderExtensions.ConfigureMauiHandlers

> [!abstract] Method of [[HandlerMauiAppBuilderExtensions|HandlerMauiAppBuilderExtensions]]
> Namespace: `Microsoft.Maui.Hosting`

Registers MAUI handler mappings by invoking the supplied delegate against the handlers collection.

## Signatures

```csharp
Microsoft.Extensions.DependencyInjection.IServiceCollection! static ConfigureMauiHandlers(this Microsoft.Extensions.DependencyInjection.IServiceCollection! services, System.Action<Microsoft.Maui.Hosting.IMauiHandlersCollection!>? configureDelegate)
Microsoft.Maui.Hosting.MauiAppBuilder! static ConfigureMauiHandlers(this Microsoft.Maui.Hosting.MauiAppBuilder! builder, System.Action<Microsoft.Maui.Hosting.IMauiHandlersCollection!>? configureDelegate)
```

## See also

- Declaring type: [[HandlerMauiAppBuilderExtensions|HandlerMauiAppBuilderExtensions]]
- [[_Microsoft.Maui.Hosting|Microsoft.Maui.Hosting namespace]]
