# CollectionView — Single Bound Selection

Ports .NET MAUI's `SingleBoundSelection` ([oracle](../../../../../src/Controls/samples/Controls.Sample/Pages/Controls/CollectionViewGalleries/SelectionGalleries/SingleBoundSelection.xaml)) as a code-first `maui::samples::single_bound_selection_page`, mirroring the `items_page` pattern (owned `observable_collection` + `data_template`). SelectionMode=Single over a country model with both legs of the TwoWay SelectedItem binding realized — selection_changed feeds the readout (forward), reset/clear push SelectedItem into the view (back).

| macOS (AppKit) | iOS (UIKit) |
| --- | --- |
| ![macOS](macos.png) | ![iOS](ios.png) |

**Running on iOS:**

![iOS demo](ios.gif)

**Platforms:** macOS ✅ demo · iOS ✅ demo · Windows ⬜ TODO · Linux ⬜ TODO · Android ⬜ TODO

**Run it:** `MAUI_SAMPLE_PAGE=single_bound_selection ./examples/build/gallery/gallery` (macOS) · `SIMCTL_CHILD_MAUI_SAMPLE_PAGE=single_bound_selection xcrun simctl launch booted dev.maui-cpp.ios-gallery` (iOS)

> The custom-struct item cells render their template-bound content natively (the data_template is instantiated, its binding-context set to the boxed struct, and a handler attached per cell — the C# `TemplatedCell.Bind` path).
