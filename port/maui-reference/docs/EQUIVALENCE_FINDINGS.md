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

> 2026-07-06 — the CollectionView **EmptyView family is CLOSED** (10 keys de-listed): the loader now
> registers `EmptyView` (string attribute, property-element text, and element/view forms — see
> `src/xaml/register_xaml_items.cpp`), the shared pages were re-authored to the real EmptyView the
> original C# galleries use (no more always-visible sibling-label hacks), and the builder twins were
> aligned (root Padding 12 / RowSpacing 6, Margin-6 cells, the twins' static item counts;
> `empty_view_rtl`'s header row is now a plain `stack_layout`). De-listed: `empty_view`,
> `empty_view_load_simulate`, `empty_view_null`, `empty_view_rtl`, `empty_view_selector`,
> `empty_view_swap`, `empty_view_template`, `empty_view_view`, `filter_collection`,
> `filter_selection`.

> 2026-07-06 — the `Image.Clip` / `View.Clip` XAML gap is **CLOSED**: the loader now registers
> `Clip` (every `view<>`-derived control, `register_xaml_helpers.hpp`) and the geometry element types
> `RectangleGeometry` / `EllipseGeometry` / `GeometryGroup` / `RoundRectangleGeometry` / `PathGeometry`
> (`src/xaml/register_xaml_geometries.cpp`) — this required giving the port's `geometry` base a
> `bindable_object` base (matching C#'s `Geometry : BindableObject, IGeometry`; see `geometry.hpp`),
> since `xaml_type_registry::register_type<T>` requires one. The four affected shared pages were
> re-authored with the REAL `<Image.Clip>`/`Clip=` markup from the original C# sources (`clip.xaml`,
> `clip_gallery.xaml`, `clip_views.xaml`, `clip_corner_radius.xaml`) and their root `StackLayout` was
> aligned to `VerticalStackLayout` (cluster A) to match the builder pages in the same change. De-listed:
> `clip_corner_radius`, `clip_gallery`, `clip_views`. **`clip` stays listed** — cluster A closed for it
> too, but its builder page separately appends a gallery-convention "Toggle clip on/off" Button + status
> Label the twin correctly omits (AUTHORING.md rule 3); see cluster E.

> 2026-07-06 — **`app_theme_binding` CLOSED** (was cluster A): the twin's root `StackLayout` is now
> `VerticalStackLayout`, and the builder page was re-aligned to the ORIGINAL AppThemeBindingPage.xaml —
> its invented "Toggle theme" button + "Theme: …" readout were removed, the headlines carry the twin's
> inline Headline stand-in (FontSize 24 Bold), and the page now binds the HOSTING app's theme via the
> `on_mounted` hook (it had owned a PRIVATE application seeded Light, which is why the cpp column never
> went dark). The xaml column's dark-theme fix is separate: the generated `Views/*.xaml` factories now
> thread `xaml_load_options{.application = …}` so `{AppThemeBinding}` resolves against the app's
> current theme and re-applies on RequestedThemeChanged (see `e2e.py` gen templates + gallery_xaml
> main.cpp).
>
> 2026-07-06 — **`data_template_selector` CLOSED** (was cluster B): builder grid now carries the twin's
> Padding 12 / RowSpacing 6, its source is the twin's 14 static rows (the C# 200-row
> DemoFilteredItemSource is a scrolling near-duplicate list the static board can't compare), and its
> AT-REST ItemTemplate is the twin's single plain Margin-6 bound label. **P3 gap-corpus item:** the
> twin cannot express a `DataTemplateSelector` (no reflection to activate the C# selector classes) —
> the builder keeps its `day_selector`/WeekendSelector machinery in code, deliberately UNWIRED at
> rest; a future `gap_data_template_selector.xaml` should pin the loader gap. The SAME policy applies
> to `varied_size_selector` (its 3-way `drink_selector` stays unwired at rest; the at-rest cell is the
> twin's uniform Wheat/100pt/Padding-8 template and the picker preselect is dropped) — that key was
> already structurally equivalent, so only its render changed.

## Cluster A — twin uses `StackLayout` where the builder uses `VerticalStackLayout`/`HorizontalStackLayout` (22 keys)

Different control type (often plus spacing/padding deltas). Alignment direction: usually fix the
SHARED XAML to the specific-orientation control the builder (and the original C# page) uses — plain
`StackLayout` is the legacy control with different default behavior.

`animation, alerts, basic_swipe, composition_gallery, ellipse_gallery,
header_footer_grid, header_footer_grid_horizontal, line_gallery, path_transform_string,
pointer_gesture, polygon_gallery, polyline_gallery, preselected_items, rectangle_gallery,
selection_synchronization, shape_app_theme, some_empty_groups`

(`app_theme_binding` closed 2026-07-06 — see the note above.)

(`clip`, `clip_corner_radius`, `clip_gallery`, `clip_views` were also cluster-A members — twin
`StackLayout` vs builder `VerticalStackLayout` — all four fixed to `VerticalStackLayout` alongside the
2026-07-06 `Image.Clip` fix above; `clip_corner_radius`/`clip_gallery`/`clip_views` fully closed and
de-listed, `clip` moved to cluster E for its remaining divergence.)

## Cluster B — root-layout Padding/Spacing set on one side only (~19 keys)

E.g. `check_box.xaml` `Padding="16"` vs builder padding 0; `transform_playground` reversed (builder
12, xaml 0); `vertical_stack`/`horizontal_stack` xaml `Spacing="6"` vs builder 0. Alignment
direction: page-by-page — whichever side matches the MAUI reference capture wins.

`behaviors, border_layout, border_playground, border_resize_content, border_stroke, check_box,
focus, gestures, horizontal_stack, input_controls,
invalidate_brush, radio_button_border, radio_button_group_gallery, radio_template_from_style,
scattered_radio_button, search_bar, selection_command_param, switch_grouping, transform_playground,
vertical_stack`

(`data_template_selector` closed 2026-07-06 — see the note above.)

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

## Cluster D — twin structurally rewritten around unsupported features / loader gaps (16 keys)

Each is either a port XAML-feature gap (belongs in the P3 gap corpus with an
`expected_port_status`) or a twin approximation that should be re-authored now that the shared page
is canonical.

(`path_aspect_gallery` closed 2026-07-06 — the builder already wraps each cell's shape in a Grid
matching the twin's structure; the divergence flagged by the visual parity sweep was in the shape
*geometry content* (heart/ECG shapes vs. the octagon MAUI/twin actually declare per cell), which
`gallery_structure_equivalence` cannot see — shapes are opaque `view` leaves by describe()'s
conservative-props policy. Fixed by aligning the builder's per-cell Path.Data to the twin's declared
geometry; see `port/cpp/examples/gallery/pages/path_aspect_gallery_page.hpp`.)

- `carousel_page` — `<CarouselView>` hydrates as bare `view` (loader doesn't materialize it)
- `tabbed_flyout` — builder root `flyout_page`, twin is a ContentPage stand-in
- `hit_testing` — builder BoxView vs twin Rectangle stand-in
- `radio_button_content` — builder `view` where twin has `image`
- `adaptive_collection` — builder VSL vs twin Grid root
- `custom_layout` — builder custom layout vs twin Grid(3×5) approximation
- `chat_example` — 2026-07-06: builder now matches the twin's at-rest content (three header buttons,
  empty message list — no more seeded bubbles), see `chat_example_page.hpp`'s header PORT NOTE; the
  KEY still diverges structurally because `describe()` treats `collection_view` as an opaque leaf (its
  ItemTemplate/selector content isn't structurally comparable) while other minor container-prop
  differences remain — not a content bug, just outside describe()'s conservative-props policy
- `borderless` — builder VSL vs twin grid+border composition
- `header_footer_view` — collection_view header/footer not hydrated as children
- `hybrid_web_view` — builder HybridWebView vs twin border+label placeholder
- `ios_scroll_view` — twin has an extra scroll_view+VSL wrapper
- `layout_is_enabled` — twin has check_box rows the builder doesn't create
- `indicator` — builder has an extra IndicatorView child the twin lacks
- `path_gallery` — twin wraps each path in an extra Grid (+ raw path-data label)
- `border_clip_playground` — builder VSL root vs twin Grid(2 rows)

## Cluster E — builder ADDS a gallery-convention interactivity widget the twin correctly omits (1 key)

AUTHORING.md rule 3 forbids event attributes in the shared XAML, so a builder page that adds its own
observable interactivity (a button + readout label with no C# counterpart) permanently diverges from
its twin — this is not a bug to fix, just a structural fact to keep tracking.

- `clip` — builder appends a "Toggle clip on/off" Button + status Label after the five images
  (`clip_page.hpp`); the twin's five images are otherwise structurally identical to the builder
  (including the real `Image.Clip` markup, 2026-07-06).

## Skipped

- `templated_view` — SIGBUS in teardown of the hydrated templated_view/content_presenter graph
  (kills the test binary); excluded with a comment in the test file. Root-cause the teardown crash.
- `device` — builder ctor throws headless (device-info fake unseeded); explicit GTEST_SKIP.
- `brushes, selection_mode, shapes_demo` — builder-only keys with no shared page yet (P3 authoring).

## Flagged MAUI-side quirks (2026-07-06) — user ruling needed, per port/CLAUDE.md ruling 3

Real MAUI's Mac Catalyst render itself is the mismatch here, not the shared twin or the builder —
verified by direct pixel inspection of the MAUI reference capture and, where noted, by reading the
real C# source to confirm the port's behavior is byte-faithful.

- **`button`** — the two `Button.ImageSource="settings.png"` rows: the port loads the image and
  correctly grows the button to the image's full native size when given an unconstrained height
  (`VerticalStackLayoutManager.Measure` passes `double.PositiveInfinity` for height to every stack
  child in BOTH the real C# source and the port — verified line-for-line against
  `src/Core/src/Layouts/VerticalStackLayoutManager.cs` and
  `src/Controls/src/Core/Button/Button.iOS.cs`'s `ResizeImageIfNecessary`). Real MAUI's own capture
  shows NO icon at all for these rows — a compact, normal-height, text-only bar — meaning the image
  fails to load/apply on real MAUI's Mac Catalyst for this asset, so its `CrossPlatformMeasure` never
  enters the image-sizing path at all. The port's image loading is arguably MORE correct (it succeeds),
  not less; matching MAUI's failure would mean deliberately breaking working image loading.
- **`empty_view_view` / `empty_view_rtl` / `filter_collection`** — MAUI's Catalyst `EmptyView` renders
  as an always-visible overlay atop a *populated* `CollectionView`, contradicting the C# source
  (`ItemsViewController2.cs` gates visibility on `ItemsSource.ItemCount == 0`).
- **9-key grouped-`CollectionView` cluster** (`basic_grouping`, `grid_grouping`,
  `grouping_no_templates`, `grouping_plus_selection`, `measure_first_strategy`, `nested_collection`,
  `scroll_to_group`, `some_empty_groups`, `switch_grouping`) — MAUI's own render shows an empty or
  near-empty grouped list while the C++ builder renders a full populated one. Likely because grouped
  collections can't be expressed in the loader's static `x:Array` XAML (no `IsGrouped`/
  `GroupDisplayBinding`-carrying source), so MAUI's blank render may be a twin-authoring ceiling rather
  than a genuine MAUI bug — not yet distinguished.
- **`carousel_page`** — MAUI's reference capture is genuinely blank (post the 2026-07-06 tint-fix
  recapture, not a stale/inactive-window artifact) — a likely real `CarouselView`-hosting failure in
  `MauiReference`/`PageDispatch`, not yet root-caused.
