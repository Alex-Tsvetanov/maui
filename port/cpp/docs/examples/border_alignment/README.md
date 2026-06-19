# Border Alignment

Ports .NET MAUI's `BorderAlignment` ([oracle](../../../../../src/Controls/samples/Controls.Sample/Pages/Core/BorderGalleries/BorderAlignment.xaml)) as a code-first `maui::samples::border_alignment_page`. Four red-bordered (RoundRectangle CR=5) blue Grids under Start/Center/End/Fill captions, each section's `HorizontalOptions` set to the matching alignment.

| macOS (AppKit) | iOS (UIKit) |
| --- | --- |
| ![macOS](macos.png) | ![iOS](ios.png) |

**Running on iOS:**

![iOS demo](ios.gif)

**Platforms:** macOS ✅ demo · iOS ✅ demo · Windows ⬜ TODO · Linux ⬜ TODO · Android ⬜ TODO

**Run it:** `MAUI_SAMPLE_PAGE=border_alignment ./build/apple/maui_macos_gallery` (macOS) · `SIMCTL_CHILD_MAUI_SAMPLE_PAGE=border_alignment xcrun simctl launch booted dev.maui-cpp.ios-gallery` (iOS)

> The four headline labels visibly align **Start** (left) / **Center** / **End** (right) / **Fill** — `View.HorizontalOptions`/`VerticalOptions` is now settable and honored at `view::arrange` (the C# `LayoutExtensions.ComputeFrame` path; previously the view surface hardcoded Fill). The bordered cells stay full-width because a Border sizes to its content and the blue grid fills the available width (a faithful content-sizing behavior — the alignment is demonstrated by the labels). This page is also the visible proof of the LayoutOptions framework fix.
