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
