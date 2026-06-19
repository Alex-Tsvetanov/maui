# iOS Specific — Safe Area

Ports .NET MAUI's `iOSSafeAreaPage` ([oracle](../../../../../src/Controls/samples/Controls.Sample/Pages/PlatformSpecifics/iOS/iOSSafeAreaPage.xaml)) as a code-first `maui::samples::ios_safe_area_page`. A Lorem-ipsum label + 'Disable Use Safe Area' button; the page seeds `SafeAreaEdges=None` via the iOS page config and the handler re-asserts `safe_area_edges(none())` + the legacy `SetUseSafeArea(false)` knob — both flow through measure/arrange on Apple backends.

| macOS (AppKit) | iOS (UIKit) |
| --- | --- |
| ![macOS](macos.png) | ![iOS](ios.png) |

**Running on iOS:**

![iOS demo](ios.gif)

**Platforms:** macOS ✅ demo · iOS ✅ demo · Windows ⬜ TODO · Linux ⬜ TODO · Android ⬜ TODO

**Run it:** `MAUI_SAMPLE_PAGE=ios_safe_area ./build/apple/maui_macos_gallery` (macOS) · `SIMCTL_CHILD_MAUI_SAMPLE_PAGE=ios_safe_area xcrun simctl launch booted dev.maui-cpp.ios-gallery` (iOS)

> Exercises the **iOSSpecific platform-configuration** surface via `element.on<ios>()` + the `ios_specific::*` knob free-functions (the C# `.On<iOS>().SetXxx()` form). The control renders on both backends; the knob is wired-real on iOS where noted and stored-inert (round-tripping) on headless/AppKit.
