---
title: "Basic Animation Dotnet9"
tags:
  - conceptual
  - area/user-interface
ms_date: "04/03/2025"
source: "https://learn.microsoft.com/dotnet/maui/user-interface/includes/basic-animation-dotnet9?view=net-maui-10.0"
---

The .NET Multi-platform App UI (.NET MAUI) animation classes target different properties of visual elements, with a typical basic animation progressively changing a property from one value to another over a period of time.

Basic animations can be created with extension methods provided by the [[ViewExtensions (Controls)|ViewExtensions]] class, which operate on [[VisualElement (Controls)|VisualElement]] objects:

- `CancelAnimations%2A` cancels any animations.
- `FadeTo%2A` animates the [[VisualElement (Controls).Opacity|Opacity]] property of a [[VisualElement (Controls)|VisualElement]].
- `RelScaleTo%2A` applies an animated incremental increase or decrease to the [[VisualElement (Controls).Scale|Scale]] property of a [[VisualElement (Controls)|VisualElement]].
- `RotateTo%2A` animates the [[VisualElement (Controls).Rotation|Rotation]] property of a [[VisualElement (Controls)|VisualElement]].
- `RelRotateTo%2A` applies an animated incremental increase or decrease to the [[VisualElement (Controls).Rotation|Rotation]] property of a [[VisualElement (Controls)|VisualElement]].
- `RotateXTo%2A` animates the [[VisualElement (Controls).RotationX|RotationX]] property of a [[VisualElement (Controls)|VisualElement]].
- `RotateYTo%2A` animates the [[VisualElement (Controls).RotationY|RotationY]] property of a [[VisualElement (Controls)|VisualElement]].
- `ScaleTo%2A` animates the [[VisualElement (Controls).Scale|Scale]] property of a [[VisualElement (Controls)|VisualElement]].
- `ScaleXTo%2A` animates the [[VisualElement (Controls).ScaleX|ScaleX]] property of a [[VisualElement (Controls)|VisualElement]].
- `ScaleYTo%2A` animates the [[VisualElement (Controls).ScaleY|ScaleY]] property of a [[VisualElement (Controls)|VisualElement]].
- `TranslateTo%2A` animates the [[VisualElement (Controls).TranslationX|TranslationX]] and [[VisualElement (Controls).TranslationY|TranslationY]] properties of a [[VisualElement (Controls)|VisualElement]].

By default, each animation will take 250 milliseconds. However, a duration for each animation can be specified when creating the animation.

> [!NOTE]
> The [[ViewExtensions (Controls)|ViewExtensions]] class also provides a `LayoutTo%2A` extension method. However, this method is intended to be used by layouts to animate transitions between layout states that contain size and position changes.

