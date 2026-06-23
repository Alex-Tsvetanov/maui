# iOS parity fix plan — Gemini-flagged diffs, validated by Claude (2026-06-23)

Gemini's fresh sweep flagged **111 of 172 pages** with `port_diffs`. The user asked Claude to validate
those before committing to fixes. Validation method: read the 2×2 montages (`build/cmp/<key>.png`,
MAUI⇄C++ × light/dark) for a representative of **every** cluster (the ones marked `(*)` below were
examined pixel-by-pixel), pattern-match the rest against the validated representative + the documented
Gemini false-positive patterns, then confirm the root cause against the C# / maui-compare reference
source. Captures are commit `374019b805` — **post** the #143 CV-sizing + button/image fixes, so every
diff below is **live in the current build**, not stale.

## Validation scorecard (Gemini vs Claude)

Of 111 Gemini-flagged pages: **~44 are real port bugs**, **~59 are not port bugs**, **8 need a
source-check / user ruling**. Gemini is a good detector (~60% precision) but over-flags four predictable
ways — which is exactly why the Claude pass matters:

| Gemini claim type | Verdict | Examples (visually checked) |
|---|---|---|
| Port shows extra sections/items "missing from MAUI" | **harness bottom-crop** — MAUI is cropped, port is full-screen (ruling #2) | `border_playground` (*, claim reversed), `basic_grouping`, `transformations` |
| Readout text differs | **runtime state**, not render | `gestures`, `scroll_view`, `web_view`, `application_control` |
| Date/time value differs | **capture-day** artifact | `pickers` |
| MAUI blank / gray image | **MAUI ref broken**, port more faithful | `radio_button_group_binding`, `clip_gallery` |
| "Control completely missing" | sometimes **wrong** — port renders it fine | `swipe_gesture` (*, port renders the card; MAUI's is garbled), `chat_example` (*, bubbles match) |

Real-bug clusters were each confirmed: `slider`/`switch` (*) fonts+thumbs, `adaptive_collection`/`empty_view` (*)
CV cell sizing, `auto_size_shapes` (*) ellipse, `nested_collection` (*) horizontal-orientation+colors,
`polygon_gallery` (*) NonZero fill.

---

## FIX clusters (real port bugs) — ~44 pages, 13 root causes

### R1 — value-control section-header font *(7 pages, HIGH value, LOW risk)*
`activity_indicator, behaviors, check_box, progress_bar, slider, stepper, switch`
**Root cause (confirmed in source):** maui-compare pages define
`static Label Headline(text) => new() { FontSize = 18, FontAttributes = Bold }`, but the port's example
pages (`src/samples/pages/*_page.hpp`) build headline labels with `set_text(...)` only — no font. They
render at default ~17pt regular → smaller/lighter than MAUI's 18pt bold.
**Fix:** add a shared `make_headline()` (font_size 18 + bold) and apply to every section-header label in
the affected `*_page.hpp` files. Pure sample-page fidelity — no framework change.
**Verify:** re-capture each page; headers match weight+size.

### R3 — CollectionView cell sizing too tight + alignment ignored *(15 pages + 3 minor, HIGH value)*
`adaptive_collection (*), empty_view (*), empty_view_rtl, empty_view_swap, empty_view_template,
empty_view_view, filter_collection, filter_selection, items, items_updating_scroll_mode,
preselected_item, preselected_items, scroll_mode_test, staggered_layout, varied_size_selector`
+ minor: `basic_swipe, collectionview, ios_swipe_transition`
**Root cause:** port iOS CV cells shrink-wrap to content (one-line label → ~17pt pitch) and **left-align**,
where MAUI cells take the CV's full width, honor the template's `HorizontalOptions`/alignment, and are
taller (~64pt). #143 made cells self-size to content but did **not** restore full-width + template
alignment + min height. This is the residual.
**Fix:** in the iOS items cell host, give the cell the collection's cross-axis width and apply the item
template's horizontal/vertical options + measured height (don't shrink-wrap to the label). Oracle:
`ItemsViewLayout`/`TemplatedCell` sizing in `src/Controls/.../CollectionView/iOS`.
**Verify:** `adaptive_collection` items become centered + tall; item count per screen matches MAUI.
⚠️ Re-capture first to confirm exactly which of the 15 still differ after #143.

### R6 — shape layout sizing *(5 pages)*
`auto_size_shapes (*), composition_gallery, ellipse_gallery, line_gallery, shapes`
**Root cause:** proportional/`*`-height shape rows get the wrong measured height → `auto_size_shapes`
renders a **circle** where MAUI shows a **wide ellipse** ("occupy half the available space" gives the row
too much height); `ellipse_gallery`/shapes render smaller; line length/position off.
**Fix:** audit the shape rows' grid/star measure in these pages against MAUI; correct the proportional
height allocation. Oracle: the maui-compare `*ShapesPage`/`AutoSizeShapesPage` definitions.

### R7 — vector path fill-rule / dash / geometry *(4 pages)*
`polygon_gallery (*) — NonZero fill rule (star points unfilled, should be solid)`,
`polyline_gallery — dash pattern (too many segments / wrong spacing)`,
`path_gallery — missing EllipseGeometry + Overlapping-Rectangles; composite arc renders as full circle`,
`path_aspect_gallery — missing gray bg rects + aspect`.
**Root cause:** CoreGraphics path rendering — `even-odd` vs `non-zero` winding, dash phase, and
unhandled geometry types. Framework fix in the CG canvas/path layer. Oracle: `PathExtensions`/`Geometry`
+ `Microsoft.Maui.Graphics` winding handling.

### R2 — Background brush on native control *(2 + 2 minor)*
`date_picker, time_picker` — the "Background" gradient field renders **solid magenta** instead of the
blue→teal gradient. Same family as the `slider`/`switch` "Background" thumb being hidden (R1 list).
+ minor: `ios_slider_update_on_tap, ios_time_picker` (thumb/height styling).
**Root cause:** a `Background` gradient brush on a native control (a) falls back to a solid color on
picker, and (b) is drawn over/above the thumb on slider/switch (z-order), hiding it.
**Fix:** route `Background` brush through the gradient-layer path for picker/slider/switch handlers and
insert the brush layer **below** the thumb/sublayers. Oracle: `UpdateBackground` brush handling.

### R4 — horizontal CollectionView orientation not applied *(2 pages)*
`nested_collection (*), header_footer_grid_horizontal` — inner/horizontal CV renders **vertically**.
**Fix:** honor `ItemsLayout` horizontal orientation in the iOS CV layout. Oracle: `LinearItemsLayout`
orientation (cf. upstream #35445 already in tree).

### R5 — CV item-template text color not applied *(1 + part of R4)*
`cv_visual_states` (dark items), `nested_collection` (*) Source/Caption colors black/blue vs red.
**Fix:** push the template-bound `TextColor` to cell labels on bind/rebind.

### R8 — layout spacing / positioning *(2 + 4 minor + 1)*
`stack_layout` (no gaps where MAUI has spacing), `horizontal_stack` (6 squares vs 4 — re-check post-#139),
`device` (text top-left vs centered); minor: `absolute_layout, flex_layout, invalidate_brush, z_index`.
**Fix:** per-page; mostly StackLayout `Spacing` / alignment fidelity in the example pages or layout managers.

### R9 — border / clip rendering *(5 pages)*
`clip` (missing examples + scale), `border_clip_playground` (image fills where MAUI shows the clip shape),
`clip_views` (search-bar bg/radius), `border_layout` (slider track/thumb + stroke/radius), `containers`
(frame stroke/padding, minor). Per-page investigation against the maui-compare page source.

### R10 — entry styling *(2 pages)*
`entry` (height, vertical spacing, dark-mode border barely visible), `visual_states` (entry border/bg).
**Fix:** entry intrinsic height + border color (esp. dark mode) to match native default.

### R11 — title_bar layout *(1 page)*  — spacing/font/alignment/label truncation in the controls list.

### R12 — header/footer CV supplementary *(2 pages)*
`header_footer_grid, header_footer_template` — text weight/alignment (#138 partial) + container merging.
(Note: `header_footer_view` moved to DISCUSS — likely MAUI image-load failure.)

### R13 — indicator *(1 page)* — IndicatorView dot size too large + CarouselView pushed to bottom (large gap).

### Misc real *(6 pages)*
`button` (CornerRadius button sharp not rounded; pink button letter-spacing "B u t t o n"),
`radio_button_border` (container padding/height + inner-circle size — residual after #131),
`radio_button_group`/`scattered_radio_button` (label spacing, minor),
`multiple_bound_selection`/`selection_synchronization` (selection highlight color too light),
`search_bar` (4th bar italic text), `styles` (button padding/height, minor),
`relative_layout` (center rect aspect, minor).

---

## NOFIX — not port bugs (~33 pages) — do NOT chase

- **Harness bottom-crop (port shows more, MAUI cropped — ruling #2):** `basic_grouping, border_playground,
  dispatcher, grid_grouping, grouping_plus_selection, input_transparent, layout_is_enabled,
  measure_first_strategy, radio_button_group_gallery, radio_content_properties, rectangle_gallery,
  scroll_to_group, swipe_item_size, swipe_threshold, swipe_view_margin, switch_grouping,
  transform_playground, transformations` (18).
- **Runtime-state readouts (text differs by live state):** `application_control, drag_drop, gestures,
  hit_testing, scroll_view, web_view` (6).
- **Capture-day date value:** `pickers` (1).
- **MAUI ref blank/broken, port more faithful:** `radio_button_group_binding, radio_template_from_style,
  clip_gallery` (3).
- **Live / native-pattern / nav-chrome (non-matchable):** `context_flyout` (live Bing WebView),
  `tabbed_flyout` (native flyout), `ios_scroll_view` (nav back-button), `carousel_page` (sample nav
  buttons), `animation` (motion — needs GIF) (5).
- **Gemini over-flag (port matches or is better — visually confirmed):** `chat_example` (*, bubbles match),
  `swipe_gesture` (*, port renders the card; MAUI garbled) (2).

## DISCUSS — verify sample source / user ruling (8 pages)

Mostly suspected **MAUI image-load failures** where the port renders the real image and MAUI shows a
blank/placeholder — if confirmed, the port is *more* faithful (NOFIX, like the oasis.jpg precedent). Needs
a source check of whether the sample sets an image or solid background, then a ruling:
- `header_footer_view` (*) — header/footer show dog/sky images in port, faint beige in MAUI.
- `border_resize_content`, `clip_corner_radius` — images vs solid/placeholder.
- `ios_blur_effect` — image at top absent in MAUI.
- `templated_view` — Gemini says port renders **fewer** cards (contradicts Claude's "match"); re-check.
- `hybrid_web_view` — labels truncated with ellipses vs full text (possible real label-width bug).
- `clipping`, `radio_button_content` — coffee-cup icons / custom-template icons (residual after #137/#140?).

---

## Execution recommendation

1. **Re-capture first** (`tools/parity/capture_all_cpp.py` after a fresh build+install) — confirms which
   R3/R8 pages still differ post-#143/#139, so no fix chases an already-closed diff.
2. **Order by value/risk:** R1 (sample-page font, trivial) → R3 (CV cell sizing, highest page count) →
   R2 (Background brush) → R4/R5 (CV orientation/color) → R6/R7 (shapes/paths, framework) → the per-page
   tail (R8–R13 + misc).
3. **Resolve DISCUSS** with the user before touching those 8 (ruling #3).
4. Each fix follows the loop: derive from the maui-compare/C# oracle → implement → `tools/dev.sh` targeted
   tests → rebuild+**reinstall**+re-capture the affected page(s) → confirm light+dark on-sim → update the
   board honestly. Full gate (`tools/gate.sh`) + clang-tidy 0 before push.

Board note: the prior Claude pass over-classified many of these as match/minor; the board should be
re-stamped to honest severities **as each fix lands and is re-verified on-sim** (not pre-emptively).

---

## Execution log (2026-06-23)

- **R1 (section fonts) — DONE+VERIFIED, commit `fa080950e5`.** slider/switch/activity_indicator/
  progress_bar/check_box/stepper headers now bold 18pt. Verified on-sim (switch headers bold).
- **R8 device (centering) — DONE+VERIFIED, commit `6461a3c574`.** Stack now horizontally centered.
- **R2 (Background brush) — DONE+VERIFIED, commit `d80d559930`.** Gradient/image sublayer moved below the
  thumb (zPosition −1 + index 0, ports C# InsertBackgroundLayer); date/time pickers drop the RoundedRect
  bezel for the gradient case so it fills. Switch thumb visible; picker gradients fill. (Salvaged from the
  U-BRUSH agent's WIP after it died on an infra timeout; picker half completed by coordinator.)
- **R3/R4/R5 (CV) — DONE+VERIFIED, commit `9e5a3835ab`** (from the U-CV agent). KEY FINDING: the iOS cell
  self-sizing was ALREADY correct post-#143. R3 = adaptive_collection's *page* had dropped its template
  chrome (HeightRequest=60, centered) — restored → centered 60pt rows. R4/R5 = new reflection-free
  `data_template::add_setup` hook pushes the inner CV's horizontal orientation + a red-italic source-title
  template. Verified on-sim (adaptive centered+tall; nested horizontal + red titles).
  → **IMPLICATION for the rest of R3:** the other ~13 "tight CV" pages are NOT one framework root cause;
  each needs an individual check — likely a mix of Gemini over-flags (MAUI also tight = harness crop) and
  per-page dropped chrome. Do NOT blanket-fix.
- **R7 (paths) — DONE+VERIFIED, commits `22fc59ea60`/`f52e85d810`/`d8179fc40b`/`2fbef928f6`** (U-PATHS
  retry; the first agent died leaving nothing, the retry's commit-early brief produced 4 clean commits).
  R7a NonZero fill → CG winding op (star fills solid); R7c Path defaults EvenOdd + reads GeometryGroup
  FillRule (composite renders alternating rings, not a disc); R7d Background+Fill paints the shape host
  (path_aspect gray rects back). **R7b dash = HONEST NO-BUG finding** — pipeline already faithful; the
  "extra segments" was harness crop (rule #2), so added regression coverage, no code change. Verified
  on-sim: polygon NonZero solid; path_gallery composite rings + Overlapping-Rectangles + EllipseGeometry
  all render; path_aspect gray rects present.
- **Agent infra note (2026-06-23):** the "Stream idle timeout" error killed 2 of 3 framework agents at
  ~13–15 min. Worktree commits survive; uncommitted WIP is salvageable via `git diff > patch`; a clean
  death leaves nothing. Lesson baked into re-spawn briefs: commit each sub-fix immediately.
- **Validation corrections found DURING fixing (oracle + on-sim re-check overturned the text triage):**
  - `stack_layout` → **NOFIX (over-flag).** Oracle: the public `StackLayout` derives from `StackBase`
    with `Spacing` default **0**, matching the port; the montage shows boxes touching in BOTH. Gemini's
    "MAUI has a gap" is wrong — adding spacing would have *introduced* a diff.
  - `horizontal_stack` → **NOFIX (over-flag).** `HorizontalStackPage.cs` has **6** boxes (Red→Purple) —
    the port's 6 is correct; MAUI's last two are simply off-screen. Gemini's "6 vs 4" is misleading.
  - Lesson: Gemini's vague "spacing tighter/looser / N vs M items" flags are low-precision; each must be
    oracle-checked + visually verified before fixing. Remaining R8-minor pages (absolute_layout, z_index,
    flex_layout, invalidate_brush, relative_layout) are likely over-flags too — verify before touching.
- **R6 (shapes) → ALL 5 NOFIX (over-flag/harness), verified by on-sim montage review.** `auto_size_shapes`
  (port layout mirrors the C# Grid Auto/*/* exactly; the ellipse renders circle-not-oval ONLY because the
  port has more vertical space than MAUI's harness card, so "half the available space" is taller — the
  ellipse correctly occupies half its (larger) row; harness, ruling #2), `ellipse_gallery` (shapes are the
  same size in both — over-flag), `composition_gallery` (shape composition same size; the card/lines are
  star-sized so the port's extra height makes them taller/longer — harness, NOT "scaled-down shapes"),
  `shapes` (ellipse/round-rect/pentagram all match; the "Line" is cropped in MAUI = unverifiable),
  `line_gallery`/`relative_layout` same star-available-space pattern. **The real-fix count keeps shrinking
  below the original ~44 estimate as validation proceeds.** Likely-real remaining = CONTROL-STYLING tail
  (entry dark border, title_bar truncation/font, indicator dot size, button corner/letter-spacing, radio
  padding, selection highlight color, search_bar italic, header_footer image-vs-solid), NOT the
  layout/spacing/count flags.
- **R13 indicator — DONE+VERIFIED, commit `1a0dc160e1`.** Dot-size scale base 6→7 (C#
  `IndicatorViewRenderer.DefaultIndicatorSize == 7`); the port over-scaled (15 → 2.5× vs 2.14×). Dots
  shrink toward MAUI on-sim; residual gap is the modern-vs-legacy handler nuance (minor). The CarouselView
  "Item 1" bottom gap is the star-available-space harness effect (R6 family), not fixed.
- **R10 entry → NOFIX (over-flag).** Fields match MAUI in height, spacing, magenta text, rounded borders
  (light + dark); the port just shows more below the fold (harness crop). `visual_states` entry is the
  same rendering (its "missing card bg" = harness wrapper).
- **button "B u t t o n" letter-spacing → DISCUSS (MAUI mapper-order quirk), per ruling #3.** Both set
  CharacterSpacing=20. MAUI's `UpdateCharacterSpacing` reads `TitleLabel.AttributedText?` — null on initial
  render because MapText (mapper order 33) runs AFTER MapCharacterSpacing (order 30) → no-op → plain
  "Button", no spacing. The port applies the kerning correctly ("B u t t o n"). Replicating MAUI's bug to
  match it is a user call — flagged, not auto-fixed. (CornerRadius sharp-vs-rounded same page: marginal.)
- **Selection-highlight color — DONE+VERIFIED, commit `99e50bcd39`.** CV `selectedBackgroundView` was
  `systemGray4` (too light); MAUI's `ItemsViewCell` default is `ColorExtensions.Gray == UIColor.systemGray`
  (medium). Fixed → matches on-sim. Covers `multiple_bound_selection` + `selection_synchronization`.
- **search_bar → NOFIX (over-flag).** Both port + oracle set Text="Italic 24pt" identically; MAUI's
  "Italic 24" is just the narrower harness search bar cropping the "pt". Bars otherwise match.
- **title_bar → NOFIX (over-flag).** The 2-column Content/Color Options form (radios, entries, headers,
  links, live readout) matches MAUI; "Set Col…/Set For…" truncates in BOTH (narrow column).
- **radio_button_border → REAL-MINOR, DEFERRED (blast radius).** The port's radio rows are more compact
  than MAUI's: MAUI's `RadioButton.BuildDefaultTemplate` wraps the content in a Border with `Padding=6`
  (+ a Grid `Padding=2`, 21pt outer / 11pt inner circle); the port's default template lacks that padding.
  Adding it would touch EVERY radio page (radio_button_group/scattered_radio_button/etc.) — some may
  currently match — so it needs the RadioButton default-template padding set + cross-page on-sim
  verification, not a blind change. Flagged for a focused follow-up.
- **Convergence:** genuine-fix count far below the original ~44. REAL fixes DONE: R1, R2, R3/R4/R5, R7,
  R13, device, selection-highlight. Still to check: header_footer (R12), clip/clip_views/border_clip,
  cv_visual_states (verify agent R5 covered it), nested caption color. Then the DISCUSS source-checks +
  final gate (`tools/gate.sh` + clang-tidy 0 + sanitizers) + push.
