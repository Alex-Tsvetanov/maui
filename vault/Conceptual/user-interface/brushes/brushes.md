---
title: "Brushes"
description: "The .NET MAUI Brush class is an abstract class that paints an area with its output."
tags:
  - conceptual
  - area/user-interface
ms_date: "09/30/2024"
source: "https://learn.microsoft.com/dotnet/maui/user-interface/brushes?view=net-maui-10.0"
---

# Brushes

[![Browse sample.](~/media/code-sample.png) Browse the sample](/samples/dotnet/maui-samples/userinterface-brushes)

A .NET Multi-platform App UI (.NET MAUI) brush enables you to paint an area, such as the background of a control, using different approaches.

The [[Brush|Brush]] class is an abstract class that paints an area with its output. Classes that derive from [[Brush|Brush]] describe different ways of painting an area. The following list describes the different brush types available in .NET MAUI:

- [[SolidColorBrush|SolidColorBrush]], which paints an area with a solid color. For more information, see [[solidcolor|Solid color brushes]].
- [[LinearGradientBrush|LinearGradientBrush]], which paints an area with a linear gradient. For more information, see [[lineargradient|Linear gradient brushes]].
- [[RadialGradientBrush|RadialGradientBrush]], which paints an area with a radial gradient. For more information, see [[radialgradient|Radial gradient brushes]].

Instances of these brush types can be assigned to the `Stroke` and `Fill` properties of a [[Shape|Shape]], the `Stroke` property of a [[Border|Border]], the `Brush` property of a `Shadow`, and the `Background` property of a [[VisualElement (Controls)|VisualElement]].

> [!NOTE]
> The `VisualElement.Background` property enables brushes to be used as the background in any control.

The [[Brush|Brush]] class also has an `IsNullOrEmpty` method that returns a `bool` that represents whether the brush is defined or not.
