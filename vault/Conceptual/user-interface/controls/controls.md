---
title: "Controls"
description: "The user interface of a .NET MAUI app is constructed from pages, layouts, and views."
tags:
  - conceptual
  - area/user-interface
ms_date: "08/30/2024"
source: "https://learn.microsoft.com/dotnet/maui/user-interface/controls?view=net-maui-10.0"
---

# Controls

[![Browse sample.](~/media/code-sample.png) Browse the sample](/samples/dotnet/maui-samples/userinterface-controlgallery)

The user interface of a .NET Multi-platform App UI (.NET MAUI) app is constructed of objects that map to the native controls of each target platform.

The main control groups used to create the user interface of a .NET MAUI app are pages, layouts, and views. A .NET MAUI page generally occupies the full screen or window. The page usually contains a layout, which contains views and possibly other layouts. Pages, layouts, and views derive from the [[VisualElement (Controls)|VisualElement]] class. This class provides a variety of properties, methods, and events that are useful in derived classes.

> [!NOTE]
> [[ListView (Controls)|ListView]] and [[TableView|TableView]] also support the use of cells. Cells are specialized elements used for items in a table, that describe how each item should be rendered.

## Pages

.NET MAUI apps consist of one or more pages. A page usually occupies all of the screen, or window, and each page typically contains at least one layout.

.NET MAUI contains the following pages:

| Page | Description |
| --- | --- |
| [[ContentPage|ContentPage]] | [[ContentPage|ContentPage]] displays a single view, and is the most common page type. For more information, see [[contentpage|ContentPage]]. |
| [[FlyoutPage (Controls)|FlyoutPage]] | [[FlyoutPage (Controls)|FlyoutPage]] is a page that manages two related pages of information – a flyout page that presents items, and a detail page that presents details about items on the flyout page. For more information, see [[flyoutpage|FlyoutPage]]. |
| [[NavigationPage (Controls)|NavigationPage]] | [[NavigationPage (Controls)|NavigationPage]] provides a hierarchical navigation experience where you're able to navigate through pages, forwards and backwards, as desired. For more information, see [[navigationpage|NavigationPage]]. |
| [[TabbedPage (Controls)|TabbedPage]] | [[TabbedPage (Controls)|TabbedPage]] consists of a series of pages that are navigable by tabs across the top or bottom of the page, with each tab loading the page content. For more information, see [[tabbedpage|TabbedPage]]. |

## Layouts

.NET MAUI layouts are used to compose user-interface controls into visual structures, and each layout typically contains multiple views. Layout classes typically contain logic to set the position and size of child elements.

.NET MAUI contains the following layouts:

| Layout | Description |
| --- | --- |
| [[AbsoluteLayout (Controls)|AbsoluteLayout]] | [[AbsoluteLayout (Controls)|AbsoluteLayout]] positions child elements at specific locations relative to its parent. For more information, see [[absolutelayout|AbsoluteLayout]]. |
| [[BindableLayout|BindableLayout]] | [[BindableLayout|BindableLayout]] enables any layout class to generate its content by binding to a collection of items, with the option to set the appearance of each item. For more information, see [[bindablelayout|BindableLayout]]. |
| [[FlexLayout (Controls)|FlexLayout]]| [[FlexLayout (Controls)|FlexLayout]] enables its children to be stacked or wrapped with different alignment and orientation options. [[FlexLayout (Controls)|FlexLayout]] is based on the CSS Flexible Box Layout Module, known as *flex layout* or *flex-box*. For more information, see [[flexlayout|FlexLayout]]. |
| [[Grid (Controls)|Grid]] | [[Grid (Controls)|Grid]] positions its child elements in a grid of rows and columns. For more information, see [[grid|Grid]]. |
| [[HorizontalStackLayout|HorizontalStackLayout]] | [[HorizontalStackLayout|HorizontalStackLayout]] positions child elements in a horizontal stack. For more information, see [[horizontalstacklayout|HorizontalStackLayout]]. |
| [[StackLayout (Controls)|StackLayout]] | [[StackLayout (Controls)|StackLayout]] positions child elements in either a vertical or horizontal stack. For more information, see [[stacklayout|StackLayout]]. |
| [[VerticalStackLayout|VerticalStackLayout]] | [[VerticalStackLayout|VerticalStackLayout]] positions child elements in a vertical stack. For more information, see [[verticalstacklayout|VerticalStackLayout]]. |

