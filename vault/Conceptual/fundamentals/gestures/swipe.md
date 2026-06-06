---
title: "Recognize a swipe gesture"
description: "This article explains how to recognize a swipe gesture occurring on a view in .NET MAUI."
tags:
  - conceptual
  - area/fundamentals
ms_date: "02/22/2022"
source: "https://learn.microsoft.com/dotnet/maui/fundamentals/gestures/swipe?view=net-maui-10.0"
---

# Recognize a swipe gesture

A .NET Multi-platform App UI (.NET MAUI) swipe gesture recognizer detects when a finger is moved across the screen in a horizontal or vertical direction, and is often used to initiate navigation through content.

In .NET MAUI, drag gesture recognition is provided by the [[SwipeGestureRecognizer|SwipeGestureRecognizer]] class. This class defines the following properties:

- [[SwipeGestureRecognizer.Command|Command]], of type `ICommand`, which is executed when a swipe gesture is recognized.
- [[SwipeGestureRecognizer.CommandParameter|CommandParameter]], of type `object`, which is the parameter that's passed to the `Command`.
- [[SwipeGestureRecognizer.Direction|Direction]], of type [[SwipeDirection|SwipeDirection]], which defines the direction
- [[SwipeGestureRecognizer.Threshold|Threshold]], of type `uint`, which represents the minimum swipe distance that must be achieved for a swipe to be recognized, in device-independent units. The default value of this property is 100, which means that any swipes that are less than 100 device-independent units will be ignored. For more information about device-independent units, see [[device-independent-units|Device-independent units]].

These properties are backed by [[BindableProperty|BindableProperty]] objects, which means that they can be targets of data bindings, and styled.

The [[SwipeGestureRecognizer|SwipeGestureRecognizer]] also defines a [[SwipeGestureRecognizer.Swiped|Swiped]] event that's raised when a swipe is recognized. The [[SwipedEventArgs|SwipedEventArgs]] object that accompanies the [[SwipeGestureRecognizer.Swiped|Swiped]] event defines the following properties:

- [[SwipedEventArgs.Direction|Direction]], of type [[SwipeDirection|SwipeDirection]], indicates the direction of the swipe gesture.
- [[SwipedEventArgs.Parameter|Parameter]], of type `object`, indicates the value passed by the `CommandParameter` property, if defined.

## Create a SwipeGestureRecognizer

To make a [[View|View]] recognize a swipe gesture, create a [[SwipeGestureRecognizer|SwipeGestureRecognizer]] object, set the [[SwipeGestureRecognizer.Direction|Direction]] property to a [[SwipeDirection|SwipeDirection]] enumeration value (`Left`, `Right`, `Up`, or `Down`), optionally set the [[SwipeGestureRecognizer.Threshold|Threshold]] property, handle the [[SwipeGestureRecognizer.Swiped|Swiped]] event, and add the new gesture recognizer to the `GestureRecognizers` collection on the view. The following example shows a [[SwipeGestureRecognizer|SwipeGestureRecognizer]] attached to a [[BoxView (Controls)|BoxView]]:

```xaml
<BoxView Color="Teal" ...>
    <BoxView.GestureRecognizers>
        <SwipeGestureRecognizer Direction="Left" Swiped="OnSwiped"/>
    </BoxView.GestureRecognizers>
</BoxView>
```

The equivalent C# code is:

```csharp
BoxView boxView = new BoxView { Color = Colors.Teal, ... };
SwipeGestureRecognizer leftSwipeGesture = new SwipeGestureRecognizer { Direction = SwipeDirection.Left };
leftSwipeGesture.Swiped += OnSwiped;

boxView.GestureRecognizers.Add(leftSwipeGesture);
```

## Recognize the swipe direction

The `SwipeGestureRecognizer.Direction` property can be set to a single value from the [[SwipeDirection|SwipeDirection]] enumeration, or multiple values. This enables the [[SwipeGestureRecognizer.Swiped|Swiped]] event to be raised in response to a swipe in more than one direction. However, the constraint is that a single [[SwipeGestureRecognizer|SwipeGestureRecognizer]] can only recognize swipes that occur on the same axis. Therefore, swipes that occur on the horizontal axis can be recognized by setting the [[SwipeGestureRecognizer.Direction|Direction]] property to `Left` and `Right`:

```xaml
<SwipeGestureRecognizer Direction="Left,Right" Swiped="OnSwiped"/>
```

Similarly, swipes that occur on the vertical axis can be recognized by setting the [[SwipeGestureRecognizer.Direction|Direction]] property to `Up` and `Down`:

```csharp
SwipeGestureRecognizer swipeGesture = new SwipeGestureRecognizer { Direction = SwipeDirection.Up | SwipeDirection.Down };
```

Alternatively, a [[SwipeGestureRecognizer|SwipeGestureRecognizer]] for each swipe direction can be created to recognize swipes in every direction:

