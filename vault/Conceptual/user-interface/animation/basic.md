---
title: "Basic animation"
description: "The .NET MAUI ViewExtensions class, in the Microsoft.Maui.Controls namespace, provides extension methods that can be used to create and cancel basic animations."
tags:
  - conceptual
  - area/user-interface
ms_date: "04/03/2025"
source: "https://learn.microsoft.com/dotnet/maui/user-interface/animation/basic?view=net-maui-10.0"
---

# Basic animation


![[basic-animation-dotnet9]]



![[basic-animation-dotnet10]]


## Canceling animations

The `CancelAnimations%2A` extension method is used to cancel any animations, such as rotation, scaling, translation, and fading, that are running on a specific [[VisualElement (Controls)|VisualElement]].

```csharp
image.CancelAnimations();
```

In this example, all animations that are running on the [[Image (Controls)|Image]] instance are immediately canceled.
