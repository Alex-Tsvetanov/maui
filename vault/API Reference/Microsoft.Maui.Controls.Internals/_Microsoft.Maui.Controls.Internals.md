---
title: "Microsoft.Maui.Controls.Internals"
tags:
  - api
  - namespace
  - ns/Microsoft-Maui-Controls-Internals
---

# Microsoft.Maui.Controls.Internals

> [!info] Namespace
> `Microsoft.Maui.Controls.Internals` — 65 public types.

- [Online namespace docs](https://learn.microsoft.com/dotnet/api/microsoft.maui.controls.internals)

## Overview

`Microsoft.Maui.Controls.Internals` holds the plumbing that the MAUI Controls layer and its platform renderers rely on but that is not part of the public, supported surface. The types here implement cross-cutting concerns — XAML name resolution, navigation proxying, dynamic resources, dependency resolution, alert/dialog argument passing, GIF decoding, and performance profiling — that the higher-level controls coordinate without exposing the details to app authors.

A large slice of the namespace is dialog and event glue: [[ActionSheetArguments]], [[AlertArguments]], and [[PromptArguments]] carry the configuration that platform code needs to raise native dialogs, while [[NavigationProxy]] and [[INavigationProxy|INavigationProxy]] route stack-based navigation requests through to the active page handler. XAML and resource infrastructure is provided by [[NameScope]] / [[INameScope|INameScope]] for resolving named elements, [[DynamicResource]] with [[IDynamicResourceHandler|IDynamicResourceHandler]] for keyed dynamic lookups, and [[DependencyResolver]] for pluggable dependency resolution.

The namespace also contains self-contained subsystems: a GIF pipeline ([[GIFImageParser]], [[GIFBitmapDecoder]], [[GIFHeader]], and related frame/color-table types) for animated-image support, and a lightweight profiling toolkit ([[Performance]], [[Profile]], [[ProfileDatum]], [[IPerformanceProvider|IPerformanceProvider]]) for measuring code execution. Because these are internal APIs, signatures and behavior can change between releases — treat them as reference for understanding MAUI internals rather than as a stable contract.

## Key types

- [[NameScope|NameScope]] — Provides XAML namescope functionality for resolving named elements.
- [[NavigationProxy|NavigationProxy]] — Handles stack-based navigation by proxying requests to the active handler.
- [[DependencyResolver|DependencyResolver]] — Static methods that register functions used for resolving dependencies.
- [[DynamicResource|DynamicResource]] — Represents a reference to a dynamic resource by key.
- [[AlertArguments|AlertArguments]] — Configuration arguments for displaying platform-specific alert dialogs.
- [[ActionSheetArguments|ActionSheetArguments]] — Arguments for an action sheet dialog.
- [[PromptArguments|PromptArguments]] — Arguments for a prompt dialog.
- [[GIFImageParser|GIFImageParser]] — Base class for parsing GIF image streams.
- [[GIFBitmapDecoder|GIFBitmapDecoder]] — Decodes LZW-compressed GIF image data.
- [[Performance|Performance]] — Internal profiling entry point for MAUI Controls.
- [[Profile|Profile]] — A disposable struct for profiling code execution.
- [[InvalidationTrigger|InvalidationTrigger]] — Flags indicating which property changes should trigger layout invalidation.

> [!info] Internal API
> Types in this namespace exist for the MAUI Controls runtime and platform renderers. They are not a supported public contract and may change without notice between releases.


## Classes

| Type | Summary |
|---|---|
| [[ActionSheetArguments\|ActionSheetArguments]] | Arguments for an action sheet dialog. |
| [[AlertArguments\|AlertArguments]] | Contains configuration arguments for displaying platform-specific alert dialogs. |
| [[AsyncValue{T}\|AsyncValue<T>]] |  |
| [[AsyncValueExtensions\|AsyncValueExtensions]] | Extension methods for creating AsyncValue instances. |
| [[AutoId\|AutoId]] |  |
| [[CellExtensions\|CellExtensions]] | Extension methods for working with cells in templated lists. |
| [[ContentPageEx\|ContentPageEx]] | Extension methods for displaying profiling data on a ContentPage. |
| [[DataTemplateExtensions\|DataTemplateExtensions]] | Extension methods for `DataTemplate` that support template selection. |
| [[Datum\|Datum]] |  |
| [[DependencyResolver\|DependencyResolver]] | Contains static methods that add functions to use for resolving dependencies. |
| [[DisposeMethod\|DisposeMethod]] |  |
| [[DynamicResource\|DynamicResource]] | Represents a reference to a dynamic resource by key. |
| [[EffectUtilities\|EffectUtilities]] | Utility methods for managing effect control providers. |
| [[EvalRequested\|EvalRequested]] | Event args for JavaScript evaluation requests in WebView. |
| [[EvaluateJavaScriptDelegate\|EvaluateJavaScriptDelegate]] |  |
| [[EventArg{T}\|EventArg<T>]] |  |
| [[ExpressionSearch\|ExpressionSearch]] | Internal API for Microsoft.Maui.Controls platform use. |
| [[GIFBitmap\|GIFBitmap]] | Represents a single frame in a GIF image. |
| [[GIFBitmapDecoder\|GIFBitmapDecoder]] | Decodes LZW-compressed GIF image data. |
| [[GIFColorTable\|GIFColorTable]] | Represents a GIF color table. |
| [[GIFDecoderFormatException\|GIFDecoderFormatException]] | Exception thrown when GIF data format is invalid. |
| [[GIFDecoderStreamReader\|GIFDecoderStreamReader]] | Reads bytes from a stream for GIF decoding. |
| [[GIFHeader\|GIFHeader]] | Represents the header of a GIF file. |
| [[GIFImageParser\|GIFImageParser]] | Base class for parsing GIF image streams. |
| [[NameScope\|NameScope]] | Provides XAML namescope functionality for resolving named elements. |
| [[NavigationProxy\|NavigationProxy]] | Represents an object capable of handling stack-based navigation via proxying. |
| [[NavigationRequestedEventArgs\|NavigationRequestedEventArgs]] | For internal use by platform renderers. |
| [[NotifyCollectionChangedEventArgsEx\|NotifyCollectionChangedEventArgsEx]] | For internal use by platform renderers. |
| [[NotifyCollectionChangedEventArgsExtensions\|NotifyCollectionChangedEventArgsExtensions]] | For internal use by platform renderers. |
| [[PageExtensions (Internals)\|PageExtensions (Internals)]] | Internal API for Microsoft.Maui.Controls platform use. |
| [[Performance\|Performance]] | Internal API for Microsoft.Maui.Controls platform use. |
| [[PreserveAttribute\|PreserveAttribute]] | For internal use by platform renderers. |
| [[ProfileDatum\|ProfileDatum]] | Stores profiling data for a single measured operation. |
| [[PromptArguments\|PromptArguments]] | Arguments for a prompt dialog. |
| [[PropertyPropagationExtensions\|PropertyPropagationExtensions]] | Extension methods for propagating property values through the visual tree. |
| [[Rect (Internals)\|Rect (Internals)]] |  |
| [[Registrar\|Registrar]] | Manages registration of renderers, effects, and other components. |
| [[Registrar{TRegistrable}\|Registrar<TRegistrable>]] | Manages registration of renderers, effects, and other components. |
| [[ResourceLoader\|ResourceLoader]] | Internal API for Microsoft.Maui.Controls platform use. |
| [[ResourceLoadingQuery\|ResourceLoadingQuery]] |  |
| [[ResourceLoadingResponse\|ResourceLoadingResponse]] |  |
| [[ResourcesChangedEventArgs\|ResourcesChangedEventArgs]] | Event arguments for resource dictionary changes. |
| [[SetValueFlags\|SetValueFlags]] |  |
| [[TableModel\|TableModel]] | Abstract base class that provides the data model for a `TableView`. |
| [[TemplatedItemsList{TView, TItem}\|TemplatedItemsList<TView, TItem>]] |  |
| [[TextTransformUtilites\|TextTransformUtilites]] |  |
| [[TextTransformUtilities\|TextTransformUtilities]] | A utilities class for text transformations. |
| [[TypedBinding{TSource, TProperty}\|TypedBinding<TSource, TProperty>]] | This factory method was added to simplify creating typed bindings for a property that isn't nested which is the most common scenario. This factory method mus… |
| [[TypedBindingBase\|TypedBindingBase]] | Provides the base class for type-safe bindings with compile-time property access. |

## Interfaces

| Type | Summary |
|---|---|
| [[IDataTemplateController\|IDataTemplateController]] |  |
| [[IDynamicResourceHandler\|IDynamicResourceHandler]] |  |
| [[IExpressionSearch\|IExpressionSearch]] |  |
| [[IFontElement\|IFontElement]] |  |
| [[IFontNamedSizeService\|IFontNamedSizeService]] |  |
| [[IGestureController\|IGestureController]] |  |
| [[INameScope\|INameScope]] |  |
| [[INavigationProxy\|INavigationProxy]] |  |
| [[IPerformanceProvider\|IPerformanceProvider]] |  |
| [[IPlatformSizeService\|IPlatformSizeService]] |  |
| [[IResourceDictionary\|IResourceDictionary]] |  |
| [[ISpatialElement\|ISpatialElement]] |  |
| [[ISystemResourcesProvider\|ISystemResourcesProvider]] |  |

## Structs

| Type | Summary |
|---|---|
| [[Profile\|Profile]] | A disposable struct for profiling code execution. |

## Enums

| Type | Summary |
|---|---|
| [[InvalidationTrigger\|InvalidationTrigger]] | Flags indicating which property changes should trigger layout invalidation. |
| [[NavigationRequestType\|NavigationRequestType]] | Specifies the type of navigation operation. |

## See also

- [[_API Reference]]
