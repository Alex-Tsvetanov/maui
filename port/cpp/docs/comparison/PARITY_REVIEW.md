# Parity review — open items needing a user ruling

Per `port/CLAUDE.md` ruling 3: a MAUI-side quirk not covered by rulings 1–10 is recorded here with its
evidence and **paused for a ruling** — neither auto-ignored nor auto-fixed. Approved rulings get appended
to the list in `port/CLAUDE.md`.

---

## 1. ✅ RESOLVED (2026-07-16) — `src/` vs SHIPPED MAUI CheckBox sizing: the RENDER wins

**Ruling (user, 2026-07-16): the render wins — drop the 44 floor.** Now `port/CLAUDE.md` ruling 11.
Implemented: `check_box_handler.mm` `minimumViewSize = k_default_size` (18pt, not 44). Verified on the
Catalyst board — `entry` +20px → **0 / 0.00**, `border_playground` → **0 / 0.37**; the checkbox row is now
18pt, its content bands identical to MAUI (150/183/209/219). Headless 3765/3765; iOS 10/2331 (== baseline,
no regression). The original analysis is kept below for the record.

### The measurement

`entry` is a **+20px pure offset** — but only for the block `y=160..460`; above and below it, the port
matches MAUI exactly (residual **0.00**). The divergence starts at one element: the page's
`<CheckBox IsChecked="True" />` (`port/maui-reference/pages/entry.xaml:9`).

Measured geometry, from the captures (spacing calibrated from the entry pitch, not assumed):

| | checkbox row |
| --- | --- |
| MAUI **Mac Catalyst** — gap between the bracketing Entry borders `150 → 183` = 33px, minus 2× spacing (9.5px) | **14px ≈ 18pt** |
| MAUI **iOS** (1:1, no Catalyst scale) — `216.7 → 259.0` = 42.3pt, minus 2× spacing (12.3pt) | **17.7pt ≈ 18pt** |
| **the port** (Catalyst) — `150 → 203` = 53px, minus 2× spacing | **34px ≈ 44pt** |

44 − 18 = 26pt = **20.02px** at the 0.77 Catalyst UIKit scale — exactly the measured offset. The glyph
itself is identical (14px) in both; only the row differs. MAUI renders 18pt on **both** platforms, so this
is **not** a Mac Catalyst "renders-less" quirk (ruling 10).

### The conflict

- **`src/` (the documented behavior oracle) says 44.** `CheckBoxHandler.iOS.cs:8` `protected virtual float
  MinimumSize => 44f`, applied via `CreatePlatformView`'s `MinimumViewSize = MinimumSize`, and
  `MauiCheckBox.SizeThatFits` (`:245-253`) floors both axes at it:
  `final = Min(Max(MinimumViewSize, h), Max(MinimumViewSize, w))` ⇒ ≥ 44. iOS's
  `ViewHandlerExtensions.iOS.GetDesiredSizeFromHandler` does call `SizeThatFits`, so this path is live.
  There is no override of `MinimumSize` anywhere in `src/` (Compatibility excluded), and the
  Controls-level `CheckBox.Mapper.cs` only remaps `Color` — it does not touch the measure.
- **The shipped MAUI renders 18** = `MauiCheckBox.DefaultSize` (`:13` `const float DefaultSize = 18.0f`),
  i.e. the natural size with **no floor applied**.

**Why they differ:** they are different MAUI versions. `src/`'s `MauiCheckBox.cs` was last modified
**2025-01-26** (`2b0d66669c`), while `port/maui-reference/app/MauiReference.csproj:12` pins
**`<MauiVersion>10.0.71</MauiVersion>`** — a much newer NuGet. The port is written against `src/`; the
ground-truth captures are rendered by 10.0.71.

The port already ports the `src/` rule faithfully (`check_box_handler.mm:245-253` mirrors
`SizeThatFits`; `:340` sets `minimumViewSize = 44.0`), which is exactly why it renders 44.

### The fork (why this is not mine to pick)

The two authorities point opposite ways and this case is not covered by rulings 1–10:

- **`port/CLAUDE.md` prime directive** — `src/` is the behavior oracle; "no invented behavior… don't ship
  a guess". Following it ⇒ keep 44, accept `entry` staying red vs the reference.
- **Ruling 1** — "Microsoft's MAUI render IS the ground truth for all page CONTENT… any content
  difference vs MAUI is a port bug to fix". Following it ⇒ drop the floor so the port renders 18.

Ruling 1 was written about MAUI's *rendered demos* vs the port, not about `src/` contradicting the shipped
binary, so applying it here would extend it. Picking either silently would be a guess about a MAUI version
I cannot read.

### What I need

A ruling on the general question, since this will recur wherever the 2025 `src/` snapshot and the 10.0.71
NuGet have diverged:

1. **When `src/` and the shipped `MauiVersion` disagree, which wins?** (My recommendation: **the render**,
   per ruling 1 — the board's whole purpose is matching what MAUI actually draws; `src/` is then the
   oracle only for *mechanism*, not for values it has since changed.) If so, I'd drop the 44 floor with a
   comment recording that `src/` says otherwise and why.
2. Or: **bump `src/`** to the 10.0.71 tag so the oracle and the reference agree (larger, but removes the
   whole class of conflict).
3. Or: **pin the reference app to a MAUI matching `src/`** (~2025) so the captures match the oracle.

Until then `entry` is 🟡 blocked, not 🔴. Everything else on the page matches at residual 0.00.

**Evidence to reproduce:**
```
python3 port/cpp/tools/parity/capture_ios_clean.py --app maui --themes light --only entry   # MAUI iOS
# then measure the Entry borders bracketing the CheckBox in a text-free column.
```

---

## 2. 🟡 `nested_collection` cpp_xaml blank — XAML loader can't realize a nested templated CollectionView

**Status:** open — larger framework work, not a ruling. The last genuinely-open macOS red
(`nested_collection`, 3.64% — mostly white). NOT a port rendering bug: the port's own framework renders
it correctly (see below); the gap is specific to the **compile-time-XAML loader**.

### The finding

`nested_collection.xaml` is an OUTER `CollectionView` (VerticalList) whose ItemTemplate root is an INNER
`CollectionView` (HorizontalList, HeightRequest=100) with its own `Header="{Binding Title}"`,
HeaderTemplate, `ItemsSource="{Binding Items}"`, and ItemTemplate (`{Binding Caption}`). The outer
ItemsSource is assigned in **code-behind** (NestedCollectionPage.xaml.cs), not in the XAML.

Measured (clean captures): `maui` 72 content rows, `cpp` (code-first) 102 content rows — both render
fully; **`xaml` 0 content rows — completely blank** (only the banner).

### Two-part cause — part 1 fixed locally, part 2 is the blocker

- **Part 1 (data — SOLVED, but reverted with part 2).** cpp_xaml used `build_page<no_view_model, …>` so
  the outer CV had NO source. I built the `super_teams`-style bindable VM (`ViewModels/nested_sources.hpp`:
  `gallery_item` with a registered `Caption`, `nested_source` with `Title` + `Items`) and wired
  `nested_collection.xaml.cpp` (`find<collection_view>("CollectionView")` → `set_items_source`). VERIFIED
  by diagnostic: `find` succeeds, **20 items set, `item_template` + `items_layout` both present**.
- **Part 2 (rendering — THE BLOCKER).** With all of the above set, the outer CV STILL realizes **zero
  cells**. `basic_grouping` uses the identical late-`set_items_source` pattern and renders, so it is not a
  data/timing issue — the XAML loader cannot **realize an outer cell whose DataTemplate root is a nested
  CollectionView** (with its own Header/ItemTemplate + `{Binding Items}`). The code-first `cpp` column
  builds that template in C++ (`data_template::of<collection_view>()` + `add_setup` for the inner
  ItemsLayout, which "is a plain slot, NOT a bindable_property, so it cannot be staged through the
  template's Values" — nested_collection_page.hpp) and works; the XAML template-inflater path does not.

The part-1 VM was reverted because alone it produces NO board change (still blank) — shipping it would be
correct-but-dead code implying progress. It is ~80 lines following `super_teams.hpp` and trivially
recreated alongside the loader work.

### What it needs

XAML-loader support for realizing a DataTemplate whose root is a templated `CollectionView` — staging the
inner CV's non-bindable `ItemsLayout`, its `{Binding Items}`/`{Binding Title}` against each outer item's
binding context, and its own Header/ItemTemplate. This is a real layer-6 feature, not a page tweak; the
port's framework (cpp) already renders the page, so it is a demo-column-only gap at low pixel impact.

**Evidence to reproduce:** wire the VM as above, add `fprintf(stderr, …)` after `set_items_source`, launch
`gallery_xaml` with `MAUI_SAMPLE_PAGE=nested_collection` and stderr redirected — shows 20 items +
item_template set, render still blank.

---

## 3. 🟡 MAUI's CollectionView FRAME is inset on Mac Catalyst but not iOS (informational — fix landed platform-forked)

**Status:** open observation, not blocking. The CV `.Never` fix (iOS, big win — 47%→~9%, 3 pages green)
had to be **forked by platform** and this records why, so a future frame-level fix can unify it.

### The finding

C# `ItemsViewController2.ViewDidLoad` (:163-183) sets `ContentInsetAdjustmentBehavior = .Never` on iOS 11+
AND Mac Catalyst 11+ (one code path, both platforms). Yet MAUI *renders* the two differently, measured:
- **iOS:** the CV content goes edge-to-edge UNDER the status bar (group header at y=0, the view-level
  Header scrolled out of sight). `.Never` matches; the UIKit default `.automatic` insets below the safe
  area and puts the whole CV a status-bar height low (the 8-page cluster, 35-47%).
- **Mac Catalyst:** the CV content sits BELOW the titlebar. The port's page-direct CV, arranged over full
  bounds (the U20 safe-area slice), reproduces that only with `.automatic` (green 0.12%); forcing `.Never`
  moves content under the titlebar and regresses to red 44%.

### The implication

Same C# line, different render ⇒ MAUI's CV **frame** (or its host's safe-area handling) must be inset on
Catalyst but NOT iOS — with `.Never` on both, the only remaining variable is the frame the CV is laid into.
The port arranges a page-direct CV over full bounds on both backends, so today `.automatic` is what happens
to supply Catalyst's inset. The landed fix forks `#if !TARGET_OS_MACCATALYST` (contentInsetAdjustmentBehavior
= .Never on iOS only), which matches BOTH renders (ruling 1). A cleaner future fix would inset the CV's
FRAME on Catalyst (matching MAUI's actual mechanism) and then set `.Never` uniformly as C# does; that needs
locating why MAUI's Catalyst CV frame is inset (likely the page/host safe-area path the U20 work touched).

### Residual after the iOS fix (separate, smaller)

The 3 still-red grouping pages (~9%) are a per-group-header height difference: the port's green group-header
band is 60px @3x (20.0pt) vs MAUI's 58px (19.33pt), accumulating +2px per group. A 2px shift of full-width
colored bands reads as 100%-different rows, inflating the pixel-%. Separate root cause (group-header cell
sizing), tracked for a later pass.

## RadioButton indicator ring on iOS-3× — native-fallback limitation (2026-07-17)

**Status:** recorded (native-fallback architecture limit); Catalyst greened, iOS residual left minor.

The port renders `RadioButton` on the iOS backend as a native `UIButton` + SF-symbol pair (`circle` /
`smallcircle.filled.circle`) — NOT the C# DefaultTemplate (`Border`→`Grid`→`Ellipse 21x21` +
`ContentPresenter`, `RadioButton.cs:516-635`). That template port is deferred. Two consequences on
`radio_button_content`:

