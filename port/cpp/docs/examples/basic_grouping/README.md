# CollectionView — Basic Grouping

Ports .NET MAUI's `BasicGrouping` ([oracle](../../../../../src/Controls/samples/Controls.Sample/Pages/Controls/CollectionViewGalleries/GroupingGalleries/BasicGrouping.xaml)) as a code-first `maui::samples::basic_grouping_page`, mirroring the `items_page` pattern (owned `observable_collection` + `data_template`). IsGrouped over six Marvel-team rosters with item / LightGreen group-header / Orange group-footer templates (footer renders "Total members: N") + view-level Header/Footer strings.

| macOS (AppKit) | iOS (UIKit) |
| --- | --- |
| ![macOS](macos.png) | ![iOS](ios.png) |

**Running on iOS:**

![iOS demo](ios.gif)

**Platforms:** macOS ✅ chrome · iOS ✅ chrome · Windows ⬜ TODO · Linux ⬜ TODO · Android ⬜ TODO

**Run it:** `MAUI_SAMPLE_PAGE=basic_grouping ./build/apple/maui_macos_gallery` (macOS) · `SIMCTL_CHILD_MAUI_SAMPLE_PAGE=basic_grouping xcrun simctl launch booted dev.maui-cpp.ios-gallery` (iOS)

> ⚠️ **Known gallery-rendering gap:** the CollectionView **chrome** (header/footer, search bar, selection picker, readouts, empty-view) renders and the full CollectionView **API is exercised in code**, but the per-item **cell text does not yet render** in the gallery when the item source is a custom struct type (the flat-`std::string` [items demo](../items/) confirms the control renders cells). Root cause: templated-cell realization/binding for non-string item types — a fix is in flight (a dedicated framework unit).
