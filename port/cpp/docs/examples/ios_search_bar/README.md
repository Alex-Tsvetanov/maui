# iOS Specific — SearchBar Style

Ports .NET MAUI's `iOSSearchBarPage` ([oracle](../../../../../src/Controls/samples/Controls.Sample/Pages/PlatformSpecifics/iOS/iOSSearchBarPage.xaml)) as a code-first `maui::samples::ios_search_bar_page`. A SearchBar with iOSSpecific `SearchBarStyle`=Minimal + a button cycling Default→Minimal→Prominent and a background-color toggle (SearchBarStyle drives the real `UISearchBar.searchBarStyle` on iOS).

| macOS (AppKit) | iOS (UIKit) |
| --- | --- |
| ![macOS](macos.png) | ![iOS](ios.png) |

**Running on iOS:**

![iOS demo](ios.gif)

**Platforms:** macOS ✅ demo · iOS ✅ demo · Windows ⬜ TODO · Linux ⬜ TODO · Android ⬜ TODO

**Run it:** `MAUI_SAMPLE_PAGE=ios_search_bar ./build/apple/maui_macos_gallery` (macOS) · `SIMCTL_CHILD_MAUI_SAMPLE_PAGE=ios_search_bar xcrun simctl launch booted dev.maui-cpp.ios-gallery` (iOS)

> Exercises the **iOSSpecific platform-configuration** surface via `element.on<ios>()` + the `ios_specific::*` knob free-functions (the C# `.On<iOS>().SetXxx()` form). The control renders on both backends; the knob is wired-real on iOS where noted and stored-inert (round-tripping) on headless/AppKit.