1. **Wrapped indicator shrinks (unfixed).** MAUI's ring is a fixed 21x21 `Ellipse` (63px @3×), invariant to
   content. UIButton renders the SF-symbol at 21pt (63px) for a single-line title but shrinks it to ~17pt
   (51px) once the title WRAPS to multiple lines (the `<Frame>`-wrapped "Can't use View…" radio). Attempts:
   - `preferredSymbolConfiguration configurationWithFont:21` → holds the ring at 63px BUT enlarges the
     symbol's layout box, inflating single-line rows +14px each (regression, reverted).
   - `imageView.contentMode = UIViewContentModeCenter` → no effect on the symbol size (reverted).
   No clean fix exists without porting the ControlTemplate (the real Ellipse) or a fragile config hack.

2. **Wrapped-height double-count + cpp padding (FIXED — see STATUS 2026-07-17).** These landed: the Frame box
   went 304→297px and the cpp twin got `Padding=16`. Catalyst `radio_button_content` is now GREEN on both
   columns (cpp 0.48%, xaml 0.51%).

**Net:** on **Mac Catalyst (0.77×)** the residual ring-AA is absorbed → green. On **iOS (3×)** the 51-vs-63
wrapped ring plus font anti-aliasing keep `radio_button_content` (~5%), `radio_content_properties` (~6%,
custom Baskerville/Arial fonts + CharacterSpacing at 3×), and `radio_button_group_gallery` (~5%) red — all
minor, dominated by high-DPI rendering the native fallback can't match pixel-for-pixel. The deferred
RadioButton ControlTemplate port is the proper settlement. Not a per-page port bug beyond what landed.

## Gesture animated-gif cluster on iOS — capture-infrastructure gap (2026-07-17)

**Status:** recorded (capture-tooling gap); static content verified correct; NOT a port rendering bug.

`pointer_gesture` (xaml 10.55% / cpp 0.11%), `gestures` (5.09% / 0.24%), `ios_swipe_transition` (~5.9%)
score against the animated `.gif` capture (build_comparison_json prefers `.gif` over `.png`; the scorer
`Image.open`s frame 0). But the **port's gesture gifs are single-frame** while MAUI's are driven animations
(measured: `pointer_gesture` maui **13 frames**, cpp **1**, xaml **1**) — the port's capture does not INJECT
the pointer/hover/gesture interaction, so it records the resting frame only. Comparing a 1-frame static port
capture to MAUI's animation frame-0 is non-representative: `cpp` coincidentally matches MAUI's frame-0
(0.11% "green"), `xaml` happens to catch a different resting frame (10.55% "red") — a frame-timing artifact,
NOT a real cpp-vs-xaml difference (their static PNGs are **0.05%** apart, i.e. identical; both are ~2.6% off
MAUI's resting frame = the pointer-trail state MAUI's driven capture leaves behind).

The port renders these pages' static content correctly. The fix is capture-infrastructure: drive the
pointer/gesture interaction for the port apps during gif capture (MAUI's own capture does this), so the port
produces a comparable multi-frame animation. That injection path does not exist yet for the port's iOS gif
capture. NOTE: a blanket "score PNG not GIF" scorer change is WRONG — some port gifs ARE valid animations
(cpp `activity_indicator`; xaml `activity_indicator`/`empty_view_load_simulate`/`swipe_item_position`), so
the gif comparison is meaningful there. Per-page capture-driving is the correct fix. Deferred.

## scroll_to_group on iOS — Grid rows 6pt tighter than shipped MAUI (2026-07-17)

**Status:** diagnosed; ruling-11 (render-vs-src) Grid divergence; fix is board-wide → escalate.

`scroll_to_group` (5.84%) is a CLEAN uniform −72px offset: rings/CV below align at residual ~0 once
shifted; the top form (two 3-row/2-col Grids of Label+Entry, ScrollToGroup.xaml) is 72px SHORTER in the
port. Root-caused to the Grid row height: each of the 4 entry-row gaps is ~6pt (18px @3×) tighter in the
port than in MAUI (4 × 18px = 72px).

Measured/probed on the iOS-26 simulator (the same sim that shot the MAUI reference):
- The Entry's visible box (33.7pt) and the "0" glyph (14pt font) are IDENTICAL between MAUI and the port.
- A **bare RoundedRect UITextField `sizeThatFits` returns 34pt, FIXED across font sizes** (14pt→37×34,
  17pt→39×34, default→39×34) — only width tracks the font. The port's Entry `get_desired_size` returns
  exactly 34pt (matches the bare field) — provably correct for this OS.
- The standalone `entry` page (a `VerticalStackLayout` of plain Entries) is GREEN — MAUI's Entries there
  are also 34pt. So MAUI's base Entry measure IS 34pt; the extra 6pt appears ONLY inside the Grid rows.

So MAUI's SHIPPED (10.0.71) Grid lays an Auto row of a text input ~6pt taller than `sizeThatFits` (a row
min / baseline pad / RowSpacing default not present in the read-only `src/` snapshot, whose Grid
`RowSpacing` default is 0 — matching the port). Per ruling 11 the render wins, but reproducing it means
changing the port's Grid Auto-row measure/default board-wide — every Grid on every platform — which is a
high-blast-radius change that must be user-ruled, not made autonomously. The port's per-control Entry
measure is correct and must NOT be inflated (would regress the green `entry` page). Deferred pending a
ruling on the Grid row behavior. NOTE: content otherwise matches (pure offset).

## ⚠️ RULING NEEDED: iOS CollectionView persistent selection band contradicts ruling 9 (2026-07-17)

**Status: ESCALATION — needs a user ruling. NO code changed. Affects the whole ruling-9 selection cluster.**

Ruling 9 (2026-07-08) states: "MAUI **iOS and Android** render a persistent gray selection background on the
selected cells … but MAUI **Mac Catalyst** does NOT," and concludes the port's band (drawn on iOS+Catalyst
because Catalyst reuses the iOS handlers) is CORRECT on iOS and only the Catalyst absence is exempt.

**The FRESH iOS MAUI reference contradicts that premise.** On `preselected_items` and
`selection_synchronization` (both iOS, MauiReference 10.0.71, zoomed + verified):
- **MAUI iOS shows NO persistent selection band** at rest on the preselected cells (photo.jpg 2 / Item 2 /
  Item 3 render on a plain white background — identical to unselected cells).
- The port's **cpp** column DOES paint the gray band (per ruling 9) → diverges from MAUI → scored RED
  (preselected_items 10.22%, selection_synchronization 12.79%).
- The port's **xaml** column shows NO band and matches MAUI (green/near-green) — but for a DIFFERENT reason:
  the XAML loader does not apply the `SelectedItems` preselection at all (a loader gap), so its CV has no
  selection to paint. (So xaml's match is coincidental, not a correct application-then-suppress.)

So shipped MAUI 10.0.71 **iOS** appears to have DROPPED the persistent at-rest selection band that ruling 9
says it draws — a ruling-11 (render-wins) divergence from the behavior ruling 9 was written against. If so,
the port's iOS band is now WRONG and should be suppressed at rest (the same way ruling 9 already exempts it
on Catalyst), which would flip these pages green.

**Two ways to resolve — user's call (do NOT decide autonomously; ruling 9 is a standing ruling):**
1. **Ruling 11 wins** → suppress the port's persistent at-rest selection band on the iOS backend
   (iOS + Catalyst), matching the fresh MAUI render. Flips the selection cluster green. Requires amending
   ruling 9's iOS claim.
2. **Ruling 9 stands** → treat the fresh MAUI-iOS no-band as an exempt MAUI quirk (like the Catalyst case)
   and stop counting the band as a diff on these iOS pages; the port keeps painting it.

