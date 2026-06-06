---
title: "Custom animation"
description: "The .NET MAUI Animation class can create and cancel animations, synchronize multiple animations, and create custom animations that animate properties that aren't animated by existing animation methods."
tags:
  - conceptual
  - area/user-interface
ms_date: "09/30/2024"
source: "https://learn.microsoft.com/dotnet/maui/user-interface/animation/custom?view=net-maui-10.0"
---

# Custom animation

The .NET Multi-platform App UI (.NET MAUI) [[Animation (Controls)|Animation]] class is the building block of all .NET MAUI animations, with the extension methods in the [[ViewExtensions (Controls)|ViewExtensions]] class creating one or more [[Animation (Controls)|Animation]] objects.

A number of parameters must be specified when creating an [[Animation (Controls)|Animation]] object, including start and end values of the property being animated, and a callback that changes the value of the property. An [[Animation (Controls)|Animation]] object can also maintain a collection of child animations that can be run and synchronized. For more information, see [Child animations](#child-animations).

Running an animation created with the [[Animation (Controls)|Animation]] class, which may or may not include child animations, is achieved by calling the `Commit%2A` method. This method specifies the duration of the animation, and amongst other items, a callback that controls whether to repeat the animation.

> [!NOTE]
> The [[Animation (Controls)|Animation]] class has an [[Animation (Controls).IsEnabled|IsEnabled]] property that can be examined to determine if animations have been disabled by the operating system, such as when power saving mode is activated.

![[animation-android]]

## Create an animation

When creating an [[Animation (Controls)|Animation]] object, typically, a minimum of three parameters are required, as demonstrated in the following code example:

```csharp
var animation = new Animation(v => image.Scale = v, 1, 2);
```

In this example, an animation of the [[VisualElement (Controls).Scale|Scale]] property of an [[Image (Controls)|Image]] instance is defined from a value of 1 to a value of 2. The animated value is passed to the callback specified as the first argument, where it's used to change the value of the [[VisualElement (Controls).Scale|Scale]] property.

The animation is started with a call to the `Commit%2A` method:

```csharp
animation.Commit(this, "SimpleAnimation", 16, 2000, Easing.Linear, (v, c) => image.Scale = 1, () => true);
```

> [!NOTE]
> The `Commit%2A` method doesn't return a `Task` object. Instead, notifications are provided through callback methods.

The following arguments are specified in the `Commit%2A` method:

- The first argument (`owner`) identifies the owner of the animation. This can be the visual element on which the animation is applied, or another visual element, such as the page.
- The second argument (`name`) identifies the animation with a name. The name is combined with the owner to uniquely identify the animation. This unique identification can then be used to determine whether the animation is running (`AnimationIsRunning%2A`), or to cancel it (`AbortAnimation%2A`).
- The third argument (`rate`) indicates the number of milliseconds between each call to the callback method defined in the [[Animation (Controls)|Animation]] constructor.
- The fourth argument (`length`) indicates the duration of the animation, in milliseconds.
- The fifth argument ([[Easing|Easing]]) defines the easing function to be used in the animation. Alternatively, the easing function can be specified as an argument to the [[Animation (Controls)|Animation]] constructor. For more information about easing functions, see [[easing|Easing functions]].
- The sixth argument (`finished`) is a callback that will be executed when the animation has completed. This callback takes two arguments, with the first argument indicating a final value, and the second argument being a `bool` that's set to `true` if the animation was canceled. Alternatively, the `finished` callback can be specified as an argument to the [[Animation (Controls)|Animation]] constructor. However, with a single animation, if `finished` callbacks are specified in both the [[Animation (Controls)|Animation]] constructor and the `Commit%2A` method, only the callback specified in the `Commit%2A` method will be executed.
- The seventh argument (`repeat`) is a callback that allows the animation to be repeated. It's called at the end of the animation, and returning `true` indicates that the animation should be repeated.

In the above example, the overall effect is to create an animation that increases the [[VisualElement (Controls).Scale|Scale]] property of an [[Image (Controls)|Image]] instance from 1 to 2, over 2 seconds (2000 milliseconds), using the [[Easing.Linear|Linear]] easing function. Each time the animation completes, its [[VisualElement (Controls).Scale|Scale]] property is reset to 1 and the animation repeats.

> [!NOTE]
> Concurrent animations, that run independently of each other can be constructed by creating an [[Animation (Controls)|Animation]] object for each animation, and then calling the `Commit%2A` method on each animation.

### Child animations

The [[Animation (Controls)|Animation]] class also supports child animations, which are [[Animation (Controls)|Animation]] objects to which other [[Animation (Controls)|Animation]] objects are added as children. This enables a series of animations to be run and synchronized. The following code example demonstrates creating and running child animations:

```csharp
var parentAnimation = new Animation();
var scaleUpAnimation = new Animation(v => image.Scale = v, 1, 2, Easing.SpringIn);
var rotateAnimation = new Animation(v => image.Rotation = v, 0, 360);
var scaleDownAnimation = new Animation(v => image.Scale = v, 2, 1, Easing.SpringOut);

parentAnimation.Add(0, 0.5, scaleUpAnimation);
parentAnimation.Add(0, 1, rotateAnimation);
parentAnimation.Add(0.5, 1, scaleDownAnimation);

parentAnimation.Commit(this, "ChildAnimations", 16, 4000, null, (v, c) => SetIsEnabledButtonState(true, false));
```

Alternatively, the code example can be written more concisely:

```csharp
new Animation
{
    { 0, 0.5, new Animation (v => image.Scale = v, 1, 2) },
    { 0, 1, new Animation (v => image.Rotation = v, 0, 360) },
    { 0.5, 1, new Animation (v => image.Scale = v, 2, 1) }
}.Commit (this, "ChildAnimations", 16, 4000, null, (v, c) => SetIsEnabledButtonState (true, false));
```

In both examples, a parent [[Animation (Controls)|Animation]] object is created, to which additional [[Animation (Controls)|Animation]] objects are then added. The first two arguments to the `Add%2A` method specify when to begin and finish the child animation. The argument values must be between 0 and 1, and represent the relative period within the parent animation that the specified child animation will be active. Therefore, in this example the `scaleUpAnimation` will be active for the first half of the animation, the `scaleDownAnimation` will be active for the second half of the animation, and the `rotateAnimation` will be active for the entire duration.

The overall effect of this example is that the animation occurs over 4 seconds (4000 milliseconds). The `scaleUpAnimation` animates the [[VisualElement (Controls).Scale|Scale]] property from 1 to 2, over 2 seconds. The `scaleDownAnimation` then animates the [[VisualElement (Controls).Scale|Scale]] property from 2 to 1, over 2 seconds. While both scale animations are occurring, the `rotateAnimation` animates the [[VisualElement (Controls).Rotation|Rotation]] property from 0 to 360, over 4 seconds. Both scaling animations also use easing functions. The [[Easing.SpringIn|SpringIn]] easing function causes the [[Image (Controls)|Image]] instance to initially shrink before getting larger, and the [[Easing.SpringOut|SpringOut]] easing function causes the [[Image (Controls)|Image]] to become smaller than its actual size towards the end of the complete animation.

There are a number of differences between an [[Animation (Controls)|Animation]] object that uses child animations, and one that doesn't:

- When using child animations, the `finished` callback on a child animation indicates when the child has completed, and the `finished` callback passed to the `Commit%2A` method indicates when the entire animation has completed.
- When using child animations, returning `true` from the `repeat` callback on the `Commit%2A` method will not cause the animation to repeat, but the animation will continue to run without new values.
- When including an easing function in the `Commit%2A` method, and the easing function returns a value greater than 1, the animation will be terminated. If the easing function returns a value less than 0, the value is clamped to 0. To use an easing function that returns a value less than 0 or greater than 1, it must be specified in one of the child animations, rather than in the `Commit%2A` method.

The [[Animation (Controls)|Animation]] class also includes `WithConcurrent%2A` methods that can be used to add child animations to a parent [[Animation (Controls)|Animation]] object. However, their `begin` and `finish` argument values aren't restricted to 0 to 1, but only that part of the child animation that corresponds to a range of 0 to 1 will be active. For example, if a `WithConcurrent%2A` method call defines a child animation that targets a [[VisualElement (Controls).Scale|Scale]] property from 1 to 6, but with `begin` and `finish` values of -2 and 3, the `begin` value of -2 corresponds to a [[VisualElement (Controls).Scale|Scale]] value of 1, and the `finish` value of 3 corresponds to a [[VisualElement (Controls).Scale|Scale]] value of 6. Because values outside the range of 0 and 1 play no part in an animation, the [[VisualElement (Controls).Scale|Scale]] property will only be animated from 3 to 6.

## Cancel an animation

An app can cancel a custom animation with a call to the `AbortAnimation%2A` extension method:

```csharp
this.AbortAnimation ("SimpleAnimation");
```

Because animations are uniquely identified by a combination of the animation owner, and the animation name, the owner and name specified when running the animation must be specified to cancel it. Therefore, this example will immediately cancel the animation named `SimpleAnimation` that's owned by the page.

## Create a custom animation

The examples shown here so far have demonstrated animations that could equally be achieved with the methods in the [[ViewExtensions (Controls)|ViewExtensions]] class. However, the advantage of the [[Animation (Controls)|Animation]] class is that it has access to the callback method, which is executed when the animated value changes. This allows the callback to implement any desired animation. For example, the following code example animates the [[VisualElement (Controls).BackgroundColor|BackgroundColor]] property of a page by setting it to [[Color|Color]] values created by the `Color.FromHsla` method, with hue values ranging from 0 to 1:

```csharp
new Animation (callback: v => BackgroundColor = Color.FromHsla (v, 1, 0.5),
  start: 0,
  end: 1).Commit (this, "Animation", 16, 4000, Easing.Linear, (v, c) => BackgroundColor = Colors.Black);
```

The resulting animation provides the appearance of advancing the page background through the colors of the rainbow.

## Create a custom animation extension method

The extension methods in the [[ViewExtensions (Controls)|ViewExtensions]] class animate a property from its current value to a specified value. This makes it difficult to create, for example, a `ColorTo` animation method that can be used to animate a color from one value to another. This is because different controls have different properties of type [[Color|Color]]. While the [[VisualElement (Controls)|VisualElement]] class defines a [[VisualElement (Controls).BackgroundColor|BackgroundColor]] property, this isn't always the desired `Color` property to animate.

The solution to this problem is to not have the `ColorTo` method target a particular `Color` property. Instead, it can be written with a callback method that passes the interpolated [[Color|Color]] value back to the caller. In addition, the method will take start and end [[Color|Color]] arguments.

The `ColorTo` method can be implemented as an extension method that uses the `Animate%2A` method in the [[AnimationExtensions|AnimationExtensions]] class to provide its functionality. This is because the `Animate%2A` method can be used to target properties that aren't of type `double`, as demonstrated in the following code example:

```csharp
public static class ViewExtensions
{
    public static Task<bool> ColorTo(this VisualElement self, Color fromColor, Color toColor, Action<Color> callback, uint length = 250, Easing easing = null)
    {
        Func<double, Color> transform = (t) =>
            Color.FromRgba(fromColor.Red + t * (toColor.Red - fromColor.Red),
                           fromColor.Green + t * (toColor.Green - fromColor.Green),
                           fromColor.Blue + t * (toColor.Blue - fromColor.Blue),
                           fromColor.Alpha + t * (toColor.Alpha - fromColor.Alpha));
        return ColorAnimation(self, "ColorTo", transform, callback, length, easing);
    }

    public static void CancelAnimation(this VisualElement self)
    {
        self.AbortAnimation("ColorTo");
    }

    static Task<bool> ColorAnimation(VisualElement element, string name, Func<double, Color> transform, Action<Color> callback, uint length, Easing easing)
    {
        easing = easing ?? Easing.Linear;
        var taskCompletionSource = new TaskCompletionSource<bool>();

        element.Animate<Color>(name, transform, callback, 16, length, easing, (v, c) => taskCompletionSource.SetResult(c));
        return taskCompletionSource.Task;
    }
}
```

The `Animate%2A` method requires a `transform` argument, which is a callback method. The input to this callback is always a `double` ranging from 0 to 1. Therefore, in this example the `ColorTo` method defines its own transform `Func` that accepts a `double` ranging from 0 to 1, and that returns a [[Color|Color]] value corresponding to that value. The [[Color|Color]] value is calculated by interpolating the `Red`, `Green`, `Blue`, and `Alpha` values of the two supplied [[Color|Color]] arguments. The [[Color|Color]] value is then passed to the callback method to be applied to a property. This approach allows the `ColorTo` method to animate any specified [[Color|Color]] property:

```csharp
await Task.WhenAll(
  label.ColorTo(Colors.Red, Colors.Blue, c => label.TextColor = c, 5000),
  label.ColorTo(Colors.Blue, Colors.Red, c => label.BackgroundColor = c, 5000));
await this.ColorTo(Color.FromRgb(0, 0, 0), Color.FromRgb(255, 255, 255), c => BackgroundColor = c, 5000);
await boxView.ColorTo(Colors.Blue, Colors.Red, c => boxView.Color = c, 4000);
```

In this code example, the `ColorTo` method animates the `TextColor` and [[VisualElement (Controls).BackgroundColor|BackgroundColor]] properties of a [[Label (Controls)|Label]], the [[VisualElement (Controls).BackgroundColor|BackgroundColor]] property of a page, and the `Color` property of a [[BoxView (Controls)|BoxView]].
