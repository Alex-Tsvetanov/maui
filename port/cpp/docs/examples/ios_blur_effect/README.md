# iOS Specific — Blur Effect

Ports .NET MAUI's `iOSBlurEffectPage` ([oracle](../../../../../src/Controls/samples/Controls.Sample/Pages/PlatformSpecifics/iOS/iOSBlurEffectPage.xaml)) as a code-first `maui::samples::ios_blur_effect_page`. An image with iOSSpecific `BlurEffect` (No / ExtraLight / Light / Dark buttons calling `use_blur_effect(...)`) — an iOS-native `UIVisualEffectView` overlay with no headless analog, so the knob is exercised + the control renders.

| macOS (AppKit) | iOS (UIKit) |
| --- | --- |
| ![macOS](macos.png) | ![iOS](ios.png) |

**Running on iOS:**

![iOS demo](ios.gif)

**Platforms:** macOS ✅ demo · iOS ✅ demo · Windows ⬜ TODO · Linux ⬜ TODO · Android ⬜ TODO

**Run it:** `MAUI_SAMPLE_PAGE=ios_blur_effect ./build/apple/maui_macos_gallery` (macOS) · `SIMCTL_CHILD_MAUI_SAMPLE_PAGE=ios_blur_effect xcrun simctl launch booted dev.maui-cpp.ios-gallery` (iOS)

> Exercises the **iOSSpecific platform-configuration** surface via `element.on<ios>()` + the `ios_specific::*` knob free-functions (the C# `.On<iOS>().SetXxx()` form). The control renders on both backends; the knob is wired-real on iOS where noted and stored-inert (round-tripping) on headless/AppKit.
