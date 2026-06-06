---
title: "Microsoft.Maui"
tags:
  - api
  - namespace
  - ns/Microsoft-Maui
---

# Microsoft.Maui

> [!info] Namespace
> `Microsoft.Maui` — 248 public types.

- [Online namespace docs](https://learn.microsoft.com/dotnet/api/microsoft.maui)

## Overview

`Microsoft.Maui` is the core abstraction layer of .NET MAUI. Rather than concrete controls, it defines the cross-platform contracts — the "virtual views" — that describe what a control *is* (its properties and behavior), leaving each platform's native implementation to the handler layer. An application is modeled by [[IApplication|IApplication]], which manages one or more [[IWindow|IWindow]] surfaces, and every visual element on screen ultimately implements [[IView|IView]] (built on the lighter [[IElement|IElement]]). Interface contracts such as [[IButton|IButton]], [[ILabel|ILabel]], [[IEntry|IEntry]], [[IImage (Maui)|IImage]], [[ILayout (Maui)|ILayout]], and [[IScrollView|IScrollView]] specify the behavior that controls expose to the framework.

The bridge from these abstractions to native UI is the handler architecture. [[IViewHandler|IViewHandler]] and [[IElementHandler|IElementHandler]] define how a virtual view is mapped to a platform view, and [[PropertyMapper|PropertyMapper]] / [[CommandMapper|CommandMapper]] declare which properties and commands flow to the native control and how. Shared services — fonts via [[FontManager|FontManager]] and [[IFontManager|IFontManager]], and images via the [[ImageSourceService|ImageSourceService]] family — are resolved through [[IMauiContext|IMauiContext]], the per-window dependency and platform context.

The namespace also supplies the value types and enums that controls are configured with: layout primitives like [[Thickness|Thickness]], [[CornerRadius|CornerRadius]], [[GridLength|GridLength]], and [[SizeRequest|SizeRequest]]; typography via [[Font (Maui)|Font]]; and behavior enums such as [[Aspect|Aspect]], [[FlowDirection|FlowDirection]], and [[ScrollOrientation|ScrollOrientation]]. Platform application entry points (for Android, iOS/Mac Catalyst, and Windows) and diagnostics helpers like [[VisualDiagnostics|VisualDiagnostics]] round out the surface.

## Key types

- [[IView|IView]] — the visual element contract for everything placed on screen; built on [[IElement|IElement]].
- [[IApplication|IApplication]] — represents a cross-platform .NET MAUI application and its windows.
- [[IWindow|IWindow]] — create, configure, show, and manage application windows.
- [[IViewHandler|IViewHandler]] — maps a virtual view to a platform view; core of the handler architecture.
- [[IElementHandler|IElementHandler]] — core behavior for creating a custom element handler.
- [[PropertyMapper|PropertyMapper]] — declares which virtual-view properties propagate to the native control.
- [[CommandMapper|CommandMapper]] — declares which commands/actions are forwarded to the native control.
- [[IMauiContext|IMauiContext]] — per-window platform and dependency-injection context.
- [[FontManager|FontManager]] — handles fonts, font families, and font sizes across the app.
- [[ImageSourceService|ImageSourceService]] — base service that loads image sources for native views.
- [[Thickness|Thickness]] — defines thickness for each edge of a rectangle (margins, padding, borders).
- [[GridLength|GridLength]] — defines the size of a Grid row or column.


## Classes

| Type | Summary |
|---|---|
| [[ActivationState\|ActivationState]] |  |
| [[ActivityLifecycleCallbacks\|ActivityLifecycleCallbacks]] |  |
| [[CommandMapper\|CommandMapper]] |  |
| [[CommandMapper{TVirtualView, TViewHandler}\|CommandMapper<TVirtualView, TViewHandler>]] |  |
| [[CommandMapper{TVirtualView}\|CommandMapper<TVirtualView>]] |  |
| [[CommandMapperExtensions\|CommandMapperExtensions]] |  |
| [[Crc64\|Crc64]] |  |
| [[CustomKeyboard\|CustomKeyboard]] |  |
| [[DisplayDensityRequest\|DisplayDensityRequest]] |  |
| [[Easing\|Easing]] | Functions that modify values non-linearly, generally used for animations. |
| [[ElementHandlerExtensions\|ElementHandlerExtensions]] |  |
| [[EmbeddedFont\|EmbeddedFont]] | Represents a font that is added as an embedded resource in the application. |
| [[EmbeddedFontLoader\|EmbeddedFontLoader]] |  |
| [[EvaluateJavaScriptAsyncRequest\|EvaluateJavaScriptAsyncRequest]] | Specifies JavasScript to be evaluated by a platform web view control |
| [[FileImageSourceService\|FileImageSourceService]] |  |
| [[FileSystemEmbeddedFontLoader\|FileSystemEmbeddedFontLoader]] |  |
| [[FilterMode\|FilterMode]] |  |
| [[FocusRequest\|FocusRequest]] |  |
| [[FontFile\|FontFile]] | Represents a font file. |
| [[FontImageSourceService\|FontImageSourceService]] |  |
| [[FontManager\|FontManager]] |  |
| [[FontRegistrar\|FontRegistrar]] |  |
| [[FontSlant\|FontSlant]] |  |
| [[FontWeight (Maui)\|FontWeight (Maui)]] |  |
| [[HandlerDisconnectPolicy\|HandlerDisconnectPolicy]] |  |
| [[HybridWebViewInvokeJavaScriptRequest\|HybridWebViewInvokeJavaScriptRequest]] |  |
| [[HybridWebViewRawMessage\|HybridWebViewRawMessage]] | Represents a raw message received used the HybridWebView. |
| [[ImageSourceExtensions\|ImageSourceExtensions]] |  |
| [[ImageSourceService\|ImageSourceService]] |  |
| [[ImageSourceServiceLoadResult\|ImageSourceServiceLoadResult]] |  |
| [[ImageSourceServiceProviderExtensions\|ImageSourceServiceProviderExtensions]] |  |
| [[ImageSourceServiceResult\|ImageSourceServiceResult]] |  |
| [[ItemDelegateList{T}\|ItemDelegateList<T>]] |  |
| [[Keyboard\|Keyboard]] | Default keyboard and base class for specialized keyboards, such as those for telephone numbers, email, and URLs. |
| [[LockableObservableListWrapper\|LockableObservableListWrapper]] |  |
| [[MauiAppCompatActivity\|MauiAppCompatActivity]] |  |
| [[MauiApplication\|MauiApplication]] | Defines the core behavior of a .NET MAUI application running on Android. |
| [[MauiContext\|MauiContext]] |  |
| [[MauiDrawable (Maui)\|MauiDrawable (Maui)]] |  |
| [[MauiIndicatorViewExtensions\|MauiIndicatorViewExtensions]] |  |
| [[MauiUIApplicationDelegate\|MauiUIApplicationDelegate]] | Defines the core behavior of a .NET MAUI application running on iOS and MacCatalyst. |
| [[MauiUISceneDelegate\|MauiUISceneDelegate]] |  |
| [[MauiViewGroup\|MauiViewGroup]] |  |
| [[MauiWinUIApplication\|MauiWinUIApplication]] | Defines the core behavior of a .NET MAUI application running on Windows. |
| [[MauiWinUIWindow\|MauiWinUIWindow]] |  |
| [[MissingMapperAttribute\|MissingMapperAttribute]] |  |
| [[NaviPage\|NaviPage]] |  |
| [[NavigationRequest\|NavigationRequest]] |  |
| [[PathAspect\|PathAspect]] |  |
| [[PersistedState\|PersistedState]] |  |
| [[PlatformAppCompatTextView\|PlatformAppCompatTextView]] |  |
| [[PlatformContentViewGroup\|PlatformContentViewGroup]] |  |
| [[PlatformViewGroup\|PlatformViewGroup]] |  |
| [[PlatformWrapperView\|PlatformWrapperView]] |  |
| [[PortHandlerAttribute\|PortHandlerAttribute]] |  |
| [[PropertyMapper\|PropertyMapper]] |  |
| [[PropertyMapper{TVirtualView, TViewHandler}\|PropertyMapper<TVirtualView, TViewHandler>]] |  |
| [[PropertyMapper{TVirtualView}\|PropertyMapper<TVirtualView>]] |  |
| [[PropertyMapperExtensions\|PropertyMapperExtensions]] |  |
| [[RectangleAdorner\|RectangleAdorner]] | Rectangle Adorner. |
| [[RectangleGridAdorner\|RectangleGridAdorner]] | Rectangle Grid Adorner. |
| [[Resource (Maui)\|Resource (Maui)]] |  |
| [[RetrievePlatformValueRequest{T}\|RetrievePlatformValueRequest<T>]] | Specifies a request for the retrieval of a platform value. |
| [[ScrollToRequest\|ScrollToRequest]] |  |
| [[SemanticExtensions (Maui)\|SemanticExtensions (Maui)]] |  |
| [[SemanticHeadingLevel\|SemanticHeadingLevel]] |  |
| [[Semantics\|Semantics]] |  |
| [[SoftInputExtensions\|SoftInputExtensions]] | Extension methods for interacting with a platform's Soft Input Pane |
| [[SourceInfo\|SourceInfo]] | The source info for a given object. Used for locating where a given object is created in a given project. |
| [[StreamImageSourceService\|StreamImageSourceService]] |  |
| [[SwipeTransitionMode\|SwipeTransitionMode]] |  |
| [[SwipeViewCloseRequest\|SwipeViewCloseRequest]] |  |
| [[SwipeViewOpenRequest\|SwipeViewOpenRequest]] |  |
| [[SwipeViewSwipeChanging\|SwipeViewSwipeChanging]] |  |
| [[SwipeViewSwipeEnded\|SwipeViewSwipeEnded]] |  |
| [[SwipeViewSwipeStarted\|SwipeViewSwipeStarted]] |  |
| [[ToolTip\|ToolTip]] |  |
| [[UriImageSourceService\|UriImageSourceService]] |  |
| [[ViewExtensions (Maui)\|ViewExtensions (Maui)]] |  |
| [[Visibility\|Visibility]] |  |
| [[VisualDiagnostics\|VisualDiagnostics]] | Provides APIs for capturing source information, monitoring visual tree changes, and capturing screenshots for XAML and UI diagnostics. |
| [[VisualDiagnosticsOverlay\|VisualDiagnosticsOverlay]] | Visual Diagnostics Overlay. |
| [[VisualTreeChangeEventArgs\|VisualTreeChangeEventArgs]] | Provides data for changes in the visual tree, such as when a child is added or removed. |
| [[VisualTreeElementExtensions\|VisualTreeElementExtensions]] |  |
| [[WeakEventManager\|WeakEventManager]] | Manages weak event subscriptions, preventing memory leaks by maintaining weak references to handlers. |
| [[WebProcessTerminatedEventArgs\|WebProcessTerminatedEventArgs]] |  |
| [[WebResourceRequestedEventArgs\|WebResourceRequestedEventArgs]] | Provides platform-specific information for the `WebResourceRequested` event. |
| [[WebViewInitializationCompletedEventArgs\|WebViewInitializationCompletedEventArgs]] | Provides platform-specific information for the `WebViewInitializationCompleted` event. |
| [[WebViewInitializationStartedEventArgs\|WebViewInitializationStartedEventArgs]] | Provides platform-specific information for the `WebViewInitializationStarted` event. |
| [[WindowExtensions (Maui)\|WindowExtensions (Maui)]] |  |
| [[WindowOverlay\|WindowOverlay]] |  |
| [[WindowOverlayTappedEventArgs\|WindowOverlayTappedEventArgs]] |  |

## Interfaces

| Type | Summary |
|---|---|
| [[IAbsoluteLayout\|IAbsoluteLayout]] | A Layout used to position and size children using explicit values. |
| [[IActivationState\|IActivationState]] |  |
| [[IActivityIndicator\|IActivityIndicator]] | Represents a View that displays an animation to show that the application is engaged in a lengthy activity. |
| [[IAdorner\|IAdorner]] | Represents an adorner around a view. |
| [[IApplication\|IApplication]] | Class that represents a cross-platform .NET MAUI application. |
| [[IBorder\|IBorder]] | Provides functionality to provide a border. |
| [[IBorderStroke\|IBorderStroke]] | Define how the Shape outline is painted on Layouts. |
| [[IBorderView\|IBorderView]] | Provides functionality to define a border around an element. |
| [[IButton\|IButton]] | Represents a `IView` that reacts to touch events. |
| [[IButtonStroke\|IButtonStroke]] | Provides functionality to be able to customize a Button border. |
| [[ICheckBox\|ICheckBox]] | Represents a View which allows the user to select a binary choice. |
| [[ICommandMapper\|ICommandMapper]] |  |
| [[ICommandMapper{TVirtualView, TViewHandler}\|ICommandMapper<TVirtualView, TViewHandler>]] |  |
| [[IContainer\|IContainer]] | Provides functionality to act as containers for views. |
| [[IContentView\|IContentView]] | A View that contains another View. |
| [[IContextFlyoutElement\|IContextFlyoutElement]] | Represents a view that can contain a context flyout menu, which is usually represented as a right-click menu. |
| [[ICrossPlatformLayout\|ICrossPlatformLayout]] |  |
| [[ICrossPlatformLayoutBacking\|ICrossPlatformLayoutBacking]] | Indicates a control which supports cross-platform layout operations |
| [[IDatePicker\|IDatePicker]] | Represents a `IView` that allows the user to select a date. |
| [[IEditor\|IEditor]] | Represents a View used to accept multi-line input. |
| [[IElement\|IElement]] |  |
| [[IElementHandler\|IElementHandler]] | Defines the core behavior necessary to create a custom element handler. Conceptual documentation on handlers |
| [[IEmbeddedFontLoader\|IEmbeddedFontLoader]] | The `IEmbeddedFontLoader` is able to load a font from the embedded resources. |
| [[IEntry\|IEntry]] | Represents a `IView` that is used for single-line text input. |
| [[IFileImageSource\|IFileImageSource]] |  |
| [[IFlexLayout\|IFlexLayout]] | A Flexbox-like layout that lays out child elements in optionally wrappable rows or columns of child elements. |
| [[IFlyout\|IFlyout]] |  |
| [[IFlyoutView\|IFlyoutView]] | Represents a Page that manages two panes of information: A master page that presents data at a high level, and a detail page that displays low-level details … |
| [[IFontImageSource\|IFontImageSource]] |  |
| [[IFontManager\|IFontManager]] | The `FontManager` handles all fonts, font families and font sizes throughout the application. |
| [[IFontRegistrar\|IFontRegistrar]] | The `IFontRegistrar` keeps track of the fonts that are registered in our application. |
| [[IGraphicsView\|IGraphicsView]] | Represents a view that can be drawn on using drawing commands. |
| [[IGridColumnDefinition\|IGridColumnDefinition]] | Provides the properties for a column in a GridLayout. |
| [[IGridLayout\|IGridLayout]] | Represents a layout that arranges views in rows and columns. |
| [[IGridRowDefinition\|IGridRowDefinition]] | Provides the properties for a row in a GridLayout. |
| [[IHybridWebView\|IHybridWebView]] |  |
| [[IImage (Maui)\|IImage (Maui)]] | Represents a View that holds an image. |
| [[IImageButton\|IImageButton]] | Represents a View that reacts to touch events. |
| [[IImageLoaderCallback\|IImageLoaderCallback]] |  |
| [[IImageSource\|IImageSource]] |  |
| [[IImageSourcePart\|IImageSourcePart]] | Gets the scaling mode for the image. |
| [[IImageSourcePartEvents\|IImageSourcePartEvents]] |  |
| [[IImageSourceService\|IImageSourceService]] |  |
| [[IImageSourceService{T}\|IImageSourceService<T>]] |  |
| [[IImageSourceServiceProvider\|IImageSourceServiceProvider]] |  |
| [[IImageSourceServiceResult\|IImageSourceServiceResult]] |  |
| [[IImageSourceServiceResult{T}\|IImageSourceServiceResult<T>]] |  |
| [[IIndicatorView\|IIndicatorView]] | A view that displays indicators that represent the number of items, and current position |
| [[IInitializationAwareWebView\|IInitializationAwareWebView]] |  |
| [[IItemDelegate{T}\|IItemDelegate<T>]] |  |
| [[IKeyboardAccelerator\|IKeyboardAccelerator]] | Represents a shortcut key for a `MenuFlyoutItem`. |
| [[ILabel\|ILabel]] | Represents a View that displays text. |
| [[ILayout (Maui)\|ILayout (Maui)]] | Provides the base properties and methods for all Layout elements. Use Layout elements to position and size child elements in .NET MAUI applications. |
| [[ILayoutHandler\|ILayoutHandler]] |  |
| [[IMauiContext\|IMauiContext]] |  |
| [[IMauiFactory\|IMauiFactory]] |  |
| [[IMauiHandlersFactory\|IMauiHandlersFactory]] |  |
| [[IMenuBar\|IMenuBar]] | Represents a specialized container that presents a set of menus in a horizontal row, typically at the top of an app window. |
| [[IMenuBarElement\|IMenuBarElement]] |  |
| [[IMenuBarItem\|IMenuBarItem]] | Represents a top-level menu in a MenuBar view. |
| [[IMenuElement\|IMenuElement]] |  |
| [[IMenuFlyout\|IMenuFlyout]] | Represents a flyout that displays a menu of commands. |
| [[IMenuFlyoutItem\|IMenuFlyoutItem]] | Represents a command in a MenuFlyout view. |
| [[IMenuFlyoutSeparator\|IMenuFlyoutSeparator]] | Represents a horizontal line that separates items in a MenuFlyout. |
| [[IMenuFlyoutSubItem\|IMenuFlyoutSubItem]] | Represents a menu item that displays a sub-menu in a MenuFlyout view. |
| [[IPadding\|IPadding]] | Provides functionality to be able to customize Padding. |
| [[IPersistedState\|IPersistedState]] |  |
| [[IPicker\|IPicker]] | Represents a View for selecting a text item from a list of data. |
| [[IPickerExtension\|IPickerExtension]] |  |
| [[IPlaceholder\|IPlaceholder]] | Provides functionality to be able to use a Placeholder. |
| [[IPlatformApplication\|IPlatformApplication]] | Represents the platform-specific application instance that hosts a .NET MAUI application. |
| [[IPlatformViewHandler\|IPlatformViewHandler]] |  |
| [[IProgress\|IProgress]] | Represents a View that show progress as a horizontal bar that is filled to a percentage represented by a float value. |
| [[IPropertyMapper\|IPropertyMapper]] |  |
| [[IPropertyMapper{TVirtualView, TViewHandler}\|IPropertyMapper<TVirtualView, TViewHandler>]] |  |
| [[IPropertyMapperView\|IPropertyMapperView]] |  |
| [[IRadioButton\|IRadioButton]] | Represents a View that provides a toggled value. |
| [[IRange\|IRange]] | Provides functionality to select a value from a range of values. |
| [[IRefreshView\|IRefreshView]] | Represents a container that provides pull to refresh functionality for scrollable content. |
| [[IReplaceableView\|IReplaceableView]] |  |
| [[ISafeAreaView\|ISafeAreaView]] | Provides functionality for requesting layout outside of the "safe" areas of the device screen. |
| [[IScrollView\|IScrollView]] |  |
| [[ISearchBar\|ISearchBar]] | Represents a View used to initiating a search. |
| [[IShadow\|IShadow]] | Represents a Shadow that can be applied to a View. |
| [[IShapeView\|IShapeView]] | Represents a View that enables you to draw a shape to the screen. |
| [[ISlider\|ISlider]] | Represents a View that inputs a linear value. |
| [[IStackLayout\|IStackLayout]] | A Layout that positions child elements in a single line which can be oriented vertically or horizontally. |
| [[IStackNavigation\|IStackNavigation]] | Provides stack based navigation for the .NET MAUI app. |
| [[IStackNavigationView\|IStackNavigationView]] | An IView that provides stack based navigation for the .NET MAUI app. |
| [[IStepper\|IStepper]] | Represents a View that consists of two buttons labeled with minus and plus signs. Use a Stepper for selecting a numeric value from a range of values. |
| [[IStreamImageSource\|IStreamImageSource]] |  |
| [[IStroke\|IStroke]] | Define how the outline is painted on elements. |
| [[ISwipeItem (Maui)\|ISwipeItem (Maui)]] | Represents an individual command in a SwipeView. |
| [[ISwipeItemMenuItem\|ISwipeItemMenuItem]] |  |
| [[ISwipeItemView\|ISwipeItemView]] | Represents a custom individual command in a SwipeView. |
| [[ISwipeItems\|ISwipeItems]] | Represents a collection of SwipeItem objects. |
| [[ISwipeView\|ISwipeView]] | Represents a container that provides access to contextual commands through touch interactions. |
| [[ISwitch\|ISwitch]] | Represents a View that provides a toggled value. |
| [[ITabbedView\|ITabbedView]] | Represents a View that consists of a list of tabs and a larger detail area, with each tab loading content into the detail area. |
| [[ITemplatedIndicatorView\|ITemplatedIndicatorView]] | The number of indicators |
| [[IText\|IText]] | Provides functionality to be able to customize Text. |
| [[ITextAlignment\|ITextAlignment]] | Provides functionality to be able to align Text. |
| [[ITextButton\|ITextButton]] | Functionality related with the Button View text. |
| [[ITextInput\|ITextInput]] | Represents a View which can take keyboard input. |
| [[ITextInputExtensions\|ITextInputExtensions]] |  |
| [[ITextStyle\|ITextStyle]] | Provides functionality to be able to customize the appearance of text. |
| [[ITimePicker\|ITimePicker]] | Represents a `IView` that allows the user to select a time. |
| [[ITitleBar\|ITitleBar]] | Title bar control |
| [[ITitledElement\|ITitledElement]] | Represent the title content used in Navigation Views. |
| [[IToolTipElement\|IToolTipElement]] | Indicates that this element has a ToolTip to show. |
| [[IToolbar\|IToolbar]] | Represents a bar that may display the page title, navigation affordances, and other interactive items. |
| [[IToolbarElement\|IToolbarElement]] |  |
| [[ITransform\|ITransform]] | Provides functionality to be able to apply transformations to a View. |
| [[IUriImageSource\|IUriImageSource]] |  |
| [[IView\|IView]] | Represents a visual element that is used to place layouts and controls on the screen. |
| [[IViewHandler\|IViewHandler]] | Defines members that view handlers should implement to provide mapping virtual views to platform views. |
| [[IVisualDiagnosticsOverlay\|IVisualDiagnosticsOverlay]] |  |
| [[IVisualTreeElement\|IVisualTreeElement]] |  |
| [[IWebRequestInterceptingWebView\|IWebRequestInterceptingWebView]] |  |
| [[IWebView\|IWebView]] | Represents a View that presents HTML content. |
| [[IWebViewDelegate\|IWebViewDelegate]] |  |
| [[IWebViewSource\|IWebViewSource]] | Provide the data for a WebView. |
| [[IWindow\|IWindow]] | Provides the ability to create, configure, show, and manage Windows. |
| [[IWindowOverlay\|IWindowOverlay]] |  |
| [[IWindowOverlayElement\|IWindowOverlayElement]] | Element drawn on top of IWindowOverlay. |

## Structs

| Type | Summary |
|---|---|
| [[CornerRadius\|CornerRadius]] | Contains methods and properties for specifying corner radiuses. |
| [[Font (Maui)\|Font (Maui)]] | Represents a font, including family, size, weight, slant, and auto-scaling settings. |
| [[FontSize\|FontSize]] | Represents the size of a font on Android. |
| [[GridLength\|GridLength]] | Used to define the size (width/height) of Grid ColumnDefinition and RowDefinition. |
| [[SafeAreaEdges\|SafeAreaEdges]] | Represents safe area settings for each edge of a layout or visual element. |
| [[SizeRequest\|SizeRequest]] | Struct that defines a preferred and minimum `Size` for layout measurement. |
| [[Thickness\|Thickness]] | Struct defining thickness for each edge of a rectangle. |

## Enums

| Type | Summary |
|---|---|
| [[Aspect\|Aspect]] | Defines how an image is displayed. |
| [[ClearButtonVisibility\|ClearButtonVisibility]] | Enumerates values that influence clear button visibility behavior on input fields. Typically this is a button inside of the input field, near the end, which … |
| [[FlowDirection\|FlowDirection]] | Enumerates values that control the layout direction for views. |
| [[FlyoutBehavior\|FlyoutBehavior]] | Enumeration of modes for the root menu of a Shell application. |
| [[GestureStatus\|GestureStatus]] | Enumerates possible gesture states. |
| [[GridUnitType\|GridUnitType]] | Enumerates values that control how the `Value` property is interpreted for row and column definitions. |
| [[KeyboardAcceleratorModifiers\|KeyboardAcceleratorModifiers]] | Enumerates modifier flags for keyboard accelerators. MacCatalyst modifiers AlphaShift and NumericPad are not currently supported. |
| [[KeyboardFlags\|KeyboardFlags]] | Enumerates keyboard option flags that controls capitalization, spellcheck, and suggestion behavior. |
| [[LineBreakMode\|LineBreakMode]] | Enumeration specifying various options for line breaking. |
| [[OpenSwipeItem\|OpenSwipeItem]] | Specifies which swipe items to open based on the swipe gesture direction. |
| [[ReturnType\|ReturnType]] | Enumerates return button styles. Typically the operating system on-screen keyboard will visually style the return key based on this value. |
| [[SafeAreaRegions\|SafeAreaRegions]] | Specifies which platform safe area edges to obey for a layout or visual element. |
| [[ScrollBarVisibility\|ScrollBarVisibility]] | Enumerates conditions under which scroll bars will be visible. |
| [[ScrollOrientation\|ScrollOrientation]] | Enumeration specifying vertical or horizontal scrolling directions. |
| [[SwipeBehaviorOnInvoked\|SwipeBehaviorOnInvoked]] | Specifies the behavior of a SwipeView when an item is invoked. |
| [[SwipeDirection\|SwipeDirection]] | Enumerates swipe directions. |
| [[SwipeMode\|SwipeMode]] | Specifies the behavior of a SwipeView when an item is invoked. |
| [[TextAlignment\|TextAlignment]] | Enumerates values that control text alignment. |
| [[TextDecorations\|TextDecorations]] | Flagging enumeration defining text decorations. |
| [[TextTransform\|TextTransform]] | Enumerates values that determine the text transformation on an element. |
| [[TextType\|TextType]] | Specifies the format type of text content. |
| [[VisualTreeChangeType\|VisualTreeChangeType]] | The type of change applied to the Visual Tree. |
| [[WebNavigationEvent\|WebNavigationEvent]] | Contains values that indicate why a navigation event was raised. |
| [[WebNavigationResult\|WebNavigationResult]] | Enumerates values that indicate the outcome of a web navigation. |

## See also

- [[_API Reference]]
