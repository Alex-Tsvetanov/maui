# AppKit (native macOS) parity findings — 2026-06-29

The AppKit backend (`MAUI_BACKEND=apple`, native NSViews) built and runs all 59 gallery pages
(`examples/build-apple`, plain-exe galleries, centered 480×720 NSWindow). Captured both columns
(`appkit_cpp`, `appkit_xaml`) under `docs/comparison/maccatalyst/appkit_{cpp,xaml}/light/` +
side-by-side montages under `montages_appkit/light/`, judged by 8 parallel agents.

**Parity criterion (per user):** AppKit can't pixel-match MAUI/Catalyst (different UI framework — NSViews
vs UIKit); the requirements are (1) every element specified in the code/XAML is PRESENT in the render, and
(2) the C++ builder and C++ & XAML columns do not differ from each other.

## CHECK 1 — appkit_cpp vs appkit_xaml (must match)

**PASS** on all pages except one: **radio_template_from_style** — the builder (`appkit_cpp`) renders the
templated RadioButton as a SOLID BLUE box (radio glyph missing), while `appkit_xaml` renders the correct
gray box + blue radio circle. A real builder-vs-XAML divergence in the AppKit ControlTemplate path. The
6 `NON_BUILDER` keys have no `appkit_cpp` (builder gallery lacks them) → xaml-only, expected.
All other `minor` flags are runtime-state only (selected index/date/time values), not structural.

## CHECK 2 — completeness (every coded/XAML element present)

**Most pages complete** (label, controls_stack, formatted_text, border/border_stroke/border_layout/
border_alignment/border_playground, content_view, items, alignment, custom_layout, z_index,
radio_button_group(_binding/_gallery), radio_content_properties, radio_button_border,
scattered_radio_button, check_box, entry, picker, stepper, date_picker, pickers, slider, progress_bar,
activity_indicator, application_control, collectionview, scroll_view, styles, …) — native look differs
(NSButton/NSStepper/NSSwitch styling, fonts, spacing, OS colors) but the elements are all present.

### SYSTEMIC BUG — colored/sized children render as a single solid fill (~12 pages)

Pages whose content is **BoxView / colored boxes / positioned-layout children** render as ONE solid-color
fill (the last/top child covering the window) or drop children entirely — IDENTICALLY in both `appkit_cpp`
and `appkit_xaml` (so it is an **AppKit-backend layout/shape bug, NOT a cpp-vs-xaml divergence**):

| page | symptom |
|---|---|
| vertical_stack_layout | solid purple (6 `BoxView` 40×40 squares + label all gone) |
| stack_layout | solid purple (colored squares + 2 headers gone) |
| grid | solid orange (2×2 colored cells + title gone) |
| grid_definitions | solid purple (header/cells/footer gone) |
| box_view | solid pink (all 5 BoxViews + labels gone) |
| relative_layout | blank/black (corner squares + boxes gone) |
| flex_layout | solid green (HEADER/CONTENT/left strip gone) |
| absolute_layout | only the AutoSized label (4 positioned rects + centered text gone) |
| radio_button_content | solid red (entire page content gone) |
| templated_view | oversized gray box fills viewport (cards/intro gone) |

**Narrowed:** the squares are `BoxView` → `shape_view_handler.mm`. Shapes INSIDE a Border render fine
(border_resize_content complete), but standalone `BoxView`s in a stack/grid/etc. fill the window. The apple
`shape_view_handler::arrange_native` (shape_view_handler.mm:148) DOES `setFrame:` the arranged rect, and
measure is cross-platform (same as Catalyst, where these pages render correctly) — so the wrong frame is
reaching the shape, or a coordinate/double-offset/host-resize issue stretches it. Needs frame-level
instrumentation on the apple arrange path to root-cause (a candidate: the apple host runs layout once at
boot then `setContentSize`/`center`, with no resize re-layout — same class as the Catalyst host-resize gap).

### Other control-specific AppKit gaps
- **switch** — labels render but the NSSwitch toggles are missing under each row.
- **image** — text labels render but the actual images (UriSource/FileSource) don't.
- **editor** — the FontSize(Large) editor expands to full height, pushing trailing rows below the fold.

## Status
AppKit is **built + tested**; the dominant remaining work is the systemic shape/layout-fill bug above
(fixes ~10 pages at once) plus the switch/image gaps and the one builder ControlTemplate divergence.
Deferred for a focused, instrumented fix (risky to fix blind; the look-difference vs MAUI is by design).

---

## UPDATE 2026-06-29 — systemic shape-fill bug FIXED

Root-caused + fixed the "colored/sized children render as a single solid fill" bug (~10 pages).

**Root cause:** `src/platform/apple/graphics_host.mm` `drawRect:` passed the OS `dirtyRect` to the shape
drawable as the draw rect. A shape drawable FILLS the rect it is given, and on a flipped/nested NSView the
OS `dirtyRect` is the whole window expressed in the view's coordinate space (probe showed
`dirty={-220,-296,480,752}` while `bounds={0,0,40,40}` for a 40×40 BoxView) — so every BoxView/shape
painted across the entire window.

**Fix:** draw over `self.bounds` (the shape's real geometry) instead of `dirtyRect` — matches MAUI (whose
`IDrawable.Draw` receives the view bounds); the context is already clipped to the invalidated region, so it
is correct and not wasteful. One-line change in `drawRect:`.

**Verified (re-captured, AppKit):** vertical_stack_layout (6 squares), box_view (all boxes), grid,
stack_layout, grid_definitions, relative_layout, flex_layout, absolute_layout, radio_button_content,
templated_view all render their children correctly now; border_resize_content (shapes inside Borders, which
worked before) is unchanged — no regression. appkit_cpp still == appkit_xaml on these.

**Switch FIXED** (commit follows): the NeedsContainer arrange framed the inner NSSwitch, not the
wrapper the layout positioned — now `platform_arrange` frames the container + fills it. All toggles render.

**Still open (separate, smaller gaps):** `image` doesn't show the actual
images; `radio_template_from_style` builder ControlTemplate renders a solid-blue box (cpp-vs-xaml). These
are NOT the shape-fill bug and are deferred.