## Views

.NET MAUI views are the UI objects such as labels, buttons, and sliders that are commonly known as *controls* or *widgets* in other environments.

.NET MAUI contains the following views:


| View | Description |
| --- | --- |
| [[ActivityIndicator|ActivityIndicator]] | [[ActivityIndicator|ActivityIndicator]] uses an animation to show that the app is engaged in a lengthy activity, without giving any indication of progress. For more information, see [[activityindicator|ActivityIndicator]]. |
| [[BlazorWebView (Maui)|BlazorWebView]] | [[BlazorWebView (Maui)|BlazorWebView]] enables you to host a Blazor web app in your .NET MAUI app. For more information, see [[blazorwebview|BlazorWebView]]. |
| [[Border|Border]] | [[Border|Border]] is a container control that draws a border, background, or both, around another control. For more information, see [[border|Border]]. |
| [[BoxView (Controls)|BoxView]] | [[BoxView (Controls)|BoxView]] draws a rectangle or square, of a specified width, height, and color. For more information, see [[boxview|BoxView]]. |
| [[Button (Controls)|Button]] | [[Button (Controls)|Button]] displays text and responds to a tap or click that directs an app to carry out a task. For more information, see [[button|Button]]. |
| [[CarouselView|CarouselView]] | [[CarouselView|CarouselView]] displays a scrollable list of data items, where users swipe to move through the collection. For more information, see [[carouselview|CarouselView]]. |
| [[CheckBox|CheckBox]] | [[CheckBox|CheckBox]] enables you to select a boolean value using a type of button that can either be checked or empty. For more information, see [[checkbox|CheckBox]]. |
| [[CollectionView|CollectionView]] | [[CollectionView|CollectionView]] displays a scrollable list of selectable data items, using different layout specifications. For more information, see [[collectionview|CollectionView]]. |
| [[ContentView (Controls)|ContentView]] | [[ContentView (Controls)|ContentView]] is a control that enables the creation of custom, reusable controls. For more information, see [[contentview|ContentView]]. |
| [[DatePicker (Controls)|DatePicker]] | [[DatePicker (Controls)|DatePicker]] enables you to select a date with the platform date picker. For more information, see [[datepicker|DatePicker]]. |
| [[Editor|Editor]] | [[Editor|Editor]] enables you to enter and edit multiple lines of text. For more information, see [[editor|Editor]]. |
| [[Ellipse|Ellipse]] | [[Ellipse|Ellipse]] displays an ellipse or circle. For more information, see [[ellipse|Ellipse]]. |
| [[Entry (Controls)|Entry]] | [[Entry (Controls)|Entry]] enables you to enter and edit a single line of text. For more information, see [[entry|Entry]]. |
| [[Frame|Frame]] | [[Frame|Frame]] is used to wrap a view or layout with a border that can be configured with color, shadow, and other options. For more information, see [[frame|Frame]]. |
| [[GraphicsView|GraphicsView]] | [[GraphicsView|GraphicsView]] is a graphics canvas on which 2D graphics can be drawn using types from the `Graphics` namespace. For more information, see [[graphicsview|GraphicsView]]. |
| [[Image (Controls)|Image]] | [[Image (Controls)|Image]] displays an image that can be loaded from a local file, a URI, an embedded resource, or a stream. For more information, see [[image|Image]]. |
| [[ImageButton (Controls)|ImageButton]] | [[ImageButton (Controls)|ImageButton]] displays an image and responds to a tap or click that direct an app to carry out a task. For more information, see [[imagebutton|ImageButton]]. |
| [[IndicatorView|IndicatorView]] | [[IndicatorView|IndicatorView]] displays indicators that represent the number of items in a [[CarouselView|CarouselView]]. For more information, see [[indicatorview|IndicatorView]]. |
| [[Label (Controls)|Label]] | [[Label (Controls)|Label]] displays single-line and multi-line text. For more information, see [[label|Label]]. |
| [[Line|Line]] | [[Line|Line]] displays a line from a start point to an end point. For more information, see [[line|Line]]. |
| [[ListView (Controls)|ListView]] | [[ListView (Controls)|ListView]] displays a scrollable list of selectable data items. For more information, see [[listview|ListView]]. |
| [[Map (Maps)|Map]] | [[Map (Maps)|Map]] displays a map, and requires the **Microsoft.Maui.Controls.Maps** NuGet package to be installed in your app. |
| [[Path|Path]] | [[Path|Path]] display curves and complex shapes. For more information, see [[path|Path]]. |
| [[Picker (Controls)|Picker]] | [[Picker (Controls)|Picker]] displays a short list of items, from which an item can be selected. For more information, see [[picker|Picker]]. |
| [[Polygon (Shapes)|Polygon]] | [[Polygon (Shapes)|Polygon]] displays a polygon. For more information, see [[polygon|Polygon]]. |
| [[Polyline (Shapes)|Polyline]] | [[Polyline (Shapes)|Polyline]] displays a series of connected straight lines. For more information, see [[polyline|Polyline]]. |
| [[ProgressBar (Controls)|ProgressBar]] | [[ProgressBar (Controls)|ProgressBar]] uses an animation to show that the app is progressing through a lengthy activity. For more information, see [[progressbar|ProgressBar]]. |
| [[RadioButton|RadioButton]] | [[RadioButton|RadioButton]] is a type of button that allows the selection of one option from a set. For more information, see [[radiobutton|RadioButton]]. |
| [[Rectangle|Rectangle]] | [[Rectangle|Rectangle]] displays a rectangle or square. For more information, see [[rectangle|Rectangle]]. |
| [[RefreshView (Controls)|RefreshView]] | [[RefreshView (Controls)|RefreshView]] is a container control that provides pull-to-refresh functionality for scrollable content. For more information, see [[refreshview|RefreshView]]. |
| [[RoundRectangle|RoundRectangle]] | [[RoundRectangle|RoundRectangle]] displays a rectangle or square with rounded corners. For more information, see [[rectangle|Rectangle]]. |
| [[ScrollView (Controls)|ScrollView]] | [[ScrollView (Controls)|ScrollView]] provides scrolling of its content, which is typically a layout. For more information, see [[scrollview|ScrollView]]. |
| [[SearchBar (Controls)|SearchBar]] | [[SearchBar (Controls)|SearchBar]] is a user input control used to initiate a search. For more information, see [[searchbar|SearchBar]]. |
| [[Slider (Controls)|Slider]] | [[Slider (Controls)|Slider]] enables you to select a `double` value from a continuous range. For more information, see [[slider|Slider]]. |
| [[Stepper|Stepper]] | [[Stepper|Stepper]] enables you to select a `double` value from a range of incremental values. For more information, see [[stepper|Stepper]]. |
| [[SwipeView (Controls)|SwipeView]] | [[SwipeView (Controls)|SwipeView]] is a container control that wraps around an item of content, and provides context menu items that are revealed by a swipe gesture. For more information, see [[swipeview|SwipeView]]. |
| [[Switch (Controls)|Switch]] | [[Switch (Controls)|Switch]] enables you to select a boolean value using a type of button that can either be on or off. For more information, see [[switch|Switch]]. |
| [[TableView|TableView]] | [[TableView|TableView]] displays a table of scrollable items that can be grouped into sections. For more information, see [[tableview|TableView]]. |
| [[TimePicker (Controls)|TimePicker]] | [[TimePicker (Controls)|TimePicker]] enables you to select a time with the platform time picker. For more information, see [[timepicker|TimePicker]]. |
| `TwoPaneView` | `TwoPaneView` represents a container with two views that size and position their content in the available space, either side-by-side or top-to-bottom. For more information, see [[twopaneview|TwoPaneView]]. |
| [[WebView (Controls)|WebView]] | [[WebView (Controls)|WebView]] displays web pages or local HTML content. For more information, see [[webview|WebView]]. |



