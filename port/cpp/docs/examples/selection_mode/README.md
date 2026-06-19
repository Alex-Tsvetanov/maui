# CollectionView — Selection Mode

Ports .NET MAUI's `SelectionModeGallery` ([oracle](../../../../../src/Controls/samples/Controls.Sample/Pages/Controls/CollectionViewGalleries/SelectionGalleries/SelectionModeGallery.xaml)) as a code-first `maui::samples::selection_mode_page`, mirroring the `items_page` pattern (owned `observable_collection` + `data_template`). A None/Single/Multiple picker driving selection_mode, with both event-driven (selection_changed) and command-driven (selection_changed_command) readouts of the SelectedItem(s), GridItemsLayout Span=3 + a header.

| macOS (AppKit) | iOS (UIKit) |
| --- | --- |
| ![macOS](macos.png) | ![iOS](ios.png) |

**Running on iOS:**

![iOS demo](ios.gif)

**Platforms:** macOS ✅ chrome · iOS ✅ chrome · Windows ⬜ TODO · Linux ⬜ TODO · Android ⬜ TODO

**Run it:** `MAUI_SAMPLE_PAGE=selection_mode ./build/apple/maui_macos_gallery` (macOS) · `SIMCTL_CHILD_MAUI_SAMPLE_PAGE=selection_mode xcrun simctl launch booted dev.maui-cpp.ios-gallery` (iOS)

> ⚠️ **Known gallery-rendering gap:** the CollectionView **chrome** (header/footer, search bar, selection picker, readouts, empty-view) renders and the full CollectionView **API is exercised in code**, but the per-item **cell text does not yet render** in the gallery when the item source is a custom struct type (the flat-`std::string` [items demo](../items/) confirms the control renders cells). Root cause: templated-cell realization/binding for non-string item types — a fix is in flight (a dedicated framework unit).
