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

## High value (both cpp+xaml, confirmed real)
2. **flex_layout** (19%) — FlexLayout bands don't fill/position: HEADER/CONTENT labels overlap top-left,
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
