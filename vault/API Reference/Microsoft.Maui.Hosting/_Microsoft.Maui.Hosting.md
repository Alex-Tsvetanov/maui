---
title: "Microsoft.Maui.Hosting"
tags:
  - api
  - namespace
  - ns/Microsoft-Maui-Hosting
---

# Microsoft.Maui.Hosting

> [!info] Namespace
> `Microsoft.Maui.Hosting` — 20 public types.

- [Online namespace docs](https://learn.microsoft.com/dotnet/api/microsoft.maui.hosting)

## Overview

`Microsoft.Maui.Hosting` is the startup and dependency-injection layer of a .NET MAUI app. It builds on the generic .NET host model to give every MAUI application a single, consistent entry point where services, handlers, fonts, image sources, and configuration are registered before the app runs. The two central pieces are [[MauiAppBuilder|MauiAppBuilder]], the builder you configure in `CreateMauiApp`, and [[MauiApp|MauiApp]], the fully constructed application — complete with its registered services and configuration — that the builder produces.

Most app setup happens through extension methods on the builder. [[AppHostBuilderExtensions (Microsoft.Maui.Hosting)|AppHostBuilderExtensions]] supplies the core `UseMauiApp`/configuration entry points, while focused extension sets wire up individual subsystems: [[FontsMauiAppBuilderExtensions|FontsMauiAppBuilderExtensions]] and [[FontCollectionExtensions|FontCollectionExtensions]] register fonts into an [[IFontCollection|IFontCollection]] (each described by a [[FontDescriptor|FontDescriptor]]); [[HandlerMauiAppBuilderExtensions|HandlerMauiAppBuilderExtensions]] registers view handlers; [[ImageSourcesMauiAppBuilderExtensions|ImageSourcesMauiAppBuilderExtensions]] and [[EssentialsExtensions|EssentialsExtensions]] enable image-source services and platform Essentials.

Underneath, the namespace defines the specialized service-collection contracts the host relies on — [[IMauiServiceCollection|IMauiServiceCollection]], [[IMauiHandlersCollection|IMauiHandlersCollection]], and [[IImageSourceServiceCollection|IImageSourceServiceCollection]] — plus initialization hooks. [[IMauiInitializeService|IMauiInitializeService]] runs during application construction and [[IMauiInitializeScopedService|IMauiInitializeScopedService]] during window construction, letting libraries perform startup work at the right point in the lifecycle. Together these types let you compose a MAUI app declaratively and extend its startup pipeline with your own services.

## Key types

- [[MauiAppBuilder|MauiAppBuilder]] — Builder for .NET MAUI cross-platform applications and services.
- [[MauiApp|MauiApp]] — A .NET MAUI application with its registered services and configuration data.
- [[AppHostBuilderExtensions (Microsoft.Maui.Hosting)|AppHostBuilderExtensions]] — Core host-builder extension methods for configuring a MAUI app.
- [[FontsMauiAppBuilderExtensions|FontsMauiAppBuilderExtensions]] — Registers fonts on the app builder.
- [[HandlerMauiAppBuilderExtensions|HandlerMauiAppBuilderExtensions]] — Registers view handlers on the app builder.
- [[ImageSourcesMauiAppBuilderExtensions|ImageSourcesMauiAppBuilderExtensions]] — Registers image-source services on the app builder.
- [[EssentialsExtensions|EssentialsExtensions]] — Configures platform Essentials via [[IEssentialsBuilder|IEssentialsBuilder]].
- [[IFontCollection|IFontCollection]] — A collection of fonts registered with the app.
- [[IMauiServiceCollection|IMauiServiceCollection]] — The service collection used by the MAUI host.
- [[IMauiHandlersCollection|IMauiHandlersCollection]] — The collection of registered view handlers.
- [[IMauiInitializeService|IMauiInitializeService]] — A service initialized during application construction.
- [[IMauiInitializeScopedService|IMauiInitializeScopedService]] — A service initialized during window construction.

> [!tip] Most apps interact with this namespace only inside `CreateMauiApp`, chaining `UseMauiApp`, `ConfigureFonts`, and similar builder extension methods before calling `Build()` to produce the `MauiApp`.


## Classes

| Type | Summary |
|---|---|
| [[AppHostBuilderExtensions (Microsoft.Maui.Hosting)\|AppHostBuilderExtensions (Microsoft.Maui.Hosting)]] |  |
| [[EssentialsExtensions\|EssentialsExtensions]] |  |
| [[FontCollectionExtensions\|FontCollectionExtensions]] |  |
| [[FontDescriptor\|FontDescriptor]] |  |
| [[FontsMauiAppBuilderExtensions\|FontsMauiAppBuilderExtensions]] |  |
| [[HandlerMauiAppBuilderExtensions\|HandlerMauiAppBuilderExtensions]] |  |
| [[HybridWebViewServiceCollectionExtensions\|HybridWebViewServiceCollectionExtensions]] | Extension methods to `IServiceCollection` for use with the HybridWebView. |
| [[ImageSourceServiceCollectionExtensions\|ImageSourceServiceCollectionExtensions]] |  |
| [[ImageSourcesMauiAppBuilderExtensions\|ImageSourcesMauiAppBuilderExtensions]] |  |
| [[MauiApp\|MauiApp]] | A .NET MAUI application with registered services and configuration data. |
| [[MauiAppBuilder\|MauiAppBuilder]] | A builder for .NET MAUI cross-platform applications and services. |
| [[MauiHandlersCollectionExtensions\|MauiHandlersCollectionExtensions]] |  |
| [[MauiHostEnvironment\|MauiHostEnvironment]] |  |

## Interfaces

| Type | Summary |
|---|---|
| [[IEssentialsBuilder\|IEssentialsBuilder]] |  |
| [[IFontCollection\|IFontCollection]] | A collection of fonts. |
| [[IImageSourceServiceCollection\|IImageSourceServiceCollection]] |  |
| [[IMauiHandlersCollection\|IMauiHandlersCollection]] |  |
| [[IMauiInitializeScopedService\|IMauiInitializeScopedService]] | Represents a service that is initialized during the window construction. |
| [[IMauiInitializeService\|IMauiInitializeService]] | Represents a service that is initialized during the application construction. |
| [[IMauiServiceCollection\|IMauiServiceCollection]] |  |

## See also

- [[_API Reference]]
