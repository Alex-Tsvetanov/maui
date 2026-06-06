---
title: "Microsoft.Maui.Controls.Platform"
tags:
  - api
  - namespace
  - ns/Microsoft-Maui-Controls-Platform
---

# Microsoft.Maui.Controls.Platform

> [!info] Namespace
> `Microsoft.Maui.Controls.Platform` — 80 public types.

- [Online namespace docs](https://learn.microsoft.com/dotnet/api/microsoft.maui.controls.platform)

## Overview

`Microsoft.Maui.Controls.Platform` is the layer where cross-platform .NET MAUI controls meet the underlying native UI toolkits. It contains the glue code that translates abstract MAUI elements into concrete native widgets, plus the helpers, extension methods, and platform-specific renderers that make that translation possible. Most of these types are infrastructure rather than APIs you call directly from app code, but they are the seams that a handler, effect, or custom renderer hooks into when you need to reach beyond what the portable control surface exposes.

A large portion of the namespace is made up of *extension method* classes that adapt MAUI properties onto native views — for example [[EditTextExtensions (Platform)|EditTextExtensions]], [[LabelExtensions (Platform)|LabelExtensions]], [[ImageExtensions (Microsoft.Maui.Controls.Platform)|ImageExtensions]], [[FontExtensions (Microsoft.Maui.Controls.Platform)|FontExtensions]], and [[ViewExtensions (Platform)|ViewExtensions]]. These apply colors, fonts, text, and layout state from the shared MAUI model down to the platform control.

The namespace also hosts the bridge between the legacy renderer model and the new handler model. [[PlatformEffect|PlatformEffect]] is the platform-side base for MAUI effects, while [[ViewToHandlerConverter|ViewToHandlerConverter]], [[ElementChangedEventArgs{TElement}|ElementChangedEventArgs<TElement>]], and [[VisualElementChangedEventArgs|VisualElementChangedEventArgs]] support wiring elements to their platform counterparts and reacting to changes. Gesture support is concentrated in [[GestureHandler|GestureHandler]] and its specializations ([[PanGestureHandler|PanGestureHandler]], [[PinchGestureHandler|PinchGestureHandler]], [[SwipeGestureHandler|SwipeGestureHandler]], [[TapGestureHandler|TapGestureHandler]]), and Shell rendering is backed by a family of native Shell views and item views. Together these pieces let MAUI present a single control model on top of each platform's native UI stack.

> [!info] Mostly infrastructure
> Many types here are platform internals (for example the `*GradientShader` types are documented as internal-only and slated for removal). Reach for this namespace when building custom handlers, effects, or platform renderers — not for everyday app development.

## Key types

- [[PlatformEffect|PlatformEffect]] — platform-side base type for attaching MAUI effects to native controls.
- [[GestureHandler|GestureHandler]] — base for translating native gesture events into MAUI gestures.
- [[ViewToHandlerConverter|ViewToHandlerConverter]] — converts a MAUI view to its corresponding platform handler/view.
- [[ElementChangedEventArgs{TElement}|ElementChangedEventArgs<TElement>]] — event data raised when a renderer's bound element changes.
- [[VisualElementExtensions|VisualElementExtensions]] — extension helpers for working with `VisualElement` instances on a platform.
- [[ViewExtensions (Platform)|ViewExtensions]] — applies shared MAUI view state onto the native view.
- [[PlatformConfigurationExtensions|PlatformConfigurationExtensions]] — entry points for platform-specific configuration on MAUI controls.
- [[NavigationPageExtensions|NavigationPageExtensions]] — platform helpers for navigation page behavior.
- [[ItemTemplateContext|ItemTemplateContext]] — carries data-template context for item-based controls during platform rendering.
- [[ShellView|ShellView]] — native Shell host view used to render MAUI Shell.
- [[INavigationView|INavigationView]] — abstraction for a platform navigation view surface.
- [[IImageSourceHandler|IImageSourceHandler]] — contract for loading a MAUI image source into a native image.


## Classes

| Type | Summary |
|---|---|
| [[AccessKeyHelper\|AccessKeyHelper]] |  |
| [[AccessibilityExtensions (Platform)\|AccessibilityExtensions (Platform)]] |  |
| [[ActionSheetContent\|ActionSheetContent]] |  |
| [[AlertDialog\|AlertDialog]] |  |
| [[ApplicationExtensions (Platform)\|ApplicationExtensions (Platform)]] |  |
| [[BottomNavigationViewUtils\|BottomNavigationViewUtils]] |  |
| [[BrushExtensions\|BrushExtensions]] |  |
| [[ButtonExtensions (Microsoft.Maui.Controls.Platform)\|ButtonExtensions (Microsoft.Maui.Controls.Platform)]] |  |
| [[CollectionViewExtensions (Microsoft.Maui.Controls.Platform)\|CollectionViewExtensions (Microsoft.Maui.Controls.Platform)]] |  |
| [[ColorChangeRevealDrawable\|ColorChangeRevealDrawable]] |  |
| [[ControlsAccessibilityDelegate\|ControlsAccessibilityDelegate]] |  |
| [[EditTextExtensions (Platform)\|EditTextExtensions (Platform)]] |  |
| [[ElementChangedEventArgs{TElement}\|ElementChangedEventArgs<TElement>]] |  |
| [[ElevationHelper\|ElevationHelper]] |  |
| [[Extensions (Platform)\|Extensions (Platform)]] |  |
| [[FontExtensions (Microsoft.Maui.Controls.Platform)\|FontExtensions (Microsoft.Maui.Controls.Platform)]] |  |
| [[FormattedStringExtensions\|FormattedStringExtensions]] |  |
| [[GenericAnimatorListener\|GenericAnimatorListener]] |  |
| [[GeometryExtensions (Platform)\|GeometryExtensions (Platform)]] |  |
| [[GestureHandler\|GestureHandler]] |  |
| [[GradientShader\|GradientShader]] | Represents a gradient. This type is not meant to be used anywhere and is for internal use only. This type will be removed in the future. |
| [[GradientStrokeDrawable\|GradientStrokeDrawable]] |  |
| [[IconConverter\|IconConverter]] |  |
| [[ImageConverter\|ImageConverter]] |  |
| [[ImageExtensions (Microsoft.Maui.Controls.Platform)\|ImageExtensions (Microsoft.Maui.Controls.Platform)]] |  |
| [[InputViewExtensions\|InputViewExtensions]] |  |
| [[ItemContentControl\|ItemContentControl]] |  |
| [[ItemTemplateContext\|ItemTemplateContext]] |  |
| [[LabelExtensions (Platform)\|LabelExtensions (Platform)]] |  |
| [[LinearGradientShader\|LinearGradientShader]] | Represents a linear gradient. This type is not meant to be used anywhere and is for internal use only. This type will be removed in the future. |
| [[MauiCommandBar\|MauiCommandBar]] |  |
| [[MauiViewPager\|MauiViewPager]] |  |
| [[NavigationContentView\|NavigationContentView]] |  |
| [[NavigationPageExtensions\|NavigationPageExtensions]] |  |
| [[NavigationView\|NavigationView]] |  |
| [[PageControl\|PageControl]] |  |
| [[PageExtensions (Platform)\|PageExtensions (Platform)]] |  |
| [[PanGestureHandler\|PanGestureHandler]] |  |
| [[PickerExtensions (Platform)\|PickerExtensions (Platform)]] |  |
| [[PinchGestureHandler\|PinchGestureHandler]] |  |
| [[PlatformBindingExtensions\|PlatformBindingExtensions]] |  |
| [[PlatformConfigurationExtensions\|PlatformConfigurationExtensions]] |  |
| [[PlatformEffect\|PlatformEffect]] |  |
| [[PromptDialog\|PromptDialog]] |  |
| [[RadialGradientShader\|RadialGradientShader]] | Represents a radial gradient. This type is not meant to be used anywhere and is for internal use only. This type will be removed in the future. |
| [[RecyclerViewExtensions\|RecyclerViewExtensions]] |  |
| [[RefreshViewExtensions\|RefreshViewExtensions]] |  |
| [[ScrollViewExtensions (Platform)\|ScrollViewExtensions (Platform)]] |  |
| [[SearchBarExtensions (Microsoft.Maui.Controls.Platform)\|SearchBarExtensions (Microsoft.Maui.Controls.Platform)]] |  |
| [[SearchViewExtensions (Platform)\|SearchViewExtensions (Platform)]] |  |
| [[SemanticExtensions (Microsoft.Maui.Controls.Platform)\|SemanticExtensions (Microsoft.Maui.Controls.Platform)]] |  |
| [[ShapesExtensions (Microsoft.Maui.Controls.Platform)\|ShapesExtensions (Microsoft.Maui.Controls.Platform)]] |  |
| [[ShellFlyoutItemView\|ShellFlyoutItemView]] |  |
| [[ShellFlyoutTemplateSelector\|ShellFlyoutTemplateSelector]] |  |
| [[ShellFooterView\|ShellFooterView]] |  |
| [[ShellHeaderView\|ShellHeaderView]] |  |
| [[ShellItemView\|ShellItemView]] |  |
| [[ShellNavigationViewItem\|ShellNavigationViewItem]] |  |
| [[ShellNavigationViewItemAutomationPeer\|ShellNavigationViewItemAutomationPeer]] |  |
| [[ShellSearchViewItemSelectedEventArgs\|ShellSearchViewItemSelectedEventArgs]] |  |
| [[ShellSearchview (Platform)\|ShellSearchview (Platform)]] |  |
| [[ShellSectionStackManager\|ShellSectionStackManager]] |  |
| [[ShellSectionView\|ShellSectionView]] |  |
| [[ShellToolbarItemView\|ShellToolbarItemView]] |  |
| [[ShellView\|ShellView]] |  |
| [[SwipeGestureHandler\|SwipeGestureHandler]] |  |
| [[TapGestureHandler\|TapGestureHandler]] |  |
| [[TextExtensions\|TextExtensions]] |  |
| [[TextViewExtensions (Platform)\|TextViewExtensions (Platform)]] |  |
| [[TransformExtensions\|TransformExtensions]] |  |
| [[ViewExtensions (Platform)\|ViewExtensions (Platform)]] |  |
| [[ViewToHandlerConverter\|ViewToHandlerConverter]] |  |
| [[VisualElementChangedEventArgs\|VisualElementChangedEventArgs]] |  |
| [[VisualElementExtensions\|VisualElementExtensions]] |  |
| [[WebViewExtensions (Microsoft.Maui.Controls.Platform)\|WebViewExtensions (Microsoft.Maui.Controls.Platform)]] |  |

## Interfaces

| Type | Summary |
|---|---|
| [[IIconElementHandler\|IIconElementHandler]] |  |
| [[IImageSourceHandler\|IImageSourceHandler]] |  |
| [[INavigationContentView\|INavigationContentView]] |  |
| [[INavigationView\|INavigationView]] |  |
| [[ITabStop\|ITabStop]] |  |

## See also

- [[_API Reference]]
