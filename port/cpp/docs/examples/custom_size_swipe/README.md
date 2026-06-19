# SwipeView — Custom Size

Ports .NET MAUI's `CustomSizeSwipeViewGallery` ([oracle](../../../../../src/Controls/samples/Controls.Sample/Pages/Controls/SwipeViewGalleries/CustomSizeSwipeViewGallery.xaml)) as a code-first `maui::samples::custom_size_swipe_page`. One SwipeView with custom-sized reveal panels on three sides — Left swipe_item_view (width 200), Right = an icon+text SwipeItem then a swipe_item_view (width 200), Top swipe_item_view (height 100) — each with buttons whose Clicked drives a readout.

| macOS (AppKit) | iOS (UIKit) |
| --- | --- |
| ![macOS](macos.png) | ![iOS](ios.png) |

**Running on iOS:**

![iOS demo](ios.gif)

**Platforms:** macOS ✅ demo · iOS ✅ demo · Windows ⬜ TODO · Linux ⬜ TODO · Android ⬜ TODO

**Run it:** `MAUI_SAMPLE_PAGE=custom_size_swipe ./build/apple/maui_macos_gallery` (macOS) · `SIMCTL_CHILD_MAUI_SAMPLE_PAGE=custom_size_swipe xcrun simctl launch booted dev.maui-cpp.ios-gallery` (iOS)

> Note: SwipeView is gesture-driven (swipe-to-reveal). The swipe content + structure render, and each page synthetically calls `open()` so the SwipeItems are wired and their `invoked` fires into a readout; the revealed item panel may not visually offset in a static capture (it needs a live drag) — the swipe_view control, its item collections, and the invoked channel are what these prove.
