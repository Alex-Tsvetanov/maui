# C++ port vs .NET MAUI — visual parity comparison

Side-by-side of pages rendered by **real, shipped .NET MAUI** (a standalone `dotnet new maui` app, `~/maui-compare`) and by the **C++ port**, on **both iOS and macOS/Mac Catalyst**. Both are captured by each framework's own pipeline (C++: the gallery self-screenshot / `simctl`; C#: the `Screenshot` essential on Catalyst, `simctl` on iOS).

Compare **layout/geometry**: the C# app uses the system dark appearance on Mac Catalyst (light on iOS), the C++ gallery a light window — chrome theme differs, layout should match. The macOS bug you spotted (bottom-up rendering) is fixed (AppKit container views now flipped to a top-left origin, matching iOS + Mac Catalyst); these C++ macOS captures are the corrected top-down render.

### iOS

| Page | .NET MAUI (iOS) | C++ port (iOS) |
| --- | --- | --- |
| Control stack | ![cs](csharp_ios/controls_stack.png) | ![cpp](../examples/value_controls/ios.png) |
| Button | ![cs](csharp_ios/button.png) | ![cpp](../examples/button/ios.png) |
| Label | ![cs](csharp_ios/label.png) | ![cpp](../examples/label/ios.png) |
| Image | ![cs](csharp_ios/image.png) | ![cpp](../examples/image/ios.png) |
| Entry | ![cs](csharp_ios/entry.png) | ![cpp](../examples/entry/ios.png) |
| Editor | ![cs](csharp_ios/editor.png) | ![cpp](../examples/editor/ios.png) |
| SearchBar | ![cs](csharp_ios/search_bar.png) | ![cpp](../examples/search_bar/ios.png) |
| CheckBox | ![cs](csharp_ios/check_box.png) | ![cpp](../examples/check_box/ios.png) |
| Switch | ![cs](csharp_ios/switch.png) | ![cpp](../examples/switch/ios.png) |
| Slider | ![cs](csharp_ios/slider.png) | ![cpp](../examples/slider/ios.png) |
| Layout alignment (Start/Center/End/Fill) | ![cs](csharp_ios/alignment.png) | ![cpp](../examples/border_alignment/ios.png) |
| Shapes | ![cs](csharp_ios/shapes.png) | ![cpp](../examples/shapes_demo/ios.png) |
| Border (RoundRectangle stroke) | ![cs](csharp_ios/border.png) | ![cpp](../examples/border_styles/ios.png) |
| CollectionView (grid, templated cells) | ![cs](csharp_ios/collectionview.png) | ![cpp](../examples/selection_mode/ios.png) |
| Fonts (sizes + attributes) | ![cs](csharp_ios/fonts.png) | ![cpp](../examples/fonts/ios.png) |
| Grid (rows × cols) | ![cs](csharp_ios/grid.png) | ![cpp](../examples/grid/ios.png) |
| Gradient brushes | ![cs](csharp_ios/gradient.png) | ![cpp](../examples/brushes/ios.png) |

### macOS / Mac Catalyst

| Page | .NET MAUI (Mac Catalyst) | C++ port (macOS) |
| --- | --- | --- |
| Control stack | ![cs](csharp_maccatalyst/controls_stack.png) | ![cpp](../examples/value_controls/macos.png) |
| Button | ![cs](csharp_maccatalyst/button.png) | ![cpp](../examples/button/macos.png) |
| Label | ![cs](csharp_maccatalyst/label.png) | ![cpp](../examples/label/macos.png) |
| Image | ![cs](csharp_maccatalyst/image.png) | ![cpp](../examples/image/macos.png) |
| Entry | ![cs](csharp_maccatalyst/entry.png) | ![cpp](../examples/entry/macos.png) |
| Editor | ![cs](csharp_maccatalyst/editor.png) | ![cpp](../examples/editor/macos.png) |
| SearchBar | ![cs](csharp_maccatalyst/search_bar.png) | ![cpp](../examples/search_bar/macos.png) |
| CheckBox | ![cs](csharp_maccatalyst/check_box.png) | ![cpp](../examples/check_box/macos.png) |
| Switch | ![cs](csharp_maccatalyst/switch.png) | ![cpp](../examples/switch/macos.png) |
| Slider | ![cs](csharp_maccatalyst/slider.png) | ![cpp](../examples/slider/macos.png) |
| Layout alignment (Start/Center/End/Fill) | ![cs](csharp_maccatalyst/alignment.png) | ![cpp](../examples/border_alignment/macos.png) |
| Shapes | ![cs](csharp_maccatalyst/shapes.png) | ![cpp](../examples/shapes_demo/macos.png) |
| Border (RoundRectangle stroke) | ![cs](csharp_maccatalyst/border.png) | ![cpp](../examples/border_styles/macos.png) |
| CollectionView (grid, templated cells) | ![cs](csharp_maccatalyst/collectionview.png) | ![cpp](../examples/selection_mode/macos.png) |
| Fonts (sizes + attributes) | ![cs](csharp_maccatalyst/fonts.png) | ![cpp](../examples/fonts/macos.png) |
| Grid (rows × cols) | ![cs](csharp_maccatalyst/grid.png) | ![cpp](../examples/grid/macos.png) |
| Gradient brushes | ![cs](csharp_maccatalyst/gradient.png) | ![cpp](../examples/brushes/macos.png) |

**Coverage:** 17 pages on both platforms so far (the per-control + representative layout/shape/border/CollectionView/gradient set). Expanding toward the full gallery (173 pages) via the reverse-port batches; this index grows as C# equivalents land.
