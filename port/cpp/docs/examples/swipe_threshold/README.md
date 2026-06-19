# SwipeView — Threshold & Mode

Ports .NET MAUI's `HorizontalSwipeThresholdGallery` ([oracle](../../../../../src/Controls/samples/Controls.Sample/Pages/Controls/SwipeViewGalleries/HorizontalSwipeThresholdGallery.xaml)) as a code-first `maui::samples::swipe_threshold_page`. SwipeView.Threshold × SwipeItems.Mode across four right-swipe blocks (Default/Custom-Reveal, Default/Custom-Execute); each custom slider collapses the C# {Binding Value}→Threshold + ValueChanged→Close() into `set_threshold(v); close()`, with live thresholds in the readout.

| macOS (AppKit) | iOS (UIKit) |
| --- | --- |
| ![macOS](macos.png) | ![iOS](ios.png) |

**Running on iOS:**

![iOS demo](ios.gif)

**Platforms:** macOS ✅ demo · iOS ✅ demo · Windows ⬜ TODO · Linux ⬜ TODO · Android ⬜ TODO

**Run it:** `MAUI_SAMPLE_PAGE=swipe_threshold ./build/apple/maui_macos_gallery` (macOS) · `SIMCTL_CHILD_MAUI_SAMPLE_PAGE=swipe_threshold xcrun simctl launch booted dev.maui-cpp.ios-gallery` (iOS)

> Note: SwipeView is gesture-driven (swipe-to-reveal). The swipe content + structure render, and each page synthetically calls `open()` so the SwipeItems are wired and their `invoked` fires into a readout; the revealed item panel may not visually offset in a static capture (it needs a live drag) — the swipe_view control, its item collections, and the invoked channel are what these prove.
