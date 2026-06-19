# SwipeView — Custom Item View

Ports .NET MAUI's `CustomSwipeItemViewGallery` ([oracle](../../../../../src/Controls/samples/Controls.Sample/Pages/Controls/SwipeViewGalleries/CustomSwipeItemViewGallery.xaml)) as a code-first `maui::samples::custom_swipe_item_view_page`. A custom-content right swipe item (`swipe_item_view`, not a plain SwipeItem) hosting a Border + 'Favourite' label over a message row (title + date); the custom item's `invoked` event is the command channel (W1-11 collapse) → readout.

| macOS (AppKit) | iOS (UIKit) |
| --- | --- |
| ![macOS](macos.png) | ![iOS](ios.png) |

**Running on iOS:**

![iOS demo](ios.gif)

**Platforms:** macOS ✅ demo · iOS ✅ demo · Windows ⬜ TODO · Linux ⬜ TODO · Android ⬜ TODO

**Run it:** `MAUI_SAMPLE_PAGE=custom_swipe_item_view ./build/apple/maui_macos_gallery` (macOS) · `SIMCTL_CHILD_MAUI_SAMPLE_PAGE=custom_swipe_item_view xcrun simctl launch booted dev.maui-cpp.ios-gallery` (iOS)

> Note: SwipeView is gesture-driven (swipe-to-reveal). The swipe content + structure render, and each page synthetically calls `open()` so the SwipeItems are wired and their `invoked` fires into a readout; the revealed item panel may not visually offset in a static capture (it needs a live drag) — the swipe_view control, its item collections, and the invoked channel are what these prove.
