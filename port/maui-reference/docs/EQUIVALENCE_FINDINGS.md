# Structure-equivalence findings — the builder↔XAML alignment backlog

Produced by the 2026-07-05 full-corpus run of `tests/ui/gallery_structure_equivalence_tests.cpp`
(174 keys compared: **97 equivalent, 77 diverging**, 1 skipped-crash, 1 skipped-headless). Every
diverging key sits in the test's `known_diverging()` list, tracked **bidirectionally**: the gate stays
green while a listed key keeps diverging, and turns red the moment a divergence silently closes (or a
new one opens). **Aligning a page = fix the builder page or the shared `pages/<key>.xaml` (per the
cluster guidance below), delete the key's line from `known_diverging()`, and re-run
`tools/dev.sh gallery_structure_equivalence`.**

These are exactly the "unit tests catch what screenshots also show" first-line-defense findings: each
cluster below is a class of visible render divergence between the C++-only and C++&XAML columns.

## Cluster A — twin uses `StackLayout` where the builder uses `VerticalStackLayout`/`HorizontalStackLayout` (23 keys)

Different control type (often plus spacing/padding deltas). Alignment direction: usually fix the
SHARED XAML to the specific-orientation control the builder (and the original C# page) uses — plain
`StackLayout` is the legacy control with different default behavior.

`animation, alerts, app_theme_binding, basic_swipe, clip, clip_corner_radius, clip_gallery,
clip_views, composition_gallery, ellipse_gallery, empty_view_rtl, header_footer_grid,
header_footer_grid_horizontal, line_gallery, path_transform_string, pointer_gesture, polygon_gallery,
polyline_gallery, preselected_items, rectangle_gallery, selection_synchronization, shape_app_theme,
some_empty_groups`

## Cluster B — root-layout Padding/Spacing set on one side only (~25 keys)

E.g. `check_box.xaml` `Padding="16"` vs builder padding 0; `transform_playground` reversed (builder
12, xaml 0); `vertical_stack`/`horizontal_stack` xaml `Spacing="6"` vs builder 0. Alignment
direction: page-by-page — whichever side matches the MAUI reference capture wins.

`behaviors, border_layout, border_playground, border_resize_content, border_stroke, check_box,
data_template_selector, empty_view_selector, empty_view_swap, empty_view_template, empty_view_view,
filter_collection, filter_selection, focus, gestures, horizontal_stack, input_controls,
invalidate_brush, radio_button_border, radio_button_group_gallery, radio_template_from_style,
scattered_radio_button, search_bar, selection_command_param, switch_grouping, transform_playground,
vertical_stack`

## Cluster C — builder computes runtime state vs the twin's static snapshot (6 keys)

The shared XAML must carry the correct INITIAL state (pages render static-deterministically for
capture); the builder's post-mount/event-driven text is the run-time state. Alignment direction:
usually fix the SHARED XAML to the initial state (as done for `scroll_view` in the pilot), or remove
the builder's synthetic on-mount mutation where it exists purely for capture.

- `activity_indicator` — label wording deltas ("Styled - Color from theme" vs "Color", +2 more)
- `pickers` — builder "No room on 7/5/2026 at 09:00" vs xaml "No room on (no date) at (no time)"
- `web_view` — builder "No navigation yet" vs xaml "new_page -> https://demo.test/welcome"
- `pan_gesture_events` — builder label empty vs xaml "StatusType: Completed, TotalX: 12, TotalY: -8"
- `swipe_threshold` — builder "Ready" vs xaml "Reveal threshold=80 / Execute threshold=80"
- `radio_button_group` — page Title suffix "(Attached Property)" only in the builder

## Cluster D — twin structurally rewritten around unsupported features / loader gaps (20 keys)

Each is either a port XAML-feature gap (belongs in the P3 gap corpus with an
`expected_port_status`) or a twin approximation that should be re-authored now that the shared page
is canonical.

- `empty_view, empty_view_load_simulate, empty_view_null` — CollectionView.EmptyView unsupported → inlined label
- `carousel_page` — `<CarouselView>` hydrates as bare `view` (loader doesn't materialize it)
- `tabbed_flyout` — builder root `flyout_page`, twin is a ContentPage stand-in
- `hit_testing` — builder BoxView vs twin Rectangle stand-in
- `radio_button_content` — builder `view` where twin has `image`
- `adaptive_collection` — builder VSL vs twin Grid root
- `custom_layout` — builder custom layout vs twin Grid(3×5) approximation
- `chat_example` — builder bare collection_view vs twin grid+entry-bar composition
- `borderless` — builder VSL vs twin grid+border composition
- `header_footer_view` — collection_view header/footer not hydrated as children
- `hybrid_web_view` — builder HybridWebView vs twin border+label placeholder
- `ios_scroll_view` — twin has an extra scroll_view+VSL wrapper
- `layout_is_enabled` — twin has check_box rows the builder doesn't create
- `single_bound_selection` — twin has Reset/Clear buttons missing from the builder
- `indicator` — builder has an extra IndicatorView child the twin lacks
- `path_aspect_gallery, path_gallery` — twin wraps each path in an extra Grid (+ raw path-data label)
- `border_clip_playground` — builder VSL root vs twin Grid(2 rows)

## Skipped

- `templated_view` — SIGBUS in teardown of the hydrated templated_view/content_presenter graph
  (kills the test binary); excluded with a comment in the test file. Root-cause the teardown crash.
- `device` — builder ctor throws headless (device-info fake unseeded); explicit GTEST_SKIP.
- `brushes, selection_mode, shapes_demo` — builder-only keys with no shared page yet (P3 authoring).
