---
title: "Specify the UI idiom for your Mac Catalyst app"
description: "Learn how to specify the user interface idiom for your .NET MAUI Mac Catalyst app."
tags:
  - conceptual
  - area/mac-catalyst
ms_date: "03/27/2023"
source: "https://learn.microsoft.com/dotnet/maui/mac-catalyst/user-interface-idiom?view=net-maui-10.0"
---

# Specify the UI idiom for your Mac Catalyst app

A .NET Multi-platform App UI (.NET MAUI) Mac Catalyst app can run in the iPad or Mac user interface idiom:

- The iPad user interface idiom tells macOS to scale the app's user interface to match the Mac display environment while preserving iPad-like appearance.
- The Mac user interface idiom doesn't scale the app's user interface to match the Mac display environment. Some controls change their size and appearance, and interacting with them feels identical to interacting with `AppKit` controls. For example, a `UIButton` appears identical to an `NSButton`.

By default, .NET MAUI Mac Catalyst apps use the iPad user interface idiom. If this is your desired behavior, ensure that the app's *Info.plist* file only specifies 2 as the value of the `UIDeviceFamily` key:

```xml
<key>UIDeviceFamily</key>
<array>
  <integer>2</integer>
</array>
```

You might discover that adopting the Mac user interface idiom enhances the user experience of your app. To do this, update your app's *Info.plist* file to specify 6 as the value of the `UIDeviceFamily` key:

```xml
<key>UIDeviceFamily</key>
<array>
  <integer>6</integer>
</array>
```

> [!IMPORTANT]
> The Mac user interface idiom requires macOS 11.0+. Therefore, to use it you'll need to set the `SupportedOSPlatformVersion` in your project file to at least 14.0, which is the Mac Catalyst version equivalent of macOS 11.0.

Adopting the Mac user interface idiom may require you to make additional changes to your app. For example, if your app uses images sized for iPad or has hard-coded sizes, you may need to update your app to accommodate the size differences.

> [!WARNING]
> `UIStepper`, `UIPickerView`, and `UIRefreshControl` aren't supported in the Mac user interface idiom by Apple. This means that the .NET MAUI controls that consume these native controls ([[Stepper|Stepper]], [[Picker (Controls)|Picker]] and [[RefreshView (Controls)|RefreshView]]) can't be used in the Mac user interface idiom. Attempting to do so will throw a macOS exception.
>
> In addition, the following constraints apply in the Mac user interface idiom:
>
> - `UISwitch` throws a macOS exception when it's title is set in a non-Mac idiom view.
> - `UIButton` throws a macOS exception when `AddGestureRecognizer%2A` is called, or when `SetTitle%2A` or `SetImage%2A` are called for any state except `UIControlStateNormal.Normal`.
> - `UISlider` throws a macOS exception when the `SetThumbImage%2A`, `SetMinTrackImage%2A`, `SetMaxTrackImage%2A` methods are called and when the `ThumbTintColor`, `MinimumTrackTintColor`, `MaximumTrackTintColor`, `MinValueImage`, `MaxValueImage` properties are set.

## Determine the user interface idiom

It's possible to determine at runtime which user interface idiom your .NET MAUI Mac Catalyst app is using. This can be achieved by examining the value of the `UserInterfaceIdiom` property on your `UIViewController`:

```csharp
#if MACCATALYST
    UIKit.UIViewController viewController = Platform.GetCurrentUIViewController();
    if (viewController.TraitCollection.UserInterfaceIdiom == UIKit.UIUserInterfaceIdiom.Mac)
        // Mac user interface idiom
    else
        // iPad user interface idiom
#endif
```

## See also

- [Choosing a user interface idiom for your Mac app](https://developer.apple.com/documentation/uikit/mac_catalyst/choosing_a_user_interface_idiom_for_your_mac_app?language=objc) on developer.apple.com.
