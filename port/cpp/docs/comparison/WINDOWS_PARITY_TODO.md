# Windows parity fix queue (Sonnet-reviewed)

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
