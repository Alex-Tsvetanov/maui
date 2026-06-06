---
title: "Microsoft.Maui.Platform"
tags:
  - api
  - namespace
  - ns/Microsoft-Maui-Platform
---

# Microsoft.Maui.Platform

> [!info] Namespace
> `Microsoft.Maui.Platform` — 177 public types.

- [Online namespace docs](https://learn.microsoft.com/dotnet/api/microsoft.maui.platform)

## Overview

`Microsoft.Maui.Platform` is the native-platform glue layer that sits beneath .NET MAUI's cross-platform handlers. Where the abstract `IView`/handler model describes *what* a control should be, this namespace contains the platform-specific machinery that realizes it on each target — Android, iOS/Mac Catalyst, and Windows. It is the layer that translates MAUI's cross-platform properties into concrete native widget state.

Its largest group is the **`*Extensions` mapping helpers**. These are static extension methods (for example [[ButtonExtensions (Platform)|ButtonExtensions]], [[EntryExtensions|EntryExtensions]], [[LabelExtensions (Microsoft.Maui.Platform)|LabelExtensions]], [[ImageExtensions (Microsoft.Maui.Platform)|ImageExtensions]], and [[ColorExtensions (Platform)|ColorExtensions]]) that a handler calls to push a single cross-platform value — text, color, font, alignment — onto the underlying native control. [[ViewExtensions (Microsoft.Maui.Platform)|ViewExtensions]] provides shared layout, scaling, and rotation helpers for any `IView`.

The second group is the set of **`Maui*` native view subclasses**. Types such as [[MauiView|MauiView]], [[MauiScrollView|MauiScrollView]], [[MauiButton|MauiButton]], and [[MauiTextField|MauiTextField]] derive from native platform views (UIView, ViewGroup, FrameworkElement, etc.) and add MAUI's cross-platform measurement, safe-area, and layout behavior so native controls participate correctly in the MAUI layout system.

Rounding it out are conversion and infrastructure helpers — [[ColorConverter (Platform)|ColorConverter]], [[Culture|Culture]], container/navigation hosts, and small interfaces like [[IImageSourcePartSetter|IImageSourcePartSetter]] and [[IPlatformMeasureInvalidationController|IPlatformMeasureInvalidationController]] that coordinate image loading and measure invalidation. Most members here are consumed by MAUI handlers rather than called directly in application code, but they are the seam to reach for when customizing native rendering.

## Key types

- [[MauiView|MauiView]] — base class for MAUI views on iOS that adds cross-platform layout and safe-area handling on top of the native `UIView`.
- [[MauiScrollView|MauiScrollView]] — custom `UIScrollView` providing cross-platform layout support and safe-area management on iOS.
- [[ViewExtensions (Microsoft.Maui.Platform)|ViewExtensions]] — extension methods for `IView`s offering animatable scaling, rotation, and layout functions.
- [[IImageSourcePartSetter|IImageSourcePartSetter]] — represents an object that knows the desired image and how to apply a loaded version to a platform view.
- [[IPlatformMeasureInvalidationController|IPlatformMeasureInvalidationController]] — provides platform-specific measure invalidation control for iOS views.
- [[KeyboardAcceleratorExtensions|KeyboardAcceleratorExtensions]] — helper extension methods related to keyboard accelerators.
- [[ImageSourcePartLoader|ImageSourcePartLoader]] — coordinates loading an image source part and applying the result to a native image view.
- [[ColorConverter (Platform)|ColorConverter]] — converts MAUI colors to and from native platform color representations.
- [[Culture|Culture]] — culture/localization helpers used by platform formatting code.
- [[NavigationRootManager|NavigationRootManager]] — manages the root native navigation host that backs MAUI navigation.
- [[ContextExtensions|ContextExtensions]] — Android `Context` helpers used to bridge MAUI views into the native view tree.
- [[FontExtensions (Platform)|FontExtensions]] — applies MAUI font settings to native text controls.

## Related guides

- [[_API Reference|API Reference]]


## Classes

| Type | Summary |
|---|---|
| [[AccessibilityDelegateCompatWrapper\|AccessibilityDelegateCompatWrapper]] |  |
| [[ActivityIndicatorExtensions\|ActivityIndicatorExtensions]] |  |
| [[ActivityResultCallbackRegistry\|ActivityResultCallbackRegistry]] |  |
| [[AlignmentExtensions\|AlignmentExtensions]] |  |
| [[ApplicationExtensions (Microsoft.Maui.Platform)\|ApplicationExtensions (Microsoft.Maui.Platform)]] |  |
| [[AspectExtensions\|AspectExtensions]] |  |
| [[AttributedStringExtensions\|AttributedStringExtensions]] |  |
| [[BorderDrawable\|BorderDrawable]] |  |
| [[BorderExtensions\|BorderExtensions]] |  |
| [[ButtonExtensions (Platform)\|ButtonExtensions (Platform)]] |  |
| [[CalendarDatePickerExtensions\|CalendarDatePickerExtensions]] |  |
| [[CharacterSpacingExtensions\|CharacterSpacingExtensions]] |  |
| [[CheckBoxExtensions\|CheckBoxExtensions]] |  |
| [[CollectionViewExtensions (Platform)\|CollectionViewExtensions (Platform)]] |  |
| [[ColorConverter (Platform)\|ColorConverter (Platform)]] |  |
| [[ColorExtensions (Platform)\|ColorExtensions (Platform)]] |  |
| [[ContainerView (Platform)\|ContainerView (Platform)]] |  |
| [[ContainerViewController\|ContainerViewController]] |  |
| [[ContentPanel\|ContentPanel]] |  |
| [[ContentView (Platform)\|ContentView (Platform)]] |  |
| [[ContentViewGroup\|ContentViewGroup]] |  |
| [[ContextExtensions\|ContextExtensions]] |  |
| [[ControlExtensions\|ControlExtensions]] |  |
| [[CoreGraphicsExtensions (Platform)\|CoreGraphicsExtensions (Platform)]] |  |
| [[Culture\|Culture]] |  |
| [[DPExtensions\|DPExtensions]] |  |
| [[DateExtensions\|DateExtensions]] |  |
| [[DatePickerExtensions\|DatePickerExtensions]] |  |
| [[DrawableExtensions\|DrawableExtensions]] |  |
| [[EditTextExtensions (Microsoft.Maui.Platform)\|EditTextExtensions (Microsoft.Maui.Platform)]] |  |
| [[EditorExtensions\|EditorExtensions]] |  |
| [[ElementExtensions\|ElementExtensions]] |  |
| [[EnergySaverListenerManager\|EnergySaverListenerManager]] |  |
| [[EntryExtensions\|EntryExtensions]] |  |
| [[FlowDirectionExtensions\|FlowDirectionExtensions]] |  |
| [[FlyoutViewExtensions\|FlyoutViewExtensions]] |  |
| [[FontExtensions (Platform)\|FontExtensions (Platform)]] |  |
| [[GraphicsExtensions (Microsoft.Maui.Platform)\|GraphicsExtensions (Microsoft.Maui.Platform)]] |  |
| [[GraphicsViewExtensions\|GraphicsViewExtensions]] |  |
| [[ImageButtonExtensions\|ImageButtonExtensions]] |  |
| [[ImageExtensions (Microsoft.Maui.Platform)\|ImageExtensions (Microsoft.Maui.Platform)]] |  |
| [[ImageSourcePartExtensions\|ImageSourcePartExtensions]] |  |
| [[ImageSourcePartLoader\|ImageSourcePartLoader]] |  |
| [[ImageViewExtensions\|ImageViewExtensions]] |  |
| [[ImeActionExtensions (Platform)\|ImeActionExtensions (Platform)]] |  |
| [[KeyboardAcceleratorExtensions\|KeyboardAcceleratorExtensions]] | Group of helper extension methods related to KeyboardAccelerators. |
| [[KeyboardAutoManagerScroll\|KeyboardAutoManagerScroll]] |  |
| [[KeyboardExtensions\|KeyboardExtensions]] |  |
| [[LabelExtensions (Microsoft.Maui.Platform)\|LabelExtensions (Microsoft.Maui.Platform)]] |  |
| [[LayerExtensions\|LayerExtensions]] |  |
| [[LayoutCanvasExtensions\|LayoutCanvasExtensions]] |  |
| [[LayoutChangedEventArgs\|LayoutChangedEventArgs]] |  |
| [[LayoutPanel\|LayoutPanel]] |  |
| [[LayoutPanelExtensions\|LayoutPanelExtensions]] |  |
| [[LayoutView\|LayoutView]] |  |
| [[LayoutViewExtensions\|LayoutViewExtensions]] |  |
| [[LayoutViewGroup\|LayoutViewGroup]] |  |
| [[LayoutViewGroupExtensions\|LayoutViewGroupExtensions]] |  |
| [[LocalizedDigitsKeyListener\|LocalizedDigitsKeyListener]] |  |
| [[MauiAccessibilityDelegateCompat\|MauiAccessibilityDelegateCompat]] |  |
| [[MauiActivityIndicator\|MauiActivityIndicator]] |  |
| [[MauiAppCompatEditText\|MauiAppCompatEditText]] |  |
| [[MauiAutoSuggestBox\|MauiAutoSuggestBox]] |  |
| [[MauiBoxView\|MauiBoxView]] |  |
| [[MauiButton\|MauiButton]] |  |
| [[MauiButtonAutomationPeer\|MauiButtonAutomationPeer]] |  |
| [[MauiCALayer\|MauiCALayer]] |  |
| [[MauiCancelButton\|MauiCancelButton]] |  |
| [[MauiCheckBox\|MauiCheckBox]] |  |
| [[MauiClipperView\|MauiClipperView]] |  |
| [[MauiDatePicker\|MauiDatePicker]] |  |
| [[MauiDateTimePicker\|MauiDateTimePicker]] |  |
| [[MauiFlyoutView\|MauiFlyoutView]] |  |
| [[MauiHorizontalScrollView\|MauiHorizontalScrollView]] |  |
| [[MauiHybridWebView\|MauiHybridWebView]] |  |
| [[MauiHybridWebViewClient\|MauiHybridWebViewClient]] |  |
| [[MauiImageButton\|MauiImageButton]] |  |
| [[MauiImageSource\|MauiImageSource]] |  |
| [[MauiImageView\|MauiImageView]] |  |
| [[MauiLabel\|MauiLabel]] |  |
| [[MauiMaterialButton\|MauiMaterialButton]] |  |
| [[MauiNavigationView\|MauiNavigationView]] |  |
| [[MauiPageControl\|MauiPageControl]] |  |
| [[MauiPanel\|MauiPanel]] |  |
| [[MauiPasswordTextBox\|MauiPasswordTextBox]] |  |
| [[MauiPicker\|MauiPicker]] |  |
| [[MauiPickerBase\|MauiPickerBase]] |  |
| [[MauiRefreshLayout\|MauiRefreshLayout]] |  |
| [[MauiRefreshView\|MauiRefreshView]] |  |
| [[MauiScrollView\|MauiScrollView]] | A custom UIScrollView implementation that provides cross-platform layout support and safe area management for .NET MAUI applications on iOS. This class handl… |
| [[MauiSearchBar\|MauiSearchBar]] |  |
| [[MauiSearchView\|MauiSearchView]] |  |
| [[MauiShapeView\|MauiShapeView]] |  |
| [[MauiShapeableImageView\|MauiShapeableImageView]] |  |
| [[MauiSlider\|MauiSlider]] |  |
| [[MauiStepper\|MauiStepper]] |  |
| [[MauiSwipeRefreshLayout\|MauiSwipeRefreshLayout]] |  |
| [[MauiSwipeView\|MauiSwipeView]] |  |
| [[MauiTVFlyoutView\|MauiTVFlyoutView]] |  |
| [[MauiTextBox\|MauiTextBox]] |  |
| [[MauiTextField\|MauiTextField]] |  |
| [[MauiTextView\|MauiTextView]] |  |
| [[MauiTimePicker\|MauiTimePicker]] |  |
| [[MauiToolbar\|MauiToolbar]] |  |
| [[MauiView\|MauiView]] | Base class for MAUI views on iOS that provides cross-platform layout capabilities and safe area handling. This view bridges the gap between iOS native UIView… |
| [[MauiWKWebView\|MauiWKWebView]] |  |
| [[MauiWebChromeClient\|MauiWebChromeClient]] |  |
| [[MauiWebView\|MauiWebView]] |  |
| [[MauiWebViewClient\|MauiWebViewClient]] |  |
| [[MauiWebViewNavigationDelegate\|MauiWebViewNavigationDelegate]] |  |
| [[MauiWebViewUIDelegate\|MauiWebViewUIDelegate]] |  |
| [[MeasureSpecExtensions\|MeasureSpecExtensions]] |  |
| [[MenuExtensions\|MenuExtensions]] |  |
| [[NavigationRootManager\|NavigationRootManager]] |  |
| [[NavigationStackExtensions\|NavigationStackExtensions]] |  |
| [[NavigationViewExtensions\|NavigationViewExtensions]] |  |
| [[NavigationViewFragment\|NavigationViewFragment]] |  |
| [[NoCaretField\|NoCaretField]] |  |
| [[PageViewController\|PageViewController]] |  |
| [[PickerExtensions (Microsoft.Maui.Platform)\|PickerExtensions (Microsoft.Maui.Platform)]] |  |
| [[PlatformTouchGraphicsView\|PlatformTouchGraphicsView]] |  |
| [[PrimitiveExtensions\|PrimitiveExtensions]] |  |
| [[ProgressBarExtensions\|ProgressBarExtensions]] |  |
| [[RadioButtonExtensions\|RadioButtonExtensions]] |  |
| [[ReturnTypeExtensions\|ReturnTypeExtensions]] |  |
| [[RootNavigationView\|RootNavigationView]] |  |
| [[RootPanel\|RootPanel]] |  |
| [[ScrollViewExtensions (Microsoft.Maui.Platform)\|ScrollViewExtensions (Microsoft.Maui.Platform)]] |  |
| [[ScrollViewerExtensions\|ScrollViewerExtensions]] |  |
| [[SearchBarExtensions (Platform)\|SearchBarExtensions (Platform)]] |  |
| [[SearchViewExtensions (Microsoft.Maui.Platform)\|SearchViewExtensions (Microsoft.Maui.Platform)]] |  |
| [[SemanticExtensions (Platform)\|SemanticExtensions (Platform)]] |  |
| [[ShapeExtensions\|ShapeExtensions]] |  |
| [[ShapeViewExtensions\|ShapeViewExtensions]] |  |
| [[ShapesExtensions (Platform)\|ShapesExtensions (Platform)]] |  |
| [[SizeExtensions\|SizeExtensions]] |  |
| [[SliderExtensions\|SliderExtensions]] |  |
| [[StackNavigationManager\|StackNavigationManager]] |  |
| [[StepperExtensions\|StepperExtensions]] |  |
| [[StepperHandlerHolder\|StepperHandlerHolder]] |  |
| [[StepperHandlerManager\|StepperHandlerManager]] |  |
| [[StrokeExtensions\|StrokeExtensions]] |  |
| [[SwipeViewExtensions\|SwipeViewExtensions]] |  |
| [[SwitchExtensions\|SwitchExtensions]] |  |
| [[TextAlignmentExtensions\|TextAlignmentExtensions]] |  |
| [[TextAlignmentToHorizontalAlignmentConverter\|TextAlignmentToHorizontalAlignmentConverter]] |  |
| [[TextBlockExtensions\|TextBlockExtensions]] |  |
| [[TextBoxExtensions\|TextBoxExtensions]] |  |
| [[TextElementExtensions\|TextElementExtensions]] |  |
| [[TextFieldExtensions\|TextFieldExtensions]] |  |
| [[TextInputExtensions\|TextInputExtensions]] |  |
| [[TextViewExtensions (Microsoft.Maui.Platform)\|TextViewExtensions (Microsoft.Maui.Platform)]] |  |
| [[ThemeExtensions\|ThemeExtensions]] |  |
| [[TimeExtensions\|TimeExtensions]] |  |
| [[TimePickerExtensions\|TimePickerExtensions]] |  |
| [[ToolbarExtensions\|ToolbarExtensions]] |  |
| [[TransformationExtensions\|TransformationExtensions]] |  |
| [[UIApplicationExtensions\|UIApplicationExtensions]] |  |
| [[UIEdgeInsetsExtensions\|UIEdgeInsetsExtensions]] |  |
| [[UIPageControlExtensions\|UIPageControlExtensions]] |  |
| [[UIWindowExtensions\|UIWindowExtensions]] |  |
| [[UnitExtensions\|UnitExtensions]] |  |
| [[ViewExtensions (Microsoft.Maui.Platform)\|ViewExtensions (Microsoft.Maui.Platform)]] | Extension methods for `IView`s, providing animatable scaling, rotation, and layout functions. |
| [[ViewGroupExtensions\|ViewGroupExtensions]] |  |
| [[WebViewExtensions (Platform)\|WebViewExtensions (Platform)]] |  |
| [[WindowExtensions (Platform)\|WindowExtensions (Platform)]] |  |
| [[WindowRootView\|WindowRootView]] |  |
| [[WindowsPlatformMessageEventArgs\|WindowsPlatformMessageEventArgs]] |  |
| [[WindowsPlatformWindowSubclassedEventArgs\|WindowsPlatformWindowSubclassedEventArgs]] |  |
| [[WrapperView\|WrapperView]] |  |

## Interfaces

| Type | Summary |
|---|---|
| [[IAndroidStepperHandler\|IAndroidStepperHandler]] |  |
| [[IAutoSizableCALayer\|IAutoSizableCALayer]] |  |
| [[IEnergySaverListener\|IEnergySaverListener]] |  |
| [[IEnergySaverListenerManager\|IEnergySaverListenerManager]] |  |
| [[IImageSourcePartSetter\|IImageSourcePartSetter]] | This represents a object that knows what the desired image is and how to apply a loaded version of the image to a platform view. |
| [[IPlatformMeasureInvalidationController\|IPlatformMeasureInvalidationController]] | Provides platform-specific measure invalidation control for iOS views. |
| [[IToolbarContainer\|IToolbarContainer]] |  |

## See also

- [[_API Reference]]
