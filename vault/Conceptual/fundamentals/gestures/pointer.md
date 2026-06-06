---
title: "Recognize a pointer gesture"
description: "Learn how to use the PointerGestureRecognizer class, to detect when the pointer enters, exits, and moves within a view on iPadOS, Mac Catalyst, and Windows."
tags:
  - conceptual
  - area/fundamentals
ms_date: "10/16/2023"
source: "https://learn.microsoft.com/dotnet/maui/fundamentals/gestures/pointer?view=net-maui-10.0"
---

# Recognize a pointer gesture

A .NET Multi-platform App UI (.NET MAUI) pointer gesture recognizer detects when the pointer enters, exits, and moves within a view and is implemented with the [[PointerGestureRecognizer|PointerGestureRecognizer]] class. This class defines the following properties:

- [[PointerGestureRecognizer.PointerEnteredCommand|PointerEnteredCommand]], of type `ICommand`, which is the command to invoke when the pointer enters the bounding area of the view.
- [[PointerGestureRecognizer.PointerEnteredCommandParameter|PointerEnteredCommandParameter]], of type `object`, which is the parameter that's passed to [[PointerGestureRecognizer.PointerEnteredCommand|PointerEnteredCommand]].
- [[PointerGestureRecognizer.PointerExitedCommand|PointerExitedCommand]], of type `ICommand`, which is the command to invoke when the pointer that's in the view's bounding area leaves that bounding area.
- [[PointerGestureRecognizer.PointerExitedCommandParameter|PointerExitedCommandParameter]], of type `object`, which is the parameter that's passed to [[PointerGestureRecognizer.PointerExitedCommand|PointerExitedCommand]].
- [[PointerGestureRecognizer.PointerMovedCommand|PointerMovedCommand]], of type `ICommand`, which is the command to invoke when the pointer moves while remaining within the bounding area of the view.
- [[PointerGestureRecognizer.PointerMovedCommandParameter|PointerMovedCommandParameter]], of type `object`, which is the parameter that's passed to [[PointerGestureRecognizer.PointerMovedCommand|PointerMovedCommand]].
- [[PointerGestureRecognizer.PointerPressedCommand|PointerPressedCommand]], of type `ICommand`, which is the command to invoke when the pointer initiates a press within the view.
- [[PointerGestureRecognizer.PointerPressedCommandParameter|PointerPressedCommandParameter]], of type `object`, which is the parameter that's passed to the [[PointerGestureRecognizer.PointerPressedCommand|PointerPressedCommand]].
- [[PointerGestureRecognizer.PointerReleasedCommand|PointerReleasedCommand]], of type `ICommand`, which is the command to invoke when the pointer that has previously initiated a press is released, while within the view.
- [[PointerGestureRecognizer.PointerReleasedCommandParameter|PointerReleasedCommandParameter]], of type `object`, which is the parameter that's passed to the [[PointerGestureRecognizer.PointerReleasedCommand|PointerReleasedCommand]].

These properties are backed by [[BindableProperty|BindableProperty]] objects, which means that they can be targets of data bindings, and styled.

The [[PointerGestureRecognizer|PointerGestureRecognizer]] class also defines the following events:

- [[PointerGestureRecognizer.PointerEntered|PointerEntered]], that's raised when the pointer enters the bounding area of the view.
- [[PointerGestureRecognizer.PointerExited|PointerExited]], that's raised when the pointer that's in the view's bounding area leaves that bounding area.
- [[PointerGestureRecognizer.PointerMoved|PointerMoved]], that's raised when the pointer moves while remaining within the bounding area of the view.
- [[PointerGestureRecognizer.PointerPressed|PointerPressed]], that's raised when the pointer initiates a press within the view.
- [[PointerGestureRecognizer.PointerReleased|PointerReleased]], that's raised when the pointer that has previously initiated a press is released, while within the view.

A [[PointerEventArgs|PointerEventArgs]] object accompanies the events, and defines a [[PointerEventArgs.PlatformArgs|PlatformArgs]] property of type [[PlatformPointerEventArgs|PlatformPointerEventArgs]] that provides access to platform-specific arguments for the event.

<!-- markdownlint-disable MD025 -->

# [Android](#tab/android)

On Android, the [[PlatformPointerEventArgs|PlatformPointerEventArgs]] class defines the following properties:

