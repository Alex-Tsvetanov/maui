# iOS parity audit — C++ port vs real .NET MAUI

Automated page-by-page audit of **layout/geometry parity** between the C++ port and shipped .NET MAUI,
comparing the iOS captures in [`csharp_ios/`](csharp_ios) (real MAUI) against [`../examples/<key>/ios.png`](../examples)
(C++ port). Theme (dark/light), iOS chrome (status/nav bars), font glyphs, and sub-pixel spacing were
explicitly **ignored** — only structural differences (missing/blank/overlapping/mis-ordered/mis-sized
controls) count. Each flagged page was re-checked by a second skeptical reviewer where the budget allowed.

**Coverage:** 127 / 171 page-pairs judged (the rest were cut off by a transient server-side rate limit and
will be completed in a follow-up pass). Tallies of the 127: **14 identical, 29 minor, 75 major, 6 cpp-blank,
3 cs-blank** — but the "major" bucket is dominated by the two harness artifacts below, not by port bugs.

## Harness artifacts — NOT port bugs (the comparison app, not the framework)

These explain the majority of "major" flags. The comparison app is a `dotnet new maui` project, which
carries template defaults the code-first C++ gallery does not:

1. **Default `Button` (and Label/Entry…) chrome.** The comparison app's `Resources/Styles/Styles.xaml`
   implicitly styles every `Button` with `BackgroundColor=Primary` (purple) + white text. Without it,
   MAUI's iOS button is a borderless `.system` `UIButton` — i.e. exactly the "bare tinted text" the C++
   port renders. **The port is framework-correct here.** Pages flagged only for this: `alerts`, `animation`,
   `application_control`, `dispatcher`, `app_theme_binding`, `context_flyout`, `ios_date_picker`,
   `ios_pan_gesture`, and the button half of `invalidate_shadow_host`.
   → To make the comparison framework-fair, strip the implicit styles from the comparison app's
   `Styles.xaml` and re-capture (pending a methodology decision).
2. **Missing image assets.** The C++ gallery doesn't bundle the sample bitmaps (dotnet_bot, oasis.jpg…),
   so clipped/▢ image regions render blank where MAUI shows the picture. Expected. Pages: `clip`,
   `clip_gallery`, `clip_corner_radius`, `border_playground`, `border_resize_content`.
3. **Locale/format text** (date/time strings) and **CollectionView row spacing** differences are cosmetic.

## Genuine framework bugs (the real fix list)

| Root cause | Status | Pages affected |
| --- | --- | --- |
| **Border ignores its own `HeightRequest`/`WidthRequest`** — `border::measure` returned `content+inset` without reconciling the view's size request, so sized Borders collapsed. | ✅ **FIXED** (`5acf8f3b4d`) | `swipe_view_shadow`, `invalidate_shadow_host`, `border_clip_playground`, `border_playground`, `border_resize_content`, `border_stroke` |
| **Shapes/views don't honor `HorizontalOptions` inside a stack** — investigated and DISPROVEN as a framework bug: `shape` inherits `view<>::arrange`→`compute_frame`, which already honors alignment. The real cause was a **sample bug** (`path_aspect_gallery` omitted the `HorizontalOptions=Start` the C# `<Style TargetType="Path">` sets). | ✅ **FIXED** (`ebc093ee23`, sample + 7 regression tests; `auto_size_shapes` already matched C#) | `path_aspect_gallery` |
| **CollectionView embedded in a layout overlaps siblings / renders full-bleed** — TWO bugs: (1) `get_desired_size` ported the obsolete `OnMeasure` screen-clamp instead of content-size (the **measure** half), and (2) `collection_view_handler::platform_arrange` never framed its native view — the only handler that didn't — so on iOS the `UICollectionViewController.collectionView` kept its full-screen frame + flexible autoresizing (the **arrange** half). | ✅ **FIXED** — measure (`61bdcc4595`) + arrange (`2cfa233be7`, per-backend `arrange_native` mirroring `border_handler`). All backends; **iOS-verified on the simulator** (`multiple_bound_selection` stacks cleanly). | `multiple_bound_selection`, `collectionview`, `data_template_selector`, `empty_view`, `empty_view_swap`, `filter_selection`, `header_footer_grid`, `items_updating_scroll_mode`, `preselected_items`, `scroll_mode_test`, `selection_synchronization`, `single_bound_selection`, `some_empty_groups`, `switch_grouping` (CV re-capture pending). `scroll_to_group` has a separate Grid-in-stack issue (next row). |
| **CollectionView grouping (group headers/footers + list header/footer) not rendered.** | ⬜ TODO | `basic_grouping`, `grid_grouping`, `grouping_plus_selection`, `header_footer_template` |
| **`flyout_page` detail-pane content not laid out on the Apple backends** (split chrome renders, detail subtree stays unsized → blank). | ⬜ TODO (confirmed HIGH) | `ios_scroll_view` |
| **Swipe rows oversized / labels detached / footer overlap** (may share the CV-cell sizing path). | ⬜ TODO — needs triage | `basic_swipe`, `ios_swipe_transition`, `swipe_gesture`, `swipe_item_size` |

### cpp-blank (C++ renders blank where MAUI shows content)
`ios_scroll_view` (flyout detail bug, above), `header_footer_view`, `empty_view_load_simulate`,
`empty_view_null` (empty-view CV variants — verify the empty-state actually renders), `radio_template_from_style`
(VSM/ControlTemplate page — also failed Mac Catalyst capture), `custom_layout` (known/documented: a
page-local custom layout type the gallery host can't resolve a handler for).

### cs-blank (the C# harness page rendered blank — comparison-app issue, not the port)
`controls_stack`, `effects`, `nested_collection`.

### Structurally identical (port matches MAUI)
`behaviors`, `borderless`, `formatted_text`, `gestures`, `line_gallery`, `line_join_gallery`,
`pan_gesture_events`, `path_gallery`, `pickers`, `pointer_gesture`, `polygon_gallery`,
`radio_content_properties`, `rectangle_gallery`, `relative_layout`.

## Not yet judged (rate-limited; follow-up pass)
`tabbed_flyout`, `transform_playground`, `swipe_threshold`, `templated_view`, `title_bar`,
`transformations`, `triggers`, `update_path_data`, `swipe_view_margin`, `varied_size_selector`,
`visual_states`, `toolbar`, `web_view`, and ~30 more.