| View | Description |
| --- | --- |
| [[ActivityIndicator|ActivityIndicator]] | [[ActivityIndicator|ActivityIndicator]] uses an animation to show that the app is engaged in a lengthy activity, without giving any indication of progress. For more information, see [[activityindicator|ActivityIndicator]]. |
| [[BlazorWebView (Maui)|BlazorWebView]] | [[BlazorWebView (Maui)|BlazorWebView]] enables you to host a Blazor web app in your .NET MAUI app. For more information, see [[blazorwebview|BlazorWebView]]. |
| [[Border|Border]] | [[Border|Border]] is a container control that draws a border, background, or both, around another control. For more information, see [[border|Border]]. |
| [[BoxView (Controls)|BoxView]] | [[BoxView (Controls)|BoxView]] draws a rectangle or square, of a specified width, height, and color. For more information, see [[boxview|BoxView]]. |
| [[Button (Controls)|Button]] | [[Button (Controls)|Button]] displays text and responds to a tap or click that directs an app to carry out a task. For more information, see [[button|Button]]. |
| [[CarouselView|CarouselView]] | [[CarouselView|CarouselView]] displays a scrollable list of data items, where users swipe to move through the collection. For more information, see [[carouselview|CarouselView]]. |
| [[CheckBox|CheckBox]] | [[CheckBox|CheckBox]] enables you to select a boolean value using a type of button that can either be checked or empty. For more information, see [[checkbox|CheckBox]]. |
| [[CollectionView|CollectionView]] | [[CollectionView|CollectionView]] displays a scrollable list of selectable data items, using different layout specifications. For more information, see [[collectionview|CollectionView]]. |
| [[ContentView (Controls)|ContentView]] | [[ContentView (Controls)|ContentView]] is a control that enables the creation of custom, reusable controls. For more information, see [[contentview|ContentView]]. |
| [[DatePicker (Controls)|DatePicker]] | [[DatePicker (Controls)|DatePicker]] enables you to select a date with the platform date picker. For more information, see [[datepicker|DatePicker]]. |
| [[Editor|Editor]] | [[Editor|Editor]] enables you to enter and edit multiple lines of text. For more information, see [[editor|Editor]]. |
| [[Ellipse|Ellipse]] | [[Ellipse|Ellipse]] displays an ellipse or circle. For more information, see [[ellipse|Ellipse]]. |
| [[Entry (Controls)|Entry]] | [[Entry (Controls)|Entry]] enables you to enter and edit a single line of text. For more information, see [[entry|Entry]]. |
| [[Frame|Frame]] | [[Frame|Frame]] is used to wrap a view or layout with a border that can be configured with color, shadow, and other options. For more information, see [[frame|Frame]]. |
| [[GraphicsView|GraphicsView]] | [[GraphicsView|GraphicsView]] is a graphics canvas on which 2D graphics can be drawn using types from the `Graphics` namespace. For more information, see [[graphicsview|GraphicsView]]. |
| [[HybridWebView|HybridWebView]] | [[HybridWebView|HybridWebView]] enables you to host arbitrary HTML/JS/CSS content in a web view, and enables communication between the code in the web view (JavaScript) and the code that hosts the web view (C#/.NET). For more information, see [[hybridwebview|HybridWebView]]. |
| [[Image (Controls)|Image]] | [[Image (Controls)|Image]] displays an image that can be loaded from a local file, a URI, an embedded resource, or a stream. For more information, see [[image|Image]]. |
| [[ImageButton (Controls)|ImageButton]] | [[ImageButton (Controls)|ImageButton]] displays an image and responds to a tap or click that direct an app to carry out a task. For more information, see [[imagebutton|ImageButton]]. |
| [[IndicatorView|IndicatorView]] | [[IndicatorView|IndicatorView]] displays indicators that represent the number of items in a [[CarouselView|CarouselView]]. For more information, see [[indicatorview|IndicatorView]]. |
| [[Label (Controls)|Label]] | [[Label (Controls)|Label]] displays single-line and multi-line text. For more information, see [[label|Label]]. |
| [[Line|Line]] | [[Line|Line]] displays a line from a start point to an end point. For more information, see [[line|Line]]. |
| [[ListView (Controls)|ListView]] | [[ListView (Controls)|ListView]] displays a scrollable list of selectable data items. For more information, see [[listview|ListView]]. |
| [[Map (Maps)|Map]] | [[Map (Maps)|Map]] displays a map, and requires the **Microsoft.Maui.Controls.Maps** NuGet package to be installed in your app. |
| [[Path|Path]] | [[Path|Path]] display curves and complex shapes. For more information, see [[path|Path]]. |
| [[Picker (Controls)|Picker]] | [[Picker (Controls)|Picker]] displays a short list of items, from which an item can be selected. For more information, see [[picker|Picker]]. |
| [[Polygon (Shapes)|Polygon]] | [[Polygon (Shapes)|Polygon]] displays a polygon. For more information, see [[polygon|Polygon]]. |
| [[Polyline (Shapes)|Polyline]] | [[Polyline (Shapes)|Polyline]] displays a series of connected straight lines. For more information, see [[polyline|Polyline]]. |
| [[ProgressBar (Controls)|ProgressBar]] | [[ProgressBar (Controls)|ProgressBar]] uses an animation to show that the app is progressing through a lengthy activity. For more information, see [[progressbar|ProgressBar]]. |
| [[RadioButton|RadioButton]] | [[RadioButton|RadioButton]] is a type of button that allows the selection of one option from a set. For more information, see [[radiobutton|RadioButton]]. |
| [[Rectangle|Rectangle]] | [[Rectangle|Rectangle]] displays a rectangle or square. For more information, see [[rectangle|Rectangle]]. |
| [[RefreshView (Controls)|RefreshView]] | [[RefreshView (Controls)|RefreshView]] is a container control that provides pull-to-refresh functionality for scrollable content. For more information, see [[refreshview|RefreshView]]. |
| [[RoundRectangle|RoundRectangle]] | [[RoundRectangle|RoundRectangle]] displays a rectangle or square with rounded corners. For more information, see [[rectangle|Rectangle]]. |
| [[ScrollView (Controls)|ScrollView]] | [[ScrollView (Controls)|ScrollView]] provides scrolling of its content, which is typically a layout. For more information, see [[scrollview|ScrollView]]. |
| [[SearchBar (Controls)|SearchBar]] | [[SearchBar (Controls)|SearchBar]] is a user input control used to initiate a search. For more information, see [[searchbar|SearchBar]]. |
| [[Slider (Controls)|Slider]] | [[Slider (Controls)|Slider]] enables you to select a `double` value from a continuous range. For more information, see [[slider|Slider]]. |
| [[Stepper|Stepper]] | [[Stepper|Stepper]] enables you to select a `double` value from a range of incremental values. For more information, see [[stepper|Stepper]]. |
| [[SwipeView (Controls)|SwipeView]] | [[SwipeView (Controls)|SwipeView]] is a container control that wraps around an item of content, and provides context menu items that are revealed by a swipe gesture. For more information, see [[swipeview|SwipeView]]. |
| [[Switch (Controls)|Switch]] | [[Switch (Controls)|Switch]] enables you to select a boolean value using a type of button that can either be on or off. For more information, see [[switch|Switch]]. |
| [[TableView|TableView]] | [[TableView|TableView]] displays a table of scrollable items that can be grouped into sections. For more information, see [[tableview|TableView]]. |
| [[TimePicker (Controls)|TimePicker]] | [[TimePicker (Controls)|TimePicker]] enables you to select a time with the platform time picker. For more information, see [[timepicker|TimePicker]]. |
| [[TitleBar|TitleBar]] | [[TitleBar|TitleBar]] enables you to add a custom title bar to a [[Window|Window]] to match the personality of your app. For more information, see [[titlebar|TitleBar]]. |
| `TwoPaneView` | `TwoPaneView` represents a container with two views that size and position their content in the available space, either side-by-side or top-to-bottom. For more information, see [[twopaneview|TwoPaneView]]. |
| [[WebView (Controls)|WebView]] | [[WebView (Controls)|WebView]] displays web pages or local HTML content. For more information, see [[webview|WebView]]. |