- `Sender`, of type `View`, represents the native view attached to the event.
- `MotionEvent`, of type `MotionEvent`, indicates the native event or handler attached to the view.

# [iOS/Mac Catalyst](#tab/macios)

On iOS and Mac Catalyst, the [[PlatformPointerEventArgs|PlatformPointerEventArgs]] class defines the following properties:

- `Sender`, of type `UIView`, represents the native view attached to the event.
- `GestureRecognizer`, of type `UIGestureRecognizer`, indicates the native event or handler attached to the view.

# [Windows](#tab/windows)

On Windows, the [[PlatformPointerEventArgs|PlatformPointerEventArgs]] class defines the following properties:

- `Sender`, of type `FrameworkElement`, represents the native view attached to the event.
- `PointerRoutedEventArgs`, of type `PointerRoutedEventArgs`, indicates the native event or handler attached to the view.

---

<!-- markdownlint-enable MD025 -->

In addition, the [[PointerEventArgs|PointerEventArgs]] object defines a `GetPosition%2A` method that returns a `Point?` object that represents the position of the pointer when the gesture was detected. For more information about the `GetPosition%2A` method, see [Get the gesture position](#get-the-gesture-position).

> [!IMPORTANT]
> Pointer gesture recognition is supported on Android, iPadOS, Mac Catalyst, and Windows.

.NET MAUI also defines a `PointerOver` visual state. This state can change the visual appearance of a view when it has a mouse cursor hovering over it, but isn't pressed. For more information, see [[visual-states|Visual states]].

## Create a PointerGestureRecognizer

To make a [[View|View]] recognize pointer gestures, create a [[PointerGestureRecognizer|PointerGestureRecognizer]] object, handle the required events, and add the gesture recognizer to the [[View.GestureRecognizers|GestureRecognizers]] collection on the view.
Alternatively, create a [[PointerGestureRecognizer|PointerGestureRecognizer]] object, and bind the required commands to `ICommand` implementations, and add the gesture recognizer to the [[View.GestureRecognizers|GestureRecognizers]] collection on the view.

The following code example shows a [[PointerGestureRecognizer|PointerGestureRecognizer]] attached to an [[Image (Controls)|Image]]. The [[PointerGestureRecognizer|PointerGestureRecognizer]] uses events to respond to the detection of pointer gestures:

```xaml
<Image Source="dotnet_bot.png">
    <Image.GestureRecognizers>
        <PointerGestureRecognizer PointerEntered="OnPointerEntered"
                                  PointerExited="OnPointerExited"
                                  PointerMoved="OnPointerMoved" />
  </Image.GestureRecognizers>
</Image>
```

The code for the event handlers should be added to the code-behind file:

```csharp
void OnPointerEntered(object sender, PointerEventArgs e)
{
    // Handle the pointer entered event
}

void OnPointerExited(object sender, PointerEventArgs e)
{
    // Handle the pointer exited event
}

void OnPointerMoved(object sender, PointerEventArgs e)
{
    // Handle the pointer moved event
}
```

The equivalent C# code is:

```csharp
PointerGestureRecognizer pointerGestureRecognizer = new PointerGestureRecognizer();
pointerGestureRecognizer.PointerEntered += (s, e) =>
{
    // Handle the pointer entered event
};
pointerGestureRecognizer.PointerExited += (s, e) =>
{
    // Handle the pointer exited event
};
pointerGestureRecognizer.PointerMoved += (s, e) =>
{
    // Handle the pointer moved event
};

Image image = new Image();
image.GestureRecognizers.Add(pointerGestureRecognizer);
```

## Get the gesture position

The position at which a pointer gesture occurred can be obtained by calling the `GetPosition%2A` method on a [[PointerEventArgs|PointerEventArgs]] object. The `GetPosition%2A` method accepts an `Element?` argument, and returns a position as a `Point?` object:

```csharp
void OnPointerExited(object sender, PointerEventArgs e)
{
    // Position inside window
    Point? windowPosition = e.GetPosition(null);

    // Position relative to an Image
    Point? relativeToImagePosition = e.GetPosition(image);

    // Position relative to the container view
    Point? relativeToContainerPosition = e.GetPosition((View)sender);
}
```

The `Element?` argument defines the element the position should be obtained relative to. Supplying a `null` value as this argument means that the `GetPosition%2A` method returns a `Point?` object that defines the position of the pointer gesture inside the window.
