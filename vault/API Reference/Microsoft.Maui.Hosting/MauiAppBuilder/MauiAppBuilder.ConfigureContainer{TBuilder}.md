---
title: "MauiAppBuilder.ConfigureContainer<TBuilder>"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Hosting
aliases:
  - "Microsoft.Maui.Hosting.MauiAppBuilder.ConfigureContainer<TBuilder>"
declaring_type: "MauiAppBuilder"
member_kind: method
---

# MauiAppBuilder.ConfigureContainer<TBuilder>

> [!abstract] Method of [[MauiAppBuilder|MauiAppBuilder]]
> Namespace: `Microsoft.Maui.Hosting`

Registers a `IServiceProviderFactory{TBuilder}` instance to be used to create the `IServiceProvider`.

## Signature

```csharp
void ConfigureContainer<TBuilder>(Microsoft.Extensions.DependencyInjection.IServiceProviderFactory<TBuilder>! factory, System.Action<TBuilder>? configure = null)
```

## Remarks

`ConfigureContainer{TBuilder}` is called by `Build` and so the delegate provided by `configure` will run after all other services have been registered. Multiple calls to `ConfigureContainer{TBuilder}` will replace the previously stored `factory` and `configure` delegate.

## Parameters

| Parameter | Description |
|---|---|
| `factory` | The `IServiceProviderFactory{TBuilder}`. |
| `configure` | A delegate used to configure the . This can be used to configure services using APIS specific to the `IServiceProviderFactory{TBuilder}` implementation. |

## See also

- Declaring type: [[MauiAppBuilder|MauiAppBuilder]]
- [[_Microsoft.Maui.Hosting|Microsoft.Maui.Hosting namespace]]
