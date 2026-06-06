---
title: "Easing Functions Dotnet10"
tags:
  - conceptual
  - area/user-interface
ms_date: "04/03/2025"
source: "https://learn.microsoft.com/dotnet/maui/user-interface/includes/easing-functions-dotnet10?view=net-maui-10.0"
---

.NET Multi-platform App UI (.NET MAUI) includes an [[Easing|Easing]] class that enables you to specify a transfer function that controls how animations speed up or slow down as they're running.

The [[Easing|Easing]] class defines a number of easing functions that can be consumed by animations:

- The [[Easing.BounceIn|BounceIn]] easing function bounces the animation at the beginning.
- The [[Easing.BounceOut|BounceOut]] easing function bounces the animation at the end.
- The [[Easing.CubicIn|CubicIn]] easing function slowly accelerates the animation.
- The [[Easing.CubicInOut|CubicInOut]] easing function accelerates the animation at the beginning, and decelerates the animation at the end.
- The [[Easing.CubicOut|CubicOut]] easing function quickly decelerates the animation.
- The [[Easing.Linear|Linear]] easing function uses a constant velocity, and is the default easing function.
- The [[Easing.SinIn|SinIn]] easing function smoothly accelerates the animation.
- The [[Easing.SinInOut|SinInOut]] easing function smoothly accelerates the animation at the beginning, and smoothly decelerates the animation at the end.
- The [[Easing.SinOut|SinOut]] easing function smoothly decelerates the animation.
- The [[Easing.SpringIn|SpringIn]] easing function causes the animation to very quickly accelerate towards the end.
- The [[Easing.SpringOut|SpringOut]] easing function causes the animation to quickly decelerate towards the end.

The `In` and `Out` suffixes indicate if the effect provided by the easing function is noticeable at the beginning of the animation, at the end, or both.

In addition, custom easing functions can be created. For more information, see [Custom easing functions](#custom-easing-functions).

## Consume an easing function

The animation extension methods in the [[ViewExtensions (Controls)|ViewExtensions]] class allow an easing function to be specified as the final method argument:

```csharp
await image.TranslateToAsync(0, 200, 2000, Easing.BounceIn);
await image.ScaleToAsync(2, 2000, Easing.CubicIn);
await image.RotateToAsync(360, 2000, Easing.SinInOut);
await image.ScaleToAsync(1, 2000, Easing.CubicOut);
await image.TranslateToAsync(0, -200, 2000, Easing.BounceOut);
```

By specifying an easing function for an animation, the animation velocity becomes non-linear and produces the effect provided by the easing function. Omitting an easing function when creating an animation causes the animation to use the default [[Easing.Linear|Linear]] easing function, which produces a linear velocity.

For more information about using the animation extension methods in the [[ViewExtensions (Controls)|ViewExtensions]] class, see [[basic|Basic animation]]. Easing functions can also be consumed by the [[Animation (Controls)|Animation]] class. For more information, see [[custom|Custom animation]].

## Custom easing functions

There are three main approaches to creating a custom easing function:

1. Create a method that takes a `double` argument, and returns a `double` result.
1. Create a `Func<double, double>`.
1. Specify the easing function as the argument to the [[Easing|Easing]] constructor.

In all three cases, the custom easing function should return a value between 0 and 1.

### Custom easing method

A custom easing function can be defined as a method that takes a `double` argument, and returns a `double` result:

```csharp
double CustomEase (double t)
{
  return t == 0 || t == 1 ? t : (int)(5 * t) / 5.0;
}

await image.TranslateToAsync(0, 200, 2000, (Easing)CustomEase);
```

In this example, the `CustomEase` method truncates the incoming value to the values 0, 0.2, 0.4, 0.6, 0.8, and 1. Therefore, the [[Image (Controls)|Image]] instance is translated in discrete jumps, rather than smoothly.

### Custom easing func

A custom easing function can also be defined as a `Func<double, double>`:

```csharp
Func<double, double> CustomEaseFunc = t => 9 * t * t * t - 13.5 * t * t + 5.5 * t;
await image.TranslateToAsync(0, 200, 2000, CustomEaseFunc);
```

In this example, the `CustomEaseFunc` represents an easing function that starts off fast, slows down and reverses course, and then reverses course again to accelerate quickly towards the end. Therefore, while the overall movement of the [[Image (Controls)|Image]] instance is downwards, it also temporarily reverses course halfway through the animation.

### Custom easing constructor

A custom easing function can also be defined as the argument to the [[Easing|Easing]] constructor:

```csharp
await image.TranslateToAsync(0, 200, 2000, new Easing (t => 1 - Math.Cos (10 * Math.PI * t) * Math.Exp (-5 * t)));
```

In this example, the custom easing function is specified as a lambda function argument to the [[Easing|Easing]] constructor, and uses the `Math.Cos` method to create a slow drop effect that's dampened by the `Math.Exp` method. Therefore, the [[Image (Controls)|Image]] instance is translated so that it appears to drop to its final position.
