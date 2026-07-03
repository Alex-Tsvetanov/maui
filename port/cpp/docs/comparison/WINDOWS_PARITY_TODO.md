# Windows parity fix queue (Sonnet-reviewed)

## ⛔ BLOCKER (2026-07-03) — gallery renders EVERY page blank after a maui_core rebuild+install+relink
The cpp/xaml gallery now renders **all pages blank/white** (window mounts, measure+arrange run with
correct numbers, but NO control paints — confirmed on a real desktop screenshot, not a capture
artifact). Signature = **templateless controls**: a Button measures 19px (content only, no chrome).
- **NOT the button icon+text change** — reverted it (commit `9a5ebe77bf` reverts `46030899bc`) and the
  blank PERSISTS, and even a forced red Button.Background does not paint. Label/entry/activity_indicator
  are all blank too → global, not button-specific.
- **NOT resources.pri** — it + Bootstrap.dll next to gallery.exe are UNCHANGED (Nov mtimes); only
  gallery.exe/maui_core.lib changed. No resource/theme/exception logged at startup.
- **Root cause (traced):** the prior *working* gallery (which produced the committed captures) linked an
  OLDER installed `windows-prefix/lib/maui_core.lib`. This session's `cmake --install ... --prefix
  build/windows-prefix` refreshed that lib to the current build/windows one, which now includes the
  **uncommitted WIP Windows-handler expansion** (untracked: border/collection_view/flyout_page/image/
  indicator_view/navigation_page/refresh_view/scroll_view/swipe_item_menu_item/swipe_view/tabbed_page
  `_handler.cpp` + their modified `.hpp`). Relinking activated that code → global templateless render.
  Most likely a resource-dictionary corruption (the class of bug fixed earlier — resource_dictionary
  iterator-invalidation UB / XAML GradientStops-Spans UAF — possibly reintroduced by one WIP handler).
- **State is safe:** all working captures + `windows_pixel_scores.json` were RESTORED from git (the
  committed versions render correctly). Only the local gallery.exe binary is bad; committed source +
  captures are intact.
- **NEXT FIRE — restore a working gallery FIRST (prerequisite to all parity work):** bisect the WIP
  handlers — temporarily exclude them from the maui_core windows sources (or stash the untracked .cpp +
  checkout the .hpp), rebuild+install+relink, capture `label`; add them back one at a time until `label`
  goes blank. Prime suspects: any handler that touches the app ResourceDictionary or a XAML brush/shape
  resource (border/shape/swipe). Then fix that handler, THEN resume the button icon+text fix (its code is
  in the reverted commit `46030899bc` — cleanly reappliable once the gallery renders again).



Driven by `windows_pixel_scores.json` (objective near-match%) + `windows_sonnet_review.json`
(Sonnet vision verdicts). Reference render (`~/maui-compare` on Windows) is ground truth.
Update as items land. `pixel_score.py` re-scores; `capture_windows.py --mode both` re-captures
(animated pages get `.gif`).

## Fixed
- **controls_stack crash** — ProgressRing infinite-measure E_FAIL → default ring box + measure_native
  catch. 0% → 97%. (`bab819f1cb`)
- **image zero-height** — WinUI deferred-decode chicken-and-egg → natural-size measure off the
  BitmapImage + bounded post-Loaded settle re-layout. FileSource + gif render. (`4091676d8d`)
- **label BackgroundColor + VerticalTextAlignment** — wrap the TextBlock in a Border container (the
  WrapperView stand-in); update_background paints it, and the fixed-height slot makes vertical
  alignment work. cpp 42%→95%, xaml 39%→92%. (`8b63845b4e`) — REMAINING: FormattedString per-run
  background highlight (the "Colors" cyan box) needs a TextHighlighter; xaml renders spans blue.
- **flex measure content cross-size** — detail/flex.cpp stretched cross-align children to the parent
  cross during the MEASURE pass, so a nested Row of stretch children reported the full viewport height
  as its basis and the outer Column pushed the footer off-screen. Gated the stretch-keep with
  `!in_measure_mode` (stretch is an arrange concern). flex_layout 19% → **match×4** (pixel-identical
  all columns+themes); headless 3490/3490 green. Cross-platform correctness — likely helps other
  nested-flex pages (staggered_layout, custom_layout, chat_example…).

## Label-fix cascade (the Border-background fix reached these — cpp side largely resolved)
- **varied_size_selector** — cpp_light 42%→**99.1%** (the Coffee/Milk cell backgrounds are Label
  BackgroundColors); cpp_dark 92% (residual = the page's single-Label reduction lacks the MAUI
  template's rounded corners + inter-item gaps — a page-authoring choice, not a handler bug). xaml
  49%/42% is an xaml-gallery-page issue (the Picker shows placeholder not the bound "Latte").
- Expect the same cascade on other pages whose cells/regions are Labels with BackgroundColor — a full
  recapture + re-score is running to quantify.

- **Picker Title as Header** — mapped Title to ComboBox.Header (caption above the box) instead of
  in-box PlaceholderText, matching MAUI. picker 63%→98%, pickers →99-100% (both columns+themes).
  date_picker/time_picker (88-90%) use different WinUI controls — separate minor-styling items.

## Sonnet round-2 findings (2026-07-03, 16 mid-range pages judged — windows_sonnet_review2.json)
- **REFERENCE FIX (done)**: maui-compare bundled only dotnet_bot.png but pages reference oasis.jpg /
  cover1.jpg / coffee.png / settings.png — so MAUI showed PLAIN where those load, while the port
  (image fix) loads them → the port scored WORSE against a broken reference on ~35 image pages
  (header_footer_*, clip_gallery, nested_collection, image, empty_view_*, swipe_*, …). Copied the
  images into maui-compare/Resources/Images + rebuilt (maui-compare commit `0ba0678`); MAUI now loads
  them. A maui-column recapture of the 35 image pages is running to refresh their scores.
- **AA-only (NOT bugs — confirmed match/near-match by Sonnet)**: relative_layout (MATCH), layout_is_enabled,
  basic_swipe(cpp), button (only the CharacterSpacing row). Stop chasing these by pixel score.
- **Real port bugs surfaced (fix queue)**:
  - **header_footer_* empty CollectionView collapses** — with images now loading, the real bug shows:
    the empty list area collapses to 0 so the footer sits under the header; MAUI stretches the empty
    list so the footer is at the BOTTOM. + footer label should carry the page's rotation transform.
    root: collection_view (empty-area fill height) + label transform.
  - **border_stroke slider default** — the Content-Height Slider initializes to its min (40) instead
    of the page's Value=60; check the slider initial-value push order (min/max before value clamps it).
  - **border_playground** — Border BACKGROUND gradient (LinearGradientBrush) not applied (flat), and
    the yellow stroke's StrokeDashArray "1,1" dash pattern not applied (solid line). root: border_handler.
  - **hit_testing (xaml only)** — Scale/Rotation transforms not applied in the xaml gallery; cpp is
    MATCH. + IsClippedToBounds rectangle renders as outline not filled (xaml). xaml-gallery markup.
  - **image_button** — the animated-GIF ImageButton renders a solid black box (gif not decoded/played).

## High value (both cpp+xaml, confirmed real)
2. **[DONE — see above] flex_layout footer off-screen** — was: after the label fix the page
   is minor for light, but the pink FOOTER band is pushed below the 800px viewport. Root cause: the
   BODY (a nested Row FlexLayout with `Grow=1` inside the outer Column) measures its height as the FULL
   available 800px instead of its content max (24px). Chain: `flex_layout_manager::measure` calls the
   engine with a DEFINITE cross-axis; in `flex.cpp layout_item`, a stretch child with `align_dim > 0`
   (the definite available cross) IGNORES its measured natural cross and fills the available — so a
   row whose children are all cross-stretch reports the available height as its basis. The outer
   column then sees body_basis≈800, so header(24)+body(800)+footer(24) overflows and the footer lands
   at y≈824. Instrumented: labels themselves measure a correct 24px (HEADER 66×24, CONTENT 80×24,
   FOOTER 65×24) with wc=480 hc=800 — the inflation is the nested-flex measure, not the label.
   FIX (needs care + the headless flex tests green): during the MEASURE pass the container's cross-axis
   should be content-derived, not the available constraint — i.e. `align_dim` should come from the
   item's EXPLICIT width/height (NaN → no stretch) not from the passed measure constraint, matching
   Xamarin.Flex (stretch only against a DEFINITE cross, which only arrange supplies). Look at
   `flex::flex_layout::init` (how align_dim is set) + `layout_item` line ~557.
3. **flex_layout (other)** (19%) — FlexLayout bands don't fill/position: HEADER/CONTENT labels overlap top-left,
   the left/right colored side strips and the FOOTER band are missing, CONTENT gray fill missing. Root:
   flex layout arrange on the windows Canvas (nested-layout measure/arrange).
3. **CharacterSpacing** — not applied (button's `B u t t o n` row renders tight). Root: label/button
   character_spacing map (to_em push not taking, or wrong property).
4. **varied_size_selector / collection_view** (42%) — DataTemplateSelector per-item background colors
   not applied; selected-item indicator missing; per-template row heights uniform.
5. **picker** (63%) — MAUI shows the Picker Title as a caption line ABOVE the ComboBox; port shows it as
   in-box placeholder. Verify against MAUI's ComboBox Header usage.
6. **web_view** in context_flyout (49%) — embedded WebView blank (both). (web_view page itself ~93%.)

## XAML-gallery-specific (cpp is fine, xaml broken)
- **borderless**, **shape_app_theme** (0% xaml) — xaml gallery renders blank/split; dark theme not
  applied in the xaml host (shape_app_theme dark == light). Root: gallery_xaml theme/AppThemeBinding
  application + those pages' xaml. cpp column is at parity (100%).
- Many xaml-only diffs (search_bar 85, rectangle_gallery 82, transform_playground 73, hybrid_web_view
  30, path_gallery 84, radio_template_from_style 84…) — likely the same xaml-host theme/markup gaps.

## Geometry / rhythm (mostly minor per Sonnet — AA noise, not bugs)
- `button` etc. at ~61% near are judged **minor** (visually match) — low pixel score is text AA + a few
  px of accumulating stack drift, NOT a fix target unless a page reads as clearly off.

## Not yet judged (Sonnet pass was rate-limited on ~43 pages)
Re-run the Sonnet review on the un-judged keys (lower concurrency) to complete the categorization:
border_*, clip_*, radio_*, empty_view_*, header_footer_* (collection_view), staggered/relative layout,
date/time_picker, entry, alerts, etc.
