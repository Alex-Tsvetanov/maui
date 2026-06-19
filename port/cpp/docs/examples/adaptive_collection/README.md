# CollectionView — Adaptive Layout

Ports .NET MAUI's `AdaptiveCollectionView` ([oracle](../../../../../src/Controls/samples/Controls.Sample/Pages/Controls/CollectionViewGalleries/AdaptiveCollectionView.xaml)) as a code-first `maui::samples::adaptive_collection_page`, mirroring the `items_page` pattern (owned `observable_collection` + `data_template`). One collection_view whose ItemsLayout swaps between vertical linear and grid (Span 3) via apply_width() (the C# Width>600 decision, verbatim) / use_grid() / use_linear(), with a readout of the mounted layout.

| macOS (AppKit) | iOS (UIKit) |
| --- | --- |
| ![macOS](macos.png) | ![iOS](ios.png) |

**Running on iOS:**

![iOS demo](ios.gif)

**Platforms:** macOS ✅ chrome · iOS ✅ chrome · Windows ⬜ TODO · Linux ⬜ TODO · Android ⬜ TODO

**Run it:** `MAUI_SAMPLE_PAGE=adaptive_collection ./build/apple/maui_macos_gallery` (macOS) · `SIMCTL_CHILD_MAUI_SAMPLE_PAGE=adaptive_collection xcrun simctl launch booted dev.maui-cpp.ios-gallery` (iOS)

> ⚠️ **Known gallery-rendering gap:** the CollectionView **chrome** (header/footer, search bar, selection picker, readouts, empty-view) renders and the full CollectionView **API is exercised in code**, but the per-item **cell text does not yet render** in the gallery when the item source is a custom struct type (the flat-`std::string` [items demo](../items/) confirms the control renders cells). Root cause: templated-cell realization/binding for non-string item types — a fix is in flight (a dedicated framework unit).
