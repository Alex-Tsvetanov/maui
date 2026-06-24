# SwipeView — Basic

Ports .NET MAUI's `BasicSwipeGallery` ([oracle](../../../../../src/Controls/samples/Controls.Sample/Pages/Controls/SwipeViewGalleries/BasicSwipeGallery.xaml)) as a code-first `maui::samples::basic_swipe_page`. Five SwipeViews over labelled grids covering every revealed-side / SwipeMode combo (Bottom-Execute, Top-Reveal, Left-Reveal, Right-Execute, all-four-directions-Reveal), each with a red 'Delete' SwipeItem whose Invoked feeds a shared readout + counter.

| macOS (AppKit) | iOS (UIKit) |
| --- | --- |
| ![macOS](macos.png) | ![iOS](ios.png) |

**Running on iOS:**

![iOS demo](ios.gif)

**Platforms:** macOS ✅ demo · iOS ✅ demo · Windows ⬜ TODO · Linux ⬜ TODO · Android ⬜ TODO

**Run it:** `MAUI_SAMPLE_PAGE=basic_swipe ./examples/build/gallery/gallery` (macOS) · `SIMCTL_CHILD_MAUI_SAMPLE_PAGE=basic_swipe xcrun simctl launch booted dev.maui-cpp.ios-gallery` (iOS)

> Note: SwipeView is gesture-driven (swipe-to-reveal). The swipe content + structure render, and each page synthetically calls `open()` so the SwipeItems are wired and their `invoked` fires into a readout; the revealed item panel may not visually offset in a static capture (it needs a live drag) — the swipe_view control, its item collections, and the invoked channel are what these prove.
