# CollectionView — Selection Mode

Ports .NET MAUI's `SelectionModeGallery` ([oracle](../../../../../src/Controls/samples/Controls.Sample/Pages/Controls/CollectionViewGalleries/SelectionGalleries/SelectionModeGallery.xaml)) as a code-first `maui::samples::selection_mode_page`, mirroring the `items_page` pattern (owned `observable_collection` + `data_template`). A None/Single/Multiple picker driving selection_mode, with both event-driven (selection_changed) and command-driven (selection_changed_command) readouts of the SelectedItem(s), GridItemsLayout Span=3 + a header.

| macOS (AppKit) | iOS (UIKit) |
| --- | --- |
| ![macOS](macos.png) | ![iOS](ios.png) |

**Running on iOS:**

![iOS demo](ios.gif)

**Platforms:** macOS ✅ demo · iOS ✅ demo · Windows ⬜ TODO · Linux ⬜ TODO · Android ⬜ TODO

**Run it:** `MAUI_SAMPLE_PAGE=selection_mode ./examples/build/gallery/gallery` (macOS) · `SIMCTL_CHILD_MAUI_SAMPLE_PAGE=selection_mode xcrun simctl launch booted dev.maui-cpp.ios-gallery` (iOS)

> The custom-struct item cells render their template-bound content natively (the data_template is instantiated, its binding-context set to the boxed struct, and a handler attached per cell — the C# `TemplatedCell.Bind` path).
