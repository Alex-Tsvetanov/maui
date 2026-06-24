# VerticalStackLayout

Ports .NET MAUI's `VerticalStackLayoutPage` ([oracle](../../../../../src/Controls/samples/Controls.Sample/Pages/Layouts/VerticalStackLayoutPage.xaml)) as a code-first `maui::samples::vertical_stack_page`. A `vertical_stack_layout` stacking a label + six colored BoxViews top-to-bottom with padding.

| macOS (AppKit) | iOS (UIKit) |
| --- | --- |
| ![macOS](macos.png) | ![iOS](ios.png) |

**Running on iOS:**

![iOS demo](ios.gif)

**Platforms:** macOS ✅ demo · iOS ✅ demo · Windows ⬜ TODO · Linux ⬜ TODO · Android ⬜ TODO

**Run it:**
- macOS — `MAUI_SAMPLE_PAGE=vertical_stack ./examples/build/gallery/gallery`
- iOS sim — `SIMCTL_CHILD_MAUI_SAMPLE_PAGE=vertical_stack xcrun simctl launch booted dev.maui-cpp.ios-gallery`

> On macOS the gallery host sizes the window to the measured content over plain (unflipped) NSViews, so
> layout pages can show a partial/single block — the iOS capture is the faithful top-down layout. 
