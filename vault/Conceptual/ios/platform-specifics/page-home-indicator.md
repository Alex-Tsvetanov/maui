---
title: "Home indicator visibility on iOS"
description: "This article explains how to consume the iOS platform-specific that sets the visibility of the home indicator on a page."
tags:
  - conceptual
  - area/ios
ms_date: "10/10/2024"
source: "https://learn.microsoft.com/dotnet/maui/ios/platform-specifics/page-home-indicator?view=net-maui-10.0"
---

# Home indicator visibility on iOS

This iOS platform-specific sets the visibility of the home indicator on a [[Page (Controls)|Page]]. It's consumed in XAML by setting the `Page.PrefersHomeIndicatorAutoHidden` bindable property to a `boolean`:

```xaml
<ContentPage ...
             xmlns:ios="clr-namespace:Microsoft.Maui.Controls.PlatformConfiguration.iOSSpecific;assembly=Microsoft.Maui.Controls"
             ios:Page.PrefersHomeIndicatorAutoHidden="true">
    ...
</ContentPage>
```

Alternatively, it can be consumed from C# using the fluent API:

```csharp
using Microsoft.Maui.Controls.PlatformConfiguration;
using Microsoft.Maui.Controls.PlatformConfiguration.iOSSpecific;
...

On<iOS>().SetPrefersHomeIndicatorAutoHidden(true);
```

The `Page.On<iOS>` method specifies that this platform-specific will only run on iOS. The
`Page.SetPrefersHomeIndicatorAutoHidden%2A` method, in the `iOSSpecific` namespace, controls the visibility of the home indicator. In addition, the `Page.PrefersHomeIndicatorAutoHidden%2A` method can be used to retrieve the visibility of the home indicator.

The result is that the visibility of the home indicator on a [[Page (Controls)|Page]] can be controlled:

![](media/page-home-indicator/home-indicator-visibility.png)

> [!NOTE]
> This platform-specific can be applied to [[ContentPage|ContentPage]], [[FlyoutPage (Controls)|FlyoutPage]], [[NavigationPage (Controls)|NavigationPage]], [[Shell|Shell]], and [[TabbedPage (Controls)|TabbedPage]] objects.
