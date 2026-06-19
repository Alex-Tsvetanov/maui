# C++ port vs .NET MAUI — visual parity comparison

Pages rendered by **real shipped .NET MAUI** vs the **C++ port**, on **iOS and macOS/Mac Catalyst**. Compare layout/geometry (C# = dark appearance on Mac Catalyst, C++ gallery = light window). C++ macOS captures are post the AppKit top-left-origin flip fix.

**44 pages** on both platforms.

### iOS

| Page | .NET MAUI (iOS) | C++ port (iOS) |
| --- | --- | --- |
| Absolute Layout | ![cs](csharp_ios/absolute_layout.png) | ![cpp](../examples/absolute_layout/ios.png) |
| Activity Indicator | ![cs](csharp_ios/activity_indicator.png) | ![cpp](../examples/activity_indicator/ios.png) |
| Alerts | ![cs](csharp_ios/alerts.png) | ![cpp](../examples/alerts/ios.png) |
| Layout alignment (Start/Center/End/Fill) | ![cs](csharp_ios/alignment.png) | ![cpp](../examples/border_alignment/ios.png) |
| Animation | ![cs](csharp_ios/animation.png) | ![cpp](../examples/animation/ios.png) |
| Behaviors | ![cs](csharp_ios/behaviors.png) | ![cpp](../examples/behaviors/ios.png) |
| Border | ![cs](csharp_ios/border.png) | ![cpp](../examples/border_styles/ios.png) |
| Box View | ![cs](csharp_ios/box_view.png) | ![cpp](../examples/box_view/ios.png) |
| Button | ![cs](csharp_ios/button.png) | ![cpp](../examples/button/ios.png) |
| Check Box | ![cs](csharp_ios/check_box.png) | ![cpp](../examples/check_box/ios.png) |
| Clipping | ![cs](csharp_ios/clipping.png) | ![cpp](../examples/clipping/ios.png) |
| CollectionView | ![cs](csharp_ios/collectionview.png) | ![cpp](../examples/selection_mode/ios.png) |
| Content View | ![cs](csharp_ios/content_view.png) | ![cpp](../examples/content_view/ios.png) |
| Control stack | ![cs](csharp_ios/controls_stack.png) | ![cpp](../examples/value_controls/ios.png) |
| Date Picker | ![cs](csharp_ios/date_picker.png) | ![cpp](../examples/date_picker/ios.png) |
| Editor | ![cs](csharp_ios/editor.png) | ![cpp](../examples/editor/ios.png) |
| Entry | ![cs](csharp_ios/entry.png) | ![cpp](../examples/entry/ios.png) |
| Flex Layout | ![cs](csharp_ios/flex_layout.png) | ![cpp](../examples/flex_layout/ios.png) |
| Fonts | ![cs](csharp_ios/fonts.png) | ![cpp](../examples/fonts/ios.png) |
| Formatted Text | ![cs](csharp_ios/formatted_text.png) | ![cpp](../examples/formatted_text/ios.png) |
| Gradient brushes | ![cs](csharp_ios/gradient.png) | ![cpp](../examples/brushes/ios.png) |
| Grid | ![cs](csharp_ios/grid.png) | ![cpp](../examples/grid/ios.png) |
| Horizontal Stack | ![cs](csharp_ios/horizontal_stack.png) | ![cpp](../examples/horizontal_stack/ios.png) |
| Image | ![cs](csharp_ios/image.png) | ![cpp](../examples/image/ios.png) |
| Image Button | ![cs](csharp_ios/image_button.png) | ![cpp](../examples/image_button/ios.png) |
| Indicator | ![cs](csharp_ios/indicator.png) | ![cpp](../examples/indicator/ios.png) |
| Label | ![cs](csharp_ios/label.png) | ![cpp](../examples/label/ios.png) |
| Picker | ![cs](csharp_ios/picker.png) | ![cpp](../examples/picker/ios.png) |
| Progress Bar | ![cs](csharp_ios/progress_bar.png) | ![cpp](../examples/progress_bar/ios.png) |
| Scroll View | ![cs](csharp_ios/scroll_view.png) | ![cpp](../examples/scroll_view/ios.png) |
| Search Bar | ![cs](csharp_ios/search_bar.png) | ![cpp](../examples/search_bar/ios.png) |
| Semantics | ![cs](csharp_ios/semantics.png) | ![cpp](../examples/semantics/ios.png) |
| Shapes | ![cs](csharp_ios/shapes.png) | ![cpp](../examples/shapes_demo/ios.png) |
| Slider | ![cs](csharp_ios/slider.png) | ![cpp](../examples/slider/ios.png) |
| Stack Layout | ![cs](csharp_ios/stack_layout.png) | ![cpp](../examples/stack_layout/ios.png) |
| Stepper | ![cs](csharp_ios/stepper.png) | ![cpp](../examples/stepper/ios.png) |
| Styles | ![cs](csharp_ios/styles.png) | ![cpp](../examples/styles/ios.png) |
| Switch | ![cs](csharp_ios/switch.png) | ![cpp](../examples/switch/ios.png) |
| Time Picker | ![cs](csharp_ios/time_picker.png) | ![cpp](../examples/time_picker/ios.png) |
| Transformations | ![cs](csharp_ios/transformations.png) | ![cpp](../examples/transformations/ios.png) |
| Triggers | ![cs](csharp_ios/triggers.png) | ![cpp](../examples/triggers/ios.png) |
| Vertical Stack | ![cs](csharp_ios/vertical_stack.png) | ![cpp](../examples/vertical_stack/ios.png) |
| Visual States | ![cs](csharp_ios/visual_states.png) | ![cpp](../examples/visual_states/ios.png) |
| Z Index | ![cs](csharp_ios/z_index.png) | ![cpp](../examples/z_index/ios.png) |

### macOS / Mac Catalyst

| Page | .NET MAUI (Mac Catalyst) | C++ port (macOS) |
| --- | --- | --- |
| Absolute Layout | ![cs](csharp_maccatalyst/absolute_layout.png) | ![cpp](../examples/absolute_layout/macos.png) |
| Activity Indicator | ![cs](csharp_maccatalyst/activity_indicator.png) | ![cpp](../examples/activity_indicator/macos.png) |
| Alerts | ![cs](csharp_maccatalyst/alerts.png) | ![cpp](../examples/alerts/macos.png) |
| Layout alignment (Start/Center/End/Fill) | ![cs](csharp_maccatalyst/alignment.png) | ![cpp](../examples/border_alignment/macos.png) |
| Animation | ![cs](csharp_maccatalyst/animation.png) | ![cpp](../examples/animation/macos.png) |
| Behaviors | ![cs](csharp_maccatalyst/behaviors.png) | ![cpp](../examples/behaviors/macos.png) |
| Border | ![cs](csharp_maccatalyst/border.png) | ![cpp](../examples/border_styles/macos.png) |
| Box View | ![cs](csharp_maccatalyst/box_view.png) | ![cpp](../examples/box_view/macos.png) |
| Button | ![cs](csharp_maccatalyst/button.png) | ![cpp](../examples/button/macos.png) |
| Check Box | ![cs](csharp_maccatalyst/check_box.png) | ![cpp](../examples/check_box/macos.png) |
| Clipping | ![cs](csharp_maccatalyst/clipping.png) | ![cpp](../examples/clipping/macos.png) |
| CollectionView | ![cs](csharp_maccatalyst/collectionview.png) | ![cpp](../examples/selection_mode/macos.png) |
| Content View | ![cs](csharp_maccatalyst/content_view.png) | ![cpp](../examples/content_view/macos.png) |
| Control stack | ![cs](csharp_maccatalyst/controls_stack.png) | ![cpp](../examples/value_controls/macos.png) |
| Date Picker | ![cs](csharp_maccatalyst/date_picker.png) | ![cpp](../examples/date_picker/macos.png) |
| Editor | ![cs](csharp_maccatalyst/editor.png) | ![cpp](../examples/editor/macos.png) |
| Entry | ![cs](csharp_maccatalyst/entry.png) | ![cpp](../examples/entry/macos.png) |
| Flex Layout | ![cs](csharp_maccatalyst/flex_layout.png) | ![cpp](../examples/flex_layout/macos.png) |
| Fonts | ![cs](csharp_maccatalyst/fonts.png) | ![cpp](../examples/fonts/macos.png) |
| Formatted Text | ![cs](csharp_maccatalyst/formatted_text.png) | ![cpp](../examples/formatted_text/macos.png) |
| Gradient brushes | ![cs](csharp_maccatalyst/gradient.png) | ![cpp](../examples/brushes/macos.png) |
| Grid | ![cs](csharp_maccatalyst/grid.png) | ![cpp](../examples/grid/macos.png) |
| Horizontal Stack | ![cs](csharp_maccatalyst/horizontal_stack.png) | ![cpp](../examples/horizontal_stack/macos.png) |
| Image | ![cs](csharp_maccatalyst/image.png) | ![cpp](../examples/image/macos.png) |
| Image Button | ![cs](csharp_maccatalyst/image_button.png) | ![cpp](../examples/image_button/macos.png) |
| Indicator | ![cs](csharp_maccatalyst/indicator.png) | ![cpp](../examples/indicator/macos.png) |
| Label | ![cs](csharp_maccatalyst/label.png) | ![cpp](../examples/label/macos.png) |
| Picker | ![cs](csharp_maccatalyst/picker.png) | ![cpp](../examples/picker/macos.png) |
| Progress Bar | ![cs](csharp_maccatalyst/progress_bar.png) | ![cpp](../examples/progress_bar/macos.png) |
| Scroll View | ![cs](csharp_maccatalyst/scroll_view.png) | ![cpp](../examples/scroll_view/macos.png) |
| Search Bar | ![cs](csharp_maccatalyst/search_bar.png) | ![cpp](../examples/search_bar/macos.png) |
| Semantics | ![cs](csharp_maccatalyst/semantics.png) | ![cpp](../examples/semantics/macos.png) |
| Shapes | ![cs](csharp_maccatalyst/shapes.png) | ![cpp](../examples/shapes_demo/macos.png) |
| Slider | ![cs](csharp_maccatalyst/slider.png) | ![cpp](../examples/slider/macos.png) |
| Stack Layout | ![cs](csharp_maccatalyst/stack_layout.png) | ![cpp](../examples/stack_layout/macos.png) |
| Stepper | ![cs](csharp_maccatalyst/stepper.png) | ![cpp](../examples/stepper/macos.png) |
| Styles | ![cs](csharp_maccatalyst/styles.png) | ![cpp](../examples/styles/macos.png) |
| Switch | ![cs](csharp_maccatalyst/switch.png) | ![cpp](../examples/switch/macos.png) |
| Time Picker | ![cs](csharp_maccatalyst/time_picker.png) | ![cpp](../examples/time_picker/macos.png) |
| Transformations | ![cs](csharp_maccatalyst/transformations.png) | ![cpp](../examples/transformations/macos.png) |
| Triggers | ![cs](csharp_maccatalyst/triggers.png) | ![cpp](../examples/triggers/macos.png) |
| Vertical Stack | ![cs](csharp_maccatalyst/vertical_stack.png) | ![cpp](../examples/vertical_stack/macos.png) |
| Visual States | ![cs](csharp_maccatalyst/visual_states.png) | ![cpp](../examples/visual_states/macos.png) |
| Z Index | ![cs](csharp_maccatalyst/z_index.png) | ![cpp](../examples/z_index/macos.png) |
