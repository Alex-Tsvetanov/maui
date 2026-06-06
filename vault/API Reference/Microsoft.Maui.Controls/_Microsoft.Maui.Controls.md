---
title: "Microsoft.Maui.Controls"
tags:
  - api
  - namespace
  - ns/Microsoft-Maui-Controls
---

# Microsoft.Maui.Controls

> [!info] Namespace
> `Microsoft.Maui.Controls` — 494 public types.

- [Online namespace docs](https://learn.microsoft.com/dotnet/api/microsoft.maui.controls)

## Overview

`Microsoft.Maui.Controls` is the core UI namespace of .NET MAUI. It provides the cross-platform control library, layout system, data-binding engine, and navigation primitives that you compose to build an application's user interface in C# or XAML. Almost every visual building block — pages, layouts, and individual controls — derives from a shared element hierarchy rooted in [[Element|Element]] and [[VisualElement (Controls)|VisualElement]], with [[View|View]] as the base for on-screen controls and [[Page (Controls)|Page]] as the base for full-screen surfaces such as [[ContentPage|ContentPage]].

A central pillar of the namespace is its data-binding and property system. [[BindableObject|BindableObject]] and [[BindableProperty|BindableProperty]] supply the backing store that powers change notification, value coercion, and styling, while [[Binding|Binding]] connects source properties to target properties so views stay in sync with view models. Appearance is shared and themed through [[Style|Style]], [[Setter|Setter]], [[ResourceDictionary|ResourceDictionary]], and the visual-state system in [[VisualStateManager|VisualStateManager]], and behavior can be extended without subclassing via [[Behavior|Behavior]] and [[Trigger|Trigger]].

For arranging content, the namespace ships a flexible layout family — [[Grid (Controls)|Grid]], [[StackLayout (Controls)|StackLayout]], [[FlexLayout (Controls)|FlexLayout]], and others — all derived from [[Layout (Controls)|Layout]]. Content-heavy scenarios are served by collection controls such as [[CollectionView|CollectionView]] and [[ListView (Controls)|ListView]], which render data with reusable [[DataTemplate|DataTemplate]]s. Navigation is handled either through page-stack types like [[NavigationPage (Controls)|NavigationPage]] or, for richer flyout and tab experiences, through [[Shell|Shell]]. The whole UI is hosted by an [[Application (Controls)|Application]] instance.

> [!tip] Start with [[VisualElement (Controls)|VisualElement]], [[Layout (Controls)|Layout]], and [[BindableObject|BindableObject]] to understand how controls, layout, and data binding fit together; nearly everything else builds on these three.

## Key types

- [[VisualElement (Controls)|VisualElement]] — base class for all on-screen visual elements (size, position, appearance, focus).
- [[View|View]] — base visual element used to place layouts and controls on the screen.
- [[Page (Controls)|Page]] — a visual element that occupies the entire screen; base of all pages.
- [[ContentPage|ContentPage]] — a page that displays a single view as its content.
- [[Layout (Controls)|Layout]] — base class for layouts that arrange and group UI controls.
- [[Grid (Controls)|Grid]] — arranges child views in rows and columns.
- [[StackLayout (Controls)|StackLayout]] — positions children in a single vertical or horizontal line.
- [[BindableObject|BindableObject]] — propagates data changes and enables binding, validation, and coercion.
- [[BindableProperty|BindableProperty]] — backing store that allows bindings on a `BindableObject`.
- [[Binding|Binding]] — connects a source property to a target property.
- [[Style|Style]] — groups property setters that can be shared across visual elements.
- [[CollectionView|CollectionView]] — presents a scrollable, selectable collection of items.
- [[Shell|Shell]] — main navigation container providing flyout and tab-based navigation.
- [[Application (Controls)|Application]] — main application class for lifecycle, resources, and theming.

## Related guides

- [[_API Reference|API Reference]]


## Classes

| Type | Summary |
|---|---|
| [[AbsoluteLayout (Controls)\|AbsoluteLayout (Controls)]] | Positions child elements at absolute positions. |
| [[ActivityIndicator\|ActivityIndicator]] | A visual control used to indicate that something is ongoing. |
| [[AdaptiveTrigger\|AdaptiveTrigger]] | A state trigger that activates when the window meets a minimum width and/or height threshold. |
| [[AndExpandLayoutManager\|AndExpandLayoutManager]] |  |
| [[Animation (Controls)\|Animation (Controls)]] | Encapsulates an animation, a collection of functions that modify properties over a user-perceptible time period. |
| [[AnimationExtensions\|AnimationExtensions]] | Extension methods for `IAnimatable` objects. |
| [[AppLinkEntry\|AppLinkEntry]] | A deep application link in an app link search index. |
| [[AppThemeChangedEventArgs\|AppThemeChangedEventArgs]] | Event arguments for the `RequestedThemeChanged` event. |
| [[Application (Controls)\|Application (Controls)]] | Represents the main application class that provides lifecycle management, resources, and theming. |
| [[AutomationProperties\|AutomationProperties]] | Contains both abbreviated and detailed UI information that is supplied to accessibility services. |
| [[BackButtonBehavior\|BackButtonBehavior]] | Customizes the appearance and behavior of the back button in a `Shell` application. |
| [[BackButtonPressedEventArgs\|BackButtonPressedEventArgs]] | Internal API that may change or be removed without notice. |
| [[BackgroundingEventArgs\|BackgroundingEventArgs]] |  |
| [[BaseMenuItem\|BaseMenuItem]] | Base class for menu items. |
| [[BaseShellItem\|BaseShellItem]] | Base class for Shell navigation items providing common properties like Title, Icon, and Route. |
| [[BaseSwipeEventArgs\|BaseSwipeEventArgs]] | Provides base event data for swipe events. |
| [[Behavior\|Behavior]] | Base class for generalized user-defined behaviors that can respond to arbitrary conditions and events. |
| [[Behavior{T}\|Behavior<T>]] | Base class for generalized user-defined behaviors that can respond to arbitrary conditions and events. |
| [[BindableLayout\|BindableLayout]] | Provides attached properties for enabling data binding on layout elements. |
| [[BindableObject\|BindableObject]] | Provides a mechanism to propagate data changes from one object to another. Enables validation, type coercion, and an event system. |
| [[BindableObjectExtensions\|BindableObjectExtensions]] | Contains convenience extension methods for `BindableObject`. |
| [[BindableProperty\|BindableProperty]] | A BindableProperty is a backing store for properties allowing bindings on `BindableObject`. |
| [[BindablePropertyConverter\|BindablePropertyConverter]] | A TypeConverter that converts strings to `BindableProperty` instances. |
| [[BindablePropertyKey\|BindablePropertyKey]] | The secret key to a BindableProperty, used to implement read-only bindable properties. |
| [[Binding\|Binding]] | A binding that connects a property on a source object to a property on a target object. |
| [[BindingBase\|BindingBase]] | An abstract base class for all bindings providing `BindingMode` selection, fallback/target null values, and formatting support. |
| [[BindingCondition\|BindingCondition]] | A condition that is satisfied when a binding evaluates to a specified value. |
| [[BindingPropertyChangedDelegate\|BindingPropertyChangedDelegate]] |  |
| [[BindingPropertyChangedDelegate{TPropertyType}\|BindingPropertyChangedDelegate<TPropertyType>]] |  |
| [[BindingPropertyChangingDelegate\|BindingPropertyChangingDelegate]] |  |
| [[BindingPropertyChangingDelegate{TPropertyType}\|BindingPropertyChangingDelegate<TPropertyType>]] |  |
| [[Border\|Border]] | A container control that draws a border, background, or both around its child content. |
| [[BoundsConstraint\|BoundsConstraint]] | A bounds layout constraint used by `RelativeLayout`s. |
| [[BoundsTypeConverter\|BoundsTypeConverter]] | A `TypeConverter` that converts strings into `Rectangle`s for use with `AbsoluteLayout`s. |
| [[BoxView (Controls)\|BoxView (Controls)]] | A `View` used to draw a solid colored rectangle. |
| [[Brush\|Brush]] | Defines the core behavior and built-in colors for painting an area. |
| [[BrushTypeConverter\|BrushTypeConverter]] | A `TypeConverter` that converts strings, colors, and paints to `Brush` objects. |
| [[Button (Controls)\|Button (Controls)]] | A button `View` that reacts to touch events. |
| [[ButtonContentLayout\|ButtonContentLayout]] | Represents the layout of the button content whenever an image is shown. |
| [[ButtonContentTypeConverter\|ButtonContentTypeConverter]] | A converter to convert a string to a `ButtonContentLayout` object. |
| [[CarouselLayoutTypeConverter\|CarouselLayoutTypeConverter]] | Converts string representations to `LinearItemsLayout` instances for use in carousel XAML. |
| [[CarouselView\|CarouselView]] | A view that presents a scrollable collection of items where each item 'snaps' into place after scrolling. |
| [[Cell (Controls)\|Cell (Controls)]] | Provides base class and capabilities for all Microsoft.Maui.Controls cells. Cells are elements meant to be added to `ListView` or `TableView`. |
| [[CheckBox\|CheckBox]] | Represents a control that a user can select or clear. |
| [[CheckedChangedEventArgs\|CheckedChangedEventArgs]] | Event Args for `CheckBox`'s `CheckedChanged` event. |
| [[ChildGestureRecognizer\|ChildGestureRecognizer]] | A gesture recognizer for use as a child of another. |
| [[CloseRequestedEventArgs\|CloseRequestedEventArgs]] |  |
| [[CoerceValueDelegate\|CoerceValueDelegate]] |  |
| [[CoerceValueDelegate{TPropertyType}\|CoerceValueDelegate<TPropertyType>]] |  |
| [[CollectionSynchronizationCallback\|CollectionSynchronizationCallback]] |  |
| [[CollectionView\|CollectionView]] | A `SelectableItemsView` that presents a collection of items. |
| [[ColumnDefinition\|ColumnDefinition]] | An `IDefinition` that defines properties for a column in a `Grid`. |
| [[ColumnDefinitionCollection\|ColumnDefinitionCollection]] | A `DefinitionCollection{T}` for `ColumnDefinition`s. |
| [[ColumnDefinitionCollectionTypeConverter\|ColumnDefinitionCollectionTypeConverter]] | Converts a comma-separated string of grid lengths to a `ColumnDefinitionCollection`. |
| [[Command\|Command]] | Defines an `ICommand` implementation that wraps a `Action`. |
| [[Command{T}\|Command<T>]] | Defines an `ICommand` implementation that wraps a `Action`. |
| [[CommonStates\|CommonStates]] |  |
| [[CompareStateTrigger\|CompareStateTrigger]] | A state trigger that activates when `Property` equals `Value`. |
| [[CompressedLayout\|CompressedLayout]] | Contains attached properties for omitting redundant renderers. |
| [[Condition\|Condition]] | Base class for trigger conditions that determine when a trigger should activate. |
| [[Configuration{TPlatform, TElement}\|Configuration<TPlatform, TElement>]] |  |
| [[ContentPage\|ContentPage]] | A `Page` that displays a single view as its content. |
| [[ContentPresenter\|ContentPresenter]] | Layout manager for templated views. |
| [[ContentPropertyAttribute\|ContentPropertyAttribute]] | Indicates the property used as the content property in XAML. |
| [[ContentView (Controls)\|ContentView (Controls)]] | An element that contains a single child element. |
| [[ControlTemplate\|ControlTemplate]] | Defines the appearance of the templated control. |
| [[ControlsColorExtensions\|ControlsColorExtensions]] |  |
| [[CreateDefaultValueDelegate\|CreateDefaultValueDelegate]] |  |
| [[CreateDefaultValueDelegate{TDeclarer, TPropertyType}\|CreateDefaultValueDelegate<TDeclarer, TPropertyType>]] |  |
| [[CurrentItemChangedEventArgs\|CurrentItemChangedEventArgs]] | Provides data for the CurrentItemChanged event in carousel and collection views. |
| [[DataPackage\|DataPackage]] | Contains data being transferred during a drag and drop operation. |
| [[DataPackagePropertySet\|DataPackagePropertySet]] | A collection of custom properties for a `DataPackage`. |
| [[DataPackagePropertySetView\|DataPackagePropertySetView]] | A read-only view of a `DataPackagePropertySet`. |
| [[DataPackageView\|DataPackageView]] | A read-only view of a `DataPackage`. |
| [[DataTemplate\|DataTemplate]] | Defines the visual structure for templated items. Used to display data objects with a consistent appearance. |
| [[DataTemplateSelector\|DataTemplateSelector]] | Selects a `DataTemplate` based on the data object and container. Override `OnSelectTemplate` to implement selection logic. |
| [[DataTrigger\|DataTrigger]] | A trigger that activates setters when a bound value matches a specified value. |
| [[DateChangedEventArgs\|DateChangedEventArgs]] | Event arguments for `DateSelected` event. |
| [[DatePicker (Controls)\|DatePicker (Controls)]] | A view control that allows date selection. |
| [[DatePickerClosedEventArgs\|DatePickerClosedEventArgs]] | Provides event data for the event that is raised when a DatePicker control is closed. |
| [[DatePickerOpenedEventArgs\|DatePickerOpenedEventArgs]] | Provides event data for the event that is raised when a DatePicker control is opened. |
| [[DefaultVisual\|DefaultVisual]] |  |
| [[DefinitionCollection{T}\|DefinitionCollection<T>]] |  |
| [[DependencyAttribute\|DependencyAttribute]] | An attribute that indicates that the specified type provides a concrete implementation of a needed interface. |
| [[DependencyService\|DependencyService]] | Static class that provides the `Get{T}` factory method for retrieving platform-specific implementations of the specified type T. |
| [[DesignMode\|DesignMode]] | Static class that developers can use to determine if the application is running in a previewer. |
| [[Device\|Device]] | A utility class to interact with the current Device/Platform. |
| [[DeviceStateTrigger\|DeviceStateTrigger]] | A state trigger that activates when the app runs on a specified device platform. |
| [[DisplayDensityChangedEventArgs\|DisplayDensityChangedEventArgs]] |  |
| [[DisplayRotationStateTrigger\|DisplayRotationStateTrigger]] | Trigger that activates when the device display rotation matches the specified `Rotation`. |
| [[DoubleCollection\|DoubleCollection]] | An observable collection of `double` values, used for stroke dash patterns and similar properties. |
| [[DoubleCollectionConverter\|DoubleCollectionConverter]] | Converts a space or comma-separated string of numbers to a `DoubleCollection`. |
| [[DragEventArgs\|DragEventArgs]] | Provides data for the `DragOver` and `DragLeave` events. |
| [[DragGestureRecognizer\|DragGestureRecognizer]] | Provides drag gesture recognition and defines the associated events for dragging and dropping. |
| [[DragStartingEventArgs\|DragStartingEventArgs]] | Provides data for the `DragStarting` event. |
| [[DropCompletedEventArgs\|DropCompletedEventArgs]] | Provides data for the event raised when a drop operation completes. |
| [[DropEventArgs\|DropEventArgs]] | Provides data for the `Drop` event. |
| [[DropGestureRecognizer\|DropGestureRecognizer]] | Recognizes drop gestures and handles data transfer during drag and drop operations. |
| [[Editor\|Editor]] | A control that can edit multiple lines of text. |
| [[Effect\|Effect]] | A collection of styles and properties that can be added to an element at run time. |
| [[EffectiveVisualExtensions\|EffectiveVisualExtensions]] | Extension methods for checking visual type. |
| [[Element\|Element]] | Provides the base class for all Microsoft.Maui.Controls hierarchal elements. This class contains all the methods and properties required to represent an elem… |
| [[ElementEventArgs\|ElementEventArgs]] | Provides data for events pertaining to a single `Element`. |
| [[ElementTemplate\|ElementTemplate]] | Base class for `DataTemplate` and `ControlTemplate` classes. |
| [[Entry (Controls)\|Entry (Controls)]] | Entry is a single line text entry. It is best used for collecting small discrete pieces of information, like usernames and passwords. |
| [[EntryCell\|EntryCell]] | A `Cell` with a label and a single line text entry field. |
| [[EventTrigger\|EventTrigger]] | A trigger that fires actions in response to a specified event on the associated element. |
| [[ExportEffectAttribute\|ExportEffectAttribute]] | Attribute that identifies a `Effect` with a unique identifier that can be used with `Resolve` to locate an effect. |
| [[ExportFontAttribute\|ExportFontAttribute]] | Registers a font file for use in the application. |
| [[FileImageSource\|FileImageSource]] | An `ImageSource` that reads an image from a file. |
| [[FileImageSourceConverter\|FileImageSourceConverter]] | A `TypeConverter` that converts to `FileImageSource`. |
| [[FlexLayout (Controls)\|FlexLayout (Controls)]] | A Flexbox-like layout that lays out child elements in optionally wrappable rows or columns of child elements. |
| [[FlowDirectionConverter\|FlowDirectionConverter]] |  |
| [[FlyoutBase\|FlyoutBase]] |  |
| [[FlyoutItem\|FlyoutItem]] | Represents a flyout menu item in a `Shell` application. |
| [[FlyoutPage (Controls)\|FlyoutPage (Controls)]] | A `Page` that manages two panes of information: a flyout that presents a menu or navigation, and a detail that presents the selected content. |
| [[FocusEventArgs\|FocusEventArgs]] | Event args for `VisualElement`'s `Focused` and `Unfocused` events. |
| [[FocusRequestArgs\|FocusRequestArgs]] |  |
| [[FontAttributesConverter\|FontAttributesConverter]] | The font is unmodified. |
| [[FontExtensions (Controls)\|FontExtensions (Controls)]] |  |
| [[FontImageSource\|FontImageSource]] | An `ImageSource` that renders a glyph from a font. |
| [[FontSizeConverter\|FontSizeConverter]] |  |
| [[FormattedString\|FormattedString]] | Represents a text with attributes applied to some parts. |
| [[Frame\|Frame]] | An element containing a single child, with some framing options. |
| [[GestureElement\|GestureElement]] | An element that can respond to gestures. |
| [[GestureRecognizer\|GestureRecognizer]] | The base class for all gesture recognizers. |
| [[GradientBrush\|GradientBrush]] | Base class for brushes that paint an area with a gradient of multiple colors. |
| [[GradientBrushParser\|GradientBrushParser]] |  |
| [[GradientStop\|GradientStop]] | Represents a color and offset within a `GradientBrush`. |
| [[GradientStopCollection\|GradientStopCollection]] | A collection of `GradientStop` objects used by `GradientBrush`. |
| [[GraphicsView\|GraphicsView]] |  |
| [[Grid (Controls)\|Grid (Controls)]] | A layout that arranges views in rows and columns. |
| [[GridExtensions\|GridExtensions]] |  |
| [[GridItemsLayout\|GridItemsLayout]] | An items layout that arranges items in a grid with configurable columns or rows. |
| [[GridLengthTypeConverter (Controls)\|GridLengthTypeConverter (Controls)]] |  |
| [[GroupableItemsView\|GroupableItemsView]] | A selectable items view that supports grouping of items. |
| [[HandlerAttribute\|HandlerAttribute]] | An abstract attribute whose subclasses specify the platform-specific renderers for Microsoft.Maui.Controls abstract controls. |
| [[HandlerChangingEventArgs\|HandlerChangingEventArgs]] |  |
| [[HandlerProperties\|HandlerProperties]] |  |
| [[HorizontalStackLayout\|HorizontalStackLayout]] |  |
| [[HtmlWebViewSource\|HtmlWebViewSource]] | A WebViewSource bound to an HTML-formatted string. |
| [[HybridWebView\|HybridWebView]] | A `View` that presents local HTML content in a web view and allows JavaScript and C# code to communicate by using messages and by invoking methods. |
| [[HybridWebViewRawMessageReceivedEventArgs\|HybridWebViewRawMessageReceivedEventArgs]] |  |
| [[Image (Controls)\|Image (Controls)]] | A view control that displays an image. |
| [[ImageButton (Controls)\|ImageButton (Controls)]] | Represents a button that displays an image and reacts to touch events. |
| [[ImageCell\|ImageCell]] | A `TextCell` that has an image. |
| [[ImageSource\|ImageSource]] | Abstract class whose implementors load images from files or the Web. |
| [[ImageSourceConverter\|ImageSourceConverter]] | A `TypeConverter` that converts strings and URIs to `ImageSource` objects. |
| [[IndicatorView\|IndicatorView]] | A view that displays a visual indicator representing the position within a collection of items. |
| [[InputView (Controls)\|InputView (Controls)]] | A base class for views that obtain text input from the user. |
| [[InvalidNavigationException\|InvalidNavigationException]] | Exception thrown when an invalid navigation operation is attempted. |
| [[ItemTappedEventArgs\|ItemTappedEventArgs]] | Event arguments for the `ItemTapped` event. |
| [[ItemVisibilityEventArgs\|ItemVisibilityEventArgs]] | Event args when an item's visibility has been changed in a `ListView`. |
| [[ItemsLayout\|ItemsLayout]] | Base class for layouts that arrange items in collection and carousel views. |
| [[ItemsLayoutTypeConverter\|ItemsLayoutTypeConverter]] | Converts string representations to `IItemsLayout` instances for use in XAML. |
| [[ItemsView (Controls)\|ItemsView (Controls)]] | A `View` that serves as a base class for views that contain a templated list of items. |
| [[ItemsView{TVisual}\|ItemsView<TVisual>]] | A `View` that serves as a base class for views that contain a templated list of items. |
| [[ItemsViewScrolledEventArgs\|ItemsViewScrolledEventArgs]] | Provides data for the Scrolled event in items views. |
| [[KeyboardAccelerator\|KeyboardAccelerator]] | Represents a shortcut key for a `MenuFlyoutItem`. |
| [[KnownColor\|KnownColor]] |  |
| [[Label (Controls)\|Label (Controls)]] | A `View` that displays text. |
| [[Layout (Controls)\|Layout (Controls)]] | Base class for layouts that allow you to arrange and group UI controls in your application. |
| [[LayoutConstraint\|LayoutConstraint]] |  |
| [[LayoutDirectionExtensions\|LayoutDirectionExtensions]] |  |
| [[LayoutOptionsConverter\|LayoutOptionsConverter]] | Class that takes a string representation of a `LayoutOptions` and returns a corresponding `LayoutOptions`. |
| [[LinearGradientBrush\|LinearGradientBrush]] | A `GradientBrush` that paints an area with a linear gradient. |
| [[LinearItemsLayout\|LinearItemsLayout]] | An items layout that arranges items in a single row or column. |
| [[ListProxyChangedEventArgs\|ListProxyChangedEventArgs]] | Event arguments for when the internal list proxy changes. |
| [[ListStringTypeConverter\|ListStringTypeConverter]] | Type converter for converting properly formatted string lists to lists. |
| [[ListView (Controls)\|ListView (Controls)]] | An `ItemsView{T}` that displays a collection of data as a vertical list. |
| [[MarshalingObservableCollection\|MarshalingObservableCollection]] | A thread-safe observable collection that marshals all collection changes to the main thread. |
| [[MenuBar\|MenuBar]] |  |
| [[MenuBarItem\|MenuBarItem]] |  |
| [[MenuFlyout\|MenuFlyout]] |  |
| [[MenuFlyoutItem\|MenuFlyoutItem]] | Represents a MenuFlyoutItem. |
| [[MenuFlyoutSeparator\|MenuFlyoutSeparator]] | Represents a horizontal line that separates items in a menu. |
| [[MenuFlyoutSubItem\|MenuFlyoutSubItem]] |  |
| [[MenuItem\|MenuItem]] | Class that presents a menu item and associates it with a command. |
| [[MenuItemCollection\|MenuItemCollection]] | A collection of `MenuItem` objects used in Shell. |
| [[ModalEventArgs\|ModalEventArgs]] | Base class for `ModalPushedEventArgs`, `ModalPushingEventArgs`, `ModalPoppedEventArgs`, and `ModalPoppingEventArgs`. |
| [[ModalPoppedEventArgs\|ModalPoppedEventArgs]] | Arguments for the event that is raised when a modal window is popped from the navigation stack. |
| [[ModalPoppingEventArgs\|ModalPoppingEventArgs]] | Arguments for the event that is raised when a modal window is popping from the navigation stack. |
| [[ModalPushedEventArgs\|ModalPushedEventArgs]] | Arguments for the event that is raised when a modal window is pushed onto the navigation stack. |
| [[ModalPushingEventArgs\|ModalPushingEventArgs]] | Arguments for the event that is raised when a modal window is being pushed onto the navigation stack. |
| [[MultiBinding\|MultiBinding]] | A binding that combines multiple source bindings into a single target value using an `IMultiValueConverter`. |
| [[MultiPage{T}\|MultiPage<T>]] |  |
| [[MultiTrigger\|MultiTrigger]] | Class that represents a list of property and binding conditions, and a list of setters that are applied when all of the conditions in the list are met. |
| [[NameScopeExtensions\|NameScopeExtensions]] | Extension methods for `Element` that adds a strongly-typed FindByName method. |
| [[NavigableElement\|NavigableElement]] | Represents an `Element` with base functionality for `Page` navigation. Does not necessarily render on screen. |
| [[NavigatedFromEventArgs\|NavigatedFromEventArgs]] |  |
| [[NavigatedToEventArgs\|NavigatedToEventArgs]] |  |
| [[NavigatingFromEventArgs\|NavigatingFromEventArgs]] |  |
| [[NavigationEventArgs\|NavigationEventArgs]] | EventArgs for the NavigationPage's navigation events. |
| [[NavigationPage (Controls)\|NavigationPage (Controls)]] | A `Page` that manages the navigation and user-experience of a stack of other pages. |
| [[NavigationType\|NavigationType]] |  |
| [[NullEffect\|NullEffect]] |  |
| [[On\|On]] | Class that is used within OnPlatform tags in XAML when specifying values on platforms. |
| [[OnIdiom{T}\|OnIdiom<T>]] |  |
| [[OnPlatform{T}\|OnPlatform<T>]] |  |
| [[OpenRequestedEventArgs\|OpenRequestedEventArgs]] |  |
| [[OrientationStateTrigger\|OrientationStateTrigger]] | A state trigger that activates when the device orientation matches the specified `Orientation`. |
| [[Page (Controls)\|Page (Controls)]] | A `VisualElement` that occupies the entire screen. |
| [[PanGestureRecognizer\|PanGestureRecognizer]] | A gesture recognizer for panning content that is larger than its parent view. |
| [[PanUpdatedEventArgs\|PanUpdatedEventArgs]] | Event arguments for the `PanUpdated` event. |
| [[ParentChangingEventArgs\|ParentChangingEventArgs]] |  |
| [[Picker (Controls)\|Picker (Controls)]] | A view control for picking an element from a list. |
| [[PickerClosedEventArgs\|PickerClosedEventArgs]] | Provides event data for the event that is raised when a Picker control is closed. |
| [[PickerOpenedEventArgs\|PickerOpenedEventArgs]] | Provides event data for the event that is raised when a Picker control is opened. |
| [[PinchGestureRecognizer\|PinchGestureRecognizer]] | Recognizer for pinch gestures. |
| [[PinchGestureUpdatedEventArgs\|PinchGestureUpdatedEventArgs]] | Event arguments for the `PinchUpdated` event. |
| [[PlatformBehavior{TView, TPlatformView}\|PlatformBehavior<TView, TPlatformView>]] | Base class for generalized user-defined behaviors that can respond to arbitrary conditions and events when connected to the platform view hierarchy. |
| [[PlatformBehavior{TView}\|PlatformBehavior<TView>]] | Base class for generalized user-defined behaviors that can respond to arbitrary conditions and events when connected to the platform view hierarchy. |
| [[PlatformConfigurationRegistry{TElement}\|PlatformConfigurationRegistry<TElement>]] |  |
| [[PlatformDragEventArgs\|PlatformDragEventArgs]] | Platform-specific arguments associated with the `DragEventArgs`. |
| [[PlatformDragStartingEventArgs\|PlatformDragStartingEventArgs]] | Platform-specific arguments associated with the DragStartingEventArgs. |
| [[PlatformDropCompletedEventArgs\|PlatformDropCompletedEventArgs]] | Platform-specific arguments associated with the DropCompletedEventArgs |
| [[PlatformDropEventArgs\|PlatformDropEventArgs]] | Platform-specific arguments associated with the DropEventArgs. |
| [[PlatformEffect{TContainer, TControl}\|PlatformEffect<TContainer, TControl>]] |  |
| [[PlatformPointerEventArgs\|PlatformPointerEventArgs]] | Platform-specific arguments associated with the PointerEventArgs. |
| [[PlatformWebViewInitializedEventArgs\|PlatformWebViewInitializedEventArgs]] | Provides platform-specific information about the `WebViewInitializedEventArgs` event. |
| [[PlatformWebViewInitializingEventArgs\|PlatformWebViewInitializingEventArgs]] | Provides platform-specific information about the `WebViewInitializingEventArgs` event. |
| [[PlatformWebViewProcessTerminatedEventArgs\|PlatformWebViewProcessTerminatedEventArgs]] |  |
| [[PlatformWebViewWebResourceRequestedEventArgs\|PlatformWebViewWebResourceRequestedEventArgs]] | Provides platform-specific information about the `WebViewWebResourceRequestedEventArgs` event. |
| [[PointCollection\|PointCollection]] |  |
| [[PointerEventArgs\|PointerEventArgs]] | Arguments for PointerGestureRecognizer events. |
| [[PointerGestureRecognizer\|PointerGestureRecognizer]] | Provides pointer gesture recognition and events. |
| [[PoppedToRootEventArgs\|PoppedToRootEventArgs]] | Event arguments for a pop-to-root navigation operation. |
| [[PositionChangedEventArgs\|PositionChangedEventArgs]] | Provides data for the PositionChanged event in carousel and collection views. |
| [[ProgressBar (Controls)\|ProgressBar (Controls)]] | A view control that displays progress as a partially filled bar. |
| [[PropertyChangingEventArgs\|PropertyChangingEventArgs]] | Event arguments for property changing notifications. |
| [[PropertyChangingEventHandler\|PropertyChangingEventHandler]] |  |
| [[PropertyCondition\|PropertyCondition]] | A condition that is satisfied when a property has a specified value. |
| [[QueryPropertyAttribute\|QueryPropertyAttribute]] | Maps a query parameter from a URI to a property on the target page or view model during Shell navigation. |
| [[RDSourceTypeConverter\|RDSourceTypeConverter]] |  |
| [[RadialGradientBrush\|RadialGradientBrush]] | A `GradientBrush` that paints an area with a radial gradient. |
| [[RadioButton\|RadioButton]] | A mutually exclusive selection control that allows users to select one option from a set. |
| [[RadioButtonGroup\|RadioButtonGroup]] | Provides attached properties for managing groups of `RadioButton` controls. |
| [[ReferenceTypeConverter\|ReferenceTypeConverter]] | Converts a string name reference (x:Name) into the referenced object in XAML. |
| [[RefreshView (Controls)\|RefreshView (Controls)]] | Represents a container that provides pull-to-refresh functionality for scrollable content. |
| [[RelativeBindingSource\|RelativeBindingSource]] | Specifies a relative source for a binding, such as Self, TemplatedParent, or FindAncestor. |
| [[RenderWithAttribute\|RenderWithAttribute]] | Specifies the renderer type to use for a control. |
| [[ReorderableItemsView\|ReorderableItemsView]] | A `GroupableItemsView` that supports reordering of items through user interaction. |
| [[ResolutionGroupNameAttribute\|ResolutionGroupNameAttribute]] | Specifies the resolution group name for effects in the assembly. |
| [[Resource (Controls)\|Resource (Controls)]] |  |
| [[ResourceDictionary\|ResourceDictionary]] | A dictionary that maps identifier strings to arbitrary resource objects. |
| [[RouteFactory\|RouteFactory]] | Base class for factories that create elements for registered routes. |
| [[Routing\|Routing]] | Provides methods and properties to manage URI-based navigation routes in Shell applications. |
| [[RoutingEffect\|RoutingEffect]] | Platform-independent effect that wraps an inner effect, which is usually platform-specific. |
| [[RowDefinition\|RowDefinition]] | Defines the height of a row in a `Grid`. |
| [[RowDefinitionCollection\|RowDefinitionCollection]] | A collection of `RowDefinition` objects that define the rows of a `Grid`. |
| [[RowDefinitionCollectionTypeConverter\|RowDefinitionCollectionTypeConverter]] | Converts a comma-separated string of grid lengths to a `RowDefinitionCollection`. |
| [[ScrollToRequestEventArgs\|ScrollToRequestEventArgs]] | Provides data for scroll-to-item requests in items views. |
| [[ScrollToRequestedEventArgs\|ScrollToRequestedEventArgs]] | Event arguments for scroll-to requests on scrollable views. |
| [[ScrollView (Controls)\|ScrollView (Controls)]] | Represents a view that is capable of scrolling if its content requires it. |
| [[ScrolledEventArgs\|ScrolledEventArgs]] | Event arguments for the `Scrolled` event. |
| [[SearchBar (Controls)\|SearchBar (Controls)]] | Represents a specialized input control for entering search text with a built-in search button and cancel button. |
| [[SearchHandler\|SearchHandler]] | Provides search functionality in a `Shell` application. |
| [[SelectableItemsView\|SelectableItemsView]] | A structured items view that supports item selection. |
| [[SelectedItemChangedEventArgs\|SelectedItemChangedEventArgs]] | Event arguments for the `ItemSelected` event. |
| [[SelectedPositionChangedEventArgs\|SelectedPositionChangedEventArgs]] | Event arguments for position changes in a CarouselPage. |
| [[SelectionChangedEventArgs\|SelectionChangedEventArgs]] | Provides data for the SelectionChanged event in selectable items views. |
| [[SemanticProperties\|SemanticProperties]] |  |
| [[Setter\|Setter]] | Sets a property value within a `Style` or `TriggerBase`. |
| [[SettersExtensions\|SettersExtensions]] | Extension methods for working with `Setter` collections. |
| [[Shadow\|Shadow]] |  |
| [[ShadowTypeConverter\|ShadowTypeConverter]] | Type converter for converting a properly formatted string to a `Shadow`. |
| [[Shell\|Shell]] | The main navigation container for .NET MAUI apps, providing flyout and tab-based navigation. |
| [[ShellAppearance\|ShellAppearance]] | Stores the appearance values for a Shell, including colors for background, foreground, tab bar, and title. |
| [[ShellContent\|ShellContent]] | Represents the content displayed within a `ShellSection` tab. |
| [[ShellGroupItem\|ShellGroupItem]] | Base class for grouping Shell items such as `ShellItem` and `ShellSection`. |
| [[ShellItem (Controls)\|ShellItem (Controls)]] | Represents a top-level navigation item in a `Shell`. Contains one or more `ShellSection` items. |
| [[ShellNavigatedEventArgs\|ShellNavigatedEventArgs]] | Provides data for the `Navigated` event. |
| [[ShellNavigatingDeferral\|ShellNavigatingDeferral]] | Allows async operations to complete before Shell navigation finishes. |
| [[ShellNavigatingEventArgs\|ShellNavigatingEventArgs]] | Provides data for the `Navigating` event. |
| [[ShellNavigationQueryParameters\|ShellNavigationQueryParameters]] |  |
| [[ShellNavigationState\|ShellNavigationState]] | Represents the current navigation state of a `Shell`, expressed as a URI. |
| [[ShellSection\|ShellSection]] | Represents a group of tabs within a `ShellItem`. Contains `ShellContent` items. |
| [[ShellTemplatedViewManager\|ShellTemplatedViewManager]] |  |
| [[Slider (Controls)\|Slider (Controls)]] | Represents a horizontal bar that a user can slide to select a value from a continuous range. |
| [[SolidColorBrush\|SolidColorBrush]] | A `Brush` that paints an area with a single solid color. |
| [[Span\|Span]] | Represents a portion of formatted text for use in a FormattedString. |
| [[StackBase\|StackBase]] |  |
| [[StackLayout (Controls)\|StackLayout (Controls)]] | A `Layout` that positions child elements in a single line which can be oriented vertically or horizontally. |
| [[StackLayoutManager (Controls)\|StackLayoutManager (Controls)]] |  |
| [[StateTrigger\|StateTrigger]] | A state trigger that activates a visual state when `IsActive` is set to `true`. |
| [[StateTriggerBase\|StateTriggerBase]] | Base class for state triggers that activate visual states based on conditions. |
| [[Stepper\|Stepper]] | Represents a control that allows a user to incrementally adjust a numeric value by tapping plus or minus buttons. |
| [[StreamImageSource\|StreamImageSource]] | `ImageSource` that loads an image from a `Stream`. |
| [[StructuredItemsView\|StructuredItemsView]] | An items view that supports headers, footers, and configurable item layouts. |
| [[Style\|Style]] | Groups property setters that can be shared between multiple visual elements. |
| [[StyleableElement\|StyleableElement]] | Represents an `Element` with base functionality for styling. Does not necessarily render on screen. |
| [[Styles\|Styles]] |  |
| [[SwipeChangingEventArgs\|SwipeChangingEventArgs]] | Provides data for the `SwipeChanging` event. |
| [[SwipeEndedEventArgs\|SwipeEndedEventArgs]] | Provides data for the `SwipeEnded` event. |
| [[SwipeGestureRecognizer\|SwipeGestureRecognizer]] | Recognizes swipe gestures on the attached element. |
| [[SwipeItem\|SwipeItem]] | Represents a menu item displayed in a `SwipeView` when the view is swiped. |
| [[SwipeItemView\|SwipeItemView]] | Represents a swipe item that displays custom content in a `SwipeView`. |
| [[SwipeItems\|SwipeItems]] | Represents a collection of `ISwipeItem` objects used by a `SwipeView`. |
| [[SwipeStartedEventArgs\|SwipeStartedEventArgs]] | Provides data for the `SwipeStarted` event. |
| [[SwipeView (Controls)\|SwipeView (Controls)]] | Represents a view that provides context-specific swipe interactions. |
| [[SwipedEventArgs\|SwipedEventArgs]] | Provides data for the `Swiped` event. |
| [[Switch (Controls)\|Switch (Controls)]] | Represents a control that the user can toggle between two states: on or off. |
| [[SwitchCell\|SwitchCell]] | A `Cell` with a label and an on/off switch. |
| [[Tab\|Tab]] | Represents a group of items within a `ShellItem`. This is an alias for `ShellSection`. |
| [[TabBar\|TabBar]] | Represents the bottom tab bar in a `Shell` application. |
| [[TabbedPage (Controls)\|TabbedPage (Controls)]] | A `MultiPage{T}` that displays an array of tabs across the top of the screen, each of which loads content onto the screen. |
| [[TableRoot\|TableRoot]] | The root element of a `TableView` that contains `TableSection` items. |
| [[TableSection\|TableSection]] | A logical grouping of cells in a `TableView`. |
| [[TableSectionBase\|TableSectionBase]] | Abstract base class for a section in a `TableView`. |
| [[TableSectionBase{T}\|TableSectionBase<T>]] | Abstract base class for a section in a `TableView`. |
| [[TableView\|TableView]] | Represents a table view control for displaying tabular data. |
| [[TapGestureRecognizer\|TapGestureRecognizer]] | Recognizes tap gestures on the attached element. |
| [[TappedEventArgs\|TappedEventArgs]] | Event arguments for the `Tapped` event. |
| [[TemplateBinding\|TemplateBinding]] | Binds a property in a control template to a templated parent property. |
| [[TemplateExtensions\|TemplateExtensions]] | Provides extension methods for working with data templates. |
| [[TemplatedPage\|TemplatedPage]] | A page that displays content using a control template, and the base class for `ContentPage`. |
| [[TemplatedView\|TemplatedView]] | A view that displays content with a control template, and the base class for `ContentView`. |
| [[TextAlignmentConverter\|TextAlignmentConverter]] |  |
| [[TextCell\|TextCell]] | A `Cell` with primary `Text` and `Detail` text. |
| [[TextChangedEventArgs\|TextChangedEventArgs]] | Event arguments for text changes. |
| [[TextDecorationConverter\|TextDecorationConverter]] | A `TypeConverter` subclass that can convert between a string and a `TextDecorations` object. |
| [[TimeChangedEventArgs\|TimeChangedEventArgs]] | Event arguments for the `TimeSelected` event. |
| [[TimePicker (Controls)\|TimePicker (Controls)]] | A view control that provides time selection. |
| [[TimePickerClosedEventArgs\|TimePickerClosedEventArgs]] | Provides event data for the event that is raised when a TimePicker control is closed. |
| [[TimePickerOpenedEventArgs\|TimePickerOpenedEventArgs]] | Provides event data for the event that is raised when a TimePicker control is opened. |
| [[TitleBar\|TitleBar]] | A `View` control that provides title bar functionality for a window. The standard title bar height is 32px, but can be set to a larger value. The title bar c… |
| [[ToggledEventArgs\|ToggledEventArgs]] | Event arguments for toggle state changes. |
| [[ToolTipProperties\|ToolTipProperties]] |  |
| [[Toolbar\|Toolbar]] |  |
| [[ToolbarItem\|ToolbarItem]] | An item in a toolbar or displayed on a Page. |
| [[TouchEventArgs\|TouchEventArgs]] |  |
| [[Trigger\|Trigger]] | A trigger that activates setters when a property on the control matches a specified value. |
| [[TriggerAction\|TriggerAction]] | A base class for user-defined actions that respond to a trigger condition with a type-safe sender parameter. |
| [[TriggerAction{T}\|TriggerAction<T>]] | A base class for user-defined actions that respond to a trigger condition with a type-safe sender parameter. |
| [[TriggerBase\|TriggerBase]] | Base class for trigger classes that define conditional behavior in response to property or data changes. |
| [[TypeTypeConverter\|TypeTypeConverter]] | Converts a string representation of a type name into a `Type` object. |
| [[UnsolvableConstraintsException\|UnsolvableConstraintsException]] | Exception thrown when layout constraints cannot be solved. |
| [[UriImageSource\|UriImageSource]] | An ImageSource that loads an image from a URI, caching the result. |
| [[UriTypeConverter\|UriTypeConverter]] | Converts a string representation of a URI into a `Uri` object. |
| [[UrlWebViewSource\|UrlWebViewSource]] | A `WebViewSource` that loads content from a URL. |
| [[ValidateValueDelegate\|ValidateValueDelegate]] |  |
| [[ValidateValueDelegate{TPropertyType}\|ValidateValueDelegate<TPropertyType>]] |  |
| [[ValueChangedEventArgs\|ValueChangedEventArgs]] | Event arguments for numeric value changes. |
| [[VerticalStackLayout\|VerticalStackLayout]] |  |
| [[View\|View]] | A visual element that is used to place layouts and controls on the screen. |
| [[ViewCell (Controls)\|ViewCell (Controls)]] | A `Cell` containing a developer-defined `View`. |
| [[ViewExtensions (Controls)\|ViewExtensions (Controls)]] | Extension methods for `VisualElement`s, providing animatable scaling, rotation, and layout functions. |
| [[VisibilityConverter\|VisibilityConverter]] |  |
| [[VisibilityExtensions\|VisibilityExtensions]] |  |
| [[VisualAttribute\|VisualAttribute]] | Specifies an assembly-level mapping between a visual key name and an `IVisual` type. |
| [[VisualElement (Controls)\|VisualElement (Controls)]] | Provides the base class for all visual elements in .NET MAUI. |
| [[VisualMarker\|VisualMarker]] | Provides static marker instances for built-in `IVisual` types. |
| [[VisualState\|VisualState]] | Represents a named visual state with setters and triggers that define the appearance of a control. |
| [[VisualStateGroup\|VisualStateGroup]] | Contains a collection of mutually exclusive `VisualState` objects and the setters to apply when transitioning between them. |
| [[VisualStateGroupList\|VisualStateGroupList]] | A list of `VisualStateGroup` objects that enforces unique group and state names. |
| [[VisualStateManager\|VisualStateManager]] | Manages visual states for controls (Normal, Focused, Disabled, etc.) and transitions between them. |
| [[VisualTypeConverter\|VisualTypeConverter]] | Converts between string representations and `IVisual` instances. |
| [[WebNavigatedEventArgs\|WebNavigatedEventArgs]] | Event arguments for the `Navigated` event, raised after navigation completes. |
| [[WebNavigatingEventArgs\|WebNavigatingEventArgs]] | Event arguments for the `Navigating` event, raised before navigation begins. |
| [[WebNavigationEventArgs\|WebNavigationEventArgs]] | Base event arguments for `WebView` navigation events. |
| [[WebView (Controls)\|WebView (Controls)]] | A `View` that presents HTML content. |
| [[WebViewInitializedEventArgs\|WebViewInitializedEventArgs]] | Event arguments for the `WebViewInitialized` event. |
| [[WebViewInitializingEventArgs\|WebViewInitializingEventArgs]] | Event arguments for the `WebViewInitializing` event. |
| [[WebViewProcessTerminatedEventArgs\|WebViewProcessTerminatedEventArgs]] |  |
| [[WebViewSource\|WebViewSource]] | Abstract class representing the source content for a `WebView`. |
| [[WebViewSourceTypeConverter\|WebViewSourceTypeConverter]] | A `TypeConverter` that converts a string to a `UrlWebViewSource`. |
| [[WebViewWebResourceRequestedEventArgs\|WebViewWebResourceRequestedEventArgs]] | Event arguments for the `WebResourceRequested` event. |
| [[Window\|Window]] |  |
| [[XmlnsDefinitionAttribute\|XmlnsDefinitionAttribute]] | Specifies the mapping between an XML namespace and a CLR namespace. |
| [[XmlnsPrefixAttribute (Controls)\|XmlnsPrefixAttribute (Controls)]] | Specifies a prefix for an XML namespace when serializing XAML. |

## Interfaces

| Type | Summary |
|---|---|
| [[IAnimatable\|IAnimatable]] |  |
| [[IAppIndexingProvider\|IAppIndexingProvider]] |  |
| [[IAppLinkEntry\|IAppLinkEntry]] |  |
| [[IAppLinks\|IAppLinks]] |  |
| [[IAppearanceObserver\|IAppearanceObserver]] |  |
| [[IApplicationController\|IApplicationController]] |  |
| [[IBindableLayout\|IBindableLayout]] |  |
| [[IBorderElement\|IBorderElement]] |  |
| [[IButtonController\|IButtonController]] |  |
| [[ICellController\|ICellController]] |  |
| [[IConfigElement{T}\|IConfigElement<T>]] |  |
| [[IConfigPlatform\|IConfigPlatform]] |  |
| [[ICornerElement\|ICornerElement]] | Defines properties for elements that can have rounded corners. |
| [[IDecorableTextElement\|IDecorableTextElement]] |  |
| [[IDefinition\|IDefinition]] |  |
| [[IEditorController\|IEditorController]] |  |
| [[IEffectControlProvider\|IEffectControlProvider]] | Provides the functionality to register an `Effect` to an element. |
| [[IElementConfiguration{TElement}\|IElementConfiguration<TElement>]] | Helper that handles storing and lookup of platform specifics implementations |
| [[IElementController\|IElementController]] | For internal use by .NET MAUI. |
| [[IElementExtensions\|IElementExtensions]] |  |
| [[IEntryCellController\|IEntryCellController]] |  |
| [[IEntryController\|IEntryController]] |  |
| [[IExtendedTypeConverter\|IExtendedTypeConverter]] |  |
| [[IFlyoutBehaviorObserver\|IFlyoutBehaviorObserver]] |  |
| [[IFlyoutPageController\|IFlyoutPageController]] |  |
| [[IGestureRecognizer\|IGestureRecognizer]] |  |
| [[IGestureRecognizers\|IGestureRecognizers]] |  |
| [[IGridController\|IGridController]] |  |
| [[IImageController\|IImageController]] |  |
| [[IImageElement\|IImageElement]] |  |
| [[IItemViewController\|IItemViewController]] |  |
| [[IItemsLayout\|IItemsLayout]] | Defines the contract for an items layout that arranges items in collection and carousel views. |
| [[IItemsView{T}\|IItemsView<T>]] |  |
| [[ILayout (Controls)\|ILayout (Controls)]] |  |
| [[ILayoutController\|ILayoutController]] |  |
| [[ILayoutManagerFactory\|ILayoutManagerFactory]] |  |
| [[ILineHeightElement\|ILineHeightElement]] | Defines properties and methods for elements that support line height customization. |
| [[IListProxy\|IListProxy]] |  |
| [[IListViewController\|IListViewController]] |  |
| [[IMenuItemController\|IMenuItemController]] |  |
| [[IMultiPageController{T}\|IMultiPageController<T>]] |  |
| [[IMultiValueConverter\|IMultiValueConverter]] |  |
| [[INavigation\|INavigation]] | Provides the functionality for handling stack-based navigation. |
| [[INavigationPageController\|INavigationPageController]] |  |
| [[IPaddingElement\|IPaddingElement]] |  |
| [[IPageContainer{T}\|IPageContainer<T>]] |  |
| [[IPageController\|IPageController]] |  |
| [[IPanGestureController\|IPanGestureController]] |  |
| [[IPinchGestureController\|IPinchGestureController]] |  |
| [[IPlatformElementConfiguration{TPlatform, TElement}\|IPlatformElementConfiguration<TPlatform, TElement>]] |  |
| [[IQueryAttributable\|IQueryAttributable]] |  |
| [[IRegisterable\|IRegisterable]] |  |
| [[IScrollViewController\|IScrollViewController]] |  |
| [[ISearchBarController\|ISearchBarController]] |  |
| [[ISearchHandlerController\|ISearchHandlerController]] |  |
| [[IShellAppearanceElement\|IShellAppearanceElement]] |  |
| [[IShellContentController\|IShellContentController]] |  |
| [[IShellContentInsetObserver\|IShellContentInsetObserver]] |  |
| [[IShellController\|IShellController]] |  |
| [[IShellItemController\|IShellItemController]] |  |
| [[IShellSectionController\|IShellSectionController]] |  |
| [[ISliderController\|ISliderController]] |  |
| [[ISwipeGestureController\|ISwipeGestureController]] |  |
| [[ISwipeItem (Controls)\|ISwipeItem (Controls)]] |  |
| [[ISwipeViewController\|ISwipeViewController]] |  |
| [[ITableModel\|ITableModel]] |  |
| [[ITableViewController\|ITableViewController]] |  |
| [[ITemplatedItemsList{TItem}\|ITemplatedItemsList<TItem>]] |  |
| [[ITemplatedItemsListScrollToRequestedEventArgs\|ITemplatedItemsListScrollToRequestedEventArgs]] |  |
| [[ITemplatedItemsView{TItem}\|ITemplatedItemsView<TItem>]] |  |
| [[ITextAlignmentElement\|ITextAlignmentElement]] | Defines properties and methods for elements that support text alignment. |
| [[ITextElement\|ITextElement]] | Defines properties and methods for elements that display text. |
| [[IValueConverter\|IValueConverter]] |  |
| [[IViewContainer{T}\|IViewContainer<T>]] |  |
| [[IViewController\|IViewController]] |  |
| [[IVisual\|IVisual]] |  |
| [[IVisualElementController\|IVisualElementController]] | For internal use by .NET MAUI. |
| [[IWebViewController\|IWebViewController]] |  |
| [[IWindowCreator\|IWindowCreator]] |  |

## Structs

| Type | Summary |
|---|---|
| [[LayoutOptions\|LayoutOptions]] | A struct whose static members define various alignment and expansion options. |
| [[Region\|Region]] | Represents a region composed of one or more rectangles. |

## Enums

| Type | Summary |
|---|---|
| [[AccessKeyPlacement\|AccessKeyPlacement]] | Enumerates access key placement relative to the control that the access key describes. |
| [[BindingMode\|BindingMode]] | Specifies the direction of data flow in a binding. |
| [[ButtonsMask\|ButtonsMask]] | Flag values that represent mouse buttons. |
| [[ConstraintType\|ConstraintType]] | Specifies how a constraint is defined. |
| [[DataPackageOperation\|DataPackageOperation]] | Specifies the type of operation performed during a drag and drop operation. |
| [[DependencyFetchTarget\|DependencyFetchTarget]] | Enumeration specifying whether `Get{T}` should return a reference to a global or new instance. |
| [[EditorAutoSizeOption\|EditorAutoSizeOption]] | Enumerates values that control whether an editor will change size to accommodate input as the user enters it. |
| [[EffectiveFlowDirection\|EffectiveFlowDirection]] | Enumerates flags that indicate whether the layout direction was explicitly set, and whether the layout direction is right-to-left. |
| [[FlyoutDisplayOptions\|FlyoutDisplayOptions]] | Specifies how items are displayed in the flyout. |
| [[FlyoutHeaderBehavior\|FlyoutHeaderBehavior]] | Specifies how the flyout header behaves when scrolling. |
| [[FlyoutLayoutBehavior\|FlyoutLayoutBehavior]] | Specifies how the flyout page displays on the screen. |
| [[FontAttributes\|FontAttributes]] | Enumerates values that describe font styles. |
| [[GestureState\|GestureState]] | Enumeration specifying the various states of a gesture. |
| [[ImagePosition\|ImagePosition]] | Enumerates values that determine the position of the image on the button. |
| [[IndicatorShape\|IndicatorShape]] | Specifies the shape of indicators in an `IndicatorView`. |
| [[InitializationFlags\|InitializationFlags]] | Flags that control framework initialization behavior. |
| [[ItemSizingStrategy\|ItemSizingStrategy]] | Specifies the strategy used to measure and size items in an items view. |
| [[ItemsLayoutOrientation\|ItemsLayoutOrientation]] | Specifies the orientation of items in an `ItemsLayout`. |
| [[ItemsUpdatingScrollMode\|ItemsUpdatingScrollMode]] | Specifies the scroll behavior when items are added, removed, or updated in an items view. |
| [[LayoutAlignment (Controls)\|LayoutAlignment (Controls)]] | Values that represent LayoutAlignment. |
| [[ListViewCachingStrategy\|ListViewCachingStrategy]] | Enumerates caching strategies for a ListView. |
| [[ListViewSelectionMode (Controls)\|ListViewSelectionMode (Controls)]] | Enumerates values that control whether items in a list view can or cannot be selected. |
| [[MeasureFlags\|MeasureFlags]] | Enumerates values that tell whether margins are included when laying out windows. |
| [[NamedSize\|NamedSize]] | Represents pre-defined font sizes. |
| [[PresentationMode\|PresentationMode]] | Specifies how pages are presented during navigation. |
| [[RelativeBindingSourceMode\|RelativeBindingSourceMode]] | Specifies the mode for resolving a relative binding source. |
| [[ScrollMode\|ScrollMode]] | Specifies scrolling behavior for a `ScrollView`. |
| [[ScrollToMode\|ScrollToMode]] | Specifies how a scroll-to request should be interpreted. |
| [[ScrollToPosition\|ScrollToPosition]] | Specifies the position to scroll an item to within a list or scroll view. |
| [[SearchBoxVisibility\|SearchBoxVisibility]] | Specifies the visibility behavior of a search box in a `SearchHandler`. |
| [[SelectionMode\|SelectionMode]] | Specifies the selection mode for selectable items views. |
| [[SeparatorVisibility\|SeparatorVisibility]] | Specifies the visibility of separators between items in a `ListView`. |
| [[ShellNavigationSource\|ShellNavigationSource]] | Indicates how Shell navigation was initiated. |
| [[SnapPointsAlignment\|SnapPointsAlignment]] | Specifies how items align to snap points in a scrollable items layout. |
| [[SnapPointsType\|SnapPointsType]] | Specifies the snap points behavior when scrolling through items in an items layout. |
| [[StackOrientation\|StackOrientation]] | Specifies the orientation of a stack layout. |
| [[Stretch\|Stretch]] | Specifies how content is scaled to fill its allocated space. |
| [[SweepDirection\|SweepDirection]] | Specifies the direction in which an arc is drawn. |
| [[TableIntent\|TableIntent]] | Specifies the visual intent of a `TableView`, which determines how it is rendered on each platform. |
| [[TabsStyle\|TabsStyle]] | Enumerates the display styles for tabs in a `TabbedPage` on macOS. |
| [[TargetIdiom\|TargetIdiom]] | Specifies the device form factor. |
| [[ToolbarItemOrder\|ToolbarItemOrder]] | Enumeration specifying whether the ToolbarItem appears on the primary toolbar surface or secondary toolbar surface. |
| [[ViewState\|ViewState]] | Defines flags that represent different interactive states of a view. |

## See also

- [[_API Reference]]
