# CollectionView — Chat (variable item size)

Ports .NET MAUI's `ChatExample` ([oracle](../../../../../src/Controls/samples/Controls.Sample/Pages/Controls/CollectionViewGalleries/ItemSizeGalleries/ChatExample.xaml)) as a code-first `maui::samples::chat_example_page`, mirroring the `items_page` pattern (owned `observable_collection` + `data_template`). Variable-height items via a chat_template_selector picking local vs remote bubble templates off chat_message.is_local, item_sizing_strategy::measure_all_items, + append/clear/add-1000 buttons (deterministic LCG length generator).

| macOS (AppKit) | iOS (UIKit) |
| --- | --- |
| ![macOS](macos.png) | ![iOS](ios.png) |

**Running on iOS:**

![iOS demo](ios.gif)

**Platforms:** macOS ✅ demo · iOS ✅ demo · Windows ⬜ TODO · Linux ⬜ TODO · Android ⬜ TODO

**Run it:** `MAUI_SAMPLE_PAGE=chat_example ./examples/build/gallery/gallery` (macOS) · `SIMCTL_CHILD_MAUI_SAMPLE_PAGE=chat_example xcrun simctl launch booted dev.maui-cpp.ios-gallery` (iOS)

> The custom-struct item cells render their template-bound content natively (the data_template is instantiated, its binding-context set to the boxed struct, and a handler attached per cell — the C# `TemplatedCell.Bind` path).