Affected pages (ruling 9's named cluster): preselected_item, preselected_items, multiple_bound_selection,
selection_synchronization, and any similar applied-selection CV page. `header_footer_grid` (4.33%) and
`nested_collection` (4.51%) are cpp-only reds too but are cluster-A structural (StackLayout-vs-VSL) — separate.

## ✅ RESOLVED (2026-07-17): ruling 9 STANDS — the twin was omitting the preselection

The escalation above ("fresh iOS MAUI shows NO selection band") was based on a FALSE premise. Root cause:
the shared-XAML twins for `preselected_items` and `selection_synchronization` declared `SelectionMode` but
**no `SelectedItems`/`SelectedItem`** — the original C# preselects in code-behind, which the twin dropped.
So MAUI (and the port's xaml column, both rendering the twin) had NOTHING selected → no band. MAUI wasn't
suppressing the band; there was no selection to paint.

User ruling (2026-07-17): "MAUI and C++ look correct on iOS, C++ & XAML is obviously wrong" — i.e. the
selection MUST be applied (cpp does it in code; xaml failed to). Fix: declared the preselection inline in
the shared XAML (`<CollectionView.SelectedItems><x:Array>` + `SelectedItem="…"`) and taught the loader to
apply it (xaml_visitors `try_set_selected_items_from_array` + a `SelectedItem` string property in
register_xaml_items; boxed via `boxed_item::of` for value-matching). Verified end-to-end on the iOS sim:
MAUI now paints the band (0 → 104177 px), and xaml matches MAUI (0.01% / 0.00%). **Ruling 9 holds** — MAUI
iOS paints the persistent selection band whenever items are actually selected. selection_synchronization is
now fully green (cpp 0.09% / xaml 0.00%); preselected_items xaml green (cpp 7.76% is a separate pre-existing
code-first cell-layout difference vs the twin's plain `<Label Margin=6>`, not the selection). maccatalyst
recapture is a follow-up (there ruling 9 keeps the port's band exempt vs MAUI-Catalyst's no-band).

## header_footer_grid cpp — GridItemsLayout item spacing degraded by the twin (ruling 12, 2026-07-17)

`header_footer_grid` cpp (4.33%) is exempt per ruling 12: the code-first page sets the original C#
`GridItemsLayout(Span=3, HorizontalItemSpacing=4, VerticalItemSpacing=2)`, so its item grid runs ~18px
taller (verified). The shared twin degrades it to `ItemsLayout="VerticalGrid, 3"` (the loader lacks the
`<GridItemsLayout>` element form — no default ctor / `set_orientation`), which can't carry item spacing, so
MAUI + C++&XAML render the grid flush. The cpp keeps the original spacing → correct per ruling 12; the
cpp-vs-maui offset is exempt. A future `<GridItemsLayout>`-element loader feature would let the twin carry
the spacing and green all three.

## nested_collection — cpp inner item-spacing (ruling 12) + xaml renders the nested CV BLANK (2026-07-17)

- **cpp (4.51%)**: exempt per ruling 12 — the code-first inner CollectionView keeps the original item
  spacing; the shared twin degrades the inner GridItemsLayout to the string form (no spacing). cpp correct.
- **C++&XAML (scored ~2.67% yellow, actually BLANK)**: a REAL deferred bug — the compile-XAML path does not
  hydrate a CollectionView nested inside another CollectionView's ItemTemplate, so the inner lists render
  empty (0.6% body content vs MAUI's 3.6%). The scorer under-reports it because the item text is sparse.
  Fixing needs the loader/template-inflater to realize a nested CollectionView cell — deferred.

---

## Windows dark-theme background delta — AWAITING A USER RULING (2026-07-30)

The port renders the page background `#202020` (32,32,32) where MAUI renders `#272727` (39,39,39), in
DARK only (light is off by 1, which is noise). The mechanism is still unknown after six mechanisms were
ruled out by measurement (recorded in the campaign brief: ApplicationPageBackgroundThemeBrush,
NavigationViewContentBackground, all 5318 SolidColorBrush entries reachable from Application.Resources
dumped from inside the running app, MicaBackdrop{BaseAlt} in both themes, any single translucent overlay
— algebraically needs c=278 — and low-alpha white compositing, which needs ~#08FFFFFF where the table has
only #00 #0D #14 #15 #1F). It has NEVER been hard-coded and must not be.

### What it costs — CORRECTED, and this supersedes what the campaign assumed

It was long recorded as costing "~4% on all 172 pages". **That was mis-attributed.** Two measurements:

1. **It costs ZERO on the diff metric.** `pixel_score.py:113` counts a pixel as differing only above
   `DIFF_THRESHOLD = 25/255`. The delta is SEVEN levels. It has never contributed to "% pixels differ".
2. **The real ~4% was CHROME**, the one cause the brief explicitly ruled out — and it is now fixed in two
   commits: the 32-DIP title-bar band (32px of an 800px frame = 4.00%, `ef892a8300`) and the opaque
   caption-button strip (138x32 = 4314px = 0.53%, `b91b2018d9`). Together those took dark GREEN 0 -> 27.

### What it DOES cost: dark SSIM, and it is the deciding factor for otherwise-perfect pages

`b91b2018d9`'s commit message claims the delta is "measured NOT to matter". **That claim is wrong and is
retracted here.** It came from a confounded experiment: adding +7 to the WHOLE dark capture, which fixes
the background *and breaks every content pixel at the same time*, so it showed almost no net gain.

Re-measured correctly — offsetting ONLY pixels whose value is the port's background — the delta gains
**+0.004 to +0.018 SSIM per page** (items 0.9424 -> 0.9596, check_box 0.9486 -> 0.9667,
menu_bar 0.9485 -> 0.9653, ios_safe_area 0.9494 -> 0.9655).

And the exact arithmetic explains why it is decisive. SSIM between two CONSTANT fields is
`(2*mu_a*mu_b + C1) / (mu_a^2 + mu_b^2 + C1)`; with mu = 39 vs 32 and `C1 = (0.01*255)^2 = 6.5025`:

    (2*39*32 + 6.5) / (39^2 + 32^2 + 6.5) = 2502.5 / 2551.5 = 0.98079

So **every uniform background region in a dark capture is capped at SSIM 0.9808** against a 0.98 gate —
essentially zero headroom. Banded SSIM confirms it directly: on `items` dark, every 100-row band from
y=200 to y=800 reads a uniform **0.9806**, while the SAME bands in light read 0.9999-1.0000.

The consequence, on pages with no content defect at all:

| page            | full-page light SSIM | full-page dark SSIM |
|-----------------|----------------------|---------------------|
| button          | 0.9962               | 0.9872              |
| grid            | 0.9963               | 0.9806              |
| styles          | 0.9965               | 0.9795              |
| absolute_layout | 0.9921               | 0.9759              |

These pages are pixel-correct in light and straddle the gate in dark purely on the background cap. Board
state as of `a2505b8a89`: dark has **123 pages under the 1% diff gate but only 27 clearing SSIM >= 0.98**.
The ~96-page difference is dominated by this delta.

### The ruling requested

(a) **Accept as a documented deviation under policy #3.** Consequence, stated honestly: dark can reach
    roughly 27-40 green out of 170 and no further, because the cap sits below the gate for any page whose
    content is not *better* than pixel-perfect. All-green on both themes becomes unreachable.
(b) **Authorise deeper instrumentation.** The two untried avenues are UI Automation over MAUI's live
    visual tree (to find what actually paints the root), and a window RECT/DPI comparison in case the two
    captured client areas are not the same region. Note the title-bar work already proved the client areas
    DID differ (the port started at y=31, MAUI at y=32), so avenue two has partial precedent.
(c) A third option not in the original framing: **raise or per-theme the SSIM gate**, on the grounds that
    a 7-level uniform offset is not a perceptual defect. This changes the measuring stick rather than the
    port, so it needs an explicit ruling — it is not a call to make silently.

Until this is ruled on, the remaining Windows work is page-shaped content defects, which are tracked in
the board README rather than here.

---

## MAUI's own CollectionView captures carry a ~0.50pp focus-visual noise floor (measured 2026-07-30)

MEASURED, with the port's binary and captures held constant: on CollectionView pages, MAUI's GROUND-TRUTH
column differs between two runs by ~4100 px (~0.50% of the frame). The port's own capture over the same two
runs is BIT-IDENTICAL (0 differing pixels on basic_grouping), so the variation is entirely MAUI's.

What varies is a WinUI FOCUS VISUAL: a 2px near-black (26,26,26) rectangle outline around the group header
-- 1008 px wide on rows 51-52, then 4 px on each of rows 53-94. Present in one run, absent in the other.
  basic_grouping          4192 px (0.512%)  band y 51-94, x 8-1015
  items                   4077 px (0.498%)  band y 52-68
  measure_first_strategy  4164 px (0.508%)  band y 60-97

CONSEQUENCES, and this changes how CollectionView scores must be read:
1. Roughly 20 CollectionView pages have a +-0.50pp run-to-run noise floor that no port change can affect.
   A page moving 0.00 -> 0.50 or 0.20 -> 0.70 across two captures is NOISE, not a regression. This is
   distinct from the board-wide +-0.15pp floor measured on 2026-07-29 for ordinary pages.
2. It explains a diagnosis that was correctly refuted earlier today. An `items` finding identified this
   exact band as a WinUI focus visual and proposed matching it; the skeptic refuted the fix as a provable
   no-op. Both were right, and the deeper reason is now measured: the band is not reliably present in the
   REFERENCE, so there is nothing stable to match.
3. It does NOT rescue the reverted item-cell margin translation, and I want the record precise about how
   much damage that change actually did. Its regressions on basic_grouping (0.01 -> 1.72),
   grouping_plus_selection (0.01 -> 1.77) and grid_grouping (1.63 -> 3.31) are ~+1.7pp, far above this
   floor and therefore real. But several pages I counted against it -- items 0.00 -> 0.50,
   nested_collection 0.00 -> 0.54, measure_first_strategy 0.00 -> 0.51 -- moved by exactly this focus
   visual and were NOT its fault. I over-counted the damage; the revert still stands on the three real
   regressions plus basic_grouping's counter-example (a bare-Label root with thickness(5,0,0,0) that was
   already pixel-correct without the translation).

NOT ACTIONABLE as a port fix. Options if it ever needs to stop polluting comparisons: capture the MAUI
column with focus suppressed or with a deterministic focus target, or treat the top band of CollectionView
pages as excluded region. Both are harness changes, not port changes, so neither should be done without a
ruling. Recording the floor is enough for now: read CollectionView deltas under +-0.50pp as noise.

### The focus visual makes the GREEN COUNT itself non-deterministic by up to ~12 pages

Quantified after the fact, and this is the part that matters for how this board is reported. Between two
consecutive scorings with the port's code byte-identical, TWELVE pages crossed the SSIM >= 0.98 gate in the
light theme. For every one of them the PORT capture was bit-identical (0 differing pixels) while MAUI's
varied by 4081-6581 px:

  page                      cpp run-to-run   maui run-to-run   ssimL swing
  multiple_bound_selection        0 px            4081 px      0.9839 -> 0.9581
  header_footer                   0 px            4123 px      0.9888 -> 0.9607
  some_empty_groups               0 px            4192 px      0.9922 -> 0.9623
  header_footer_template          0 px            4593 px      0.9898 -> 0.9627
  varied_size_selector            0 px            6581 px      0.9923 -> 0.9661
  basic_grouping                  0 px            4192 px      0.9963 -> 0.9668
  items                           0 px            4077 px      0.9963 -> 0.9679
  measure_first_strategy          0 px            4164 px      0.9970 -> 0.9682
  nested_collection               0 px            4418 px      0.9965 -> 0.9682
  adaptive_collection             0 px            4237 px      0.9950 -> 0.9685
  single_bound_selection          0 px            4164 px      0.9972 -> 0.9687
  grouping_plus_selection         0 px            4192 px      0.9967 -> 0.9702

The focus rect costs ~0.03 SSIM, and these pages sit at 0.984-0.997 -- so it straddles the gate. It is
also PER-PAGE and nondeterministic, not per-run: in the current pass 20 of 43 CollectionView pages carry it
and 23 do not, and the membership of those sets changes between runs (basic_grouping has it now and did not
last run).

CONSEQUENCE: the light green count carries a +-12-page uncertainty attributable entirely to the reference
column. Green counts quoted across this campaign should be read with that band; a swing of ten green pages
on CollectionView-heavy passes may be nothing but this. ALL-GREEN ON BOTH THEMES IS UNREACHABLE while ~20
reference captures vary, independent of anything the port does.

MITIGATION OPTIONS -- all HARNESS changes, none a port change, so none should be taken without a ruling:
  (a) Suppress focus before capturing the MAUI column (click a non-focusable region, or drive the app so no
      item has focus), making the reference deterministic. Cleanest, and it fixes the cause.
  (b) Capture the MAUI column N times per page and keep the modal frame, so a stray focus rect is outvoted.
      Costs N x MAUI capture time on every full pass.
  (c) Exclude the focus-band region from scoring on CollectionView pages. Cheap, but it blinds the board to
      real defects in that band.
  (d) Mark the ~20 affected pages volatile. Honest but it removes a fifth of the board from the metric.
(a) is the recommendation: it removes the nondeterminism rather than averaging or hiding it.

### The reference-column noise is NOT confined to CollectionView pages (measured 2026-07-30)

The focus-visual section above scoped the run-to-run reference variation to ~20 CollectionView pages. That
scope is too narrow. Measured on a later pass, with the port's code byte-identical and its captures
BIT-IDENTICAL (0 differing px):

  border_stroke       cpp 0 px changed   maui 4104 px changed   (+0.50pp)
  border_playground   cpp 0 px changed   maui 3968 px changed   (+0.49pp)

Neither is a CollectionView page. So the ~4000-px / ~0.50pp reference-side swing occurs more broadly than
first recorded, and the +-12-page green-count uncertainty derived from the CollectionView set is a FLOOR,
not a ceiling.

This does not change the recommendation -- suppress focus before capturing the MAUI column -- but it raises
its value: more of the board is affected than the first measurement suggested, and any cross-run comparison
of ~0.5pp on ANY page should be treated as unattributed until the port-vs-reference capture diff is checked.
The check is cheap and worth making routine: compare the page's cpp capture against its previous blob; if it
is 0 px, the movement is not the port's.

---

## Shape stroke deflate: a 4-platform geometry change made on a Windows-only measurement (2026-07-30)

`maui::graphics::shapes::{rectangle, ellipse, round_rectangle}::path_for_bounds` now applies MAUI's
`Shape.TransformPathForBounds` net **0.5 DIP/side deflate**. Oracle chain, all verified against `src/`:

- `Shape.cs:320-323` -- `viewBounds.X += StrokeThickness/2; Width -= StrokeThickness`, with
  `StrokeThicknessProperty` defaulting to `1.0` (`Shape.cs:80-81`) => +0.5 / -1.0 per axis.
- `Rectangle.cs:18`, `Ellipse.cs:17`, `RoundRectangle.cs:12` -- all three constructors set
  `Aspect = Stretch.Fill`, overriding `Shape`'s own `Stretch.None` default (`Shape.cs:106`), so the
  Fill branch (`Shape.cs:378-383`) scales by `(W-1)/W` and translates to `viewBounds.Left = 0.5`.
- `Border.cs:81` -- `StrokeShapeProperty` defaults to `new Rectangle()`, a *Controls* Shape. So MAUI's
  default Border stroke genuinely deflates; the port's did not.

**Why the shared graphics layer and not a Windows handler.** `maui::controls::shapes::rectangle` (the
full Shape port) already implemented this deflate -- which is why the `shapes` page scored 0.04% while
the Border family did not. The two families had *diverged*, and the graphics one was the wrong one.
The XAML loader shows the same split from the other side: `xaml_visitors.cpp:1866`'s comment records
that `<Ellipse>`/`<Rectangle>` resolve to `register_type`'d **controls** shapes (already deflating)
while `<RoundRectangle>` alone minted the **graphics** type. This change removes that inconsistency.

**Use-site sweep (every `graphics::shapes::` site outside tests was classified):** all 23 non-test
sites are `Border.StrokeShape` / `Frame` -- MAUI counterpart is a Controls Shape, so the deflate is
correct at every one. Exactly **one** site was wrong and is fixed in the same change:
`clipping_page.hpp:223` passed a graphics *Shape* to `set_clip` where the C# it ports uses
`RoundRectangleGeometry` -- and `Geometry.PathForBounds` (`Geometry.cs:17`) returns the raw
`AppendPath` result with **no** deflate. It now uses `controls::shapes::round_rectangle_geometry` with
the explicit `Rect(0,0,400,50)` the C# passes. XAML clips cannot hit this: `VisualElement.Clip` is
typed `Geometry` in MAUI, so no XAML clip ever resolves to a Shape.

### OUTSTANDING DEBT -- iOS / macOS / Android are unrescored

This was measured on the **Windows** board only. The change is in shared code, and the other three
backends' `IView.Clip` and Border content-clip paths DO call `path_for_bounds`, so their rendering
moves 0.5 DIP. Windows is unaffected on the clip path specifically -- `view_chrome_ops.cpp` dispatches
on concrete shape type and builds Composition geometry directly, never calling `path_for_bounds`.

Per oracle the new position is the *correct* one on all four platforms, so this is expected convergence,
not regression -- but it is **unverified by pixels** on three of them. The headless suite is 3775/3775,
which does not render. Exposed families to rescore: `border*`, `clip*`, `clipping`, `containers`,
`box_view`, `chat_example`, `swipe_view_shadow`, `custom_swipe_item_view`, `alignment`, `borderless`,
`invalidate_shadow_host`, `radio_button_content`, `radio_template_from_style`, `varied_size_selector`.
If an iOS/macOS/Android board moves by ~1px on stroke edges, this change is the first thing to check.

### Minor: a masked precision bug

`view_mapper_tests.cpp`'s ellipse tolerance was widened to `0.3F` to absorb the deflated coordinates.
That tolerance now also absorbs a **pre-existing** `path_f::get_bounds_by_flattening` bug -- flattening
overshoot grows with the absolute coordinate magnitude, not just the 0.5 fractional offset (probed at
(0,0), (0.5,0.5), (1,1), (100.5,100.5)). The slack is not intentional headroom; it is masking that bug,
and a regression in it would no longer be caught there.

### RETRACTED 2026-07-30 (same day) — the deflate above was REVERTED, and the debt it created is VOID

The section immediately above describes the shared-layer 0.5 DIP deflate as landed, and books a
"full iOS/macOS/Android rescore" obligation against it. **Both statements are now wrong.** The change
was reverted in `f1a5a17658` after being measured on the board; it never shipped, and **no other
platform's geometry moved**, so there is NO rescore debt. Read the section above only as the record of
a hypothesis that was tested and rejected.

What the measurement showed (all port-vs-reference attributed, not noise):

  improved   border_stroke 3.51 -> 1.99, swipe_view_shadow 1.02 -> 0.03,
             invalidate_shadow_host 1.14 -> 0.51, border_playground 1.97 -> 1.74,
             alignment 0.37 -> 0.17, border_resize_content 2.07 -> 1.87
  REGRESSED  clipping 0.00 -> 5.12 (40000 cpp px moved / 1960 maui),
             borderless 0.00 -> 0.68 (5544 / 0), clip_views 0.10 -> 0.74 (6128 / 1662)
  net        mean 0.41% -> 0.42%, cpp_light BOTH 119 -> 116

Two pages that were EXACTLY green went red. The oracle chain was sound and the stroke edge did match
MAUI afterwards ((255,0,0) -> (249,121,121), MAUI's exact antialiased value) — the defect was the
LAYER, not the geometry.

**Why the shared shape layer is the wrong home.** The graphics shapes are consumed by the CLIP paths
too — `layout_handler`'s ClipsToBounds rectangle and `view_chrome_ops`' `build_geometry`. MAUI never
deflates a clip (`Geometry.PathForBounds`, Geometry.cs:17, returns the raw `AppendPath` result), so
every clipped surface shifted 0.5 DIP against a reference that did not move. Proof it was the deflate
and not the collateral `clipping_page` Shape->Geometry swap: reverting that swap ALONE left clipping at
5.12%.

**Where it actually belongs (for whoever retries this).** Windows applies the deflate in the BORDER
HANDLER, from the *Border's* own StrokeThickness — `BorderExtensions.UpdatePath`
(Core/src/Platform/Windows/BorderExtensions.cs:17-32):

    strokeThickness = borderPath.StrokeThickness            // the BORDER's, not the Shape's
    pathSize        = Rect(0, 0, width - strokeThickness, height - strokeThickness)
    shapePath       = borderShape.PathForBounds(pathSize)
    borderPath.RenderTransform = TranslateTransform(strokeThickness/2, strokeThickness/2)

Same +0.5/-1.0 at the default thickness 1.0, but correctly **zero** at StrokeThickness=0 — which is
exactly the `borderless` page (its twin sets `<Setter Property="StrokeThickness" Value="0" />`). A
hard-coded 0.5 in the shape can express neither that nor "don't touch clips". Note also that MAUI does
NOT propagate Border.StrokeThickness to the StrokeShape (`Border.StrokeThicknessChanged`,
Border.cs:388, only invalidates measure), so a Shape's own deflate always uses the Shape's own
thickness — the two mechanisms are genuinely separate.

Also still true from the retracted section: the `view_mapper_tests` ellipse tolerance and the
`path_f::get_bounds_by_flattening` precision bug were reverted with it, so that bug is once again
plainly visible rather than masked.

**Separately confirmed while investigating:** `view_chrome_ops`' `build_geometry` covers only
ellipse/rectangle (bounds-relative and *_geometry); **rounded rectangles are not covered at all**
(its own file header says so). So `Clip` set to a RoundRectangle/RoundRectangleGeometry silently
applies NO clip on Windows. That is an independent, still-open gap.

#### Correction (2026-07-31): the "where it belongs" paragraph above was ALREADY DONE

The retraction section closes by saying the deflate "belongs" in the border handler per
`BorderExtensions.UpdatePath`, phrased as guidance "for whoever retries this". That was written without
checking, and it is wrong in one important way: **the Windows border handler had already implemented it**,
in `ef892a8300`, before the shared-layer attempt was ever made.
`port/cpp/src/platform/windows/border_handler.cpp:216-232` computes
`path_bounds{0, 0, max(0, width - thickness), max(0, height - thickness)}`, calls
`spec.shape->path_for_bounds(path_bounds)`, and bakes `translate(thickness/2, thickness/2)` into the
geometry via `path_f::transform` (one fewer native object than a XAML RenderTransform, same net
position). `spec.thickness` is the BORDER's own `stroke_thickness()` (`src/core/border_handler.cpp:70`),
so it is correctly zero for `borderless`. The mapping to the oracle is line-for-line.

So the shared-layer change was not "the deflate finally being added" — it was a **SECOND** deflate
stacked on an existing correct one.

That reframes, but does NOT fully explain, the measurements: a pure double-deflate should have made the
border pages worse, and `border_stroke` measurably IMPROVED (3.51% -> 1.99%). MAUI's own chain does
contain two nested steps (the handler's `pathSize` shrink, and then `PathForBounds` on a *Controls*
Shape, which applies `TransformPathForBounds` with the SHAPE's own StrokeThickness), so "MAUI deflates
twice and the port only once" is a live hypothesis for `border_stroke`'s residue. But it is contradicted
on `borderless`, where the port matches MAUI at exactly 0.00% while applying only ONE deflate. Both
cannot be universally true, so the remaining difference is page-specific, not a uniform missing step.
**Do not act on either reading without a per-page measurement.** Recorded as an open question, not a
conclusion.

#### RESOLVED (2026-07-31, later same day) — subpixel centroid measurement: the second deflate is real, gated on thickness > 0

Run-length pixel classification (the measurement above) can't see a half-pixel shift — a hard band at
`[20.0, 25.0]` and a soft band at `[20.5, 25.5]` classify almost identically under a color threshold. Redone
with per-pixel alpha solved against the known flanking colors (`bg`/`orange`/`red`), then compared by total
coverage and coverage CENTROID on isolated stroke edges, on `border_stroke` (T=1, 5, 10; all four edges,
`docs/comparison/captures/windows/{maui,cpp}/border_stroke_light.png`):

    edge                    MAUI centroid   cpp centroid   delta
    row1 top    (T=1)         75.517           75.000       +0.517
    row10 bottom(T=10)       160.983          161.500       -0.517
    row1 left   (T=1)         20.517           20.000       +0.517
    row5 left   (T=5)         22.517           22.000       +0.517
    row10 left  (T=10)        25.017           24.500       +0.517
    row1 right  (T=1)       1002.483         1003.000       -0.517
    row5 right  (T=5)       1000.483         1001.000       -0.517
    row10 right (T=10)       997.983          998.500       -0.517

Every edge, at every thickness, moves ~0.5 DIP INWARD relative to the cpp render, independent of `T` — the
signature of a fixed 0.5 DIP/side inset, not a scale error. Left AND right move toward each other (not the
same direction), so it is a symmetric SIZE shrink, not a translate. This is exactly `Shape.cs`'s own default
`StrokeThickness = 1.0` (0.5/side) feeding `TransformPathForBounds` unconditionally on the Controls-Shape
oracle — confirmed by re-reading `Shape.cs:300-411` + `Rectangle.cs:57-83` directly: `GetPath()` bakes a
`StrokeThickness/2` inset using the SHAPE's own thickness, and `TransformPathForBounds`'s subsequent
Aspect=Fill scale+translate — computed from the SAME shape thickness — reduces to the identity transform for
an unrounded Rectangle (numerator and denominator shrink by the identical amount), so the NET effect of the
whole chain is a single, constant 0.5 DIP/side inset. Not a double deflate; one 0.5 DIP inset, arrived at via
two steps that cancel everywhere except that shared 0.5.

The `borderless` tiebreaker: measured directly on `borderless` (a StrokeThickness=0 pink/red Border pair
sharing a Grid boundary, `captures/windows/{maui,cpp}/borderless_light.png`), the pink/red fill boundary is
**bit-identical** between MAUI and cpp at `y=412`, zero blended pixels, no gap — MAUI does NOT apply the
inset when the Border has no stroke. So the fix (`src/platform/windows/border_handler.cpp`,
`update_border()`) gates the extra 0.5 DIP/side inset on `thickness > 0`, scoped to the border handler alone
(not the shared graphics shape layer, so clips stay undeflated, same as before).

> **Correction (2026-07-31, cross-platform pass).** This paragraph originally called the gate "a deliberate,
> MEASURED deviation from the literal `src/` chain", on the reading that `src/` makes the inset
> unconditional. That reading was incomplete — **the gate IS in `src/`**, in `Border.UpdateStrokeShape`
> (`src/Controls/src/Core/Border/Border.cs:433-439`), which was not consulted when the above was written:
>
>     if (StrokeShape is Shape strokeShape && StrokeThickness == 0)
>         strokeShape.StrokeThickness = StrokeThickness;
>
> It fires from `OnPropertyChanged` for `StrokeThickness` / `StrokeShape` (`Border.cs:417-420`) and zeroes
> the *shape's* own thickness whenever the Border's is set to 0. Note what it is precisely: a **one-way
> latch**, not a function of the thickness — it only ever writes 0, never restores 1.0, so a Border driven
> 5 -> 0 -> 5 at runtime keeps a zeroed shape and therefore keeps no inset. For the static markup this port
> renders, latch and gate coincide exactly, which is why the measurement and the oracle agree. There is no
> `src/`-vs-shipped drift here and no ruling-11 call to defend; the port's `thickness > 0` gate is
> straightforwardly correct, and only the runtime-latch tail is (harmlessly) unmodelled.

Also resolves the dangling "border_stroke measurably improved 3.51 -> 1.99 under the wrongly-placed shared
deflate" fact from the retraction above: that improvement was this SAME correct 0.5 DIP inset, just applied
in the wrong layer (also hitting clips, which is why it net-regressed the board overall).

**Root cause of the missing inset, for the record:** `include/maui/graphics/shapes/{rectangle,round_rectangle,
ellipse}.hpp` are documented "SIMPLIFIED PORT"s whose header comments state the shape's own default
`StrokeThickness` is `0`. It is not — `Shape.cs:80-81` defaults it to `1.0`. That incorrect premise is why
this port never had ANY shape-level self-inset anywhere. Still not fixed *at the shape layer* (the shared
layer is the wrong home per the retraction above) — but the three header comments themselves were corrected
in the cross-platform pass below, so a future reader no longer re-derives the same wrong premise.

**`borderless`'s OUTER edges also confirm zero inset** (not just the pink/red interior seam above): the
top edge (title bar -> pink), left/right edges (window frame -> pink), all measured bit-identical between
MAUI and cpp at `captures/windows/{maui,cpp}/borderless_light.png`, zero blended pixels. This was the
load-bearing claim behind the `thickness > 0` gate and it is now confirmed two independent ways, not one.

**UNVERIFIED: `radio_template_from_style` (StrokeThickness=0.5) is a real, unresolved risk.** At 0.5 the
extra 0.5 DIP inset is proportionally the largest of any tested case (the stroke's own thickness), but its
rendered stroke/fill color is only 1-3 levels off the page background (244,244,244 vs 243,242,241) — an
8-bit color depth too coarse to resolve a sub-pixel alpha shift this small; the centroid technique that
settled T=1/5/10 and T=0 is inconclusive here by measurement floor, not by finding "no effect". This page
is currently green (0.06%/0.16%). Do not assume the fix is neutral here; recapture and check first.

**Expect the change to move roughly 15 pages, not 3.** Every Border with a nonzero stroke goes through this
code path: `alignment` (5), `border_alignment`, `border_clip_playground` (slider), `border_layout` (5),
`border_playground` (5, slider), `border_resize_content` (8), `border_stroke` (1/5/10), `border_styles`,
`borderless` (0, expected unaffected), `carousel` (2), `containers` (2), `custom_swipe_item_view`,
`invalidate_shadow_host` (4), `radio_template_from_style` (0.5, see above), `swipe_view_shadow` (3),
`varied_size_selector`. Read the next board pass expecting broad movement across this list, not just the
three pages this task targeted — an unmoved page outside {`borderless`, `shapes`} would itself be a signal
worth checking, and a moved page inside this list is the fix working, not a new regression to chase.

---

## TASK 1 SOLVED (2026-07-31): the dark/light background delta is a MISSING CONTENT LAYER

Mechanism identified, oracle-grounded, no hard-coded colour. The user authorised deeper instrumentation;
this is its result.

### What the delta actually is

It is NOT the window background, and NOT colour management. Measured on `box_view` (modal colour of the
title-bar band vs the page body):

           title bar    body      delta
  MAUI light   232       244       +12
  MAUI dark     32        39        +7
  port  light  243       243         0
  port  dark    32        32         0

MAUI paints the page content on a translucent LAYER over the window base. The port paints the content
with the window base itself, so it has no layer at all. (The port's light TITLE BAR is also wrong -- 243
where MAUI is 232 -- which had not been noticed before.)

### The named mechanism

WinUI's own `generic.xaml` (WindowsAppSDK 1.7.250606001, `lib/uap10.0/Microsoft.UI/Themes/generic.xaml`):

  line  204  <Color x:Key="SolidBackgroundFillColorBase">#202020</Color>      (dark)
  line 5783  <Color x:Key="SolidBackgroundFillColorBase">#F3F3F3</Color>      (light)
  line 2019  <Color x:Key="LayerFillColorDefault">#4C3A3A3A</Color>           (dark)
  line 7598  <Color x:Key="LayerFillColorDefault">#80FFFFFF</Color>           (light)
  line 2433  <StaticResource x:Key="NavigationViewContentBackground" ResourceKey="LayerFillColorDefaultBrush" />
  line 8015  ... same for light

MAUI's Windows shell hosts page content in a NavigationView (`WindowRootView.cs` references
`NavigationViewControl`), whose content area is painted `NavigationViewContentBackground` ==
`LayerFillColorDefaultBrush`. Composited over the measured base:

  dark   32 + (76/255)*(58-32)   = 39.75  -> observed 39   MATCH
  light 232 + (128/255)*(255-232)= 243.55 -> observed 244  MATCH

(The two round in opposite directions at the 0.5 boundary; both land within 1 level, versus the current
error of 7 and 12. Exact 8-bit rounding is worth confirming once implemented, not assumed.)

### Why this was ruled out before, and why that ruling was wrong

The earlier sweep recorded "NavigationViewContentBackground: dark 17, light 255 -- worse in both" and
retired it. Those numbers are reproduced EXACTLY by compositing that brush over the WRONG base:
over BLACK, dark gives (76/255)*58 = 17.3 -> 17; over WHITE, light saturates to 255. The brush is
translucent, so measuring it without the window base underneath measures nothing. It was a measurement
artifact, not a refutation -- the resource was right all along.

Likewise "any single translucent overlay is algebraically impossible -- needs c=278" is only true if the
SAME (colour, alpha) is assumed in BOTH themes. Theme brushes differ per theme; solving the two equations
independently is satisfiable, and `LayerFillColorDefault` satisfies both.

Also newly established, and worth keeping: **explicit colours are bit-identical between the columns.**
Sampling every distinct MAUI colour and taking the modal port colour at those same pixels gives exact
matches for (100,149,237), (128,0,128), (144,238,144), (255,165,0), (173,216,230), (0,0,0) -- while the
theme greys are off by exactly 7 at three different levels (39->32, 52->45, 55->48, i.e. the base plus
alpha-composited layers inheriting it). That rules out gamma/ICC/colour-management as a class.

### What to implement (NOT a hard-coded #272727)

Paint the port's page-content root with the `NavigationViewContentBackground` / `LayerFillColorDefaultBrush`
THEME RESOURCE, so it resolves per theme automatically, over the existing window base. Separately fix the
light title-bar base (port 243 vs MAUI 232). Then rescore: ~129 dark pages currently sit at diff<=1% but
SSIM<0.98 and are gated on exactly this, plus 36 light pages.

---

## Rounded-rectangle CLIP coverage: works, but held back by an unexplained `clipping` regression (2026-07-31)

`view_chrome_ops.cpp`'s `build_geometry()` covers only ellipse + rectangle; its own header states rounded
rectangles are unsupported, and an unsupported shape returns nothing, so the caller **leaves any existing
clip alone** — i.e. a RoundRectangle/RoundRectangleGeometry `Clip` currently applies NO CLIP AT ALL on
Windows. A branch was written using `Compositor::CreateRoundedRectangleGeometry` for the uniform-radii
case only (non-uniform radii genuinely cannot be expressed by its single `Vector2` CornerRadius, so those
still fall through to unsupported rather than silently approximating). It COMPILES on the guest, so the
API shape is confirmed real.

Measured (light, cpp column; movement attributed on both sides):

  clip_gallery            3.02% -> 1.46%  (-1.56)   cpp moved 22844, maui 0   REAL WIN
  clip_corner_radius      2.16% -> 0.60%  (-1.56)   cpp moved 22650, maui 0   REAL WIN
  clipping                0.00% -> 4.89%  (+4.89)   cpp moved 40000, maui 0   REGRESSION
  clip / border_clip_playground   unchanged, 0 px moved either side

NOT IMPORTED and REVERTED pending diagnosis — net effect is ~zero-to-negative on green, and `clipping`
was exactly 0.00%.

### What the `clipping` regression is NOT

- Not the clip *type*. The page passed a bounds-relative `graphics::shapes::round_rectangle` where the
  C# names `new RoundRectangleGeometry { CornerRadius = 8, Rect = (0,0,400,50) }` — an ABSOLUTE rect.
  Switching it to `controls::shapes::round_rectangle_geometry` with that explicit rect left the page at
  **exactly 4.89%**, unchanged. So the geometry choice is not the cause (though the absolute type IS the
  faithful port of the C# and should be adopted whenever this is fixed).
- Not the toggle state. The capture is the `initial` frame, `clipping` has no scenario file, the ctor
  never calls `on_toggle_clip()`, and the status label starts "Not clipping" — so no clip should be
  applied at capture time at all.

### The actual clue

The changed pixels are **exactly 40000 = 200x200**, at rows 123-322, cols 512-711. That is the
translucent-red `BoxView` overlay in the overflow Grid's second column — **not** the 8-button row that
the clip is attached to. So enabling rounded-clip support changes the rendering of a sibling BoxView, not
the clipped row. That points at something shared — most likely the `clips_to_bounds` / layout clip path
(`layout_handler.cpp`'s ClipsToBounds rebuilds its clip rectangle through the same helper), or the
null-shape path no longer clearing an existing clip — rather than at the new rounded branch itself.

Next diagnostic (cheap, and it ANSWERS rather than predicts): capture `clipping` with the rounded branch
present but with the BoxView overlay temporarily removed, or dump the clip actually installed on each
element. Do not re-land the branch until that 200x200 square is explained; the two wins above are real
and worth recovering once it is.

### CONFIRMED BY CAPTURE (2026-07-31): the layer fix lands DARK EXACTLY

`da6ed0bb0c` (host_run.cpp step (3d), `NavigationViewContentBackground` looked up as a theme resource,
no literal colour) built on the guest and was captured. Measured modal colours, title-bar band vs body:

              title bar   body
  MAUI  light    232       244
  port  light    243       249     <- layer CORRECT, base wrong
  MAUI  dark      32        39
  port  dark      32        39     <- EXACT

Whole-image mean absolute error, both verification pages:

  box_view      dark 0.03   light 4.50
  chat_example  dark 0.02   light 5.05

Dark was ~7.0 on every background pixel before this change; it is now ~0.02. The hypothesis is confirmed
by capture, not by argument.

**The layer composites correctly in BOTH themes.** Light is wrong only because the port's window BASE is
243 (`ApplicationPageBackgroundThemeBrush`, #F3F3F3) where MAUI's is 232. With a 232 base the arithmetic
completes exactly: 232 + (128/255)*(255-232) = 243.55 -> 244 = MAUI's observed body, and the base itself
becomes the title-bar band, fixing the separate 243-vs-232 title-bar defect in the same stroke.

The remaining piece is therefore the light window base, for which Mica `BaseAlt` is the candidate —
previously measured at exactly light 232 / dark 32 (dark unchanged, so it cannot regress the now-exact
dark result). It was retired earlier as "worse" because it was measured WITHOUT this layer, leaving the
body at the bare 232 against MAUI's 244.

**That is now the THIRD component of this one mechanism retired for being measured in isolation**
(`NavigationViewContentBackground` -> measured over black/white instead of over the base; the "c=278
impossible" overlay -> assumed one brush for both themes; Mica BaseAlt -> measured without the layer).
The lesson is specific and worth keeping: a composited effect cannot be evaluated one component at a
time. Judge the COMPOSED result, or the measurement will be true and the conclusion false.

---

## `clipping` 0.00 -> 4.89: a DOUBLE-COMPOSITE of translucent content (2026-07-31) — MISATTRIBUTED EARLIER

Correction to the entry above titled "Rounded-rectangle CLIP coverage ... unexplained `clipping`
regression". That entry blames the rounded-rect clip branch for the 200x200 square. **That attribution was
wrong.** The clip branch is reverted (verified absent from HEAD *and* from the guest source tree, 0
occurrences of `round_rectangle_geometry` in both), yet `clipping` still scores 4.89% on the full board.

**What it actually is.** In the diff region (rows 95-322, cols 512-711, exactly 40020 px) both columns
have the SAME pixel count of the affected colour; only the value differs:

    MAUI  (255, 82, 0)   36778 px
    port  (255, 41, 0)   36778 px

That is the page's translucent red BoxView (`Background=Red`, `Opacity=0.5`) over the orange page
background (255,165,0):

    correct    0.5*0   + 0.5*165 = 82.5 -> 82     = MAUI
    port       0.5*0   + 0.5*82  = 41.0 -> 41     = one EXTRA 0.5 blend

So the port composites the translucent overlay TWICE. It is an OPACITY bug, not a clip bug — which is why
reverting the clip work did not and could not fix it, and why the symptom survived that revert.

**When it appeared.** `clipping` was 0.00% before this session's background work. It is therefore a
regression from the content-layer / Mica change (`da6ed0bb0c` + `47b97adf18`), not from the clip branch.
Plausible mechanism to check first: with the opaque `(3c)` base paint now skipped under Mica, a surface
that used to be composited once against an opaque background is being drawn against a transparent one and
picking up an extra blend — i.e. the page background and the layer are both contributing. Check whether
the page's own view and the `(3c)` panel are BOTH painting, and whether `Opacity` is applied at two levels.

**Scope.** Only pages with translucent content over a page background are at risk. `clipping` is the one
currently caught; a sweep for `set_opacity` + an explicit page background would bound it. This does NOT
affect the background mechanism itself, which is measured exact in both themes on opaque pages.

## `activity_indicator` — NOT animation-phase noise; a real Fill/container defect, FIXED (2026-07-31)

Windows board's #2 worst page (light 4.17% SSIM 0.977, dark 4.48% SSIM 0.941), flagged as possibly
VOLATILE going in (ProgressRing is an animating indeterminate spinner, so two captures at different
animation phases can legitimately differ). Diagnosed before touching any code, per two decisive checks.

**Check 1 — is the port's own render deterministic?** MD5 of `activity_indicator_light`'s `cpp` capture
across every dated run under `docs/comparison/2026-07-*` where the handler file was unchanged (all of them
— it was last touched at `d2c70a0803`, before every run compared):

    2026-07-29-01_22_51  f7b60f857738714b473914845101a062
    2026-07-29-03_47_17  f7b60f857738714b473914845101a062
    2026-07-29-05_30_30  f7b60f857738714b473914845101a062
    2026-07-29-07_42_14  f7b60f857738714b473914845101a062
    2026-07-29-17_10_40  f7b60f857738714b473914845101a062
    2026-07-29-18_27_25  f7b60f857738714b473914845101a062

Six independent runs spanning many hours, byte-identical. (The three other MD5s seen across the full
history change lockstep with unrelated page/background commits, not randomly — consistent with a
deterministic capture, not a spinning-phase race.) This refutes "animation phase noise" as the
explanation for the page's SCORE — a genuinely phase-random capture would not be reproducible six times
running.

**Check 2 — where is the diff, and how much of it is which shape?** Region breakdown of the maui-vs-cpp
pixel diff (light theme, 34346 px total / 4.19%):

    Default ring (y78-118)                 155 px   0.5%
    Color ring (y145-185)                  153 px   0.4%
    Yellow BackgroundColor bar (y195-265)  31347 px  91.3%
    Larger ring (y280-435)                  995 px   2.9%
    Smaller ring + downstream label shift  1696 px   4.9%

Dark theme (36804 px / 4.49%) matches the same shape: yellow bar 85.3%, Larger-ring 8.8%, Smaller+shift
4.6%, Default/Color rings 0.5%/0.5% each. The dominant defect by two orders of magnitude is the
`BackgroundColor="Yellow"` row, not ring position or ring size (the brief's own steer — "the ring
measures 32 in both columns" — is correct about the ring, and misleading as guidance: the real defect is
one level up, in how much of the ROW the container occupies, not how big the ring drawn inside it is).

**Root cause.** `View.HorizontalOptionsProperty`'s default is `LayoutOptions.Fill` (`View.cs:31-34`), and
neither `ActivityIndicator.cs` nor the XAML overrides it, so `LayoutExtensions.ComputeFrame`/
`AlignHorizontal` hands `platform_arrange` a `frame` as wide as the whole `VerticalStackLayout` row
(~984 DIP of a 1024-wide window). Measured on the light capture: MAUI's default-size ring sits horizontally
CENTRED (x-centre ~504 of that row) and its Yellow background spans the full row (x 20..1003). The
unfixed port pinned `Canvas.SetLeft(ring, frame.x)` — the row's LEFT edge — with no centring math, and
painted `Background` directly onto the bare ring (ProgressRing IS a `Control`, so `update_background`
"worked" for colour), so the port's ring sat at x-centre ~32 and its yellow patch spanned only x 21..50: a
small square hugging the ring at the left, instead of MAUI's full-width bar. MAUI achieves the wide bar
(and the ring recentring inside it) via `ActivityIndicatorHandler.Windows.cs`'s `NeedsContainer` — when
`Background != null`, MAUI wraps the `ProgressRing` in a `WrapperView`/`ContainerView` that owns the
Fill-arranged frame and Background paint, with the ring centred as its content. This port's Windows
backend had never ported that container seam for ActivityIndicator (its own file-header comment said so
explicitly) — `label_handler.cpp` and `image_handler.cpp` had already closed the identical gap for
TextBlock/Image on this Canvas-based backend by wrapping unconditionally in a chromeless `Border`.

**The XAML column independently corroborates the same defect** (not just cpp): `xaml`'s
`activity_indicator_{light,dark}.png` shows the identical shape — ring x-centre ~28.5 (left-aligned, not
~504/~511 like MAUI), yellow bar x 21..50 (not the full row) — in BOTH themes. Since `cpp` and `xaml`
route through the same `src/platform/windows/activity_indicator_handler.cpp`, this is conclusive evidence
the defect is handler-level, not page-construction — one fix should move both board columns
(`pixel` 4.17%/4.48% AND `pixel_xaml` 4.44%/4.23%).

**Fix landed** (`port/cpp/src/platform/windows/activity_indicator_handler.cpp`, commit pending): wrap the
`ProgressRing` in a `Border` host unconditionally, mirroring `label_handler.cpp`/`image_handler.cpp`
exactly — `native` now boxes the host; `as_ring()` resolves `host.Child()`. `create_platform_view` sets the
ring's own `HorizontalAlignment`/`VerticalAlignment` to `Center` (unlike label/image's `Stretch` default,
since a ring should never distort to fill space). `platform_arrange` now stamps the HOST's Width/Height to
the resolved `frame` unconditionally (frame.width already resolves to the right value for both the
Fill-unset and the explicit-WidthRequest case via the shared `compute_frame`, so no extra branching is
needed there), while the RING's own Width/Height keep the pre-existing explicit-only pin (a ProgressRing
has no Image-style Stretch/Aspect knob, so only an explicit WidthRequest/HeightRequest may resize the
glyph itself — this is what keeps the "Larger"/"Smaller" rows' explicit 150×150 / 10×10 requests rendering
at their current, already-correct sizes). `update_is_enabled`/`update_automation_id` are redirected to
resolve the ring specifically (not the new host) — `apply_is_enabled`'s `try_as<Control>` silently no-ops
on a `Border`, and unlike label/image (whose wrapped content was never a `Control` either), this port's
ActivityIndicator's IsEnabled currently DOES work (ProgressRing IS a `Control`), so redirecting avoids a
new regression rather than accepting label/image's pre-existing gap. `apply_native_clip`
(`view_chrome_ops.cpp`) already special-cases a Border-boxed native with no changes needed — its own
comment anticipates "any future Border-host handler."

**Predicted outcome** (unverified — this Mac cannot compile or run the Windows backend; the next VM
rescore should confirm): the 91%/85% yellow-bar component and the 0.5%/0.4% ring-centring component both
resolve, predicting roughly light 4.19% → ~0.3-0.4%, dark 4.49% → ~0.5-0.6% — i.e. GREEN, driven mostly by
the two residuals below staying open. Do not treat a from-code prediction as a landed score; rescore before
updating the board.

**Two residuals NOT fixed by this change** (small, and NOT the reason this page was investigated):

1. **Larger-ring arc-shape mismatch (~3-9% of the diff).** The `WidthRequest="150" HeightRequest="150"`
   row's visible arc bounding box differs between MAUI and cpp (e.g. light: MAUI x440-511/y356-425 vs cpp
   x454-536/y387-425) beyond what position alone explains. Both rings ARE indeterminate spinners — unlike
   the page-level determinism proven above (which held across independent RUNS of the SAME app), this is
   MAUI-vs-cpp at a single instant, where the two apps' animation timers are not synchronized — so this
   looks like genuine phase variance in miniature, isolated to the one row big enough for the arc's sweep
   angle to matter visually. Not investigated further; flagged for whoever picks this back up.
2. **6px downstream vertical shift below the `Smaller` row (~5% of the diff).** The `NotRunning`/`-End of
   page-` labels sit 6px higher in cpp than in MAUI (measured on light: y486 vs y492, y553 vs y559), while
   every label above (`Larger`, `Smaller`) matches exactly. The `Smaller` row is
   `WidthRequest="10" HeightRequest="10"` — an explicit, non-animated size that should not vary by
   animation phase at all, so a fixed 6px height difference between columns suggests MAUI's `ProgressRing`
   honors a minimum-size floor (e.g. `MinHeight`) below which an explicit HeightRequest cannot shrink it,
   and this port's ring does not. Not fixed here (out of scope for the container defect this entry
   diagnoses); worth a follow-up if this page doesn't fully clear after the container fix rescopes it.

**Blast radius checked.** Two other Windows-board pages construct an `<ActivityIndicator>`:
`controls_stack.xaml` (`IsRunning="True"`, no Background, inside a `HorizontalStackLayout` — currently
GREEN 0.18%/0.21%) and `value_controls.xaml` (`IsRunning="False"`, not on the Windows board). For
`controls_stack`, this fix changes the ring's CROSS-axis (vertical) centring within that row — visually
checked against both captures and the row's siblings (CheckBox/Switch) are close enough in natural height
to the ring's own ~32px that the expected shift is a few px at most, well under what would move a
0.18%-diff page out of green. `value_controls` isn't tracked on the Windows board, so it is not a scoring
risk either way.

**Verification performed on this Mac:** `check_winrt_includes.py` (0 problems), `clang-format --dry-run`
(clean), brace/paren balance. The Windows backend cannot be compiled here — no build or runtime
verification was done or claimed; the numbers above are static analysis of committed capture PNGs plus
manual review against the already-compiling `label_handler.cpp`/`image_handler.cpp` precedent this fix
mirrors.

---

## The "passes diff, fails SSIM" cluster: 22/24 pages are the known focus-visual noise floor, not a port defect (2026-07-31)

Investigated a cluster the brief defined by predicate `diff<=1.0% on both themes AND SSIM<0.98 on either`
against `comparison.json`'s `pixel` slot — enumerated mechanically, **24 pages, not the ~20 estimated**:
`multiple_bound_selection, layout_is_enabled, modal, input_transparent, nested_collection,
preselected_item, picker, selection_synchronization, ios_picker, navigation_gallery, ios_pan_gesture,
ios_safe_area, label, ios_scroll_view, menu_bar, ios_slider_update_on_tap, footer_only_string,
header_footer, progress_bar, items, search_bar, header_footer_grid_horizontal, header_footer_grid,
title_bar`.

**This is the same noise floor the "MAUI's own CollectionView captures carry a ~0.50pp focus-visual noise
floor" and "NOT confined to CollectionView pages" sections above already identified (2026-07-30) — this
entry corroborates it on a disjoint, mostly-non-CollectionView page set and corrects/sharpens three of its
claims.** It is not new; treat this as confirmation + generalization, not a new finding.

**Corroboration, done independently before re-reading the sections above.** For each page, connected-
component-labeled the maui-vs-cpp diff mask (4-connectivity, `>25`/255 per-channel threshold). Nearly
every page's #1 component is a thin (2-4px) rounded-rectangle OUTLINE anchored at the top of the page,
full- or partial-width depending on the focused control's width, magnitude ~216-218/255 (i.e. genuinely
near-black-vs-near-background, not antialiasing noise). Visually this is a WinUI keyboard-focus visual
around the page's first focusable control in tab order — `label`'s "Change Formatted String" `Button`,
`modal`'s "Push Page" `Button`, `picker`'s first `Picker` (rendered as a `ComboBox` with a text-insertion
caret, confirming it is genuinely *focused*, not just decorated), `layout_is_enabled`'s "Enabled" `Button`
in a half-width column. **This falsifies the investigation brief's own steer that CollectionView might be
the shared thread** — `label`, `menu_bar`, `progress_bar`, `ios_safe_area`, `ios_scroll_view` etc. show
the identical band and contain no `CollectionView` at all; it lands on the first tab-stop, whatever type
it is.

**Determinism check, both directions, matching this file's established method (hold the port capture
constant, watch the reference):**

    cpp  vs cpp,  picker,  4 consecutive dated runs:  0, 0, 0 px        (bit-identical every time)
    maui vs maui, modal/label/multiple_bound_selection, SAME 4 run-boundaries:
      07-29 07:42 -> 07-29 21:47   4164 / 4088 / 4081 px  (~0.50%)
      07-29 21:47 -> 07-30 04:03      0 /    0 /    0 px
      07-30 04:03 -> 07-30 08:51   4164 / 4088 / 4081 px  (~0.50%)
      07-30 08:51 -> 07-31 02:01      0 /    0 /    0 px

Three unrelated pages toggle at **exactly the same run boundaries**, by nearly the same pixel count each
time, while the port is bit-identical throughout. That is stronger than "present/absent, membership
changes between runs" (this file's earlier, correct characterization from a 43-page CollectionView
sample) — on this 3-page sample the toggle is in lockstep, suggesting whatever decides it may be a
per-capture-*session* condition (e.g. whether the VM/window had OS-level input focus at the moment that
batch was screenshotted) rather than a fully independent per-page coin flip. Both observations are
measured, on different page sets, and not necessarily in tension — a session-level factor could still
produce page-level variation if a page's own Loaded timing races the batch boundary. Recording as an open
mechanism question, not resolving it.

**Quantified explanatory power.** For each page/theme, patched the top-2 connected components (the focus
band + — dark only — a ~50-100px titlebar caption-button hover-highlight cluster at `y11-20,x880-1005`,
present on every dark capture, independently confirmed stable-vs-absent the same way) from `maui` onto
`cpp`'s pixels, then rescored with the exact `ssim()`/`diff_pct` from `tools/parity/pixel_score.py`
(reused, not reimplemented):

  **22 of 24 pages go fully green (both themes) once those two components are neutralized.** The 2 that
  don't are real, separate, page-specific port defects (below) — not part of this noise floor.

**Why 0.5% of pixels costs 3pp of SSIM (the brief asked for this):** SSIM is an 11×11-windowed local
score; a full-width 2-4px band touches on the order of 6% of the image's windows at a near-zero local
score (near-black vs. near-white inside the window), and even a modest fraction of near-zero windows pulls
the frame-mean SSIM down disproportionately to the raw pixel fraction. That the diff stays under the 1%
gate while SSIM crosses under 0.98 is exactly this metric's known behavior for a thin, high-contrast,
spatially-concentrated defect — consistent with, not contradicting, the earlier "0.03 SSIM cost" note
above.

**Dark-worse-than-light, explained (the brief flagged this as a possible clue):** two additive, both
reference-side: (1) the focus band itself is taller/darker in the dark-theme render (~6100px vs ~4100px
light — same control, more contrast against the dark background), and (2) a second, dark-only ~50-100px
cluster at the window's minimize/close caption buttons (`y11-20`), present on every dark page checked and
absent on every light one — looks like a stray hover/press highlight on the OS-drawn titlebar chrome, not
app content. Confirmed present/absent the same cross-run way as the focus band.

**NOT ACTIONABLE as a port fix**, same conclusion and same mitigation menu as the sections above (suppress
focus before capturing the MAUI column is the standing recommendation) — this entry adds no new mitigation
option, just a second, independent measurement supporting the existing one.

### The 2 pages NOT explained by the noise floor — separate, real, page-specific defects

**`layout_is_enabled` — child-order bug, FOUND AND FIXED (commit `38fe6277b7`).** Its own top component IS
the focus band (partial-width, ~3065px, on the "Enabled" button in the left/top column — missed by a
width-heuristic on the first pass, confirmed present by direct visual inspection and by connected-
component analysis). But even after neutralizing that band the page still misses green by a second,
smaller (~few-hundred px) component: `examples/gallery/pages/layout_is_enabled_page.hpp`'s
`build_right_controls()` added all three row captions ("Enable/Disable Layout/Button/Command") FIRST, then
all three `CheckBox`es afterward, instead of interleaving them — so the port's right-column
`VerticalStackLayout` rendered three captions stacked with no checkbox between them, then three checkboxes
bunched at the bottom, while `LayoutIsEnabledPage.xaml:109-114` (the oracle) interleaves
`Label,CheckBox,Label,CheckBox,Label,CheckBox`. Fixed by moving each `right_controls_.add(check)` to
directly follow its caption (3-line diff, pure reordering of already-valid statements, no new types or
signatures). **Confirmed correctly scoped two ways:** (a) `docs/comparison/captures/windows/xaml/
layout_is_enabled_{light,dark}.png` — the compile-time-XAML twin, a completely independent authoring/
render path — already shows the correct interleaved order, so this was a code-first-builder-only bug, not
shared with the XAML loader (no companion fix needed there); (b) the rescore above shows patching just the
top-2 components (focus band + this cluster) reaches green using the OLD, unfixed captures, i.e. the
region my fix targets is exactly the page's second-largest diff contributor. **This fix will NOT green the
page on its own** — the dominant ~3000px component is the same unfixable reference noise as the other 22
pages; the next capture will very likely still show `layout_is_enabled` yellow, just with a smaller
number. Since this page file compiles into every backend's gallery app, the fix also affects
iOS/macOS/Android captures of this page, not just Windows.

**`selection_synchronization` — CollectionView checked-cell layout, NOT fixed (reserved territory).** The
`CheckBox` in each selectable row's cell is positioned mid-label-text in `cpp` (`"It[box]em 1"`) instead of
before all of it (`maui`: `"[box]Item 1"`) — a real, deterministic (stable across the same run set) cell-
layout defect, visually distinct from both the focus-band pattern and from `layout_is_enabled`'s ordering
bug. This is CollectionView item-cell chrome, adjacent to `border_handler.cpp`/`view_chrome_ops.cpp`,
which this task was explicitly told not to touch (another agent's territory) — documented here, not fixed.

**`title_bar` — CheckBox-to-Label gap, NOT fixed (untestable on this Mac).** Every `check_row` on this page
(`HorizontalStackLayout` of `CheckBox` + `Label`, structurally identical to the C# XAML) renders the
checkbox glyph at an IDENTICAL position/size to MAUI in both columns, but the label starts ~88px further
right in the MAUI reference than in `cpp` — stable across all 5 runs checked on the MAUI side (0px
run-to-run diff), so this is a real, repeatable behavioral difference, not capture noise. Likely cause:
`CheckBoxHandler.Windows.cs`'s `AdjustCheckBoxForNoText` (faithfully ported at
`src/platform/windows/check_box_handler.cpp:150-175`) sets `MinWidth/MinHeight=0` synchronously but defers
shrinking the template root `Grid`'s margin to a `Loaded` handler; if real WinUI's `Loaded` fires before
the control template is realized (`VisualTreeHelper.GetChildrenCount(checkBox) <= 0`), the margin-shrink
silently no-ops forever, leaving the wider default margin — a race in the ORIGINAL C# this port's `Loaded`
analog may not reproduce. **Not fixed**: this is a hypothesis about real WinUI timing, not something
verifiable without compiling and running on Windows (explicitly out of scope for this Mac), and
deliberately reproducing a race rather than a deterministic behavior is a bad trade for one page.
`header_footer_grid`/`header_footer_grid_horizontal`'s dark-theme residual (the two pages that reach green
in light but fall just short in dark after patching the top-2 components) likely share this same
CheckBox-gap cause — both pages contain `CheckBox` rows — but this was not separately confirmed.

**Verification performed on this Mac:** `check_winrt_includes.py` (0 problems, no Windows/WinRT files
touched by the one fix). The `layout_is_enabled_page.hpp` fix could not be compiled on this Mac (no
configured `examples/` build tree for any backend in this worktree) — reviewed by hand: pure statement
reordering, all three `right_controls_.add(...)` calls and their member types were already valid at their
old call sites, so this carries effectively no syntax/type risk. No Windows backend build or run was
performed or claimed anywhere in this entry.

---

## Focus-visual suppression: mechanism WORKS, but it KILLS the capture transport (2026-07-31)

The user authorised suppressing the reference column's WinUI keyboard-focus visual. Implemented as an
OS-foreground handoff (`SetForegroundWindow(GetShellWindow())`) immediately before each `--shot`, inside
`cmd_present` so all three columns get it identically.

**It works, and it is not safe to leave on.** Both halves are measured:

WORKS -- `label` light, maui-vs-cpp: **0.50% -> 0.01%**, with 6228 reference px moved (the focus outline
plus inactive title-bar chrome). The band is gone. The hypothesis was correct.

KILLS THE RUN -- the same run then died. Requested `label,modal,picker`; only `label` completed, and even
its `cpp_xaml` column was dropped:

    ~ present failed (empty reply from agent) -- resolution-toggle self-heal, retrying
    ~ present failed (transport: ConnectionRefusedError: [Errno 61] Connection refused)
    ! label/cpp_xaml/light#1: DROPPED -- present failed after self-heal (no window to capture)
    ! modal/{maui_xaml,cpp,cpp_xaml}/light:  launch failed: ConnectionRefusedError
    ! picker/{maui_xaml,cpp,cpp_xaml}/light: launch failed: ConnectionRefusedError

Attribution is clean: every prior run on this guest captured its full set (the 2026-07-31-02_01_46 full
board took 1104/1104 with 0 failures). The FIRST run with defocus enabled died after one page. Handing OS
foreground to the shell evidently tears down the session-1 agent's own desktop session -- which is exactly
the environment the loop's own notes warn about (an app in session 0 has no desktop and never reaches
OnLaunched; the agent depends on holding a real session-1 desktop).

**Action taken: the flag is now OPT-IN (`--defocus`), default OFF**, so the harness can always complete a
run. The unit test was updated to encode the safe default and the reason. Do NOT enable it for a scoring
run until the transport teardown is solved.

**Worth noting the harness behaved WELL here:** it did not silently drop pages. It reported each failure
loudly, self-healed once, and exited non-zero-ish with an explicit DROPPED line. That is why this was
caught in a 3-page verification run instead of corrupting a 1104-shot board.

**If someone retries this**, the promising direction is a suppression that does NOT touch OS foreground:
WinUI draws the focus visual only for `FocusState::Keyboard`, so setting focus to `FocusState::Pointer`
(or clearing it) INSIDE the app -- e.g. a diagnostic switch in the MauiReference app and the gallery,
compiled in for capture builds -- would suppress the outline without disturbing the window manager or the
agent's desktop. That keeps the fix in-process, where it cannot break the transport.
