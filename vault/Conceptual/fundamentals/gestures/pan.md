---
title: "Recognize a pan gesture"
description: "This article explains how to use a .NET MAUI pan gesture to horizontally and vertically pan an image, so that all of the image content can be viewed when it's being displayed in a viewport smaller than the image dimensions."
tags:
  - conceptual
  - area/fundamentals
ms_date: "03/24/2026"
source: "https://learn.microsoft.com/dotnet/maui/fundamentals/gestures/pan?view=net-maui-10.0"
---

# Recognize a pan gesture

A .NET Multi-platform App UI (.NET MAUI) pan gesture recognizer detects the movement of fingers around the screen and can be used to apply that movement to content. A typical scenario for the pan gesture is to horizontally and vertically pan an image, so that all of the image content can be viewed when it's being displayed in a viewport smaller than the image dimensions. This is accomplished by moving the image within the viewport.

In .NET MAUI, pan gesture recognition is provided by the [[PanGestureRecognizer|PanGestureRecognizer]] class. This class defines the [[PanGestureRecognizer.TouchPoints|TouchPoints]] property, of type `int`, which represents the number of touch points in the gesture. The default value of this property is 1. This property is backed by a [[BindableProperty|BindableProperty]] object, which means that it can be the target of data bindings, and styled.

The [[PanGestureRecognizer|PanGestureRecognizer]] class also defines a [[PanGestureRecognizer.PanUpdated|PanUpdated]] event that's raised when the detected pan gesture changes. The [[PanUpdatedEventArgs|PanUpdatedEventArgs]] object that accompanies this event defines the following properties:

- [[PanUpdatedEventArgs.GestureId|GestureId]], of type `int`, which represents the id of the gesture that raised the event.
- [[PanUpdatedEventArgs.StatusType|StatusType]], of type `GestureStatus`, which indicates if the event has been raised for a newly started gesture, a running gesture, a completed gesture, or a canceled gesture.
- [[PanUpdatedEventArgs.TotalX|TotalX]], of type `double`, which indicates the total change in the X direction since the beginning of the gesture.
- [[PanUpdatedEventArgs.TotalY|TotalY]], of type `double`, which indicates the total change in the Y direction since the beginning of the gesture.

## Create a PanGestureRecognizer

To make a [[View|View]] recognize a pan gesture, create a [[PanGestureRecognizer|PanGestureRecognizer]] object, handle the [[PanGestureRecognizer.PanUpdated|PanUpdated]] event, and add the new gesture recognizer to the `GestureRecognizers` collection on the view. The following code example shows a [[PanGestureRecognizer|PanGestureRecognizer]] attached to an [[Image (Controls)|Image]]:

```xaml
<Image Source="monkey.jpg">
    <Image.GestureRecognizers>
        <PanGestureRecognizer PanUpdated="OnPanUpdated" />
    </Image.GestureRecognizers>
</Image>
```

The code for the `OnPanUpdated` event handler should be added to the code-behind file:

```csharp
void OnPanUpdated(object sender, PanUpdatedEventArgs e)
{
    // Handle the pan
}
```

The equivalent C# code is:

```csharp
PanGestureRecognizer panGesture = new PanGestureRecognizer();
panGesture.PanUpdated += (s, e) =>
{
    // Handle the pan
};
image.GestureRecognizers.Add(panGesture);
```

## Create a pan container

Freeform panning is typically suited to navigating within images and maps. The `PanContainer` class, which is shown in the following example, is a generalized helper class that performs freeform panning:

```csharp
public class PanContainer : ContentView
{
    double panX, panY;

    public PanContainer()
    {
        // Set PanGestureRecognizer.TouchPoints to control the
        // number of touch points needed to pan
        PanGestureRecognizer panGesture = new PanGestureRecognizer();
        panGesture.PanUpdated += OnPanUpdated;
        GestureRecognizers.Add(panGesture);
    }

    void OnPanUpdated(object sender, PanUpdatedEventArgs e)
    {
        switch (e.StatusType)
        {
            case GestureStatus.Running:
                // Translate and pan.
                double boundsX = Content.Width;
                double boundsY = Content.Height;
                Content.TranslationX = Math.Clamp(panX + e.TotalX, -boundsX, boundsX);
                Content.TranslationY = Math.Clamp(panY + e.TotalY, -boundsY, boundsY);
                break;

            case GestureStatus.Completed:
                // Store the translation applied during the pan
                panX = Content.TranslationX;
                panY = Content.TranslationY;
                break;
        }
    }
}
```

In this example, the `OnPanUpdated` method updates the viewable content of the wrapped view, based on the user's pan gesture. This is achieved by using the values of the [[PanUpdatedEventArgs.TotalX|TotalX]] and [[PanUpdatedEventArgs.TotalY|TotalY]] properties of the [[PanUpdatedEventArgs|PanUpdatedEventArgs]] instance to calculate the direction and distance of the pan. The wrapped user element is then panned by setting its `TranslationX` and `TranslationY` properties to the calculated values. When panning content in an element that does not occupy the full screen, the height and width of the viewport can be obtained from the element's `Height` and `Width` properties.

The `PanContainer` class can be wrapped around a [[View|View]] so that a recognized pan gesture will pan the wrapped view. The following XAML example shows the `PanContainer` wrapping an [[Image (Controls)|Image]]:

```xaml
<ContentPage xmlns="http://schemas.microsoft.com/dotnet/2021/maui"
             xmlns:x="http://schemas.microsoft.com/winfx/2009/xaml"
             xmlns:local="clr-namespace:PanGesture"
             x:Class="PanGesture.MainPage">
    <Grid>
        <local:PanContainer>
            <Image Source="dotnet_bot.png" />
        </local:PanContainer>
    </Grid>
</ContentPage>
```

In this example, when the [[Image (Controls)|Image]] receives a pan gesture, the displayed image will be panned.
