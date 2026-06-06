---
title: "BoxView"
description: "The .NET MAUI BoxView draws a simple rectangle or square, of a specified width, height, and color."
tags:
  - conceptual
  - area/user-interface
ms_date: "08/30/2024"
source: "https://learn.microsoft.com/dotnet/maui/user-interface/controls/boxview?view=net-maui-10.0"
---

# BoxView

The .NET Multi-platform App UI (.NET MAUI) [[BoxView (Controls)|BoxView]] draws a simple rectangle or square, of a specified width, height, and color.

[[BoxView (Controls)|BoxView]] defines the following properties:

- `Color`, of type [[Color|Color]], which defines the color of the [[BoxView (Controls)|BoxView]].
- `CornerRadius`, of type `CornerRadius`, which defines the corner radius of the [[BoxView (Controls)|BoxView]]. This property can be set to a single `double` uniform corner radius value, or a `CornerRadius` structure defined by four `double` values that are applied to the top left, top right, bottom left, and bottom right of the [[BoxView (Controls)|BoxView]].

These properties are backed by [[BindableProperty|BindableProperty]] objects, which means that they can be targets of data bindings, and styled.

> [!NOTE]
> Although [[BoxView (Controls)|BoxView]] can mimic simple graphics, a better alternative is to use .NET MAUI Shapes or [[graphics|.NET MAUI Graphics]].

## Create a BoxView

To draw a rectangle or square, create a [[BoxView (Controls)|BoxView]] object and set its `Color`, [[VisualElement (Controls).WidthRequest|WidthRequest]], and [[VisualElement (Controls).HeightRequest|HeightRequest]] properties. Optionally, you can also set its `CornerRadius` property.

The following XAML example shows how to create a [[BoxView (Controls)|BoxView]]:

```xaml
<ContentPage xmlns="http://schemas.microsoft.com/dotnet/2021/maui"
             xmlns:x="http://schemas.microsoft.com/winfx/2009/xaml"
             xmlns:local="clr-namespace:BasicBoxView"
             x:Class="BasicBoxView.MainPage">
    <BoxView Color="CornflowerBlue"
             CornerRadius="10"
             WidthRequest="160"
             HeightRequest="160"
             VerticalOptions="Center"
             HorizontalOptions="Center" />
</ContentPage>
```

In this example, a cornflower blue [[BoxView (Controls)|BoxView]] is displayed in the center of the page:

![](media/boxview/boxview-basic.png)

The [[VisualElement (Controls).WidthRequest|WidthRequest]] and [[VisualElement (Controls).HeightRequest|HeightRequest]] properties are measured in device-independent units.

> [!NOTE]
> A [[BoxView (Controls)|BoxView]] can also be a child of an [[AbsoluteLayout (Controls)|AbsoluteLayout]]. In this case, both the location and size of the [[BoxView (Controls)|BoxView]] are set using the `LayoutBounds` attached bindable property.

A [[BoxView (Controls)|BoxView]] can also be sized to resemble a line of a specific width and thickness.
