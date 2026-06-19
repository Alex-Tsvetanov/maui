# C++ port vs .NET MAUI — visual parity comparison

Side-by-side of pages rendered by **real, shipped .NET MAUI** (standalone `dotnet new maui` app) and by the **C++ port**, on **both iOS and macOS/Mac Catalyst**. Compare layout/geometry (the C# app uses the system dark appearance on Mac Catalyst; the C++ gallery a light window — chrome theme differs).

The macOS bottom-up bug is fixed (AppKit container views flipped to a top-left origin); the C++ macOS captures are the corrected top-down render, matching iOS + C# Mac Catalyst.

**26 pages** compared on both platforms (expanding toward the full 173-page gallery via the reverse-port batches).

### iOS

| Page | .NET MAUI (iOS) | C++ port (iOS) |
| --- | --- | --- |
| Activity Indicator | ![cs](csharp_ios/activity_indicator.png) | ![cpp](../examples/activity_indicator/ios.png) |
| Layout alignment (Start/Center/End/Fill) | ![cs](csharp_ios/alignment.png) | ![cpp](../examples/border_alignment/ios.png) |
| Border | ![cs](csharp_ios/border.png) | ![cpp](../examples/border_styles/ios.png) |
| Box View | ![cs](csharp_ios/box_view.png) | ![cpp](../examples/box_view/ios.png) |
| Button | ![cs](csharp_ios/button.png) | ![cpp](../examples/button/ios.png) |
| Check Box | ![cs](csharp_ios/check_box.png) | ![cpp](../examples/check_box/ios.png) |
| CollectionView | ![cs](csharp_ios/collectionview.png) | ![cpp](../examples/selection_mode/ios.png) |
| Control stack | ![cs](csharp_ios/controls_stack.png) | ![cpp](../examples/value_controls/ios.png) |
| Date Picker | ![cs](csharp_ios/date_picker.png) | ![cpp](../examples/date_picker/ios.png) |
| Editor | ![cs](csharp_ios/editor.png) | ![cpp](../examples/editor/ios.png) |
| Entry | ![cs](csharp_ios/entry.png) | ![cpp](../examples/entry/ios.png) |
| Fonts | ![cs](csharp_ios/fonts.png) | ![cpp](../examples/fonts/ios.png) |
| Gradient brushes | ![cs](csharp_ios/gradient.png) | ![cpp](../examples/brushes/ios.png) |
| Grid | ![cs](csharp_ios/grid.png) | ![cpp](../examples/grid/ios.png) |
| Image | ![cs](csharp_ios/image.png) | ![cpp](../examples/image/ios.png) |
| Image Button | ![cs](csharp_ios/image_button.png) | ![cpp](../examples/image_button/ios.png) |
| Indicator | ![cs](csharp_ios/indicator.png) | ![cpp](../examples/indicator/ios.png) |
| Label | ![cs](csharp_ios/label.png) | ![cpp](../examples/label/ios.png) |
| Picker | ![cs](csharp_ios/picker.png) | ![cpp](../examples/picker/ios.png) |
| Progress Bar | ![cs](csharp_ios/progress_bar.png) | ![cpp](../examples/progress_bar/ios.png) |
| Search Bar | ![cs](csharp_ios/search_bar.png) | ![cpp](../examples/search_bar/ios.png) |
| Shapes | ![cs](csharp_ios/shapes.png) | ![cpp](../examples/shapes_demo/ios.png) |
| Slider | ![cs](csharp_ios/slider.png) | ![cpp](../examples/slider/ios.png) |
| Stepper | ![cs](csharp_ios/stepper.png) | ![cpp](../examples/stepper/ios.png) |
| Switch | ![cs](csharp_ios/switch.png) | ![cpp](../examples/switch/ios.png) |
| Time Picker | ![cs](csharp_ios/time_picker.png) | ![cpp](../examples/time_picker/ios.png) |

### macOS / Mac Catalyst

| Page | .NET MAUI (Mac Catalyst) | C++ port (macOS) |
| --- | --- | --- |
| Activity Indicator | ![cs](csharp_maccatalyst/activity_indicator.png) | ![cpp](../examples/activity_indicator/macos.png) |
| Layout alignment (Start/Center/End/Fill) | ![cs](csharp_maccatalyst/alignment.png) | ![cpp](../examples/border_alignment/macos.png) |
| Border | ![cs](csharp_maccatalyst/border.png) | ![cpp](../examples/border_styles/macos.png) |
| Box View | ![cs](csharp_maccatalyst/box_view.png) | ![cpp](../examples/box_view/macos.png) |
| Button | ![cs](csharp_maccatalyst/button.png) | ![cpp](../examples/button/macos.png) |
| Check Box | ![cs](csharp_maccatalyst/check_box.png) | ![cpp](../examples/check_box/macos.png) |
| CollectionView | ![cs](csharp_maccatalyst/collectionview.png) | ![cpp](../examples/selection_mode/macos.png) |
| Control stack | ![cs](csharp_maccatalyst/controls_stack.png) | ![cpp](../examples/value_controls/macos.png) |
| Date Picker | ![cs](csharp_maccatalyst/date_picker.png) | ![cpp](../examples/date_picker/macos.png) |
| Editor | ![cs](csharp_maccatalyst/editor.png) | ![cpp](../examples/editor/macos.png) |
| Entry | ![cs](csharp_maccatalyst/entry.png) | ![cpp](../examples/entry/macos.png) |
| Fonts | ![cs](csharp_maccatalyst/fonts.png) | ![cpp](../examples/fonts/macos.png) |
| Gradient brushes | ![cs](csharp_maccatalyst/gradient.png) | ![cpp](../examples/brushes/macos.png) |
| Grid | ![cs](csharp_maccatalyst/grid.png) | ![cpp](../examples/grid/macos.png) |
| Image | ![cs](csharp_maccatalyst/image.png) | ![cpp](../examples/image/macos.png) |
| Image Button | ![cs](csharp_maccatalyst/image_button.png) | ![cpp](../examples/image_button/macos.png) |
| Indicator | ![cs](csharp_maccatalyst/indicator.png) | ![cpp](../examples/indicator/macos.png) |
| Label | ![cs](csharp_maccatalyst/label.png) | ![cpp](../examples/label/macos.png) |
| Picker | ![cs](csharp_maccatalyst/picker.png) | ![cpp](../examples/picker/macos.png) |
| Progress Bar | ![cs](csharp_maccatalyst/progress_bar.png) | ![cpp](../examples/progress_bar/macos.png) |
| Search Bar | ![cs](csharp_maccatalyst/search_bar.png) | ![cpp](../examples/search_bar/macos.png) |
| Shapes | ![cs](csharp_maccatalyst/shapes.png) | ![cpp](../examples/shapes_demo/macos.png) |
| Slider | ![cs](csharp_maccatalyst/slider.png) | ![cpp](../examples/slider/macos.png) |
| Stepper | ![cs](csharp_maccatalyst/stepper.png) | ![cpp](../examples/stepper/macos.png) |
| Switch | ![cs](csharp_maccatalyst/switch.png) | ![cpp](../examples/switch/macos.png) |
| Time Picker | ![cs](csharp_maccatalyst/time_picker.png) | ![cpp](../examples/time_picker/macos.png) |
