---
title: "Simultaneous pan gesture recognition on iOS"
description: "This article explains how to consume the .NET MAUI iOS platform-specific that enables simultaneous pan gesture recognition to be used in an app."
tags:
  - conceptual
  - area/ios
ms_date: "04/05/2022"
source: "https://learn.microsoft.com/dotnet/maui/ios/platform-specifics/application-pan-gesture?view=net-maui-10.0"
---

# Simultaneous pan gesture recognition on iOS

When a [[PanGestureRecognizer|PanGestureRecognizer]] is attached to a view inside a scrolling view, all of the pan gestures are captured by the [[PanGestureRecognizer|PanGestureRecognizer]] and aren't passed to the scrolling view. Therefore, the scrolling view will no longer scroll.

This .NET Multi-platform App UI (.NET MAUI) iOS platform-specific enables a [[PanGestureRecognizer|PanGestureRecognizer]] in a scrolling view to capture and share the pan gesture with the scrolling view. It's consumed in XAML by setting the `Application.PanGestureRecognizerShouldRecognizeSimultaneously` attached property to `true`:

```xaml
<Application ...
             xmlns:ios="clr-namespace:Microsoft.Maui.Controls.PlatformConfiguration.iOSSpecific;assembly=Microsoft.Maui.Controls"
             ios:Application.PanGestureRecognizerShouldRecognizeSimultaneously="true">
    ...
</Application>
```

Alternatively, it can be consumed from C# using the fluent API:

```csharp
using Microsoft.Maui.Controls.PlatformConfiguration;
using Microsoft.Maui.Controls.PlatformConfiguration.iOSSpecific;
...

Application.Current.On<iOS>().SetPanGestureRecognizerShouldRecognizeSimultaneously(true);
```

The `Application.On<iOS>` method specifies that this platform-specific will only run on iOS. The `Application.SetPanGestureRecognizerShouldRecognizeSimultaneously` method, in the `Microsoft.Maui.Controls.PlatformConfiguration.iOSSpecific` namespace, is used to control whether a pan gesture recognizer in a scrolling view will capture the pan gesture, or capture and share the pan gesture with the scrolling view. In addition, the `Application.GetPanGestureRecognizerShouldRecognizeSimultaneously` method can be used to return whether the pan gesture is shared with the scrolling view that contains the [[PanGestureRecognizer|PanGestureRecognizer]].

Therefore, with this platform-specific enabled, when a [[ListView (Controls)|ListView]] contains a [[PanGestureRecognizer|PanGestureRecognizer]], both the [[ListView (Controls)|ListView]] and the [[PanGestureRecognizer|PanGestureRecognizer]] will receive the pan gesture and process it. However, with this platform-specific disabled, when a [[ListView (Controls)|ListView]] contains a [[PanGestureRecognizer|PanGestureRecognizer]], the [[PanGestureRecognizer|PanGestureRecognizer]] will capture the pan gesture and process it, and the [[ListView (Controls)|ListView]] won't receive the pan gesture.
