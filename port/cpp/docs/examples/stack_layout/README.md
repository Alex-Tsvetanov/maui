# StackLayout

Ports .NET MAUI's `StackLayoutPage` ([oracle](../../../../../src/Controls/samples/Controls.Sample/Pages/Layouts/StackLayoutPage.xaml)) as a code-first `maui::samples::stack_layout_page`. The generic `stack_layout` with runtime orientation — a vertical inner stack and a horizontal inner stack (six colored BoxViews each) under a padded outer vertical stack.

| macOS (AppKit) | iOS (UIKit) |
| --- | --- |
| ![macOS](macos.png) | ![iOS](ios.png) |

**Running on iOS:**

![iOS demo](ios.gif)

**Platforms:** macOS ✅ demo · iOS ✅ demo · Windows ⬜ TODO · Linux ⬜ TODO · Android ⬜ TODO

**Run it:**
- macOS — `MAUI_SAMPLE_PAGE=stack_layout ./build/apple/maui_macos_gallery`
- iOS sim — `SIMCTL_CHILD_MAUI_SAMPLE_PAGE=stack_layout xcrun simctl launch booted dev.maui-cpp.ios-gallery`

> On macOS the gallery host sizes the window to the measured content over plain (unflipped) NSViews, so
> layout pages can show a partial/single block — the iOS capture is the faithful top-down layout. Notes: XAML Margin maps to stack padding.
