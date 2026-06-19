# CollectionView — Empty View

Ports .NET MAUI's `EmptyViewStringGallery` ([oracle](../../../../../src/Controls/samples/Controls.Sample/Pages/Controls/CollectionViewGalleries/EmptyViewGalleries/EmptyViewStringGallery.xaml)) as a code-first `maui::samples::empty_view_page`, mirroring the `items_page` pattern (owned `observable_collection` + `data_template`). A search_bar over a collection_view whose EmptyView is a boxed string; the search_command filters a 50-row source via add/remove reconcile, and add/clear toggle the source empty so the EmptyView appears.

| macOS (AppKit) | iOS (UIKit) |
| --- | --- |
| ![macOS](macos.png) | ![iOS](ios.png) |

**Running on iOS:**

![iOS demo](ios.gif)

**Platforms:** macOS ✅ chrome · iOS ✅ chrome · Windows ⬜ TODO · Linux ⬜ TODO · Android ⬜ TODO

**Run it:** `MAUI_SAMPLE_PAGE=empty_view ./build/apple/maui_macos_gallery` (macOS) · `SIMCTL_CHILD_MAUI_SAMPLE_PAGE=empty_view xcrun simctl launch booted dev.maui-cpp.ios-gallery` (iOS)

> ⚠️ **Known gallery-rendering gap:** the CollectionView **chrome** (header/footer, search bar, selection picker, readouts, empty-view) renders and the full CollectionView **API is exercised in code**, but the per-item **cell text does not yet render** in the gallery when the item source is a custom struct type (the flat-`std::string` [items demo](../items/) confirms the control renders cells). Root cause: templated-cell realization/binding for non-string item types — a fix is in flight (a dedicated framework unit).