The animation extension methods in the [[ViewExtensions (Controls)|ViewExtensions]] class are all asynchronous and return a `Task<bool>` object. The return value is `false` if the animation completes, and `true` if the animation is cancelled. Therefore, when animation operations are combined with the `await` operator it becomes possible to create sequential animations with subsequent animation methods executing after the previous method has completed. For more information, see [Compound animations](#compound-animations).

If there's a requirement to let an animation complete in the background, then the `await` operator can be omitted. In this scenario, the animation extension methods will quickly return after initiating the animation, with the animation occurring in the background. This operation can be taken advantage of when creating composite animations. For more information, see [Composite animations](#composite-animations).

![[animation-android]]

## Single animations

Each extension method in the [[ViewExtensions (Controls)|ViewExtensions]] class implements a single animation operation that progressively changes a property from one value to another value over a period of time.

### Rotation

Rotation is performed with the `RotateTo%2A` method, which progressively changes the [[VisualElement (Controls).Rotation|Rotation]] property of an element:

```csharp
await image.RotateTo(360, 2000);
image.Rotation = 0;
```

In this example, an [[Image (Controls)|Image]] instance is rotated up to 360 degrees over 2 seconds (2000 milliseconds). The `RotateTo%2A` method obtains the current [[VisualElement (Controls).Rotation|Rotation]] property value of the element for the start of the animation, and then rotates from that value to its first argument (360). Once the animation is complete, the image's [[VisualElement (Controls).Rotation|Rotation]] property is reset to 0. This ensures that the [[VisualElement (Controls).Rotation|Rotation]] property doesn't remain at 360 after the animation concludes, which would prevent additional rotations.

> [!NOTE]
> In addition to the `RotateTo%2A` method, there are also `RotateXTo%2A` and `RotateYTo%2A` methods that animate the `RotationX` and `RotationY` properties, respectively.

### Relative rotation

Relative rotation is performed with the `RelRotateTo%2A` method, which progressively changes the [[VisualElement (Controls).Rotation|Rotation]] property of an element:

```csharp
await image.RelRotateTo(360, 2000);
```

In this example, an [[Image (Controls)|Image]] instance is rotated 360 degrees from its starting position over 2 seconds (2000 milliseconds). The `RelRotateTo%2A` method obtains the current [[VisualElement (Controls).Rotation|Rotation]] property value of the element for the start of the animation, and then rotates from that value to the value plus its first argument (360). This ensures that each animation will always be a 360 degrees rotation from the starting position. Therefore, if a new animation is invoked while an animation is already in progress, it will start from the current position and may end at a position that is not an increment of 360 degrees.

### Scaling

Scaling is performed with the `ScaleTo%2A` method, which progressively changes the `Scale` property of an element:

```csharp
await image.ScaleTo(2, 2000);
```

In this example, an [[Image (Controls)|Image]] instance is scaled up to twice its size over 2 seconds (2000 milliseconds). The `ScaleTo%2A` method obtains the current [[VisualElement (Controls).Scale|Scale]] property value of the element for the start of the animation, and then scales from that value to its first argument. This has the effect of expanding the size of the image to twice its size.

> [!NOTE]
> In addition to the `ScaleTo%2A` method, there are also `ScaleXTo%2A` and `ScaleYTo%2A` methods that animate the `ScaleX` and `ScaleY` properties, respectively.

### Relative scaling

Relative scaling is performed with the `RelScaleTo%2A` method, which progressively changes the [[VisualElement (Controls).Scale|Scale]] property of an element:

```csharp
await image.RelScaleTo(2, 2000);
```

In this example, an [[Image (Controls)|Image]] instance is scaled up to twice its size over 2 seconds (2000 milliseconds). The `RelScaleTo%2A` method obtains the current [[VisualElement (Controls).Scale|Scale]] property value of the element for the start of the animation, and then scales from that value to the value plus its first argument. This ensures that each animation will always be a scaling of 2 from the starting position.

### Scaling and rotation with anchors

The `AnchorX` and `AnchorY` properties of a visual element set the center of scaling or rotation for the [[VisualElement (Controls).Rotation|Rotation]] and [[VisualElement (Controls).Scale|Scale]] properties. Therefore, their values also affect the `RotateTo%2A` and `ScaleTo%2A` methods.

Given an [[Image (Controls)|Image]] that has been placed at the center of a layout, the following code example demonstrates rotating the image around the center of the layout by setting its `AnchorY` property:

```csharp
double radius = Math.Min(absoluteLayout.Width, absoluteLayout.Height) / 2;
image.AnchorY = radius / image.Height;
await image.RotateTo(360, 2000);
```

To rotate the [[Image (Controls)|Image]] instance around the center of the layout, the [[VisualElement (Controls).AnchorX|AnchorX]] and [[VisualElement (Controls).AnchorY|AnchorY]] properties must be set to values that are relative to the width and height of the [[Image (Controls)|Image]]. In this example, the center of the [[Image (Controls)|Image]] is defined to be at the center of the layout, and so the default [[VisualElement (Controls).AnchorX|AnchorX]] value of 0.5 does not require changing. However, the [[VisualElement (Controls).AnchorY|AnchorY]] property is redefined to be a value from the top of the [[Image (Controls)|Image]] to the center point of the layout. This ensures that the [[Image (Controls)|Image]] makes a full rotation of 360 degrees around the center point of the layout.

### Translation

Translation is performed with the `TranslateTo%2A` method, which progressively changes the [[VisualElement (Controls).TranslationX|TranslationX]] and [[VisualElement (Controls).TranslationY|TranslationY]] properties of an element:

```csharp
await image.TranslateTo(-100, -100, 1000);
```

In this example, the [[Image (Controls)|Image]] instance is translated horizontally and vertically over 1 second (1000 milliseconds). The `TranslateTo%2A` method simultaneously translates the image 100 device-independent units to the left, and 100 device-independent units upwards. This is because the first and second arguments are both negative numbers. Providing positive numbers would translate the image to the right, and down.

> [!IMPORTANT]
> If an element is initially laid out off screen and then translated onto the screen, after translation the element's input layout remains off screen and the user can't interact with it. Therefore, it's recommended that a view should be laid out in its final position, and then any required translations performed.

### Fading

Fading is performed with the `FadeTo%2A` method, which progressively changes the [[VisualElement (Controls).Opacity|Opacity]] property of an element:

```csharp
image.Opacity = 0;
await image.FadeTo(1, 4000);
```

In this example, the [[Image (Controls)|Image]] instance fades in over 4 seconds (4000 milliseconds). The `FadeTo%2A` method obtains the current [[VisualElement (Controls).Opacity|Opacity]] property value of the element for the start of the animation, and then fades in from that value to its first argument.

## Compound animations

A compound animation is a sequential combination of animations, and can be created with the `await` operator:

```csharp
await image.TranslateTo(-100, 0, 1000);    // Move image left
await image.TranslateTo(-100, -100, 1000); // Move image diagonally up and left
await image.TranslateTo(100, 100, 2000);   // Move image diagonally down and right
await image.TranslateTo(0, 100, 1000);     // Move image left
await image.TranslateTo(0, 0, 1000);       // Move image up
```

In this example, the [[Image (Controls)|Image]] instance is translated over 6 seconds (6000 milliseconds). The translation of the [[Image (Controls)|Image]] uses five animations, with the `await` operator indicating that each animation executes sequentially. Therefore, subsequent animation methods execute after the previous method has completed.

## Composite animations

A composite animation is a combination of animations where two or more animations run simultaneously. Composite animations can be created by combining awaited and non-awaited animations:

```csharp
image.RotateTo(360, 4000);
await image.ScaleTo(2, 2000);
await image.ScaleTo(1, 2000);
```

In this example, the [[Image (Controls)|Image]] instance is scaled and simultaneously rotated over 4 seconds (4000 milliseconds). The scaling of the [[Image (Controls)|Image]] uses two sequential animations that occur at the same time as the rotation. The `RotateTo%2A` method executes without an `await` operator and returns immediately, with the first `ScaleTo%2A` animation then beginning. The `await` operator on the first `ScaleTo%2A` method delays the second `ScaleTo%2A` method until the first `ScaleTo%2A` method has completed. At this point the `RotateTo%2A` animation is half completed and the [[Image (Controls)|Image]] will be rotated 180 degrees. During the final 2 seconds (2000 milliseconds), the second `ScaleTo%2A` animation and the `RotateTo%2A` animation both complete.

### Run multiple animations concurrently

The `Task.WhenAny` and `Task.WhenAll` methods can be used to run multiple asynchronous methods concurrently, and therefore can create composite animations. Both methods return a `Task` object and accept a collection of methods that each return a `Task` object. The `Task.WhenAny` method completes when any method in its collection completes execution, as demonstrated in the following code example:

```csharp
await Task.WhenAny<bool>
(
  image.RotateTo(360, 4000),
  image.ScaleTo(2, 2000)
);
await image.ScaleTo(1, 2000);
```

In this example, the `Task.WhenAny` method contains two tasks. The first task rotates an [[Image (Controls)|Image]] instance over 4 seconds (4000 milliseconds), and the second task scales the image over 2 seconds (2000 milliseconds). When the second task completes, the `Task.WhenAny` method call completes. However, even though the `RotateTo%2A` method is still running, the second `ScaleTo%2A` method can begin.

The `Task.WhenAll` method completes when all the methods in its collection have completed, as demonstrated in the following code example:

```csharp
// 10 minute animation
uint duration = 10 * 60 * 1000;
await Task.WhenAll
(
  image.RotateTo(307 * 360, duration),
  image.RotateXTo(251 * 360, duration),
  image.RotateYTo(199 * 360, duration)
);
```

In this example, the `Task.WhenAll` method contains three tasks, each of which executes over 10 minutes. Each `Task` makes a different number of 360 degree rotations – 307 rotations for `RotateTo%2A`, 251 rotations for `RotateXTo%2A`, and 199 rotations for `RotateYTo%2A`. These values are prime numbers, therefore ensuring that the rotations aren't synchronized and hence won't result in repetitive patterns.
