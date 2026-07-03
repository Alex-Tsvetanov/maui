# Windows parity fix queue (Sonnet-reviewed)

## ✅ RESOLVED (2026-07-03) — runtime fixed by user; loop RESUMED
The user repaired the environment; the gallery paints again (verified: `label` + `button` render fully).
The blank was 100% the overnight WinAppSDK runtime regression — NOT port code.

**Button icon+text composition: VALIDATED + shipped.** Reapplied the reverted fix (`6aa7284240`) and it
renders correctly now: gear icon + "settings" text compose, CharacterSpacing lands ("B u t t o n").
Then capped the icon to natural size (ImageOpened → MaxWidth/Height = decoded PixelWidth/Height; Uniform
scales DOWN only) so the 128px gear no longer upscales to fill the 456px button. button 18.7%→**76.7%**
(light) / 41.0%→**74.0%** (dark); image_button ~**80%**. Two natural-size gear+"settings" buttons now
match MAUI's structure. (`0716…` image-cap commit.)

**IN FLIGHT:** a detached FULL recapture (all pages, cpp+xaml, both themes, PNG + animated GIF) is running
to refresh every stale (pre-outage) capture against the restored gallery, then a full rescore. Next fire:
when `$env:TEMP/full_recap_done.marker` exists → commit fresh captures+scores, then run the Sonnet vision
judge (Workflow) across all pages for cpp-vs-maui AND xaml-vs-maui, and work the confirmed real-bug queue
(button image-box height, border_playground gradient/dash, header_footer empty-collapse, …).

## (historical) ⏸ LOOP PAUSED (2026-07-03) — user was fixing the runtime

**Every fire until then — cheap gate (do this FIRST, ~5s, then exit if still blank):**
1. `python tools/parity/capture_windows.py label --framework cpp --theme light --mode static`
2. Read `captures/windows/cpp/label_light.png`. If BLANK → runtime still broken, report "still paused,
   awaiting your runtime fix" and exit (do NOT re-investigate; do NOT restore the blank capture is fine —
   but prefer `git checkout -- <that file>` to keep the committed working capture pristine).
3. If it RENDERS (label text visible) → the environment is fixed. RESUME:
   a. Reapply the button icon+text fix: `git revert --no-edit 9a5ebe77bf` (un-reverts `46030899bc`), rebuild
      maui_core + install + relink gallery, recapture `button`/`image_button`, rescore, verify it renders +
      the gear/CharacterSpacing land (see below).
   b. Continue the real-bug queue: border_playground gradient/dash, header_footer empty-collapse, etc.

## ⛔ BLOCKER (2026-07-03, REFINED) — environment/runtime regression: gallery paints NOTHING
**Confirmed it is an ENVIRONMENT change, not port code.** Evidence chain:
- Port CODE at HEAD == the last-known-good `2a65c88f0c`/`aec4afe43c` (only docs + a reverted button
  commit since) — the code that SHIPPED yesterday's working captures (last good capture 2026-07-02 20:34).
- The gallery bootstraps OK, `mux::Application::Start` runs, the window mounts, measure+arrange log real
  numbers — but NOTHING paints (even the `label` page's raw TextBlock text is blank on a real desktop
  screenshot, not just a capture artifact). "Nothing paints, not even text" ⇒ NOT a template/style issue.
- **resources.pri mismatch RULED OUT:** the loaded runtime is `Microsoft.WindowsAppRuntime.1.8_8000.879.2017.0`
  (bootstrap picks the highest installed 1.8; also present: 859.21.0). Deployed that exact runtime's
  resources.pri (1351648 B) beside gallery.exe replacing the stale Nov one (1349064 B) → STILL blank. So a
  matched pri+runtime still fails.
- The machine now has **two 1.8 runtimes** (859.21.0 and 879.2017.0); 879 looks like an overnight Store/
  Windows update. The SAME app binary + code that rendered yesterday renders blank today ⇒ the WinAppSDK
  **runtime 879.2017.0 broke unpackaged-app rendering** (or a half-applied update left it inconsistent).
- **Can't cleanly force the older 859 from code:** MddBootstrapInitialize2 takes a minVersion floor and
  picks the HIGHEST match, so it can't exclude the newer 879 via the bootstrap API alone.
- **Fix is environment-level (user domain):** repair/complete the pending Windows/Store updates + reboot,
  or remove/repair the 879.2017.0 runtime so the app resolves the working 859, or install the runtime that
  matches the app's SDK nuget (1.8.251104000) and pin the bootstrap to it. Port code needs no change.
- State is safe: all working captures + scores are the committed versions (git). Button icon+text fix is
  intact in reverted commit `46030899bc`, reappliable once the gallery renders again.

## (superseded) earlier BLOCKER note — gallery blank after rebuild+install+relink
The cpp/xaml gallery now renders **all pages blank/white** (window mounts, measure+arrange run with
correct numbers, but NO control paints — confirmed on a real desktop screenshot, not a capture
artifact). Signature = **templateless controls**: a Button measures 19px (content only, no chrome).
- **NOT the button icon+text change** — reverted it (commit `9a5ebe77bf` reverts `46030899bc`) and the
  blank PERSISTS, and even a forced red Button.Background does not paint. Label/entry/activity_indicator
  are all blank too → global, not button-specific.
- **NOT resources.pri** — it + Bootstrap.dll next to gallery.exe are UNCHANGED (Nov mtimes); only
  gallery.exe/maui_core.lib changed. No resource/theme/exception logged at startup.
- **Root cause (revised — the git tree is CLEAN, so the Windows handlers are all COMMITTED, not WIP):**
  building from HEAD (button reverted → source == the last docs commit `a4434b48b8`) produces a gallery
  that renders blank. So the **committed parity captures were made by an OLDER gallery.exe binary that no
  longer matches** what the current committed source builds — this session's rebuild was the first actual
  gallery build in a while and exposed the true current-source render. The break is therefore EITHER (a) a
  latent global-render regression already sitting in committed Windows source (never rebuilt+run until
  now), OR (b) an environment/runtime change since the last gallery run (e.g. a Windows App SDK framework
  update — resources.pri + Bootstrap.dll are unchanged, but the *installed* WinAppSDK runtime the bootstrap
  resolves could have moved). Templateless-everything + no logged exception fits a resource/theme-merge
  failure at framework init more than a single handler bug.
- **State is safe:** all working captures + `windows_pixel_scores.json` were RESTORED from git (the older
  binary's correct output). Committed source + captures are intact; only the local gallery.exe binary
  renders blank.
- **NEXT FIRE — restore a working gallery FIRST (prerequisite to all parity work):**
  1. Rule out environment: check the installed Windows App SDK runtime version vs what the gallery links
     (the bootstrap MddBootstrapInitialize2 major/minor); a mismatch/newer runtime is the fastest
     explanation. Confirm host_run's app-resource/theme merge still finds the WinUI control templates.
  2. If env is fine, `git bisect` the Windows *code* commits (last-known-good ≈ `2a65c88f0c` WinUI 3
     backend / `aec4afe43c` first sweep — those SHIPPED working captures, so that binary rendered) → HEAD,
     rebuilding + `capture_windows.py label --framework cpp` at each step until `label` goes blank.
  3. Fix the culprit, THEN resume the button icon+text fix — its code is intact in reverted commit
     `46030899bc`, cleanly reappliable once the gallery renders again (it compiled + linked fine; it was
     never the cause of the blank).



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