```xaml
<BoxView Color="Teal" ...>
    <BoxView.GestureRecognizers>
        <SwipeGestureRecognizer Direction="Left" Swiped="OnSwiped"/>
        <SwipeGestureRecognizer Direction="Right" Swiped="OnSwiped"/>
        <SwipeGestureRecognizer Direction="Up" Swiped="OnSwiped"/>
        <SwipeGestureRecognizer Direction="Down" Swiped="OnSwiped"/>
    </BoxView.GestureRecognizers>
</BoxView>
```

The equivalent C# code is:

```csharp
BoxView boxView = new BoxView { Color = Colors.Teal, ... };
SwipeGestureRecognizer leftSwipeGesture = new SwipeGestureRecognizer { Direction = SwipeDirection.Left };
leftSwipeGesture.Swiped += OnSwiped;
SwipeGestureRecognizer  rightSwipeGesture = new SwipeGestureRecognizer { Direction = SwipeDirection.Right };
rightSwipeGesture.Swiped += OnSwiped;
SwipeGestureRecognizer  upSwipeGesture = new SwipeGestureRecognizer { Direction = SwipeDirection.Up };
upSwipeGesture.Swiped += OnSwiped;
SwipeGestureRecognizer  downSwipeGesture = new SwipeGestureRecognizer { Direction = SwipeDirection.Down };
downSwipeGesture.Swiped += OnSwiped;

boxView.GestureRecognizers.Add(leftSwipeGesture);
boxView.GestureRecognizers.Add(rightSwipeGesture);
boxView.GestureRecognizers.Add(upSwipeGesture);
boxView.GestureRecognizers.Add(downSwipeGesture);
```

## Respond to a swipe

A recognized swipe can be responded to by a handler for the [[SwipeGestureRecognizer.Swiped|Swiped]] event:

```csharp
void OnSwiped(object sender, SwipedEventArgs e)
{
    switch (e.Direction)
    {
        case SwipeDirection.Left:
            // Handle the swipe
            break;
        case SwipeDirection.Right:
            // Handle the swipe
            break;
        case SwipeDirection.Up:
            // Handle the swipe
            break;
        case SwipeDirection.Down:
            // Handle the swipe
            break;
    }
}
```

The [[SwipedEventArgs|SwipedEventArgs]] can be examined to determine the direction of the swipe, with custom logic responding to the swipe as required. The direction of the swipe can be obtained from the [[SwipedEventArgs.Direction|Direction]] property of the event arguments, which will be set to one of the values of the [[SwipeDirection|SwipeDirection]] enumeration. In addition, the event arguments also have a [[SwipedEventArgs.Parameter|Parameter]] property that will be set to the value of the `CommandParameter` property, if defined.

## Create a swipe container

The `SwipeContainer` class, which is shown in the following example, is a generalized swipe recognition class that be wrapped around a [[View|View]] to perform swipe gesture recognition:

```csharp
public class SwipeContainer : ContentView
{
    public event EventHandler<SwipedEventArgs> Swipe;

    public SwipeContainer()
    {
        GestureRecognizers.Add(GetSwipeGestureRecognizer(SwipeDirection.Left));
        GestureRecognizers.Add(GetSwipeGestureRecognizer(SwipeDirection.Right));
        GestureRecognizers.Add(GetSwipeGestureRecognizer(SwipeDirection.Up));
        GestureRecognizers.Add(GetSwipeGestureRecognizer(SwipeDirection.Down));
    }

    SwipeGestureRecognizer GetSwipeGestureRecognizer(SwipeDirection direction)
    {
        SwipeGestureRecognizer swipe = new SwipeGestureRecognizer { Direction = direction };
        swipe.Swiped += (sender, e) => Swipe?.Invoke(this, e);
        return swipe;
    }
}
```

The `SwipeContainer` class creates [[SwipeGestureRecognizer|SwipeGestureRecognizer]] objects for all four swipe directions, and attaches `Swipe` event handlers. These event handlers invoke the `Swipe` event defined by the `SwipeContainer`.

The following XAML code example shows the `SwipeContainer` class wrapping a [[BoxView (Controls)|BoxView]]:

```xaml
<StackLayout>
    <local:SwipeContainer Swipe="OnSwiped" ...>
        <BoxView Color="Teal" ... />
    </local:SwipeContainer>
</StackLayout>
```

In this example, when the [[BoxView (Controls)|BoxView]] receives a swipe gesture, the [[SwipeGestureRecognizer.Swiped|Swiped]] event in the [[SwipeGestureRecognizer|SwipeGestureRecognizer]] is raised. This is handled by the `SwipeContainer` class, which raises its own `Swipe` event. This `Swipe` event is handled on the page. The [[SwipedEventArgs|SwipedEventArgs]] can then be examined to determine the direction of the swipe, with custom logic responding to the swipe as required.

The equivalent C# code is:

```csharp
BoxView boxView = new BoxView { Color = Colors.Teal, ... };
SwipeContainer swipeContainer = new SwipeContainer { Content = boxView, ... };
swipeContainer.Swipe += (sender, e) =>
{
  // Handle the swipe
};

StackLayout stackLayout = new StackLayout();
stackLayout.Add(swipeContainer);
```
