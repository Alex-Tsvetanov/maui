# iOS Specific — DatePicker UpdateMode

Ports .NET MAUI's `iOSDatePickerPage` ([oracle](../../../../../src/Controls/samples/Controls.Sample/Pages/PlatformSpecifics/iOS/iOSDatePickerPage.xaml)) as a code-first `maui::samples::ios_date_picker_page`. A DatePicker (clamped to 2020) with iOSSpecific `UpdateMode`=WhenFinished + a button toggling Immediately↔WhenFinished (the UIKit done-button commit policy).

| macOS (AppKit) | iOS (UIKit) |
| --- | --- |
| ![macOS](macos.png) | ![iOS](ios.png) |

**Running on iOS:**

![iOS demo](ios.gif)

**Platforms:** macOS ✅ demo · iOS ✅ demo · Windows ⬜ TODO · Linux ⬜ TODO · Android ⬜ TODO

**Run it:** `MAUI_SAMPLE_PAGE=ios_date_picker ./build/apple/maui_macos_gallery` (macOS) · `SIMCTL_CHILD_MAUI_SAMPLE_PAGE=ios_date_picker xcrun simctl launch booted dev.maui-cpp.ios-gallery` (iOS)

> Exercises the **iOSSpecific platform-configuration** surface via `element.on<ios>()` + the `ios_specific::*` knob free-functions (the C# `.On<iOS>().SetXxx()` form). The control renders on both backends; the knob is wired-real on iOS where noted and stored-inert (round-tripping) on headless/AppKit.
