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

#### CROSS-PLATFORM (2026-07-31, later): the same inset was missing on iOS, Catalyst, AppKit and Android

The Windows fix above was scoped to one backend. It is not a Windows defect — the missing inset comes from a
SHARED wrong premise (the three `graphics::shapes::*` headers), so every backend whose MAUI counterpart
routes a Border's geometry through `Shape.PathForBounds` had it too. All four were checked against `src/`
and then measured; three needed the fix, and the fourth turned out not to be exempt after all.

**Oracle, per backend.** Where each backend's MAUI counterpart gets the Border's path:

| backend | MAUI chain | routes through `Shape.PathForBounds`? |
|---|---|---|
| Windows | `BorderExtensions.UpdatePath` | yes (already fixed, `229c66a407`) |
| iOS / Catalyst | `MauiCALayer.GetClipPath()` -> `shape.PathForBounds(bounds)` (`MauiCALayer.cs:307-313`), consumed by BOTH `DrawBackground` and `DrawBorder` | **yes** |
| macOS AppKit | no MAUI counterpart; twin of the iOS ops file, ground truth is the Catalyst MAUI column | **yes** (by construction) |
| Android | `MauiDrawable.UpdateClipPath` -> `_shape.ToPlatform(bounds, sw, density)` (`MauiDrawable.Android.cs:410`) -> `shape.PathForBounds(pathBounds)` (`ShapeExtensions.cs:34`) | **yes** |

One dead end worth recording so it is not re-walked: `src/Core/src/Platform/Android/BorderDrawable.cs` looks
like the Android Border drawable and builds its path from raw geometry (`GetPath`, line 494) with no
`IShape` involved at all — which reads as "Android is exempt". It is **not** the class `Border` uses.
`StrokeExtensions.UpdateMauiDrawable` instantiates `MauiDrawable` (`StrokeExtensions.cs:136-146`), which
lives in `src/Core/src/Graphics/MauiDrawable.Android.cs`, not under `Platform/Android/` — which is why a
`grep PathForBounds src/Core/src/Platform/Android/` misses it entirely.

**Measurement.** Same subpixel technique, re-derived per platform rather than reusing the Windows numbers.
Four rays walk outward from a seed inside each Border on `border_stroke_light`; each crosses the red stroke
into the page background, so the outer edge is a clean two-colour red/white ramp and a ramp pixel's red
coverage is exactly `1 - G/255`. With pixel `k` covering `[k, k+1)` in outward-step coordinates the edge is
`e = k_lastPureRed + sum(coverage)`. The SAME seed is used for both columns, so `e_cpp - e_maui` is the
shift with no alignment assumption. Delta in DIP, `+` = the port's stroke sits further OUT:

    platform          px/DIP   T=1 left/right  T=5 left/right  T=10 left/right  outermost top/bottom
    ios                3.0     +0.50 / +0.50   +0.50 / +0.50   +0.50 / +0.50     +0.50 / +0.50
    android            2.75    +0.50 / +0.50   +0.50 / +0.50   +0.50 / +0.50     +0.50 / (n/a)
    windows (pre-fix)  1.0     +0.50 / +0.50   +0.50 / +0.50   +0.50 / +0.50     +0.50 / +0.50

Windows is listed because its committed `cpp` column still predates `229c66a407` (last touched in
`2bf6d6e6b3`): reproducing the exact signature the Windows commit already fixed is the **positive control**
for the technique. Constant in `T`, symmetric left-vs-right (a size shrink, not a translate), identical
across both grids on the page.

`maccatalyst` is measured differently on purpose. Its capture shows two-pixel ramps on hard edges in BOTH
columns — the column is resampled somewhere in the VM capture path — so its coverage deltas read
+0.21..+0.32 DIP and that magnitude is not trustworthy. The resampling-immune observation is integer-level
and unambiguous: adjacent Borders in the `*,*,*` Grid are separated by a **2-row gap** in the MAUI column
and **abut exactly** in the cpp column (`maui` runs `81..101`, `104..132`; `cpp` one run `80..133`). Two
adjacent Borders each pulling in 0.5 DIP is exactly a 1 DIP gap. iOS shows the same thing at 3x (`maui`
`311..372`, `377..462`; `cpp` one run `309..584`). Catalyst compiles the *same* `ios_border_ops.hpp` the
iOS column proves, so it is fixed by construction, not by its own weaker measurement.

**Negative control at StrokeThickness = 0** (`borderless_light`, a Pink/Red Border pair filling a `*,*`
Grid): the Pink->Red transition row is **identical** between the MAUI and cpp columns on all three
platforms — ios `y=1353`, maccatalyst `y=415/416`, android `y=1205` — with no gap and no blended row. Zero
inset in both columns at T=0, on every platform. The `thickness > 0` gate is confirmed independently of
Windows.

**iOS/macOS needed a shape the Windows fix did not.** On Windows the Border's fill comes from a separate
`ContentPanel` background, so deflating the stroke geometry alone was correct. On iOS/macOS `MauiCALayer`
feeds ONE path — `GetClipPath()` — to both `DrawBackground` and `DrawBorder`, and the port mirrors that with
`apply_clip(...)` plus the `CAShapeLayer` stroke at double width with the mask cutting the outer half.
Deflating only the stroke path there would have moved its inner edge while the mask still cut at the
undeflated outer edge: a `thickness + 0.5` wide band instead of a `thickness` band shifted by 0.5 — and a
single-edge centroid check would have PASSED on that. So the inset is taken once, at the top of
`apply_border_stroke`, and feeds both. (The gate has to cover the `apply_clip` call too, which runs before
the `draws_border` early-return.)

**Android needed the content clip as well.** `ShapeExtensions.ToPlatform(..., innerPath: true)` splits: the
`IRoundRectangle` branch calls `InnerPathForBounds` directly (no `PathForBounds`, so **no** inset), every
other shape falls through to `shape.PathForBounds(Rect(1.5st, 1.5st, W-3st, H-3st))` (**inset applies**).
Measured on `border_stroke`, MAUI's orange content edge sits `+0.50` pt inward of the port's at both T=1 and
T=5 — the same constant as the stroke — so `border_content_inner_path_points`'s non-round-rect branch is
inset too, and its round-rectangle branch deliberately is not.

**What landed.** One shared helper, `maui::core::shape_self_inset(bounds, thickness)` in
`include/maui/core/border_handler.hpp`, carrying the whole derivation (including the `Border.cs:433-439`
latch) in one place instead of four copies; called from `ios_border_ops.hpp`, `apple_border_ops.hpp`, and
`android/border_handler.cpp` (`border_shape_path_points` + the inner-path non-round-rect branch). The three
`graphics::shapes::*` headers' "default StrokeThickness 0" claim is corrected in place — comment-only, the
shapes still deliberately do NOT self-inset, and each now points at the helper and at the `f1a5a17658`
revert so the premise cannot be re-derived. `view_chrome_ops.cpp` (Windows clips), `ios_visual_ops.hpp` /
`apple_visual_ops.hpp` `apply_clip` (the general `View.Clip` route) and `android_clip_ops.hpp` are all
untouched — MAUI never deflates those.

`ios_border_ops.hpp` has a THIRD `path_for_bounds` caller besides the mask and the stroke —
`border_shadow_silhouette_path` — and it is inset too, for the reason its own header already states: MAUI
sets no explicit `ShadowPath`, so the silhouette is whatever `MauiCALayer` DREW, and on iOS everything it
draws goes through the (now inset) `GetClipPath()`. Leaving it undeflated would cast a shadow 0.5 DIP
larger than the fill and stroke it belongs to. AppKit has no silhouette twin, so there is nothing to
mirror there. `swipe_view_shadow` and `invalidate_shadow_host` — both already in the movement list above —
are the pages to read this on; their improvement under the retracted shared-layer deflate is *consistent
with* this but is not proof of the silhouette specifically, since that attempt moved the mask and stroke
at the same time.

`controls::frame` routes through these same handlers with its own `graphics::shapes::round_rectangle`
default (`frame.cpp:114`), so Frame pages (`containers`) take the inset as well. Correct per oracle —
MAUI's `Frame` likewise carries a Controls RoundRectangle — but recorded here rather than left to read as
collateral.

**Verification.** `build/apple` builds clean, 3186/3187 ctest pass; `build/ios` builds clean;
`build/android`'s `border_handler.cpp.o` compiles clean. **Windows is compile-unverified on this host** —
its edit is a pure refactor onto the shared helper (identical arithmetic, same gate) plus a comment
correction, but it has not been through MSVC on the VM. The single apple failure,
`gallery_structure_equivalence.layout_is_enabled`, is a *de-list* assertion ("divergence closed — remove
'layout_is_enabled' from known_diverging()") on the gallery view-tree comparison, which never renders — it
belongs to the concurrent `IsEnabled` cascade work in this worktree, not to this change. The full
`build/android` target still fails before linking for an unrelated environment reason: the compile-time-XAML
codegen step runs `maui_xaml_codegen` on the emulator via `tools/android-emu-run.sh`, and the emulator
cannot open the host-side `tools/xaml_codegen/samples/*.xaml`. **No board recapture was run** — all four
`cpp` capture columns still show the pre-fix geometry, so the next board pass should move the ~15-page list
above on iOS, Catalyst, AppKit and Android too, not only on Windows.

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

### In-process suppression IMPLEMENTED, NOT BUILD-VERIFIED (2026-07-31, `2d224abf1f`)

Implemented the retry direction above, with one correction to it and one addition beyond it.

**Correction: the auto-focus is `FocusState.Programmatic`, not `FocusState.Keyboard`.** Read
`WindowRootViewContainer.AddPage` -> `TryMoveFocusToPage` -> `SetFocusToFirstElement`
(`src/Core/src/Platform/Windows/WindowRootViewContainer.cs`) rather than assuming: MAUI's own initial-focus
call is `focusableElement.Focus(FocusState.Programmatic)`, fired on every page add. WinUI's stock control
templates paint the keyboard-style outline for Programmatic focus the same as Keyboard focus -- only
Pointer/touch is exempt -- so the fix only relies on the Pointer-is-exempt half, not on the state being
literally Keyboard, and survives the correction.

**What shipped** (`port/maui-reference/app/App.xaml.cs`, `#if WINDOWS`, gated on
`MAUI_SUPPRESS_FOCUS_VISUAL=1` which `run_comparison.py` now passes to every launch unconditionally, same
shape as `MAUI_CAPTURE_TINT_NORMAL`): 200ms after `page.Loaded` (run_comparison's own `--settle` default is
1.0s, applied twice before any `--shot`, so this has a wide margin), find whatever element
`FocusManager.GetFocusedElement` reports and apply TWO independent levers -- re-`Focus` it with
`FocusState.Pointer`, and if it is a `Control`, also set `UseSystemFocusVisuals = false` on it (a
template-level switch independent of any FocusState transition). Both outcomes are logged
(`[reference] DEFOCUS <type> refocused=<bool>`) rather than assumed, matching this file's
`_defocus_before_shot` `requested_ok`/`verified` precedent -- a board that still shows the band should
point at that log line, not a fresh forensics pass.

**The C++ side needed no change**, and this was verified, not assumed: `port/cpp/src/platform/windows/
host_run.cpp` hosts a page's native view directly via `native.Content(...)`, with no Frame-style
AddPage/RemovePage container and no initial-focus `Focus()` call anywhere in the Windows backend -- and the
review's own repeated cross-run measurements above already show the port's captures bit-identical while
MAUI's toggled on the exact same pages. Documented in place (`host_run.cpp`, at the `Activate()` call) so a
future change to this backend's page-hosting re-checks the assumption instead of inheriting it silently.

**NOT build-verified.** The guest is unreachable for this task (nobody logged in at console, session-1
agent cannot start) and this Mac cannot compile the Windows TFM at all
(`net10.0-windows10.0.19041.0` is only added to `App.csproj`'s `TargetFrameworks` under
`$([MSBuild]::IsOSPlatform('windows'))`, which is false here) -- so `page.Handler?.PlatformView`,
`FrameworkElement.XamlRoot`, and `Control.UseSystemFocusVisuals` are unverified against the actual WinUI 3
API surface beyond web documentation. **First guest action should be `build_maui_reference.ps1`**, the
cheapest gate, before spending a board run on it. `check_winrt_includes.py`: 0 problems (the C++-side change
is comment-only).

**The acceptance test needs two halves, not one.** A single page improving (e.g. `label` light
0.50% -> 0.01%, the number this section already measured for the foreground-based attempt) only shows the
reference side improved -- it does not rule out the fix silently introducing a NEW band on the C++ side
(which the "no change needed" finding above says shouldn't happen, but "shouldn't" is not "verified"). Gate
on BOTH: a page already known to carry the band drops, AND a page that was already 0.00 stays 0.00 (not
just on the `maui` column -- on `cpp`/`cpp_xaml` too, which is the actual test of the "no C++ change needed"
claim).

**`--defocus` (`vm_agent_windows.py`, opt-in / OFF by default) must stay OFF together with this.** Nothing
in either mechanism stops someone passing both on the same run; doing so would reintroduce the exact
session-1 desktop teardown this whole line of work exists to avoid, for no additional benefit once the
in-process suppression is on.

### Capture-harness recovery: orphaned guest processes block every subsequent run (2026-07-31)

Symptom: `run_comparison.py` reports `ConnectionRefusedError` / `ConnectionResetError` on `present` and
`launch`, self-heals once, then emits `DROPPED — present failed after self-heal (no window to capture)`
and/or `launch failed` for whole pages or single columns. Worst case the run aborts with a
`FileNotFoundError` on its own `summary.json`.

Cause: a previous run left GUEST-SIDE ORPHANS. Two independent kinds, both must be cleared:
  1. **App processes** — `gallery`, `gallery_xaml`, `MauiReference` still running in session 1. A stale
     `gallery_xaml` is why the `cpp_xaml` COLUMN specifically kept dropping while the other two worked:
     its process was already alive, so the launch had nothing new to present.
  2. **Agent processes / scheduled task** — a stale `maui-agent-session1` task plus zombie `py`/`python`
     processes holding 127.0.0.1:8770, so the new agent cannot bind and every connect is refused.

Recovery (verified to restore a clean 3/3-column run, exit 0, zero failures):

    ssh <guest> "Get-Process | Where-Object { $_.ProcessName -match 'gallery|Maui' } |
                 Stop-Process -Force -ErrorAction SilentlyContinue"
    ssh <guest> "schtasks /End /TN maui-agent-session1"
    ssh <guest> "Get-Process python,py -ErrorAction SilentlyContinue | Stop-Process -Force"

then re-run. The runner relaunches its own session-1 agent.

**Check this FIRST whenever a run drops frames.** A dropped column silently produces a partial board, and
a partial board scored as if complete is the failure mode this project is least able to detect from the
numbers alone. Note the harness itself behaves well here — it names every dropped page/column explicitly
rather than skipping quietly, which is what makes a 1-page health check a sufficient gate before a
1104-shot pass.

**Worth automating:** the runner clears remote STAGING before a run but not guest PROCESSES. Killing
orphans at run start would make this class of failure impossible rather than merely diagnosable.

---

## varied_size_selector dark 3.05%: edge ANTIALIASING, not geometry — no fix shipped (2026-07-31)

Dark 3.05% / SSIM 0.975, light 0.00%. Investigated and deliberately left unfixed. Recording the negative
result in full because the geometry hypotheses are now closed and re-deriving them would cost a cycle.

**The alpha reading was right; the mechanism was not what it implied.** MAUI's two dominant diff colours
are Wheat (245,222,179) composited over the dark background (39,39,39) at TWO consistent alphas:
    (194,177,145) -> a = 0.752 / 0.754 / 0.757 across R/G/B
    ( 92, 86, 75) -> a = 0.257 / 0.257 / 0.257
The port renders fully opaque (1.0) and not-at-all (0.0). That looked like a missing Opacity or an item
container visual state. It is neither.

**What was ruled out, by measurement or primary source:**
- Per-cell / per-column state: all 6 cell boundaries (rows 32-33, 130-133, 230-233, 330-333, 430-433,
  530-533) dumped at x = 20/100/300/500/700/900/1000 are BYTE-IDENTICAL at every column. Uniform and
  position-independent — not a per-item state.
- A geometry/pitch bug: sub-pixel edge centroids fitted by 50%-coverage crossing give top edges at
  32.488/132.488/.../532.488 and bottom edges at 130.512/.../530.512 — pitch exactly 100.0, ZERO drift
  over 5 independent boundaries, fill span 98.024 = 100 - 2x1.0, matching the documented deflate. The
  port's integer positions match this to sub-pixel precision. `collection_view_handler` arrange is correct.
- A stroke-matches-background hack: `StrokeExtensions.UpdateStroke` returns early on null and never
  touches Path.Stroke; `new Path()` sets nothing. MAUI's Path.Stroke is null, same as the port's.
- Item-container chrome: `StructuredItemsViewHandler.Windows.cs`'s GetVerticalItemContainerStyle sets
  Margin(0, ItemSpacing, 0, ItemSpacing) and LinearItemsLayout.ItemSpacing defaults to 0. Verified on the
  guest that `ListViewItemBackground` resolves to SubtleFillColorTransparentBrush (transparent at rest).

**The actual mechanism:** every fill edge in MAUI's render — including the ISOLATED top edge of cell 1,
which has no neighbour above to confound it — carries an inherent ~2-pixel-row antialiasing footprint
(a 0.257/0.755 coverage split). The shared inter-cell boundary is exactly the superposition of two such
independent edge footprints, which reproduces all four observed rows with no extra paint required. The
port draws the same edge with NO antialiasing — a hard binary cutoff. 12 edges x ~1000 px is consistent
with the ~11000-px-per-alpha-group counts.

**Theme-independence:** re-solving on the light pair via G/B (R is useless there, 245 vs 244) shows the
identical 0.75/0.25 coverage structure. This is NOT a dark-theme bug; Wheat at 75% over (244,244,244)
blends to ~Wheat, so the same defect is simply unmeasurable in light. Do not go looking for a theme branch.

**Why nothing was shipped:** the deflate integral matches exactly on both sides, so `border_handler` is
correct — and its last speculative change was reverted (`f1a5a17658`) for leaking into clip paths. A change
premised on "geometry matches, only AA differs" would be either a no-op or a regression risk to the
already-green `border` / `border_layout` / `borderless` pages, unverifiable without a Windows compiler.

**Narrowed, NOT confirmed:** the leading remaining explanation is that real MAUI hosts each cell in a
native ListViewItem/ContentPresenter whose compositing/rasterization introduces the AA, while this port
realizes cells by direct placement on a bare Canvas (a scope cut the handler's own header documents).
Closing it needs either live Windows inspection of `winui::Shapes::Path` fill AA under Canvas vs
ListViewItem hosting, or a decision to implement real native item containers — a large architectural
change, not a targeted fix.

**Coverage:** `varied_size_selector.xaml` is the ONLY board page combining a CollectionView with a Border
cell, so there is no second data point. (`borderless.xaml` sets StrokeThickness="0", making
shape_self_inset a no-op, which is why it is green while testing something structurally different.)

---

## The three non-Win2D Windows reds/yellows do NOT share one cause — one strong lead, no fix shipped (2026-07-31)

Task: check whether `border_stroke`, `border_playground`, `varied_size_selector` (the board's last three
non-green, non-Win2D Windows pages) share one systemic "missing edge antialiasing" cause. They do not — three
separate mechanisms, evidenced below. For `border_stroke` a specific, well-corroborated candidate fix was
found and DRAFTED, then DELIBERATELY NOT SHIPPED after a second review surfaced an un-ruled-out alternative
that the fix could not distinguish itself from using only the committed captures. Recording the full
derivation here, including the reason it stopped short, so the next agent does not re-walk the same ground
before doing the one live-Windows check that would settle it.

### The brief's own premise is falsified by row 75

`border_stroke`'s T=1 cell has an ISOLATED outer edge (Red stroke fill against the page background,
`captures/windows/{maui,cpp}/border_stroke_light.png`, row 75, cols 200/500/800): MAUI and cpp are
**bit-identical**, `(249,121,121)` on both sides. Solving `pixel = a*(255,0,0) + (1-a)*(244,244,244)`
gives `a≈0.504` consistently across R/G/B. So the port's WinUI `Shapes::Path` fill DOES antialias
fractional-position edges correctly — "the port's Windows shape rendering is missing antialiasing
generally" is dead on its own evidence. The three pages' failures needed separate diagnosis, not one
mechanism hunt.

### border_stroke: content-clip geometry, not rendering — strong lead, NOT shipped

Same page, one row down (row 76, the RED-stroke/ORANGE-content seam): MAUI shows genuine fractional
coverage, cpp shows an EXACTLY-zero-coverage cutoff — pure `(255,165,0)` orange, no blend at all. This
repeats identically at all 12 edges on the page (T=1/5/10, both grids; horizontal AND vertical, confirmed
by scanning both a row range and, separately, a column range through the T=10 cell's LEFT edge at row 146 —
same values, same zero-coverage cutoff at col 30). `alignment.xaml` (4x default-Rectangle Border + Label
BackgroundColor="Blue") shows the identical signature at 8/8 edges: MAUI blends `(188,92,155)`, cpp cuts
hard to pure Blue `(0,0,255)`.

**Why "hard cutoff" is not itself proof of "AA disabled."** The STROKE's own outer edge (row 75, above)
sits at a FRACTIONAL row position — `border_handler.cpp`'s `update_border()` already applies
`maui::core::shape_self_inset` (a constant 0.5 DIP/side, gated `thickness > 0`) to the stroke geometry, and
the resulting sub-pixel centroid (`+0.517`, per this doc's own earlier measurement) is exactly why that
edge blends. `apply_content_clip()`'s non-round-rect branch — the ORANGE content seam — called the port's
own (deliberately non-self-insetting) `spec.shape->path_for_bounds(path_size)` directly, with NO analogous
inset. An edge that lands EXACTLY on an integer pixel row has nothing fractional to blend by construction —
indistinguishable from "AA disabled" by symptom, but a geometry bug, not a rendering one.

**Oracle.** C#'s `ContentPanel.UpdateClip` branches on `IRoundRectangle`: that branch calls
`RoundRectangle.InnerPathForBounds` (already ported here as `inner_round_rectangle_path`, measured correct
below). Every OTHER shape falls through to the plain `strokeShape.PathForBounds(bounds)` — and
`PathForBounds` on any Controls Shape unconditionally folds in `TransformPathForBounds`'s 0.5 DIP/side
self-inset (the same chain `shape_self_inset` already carries for the STROKE, three call sites up in this
same file). The port's non-round-rect content-clip branch was calling the equivalent of `PathForBounds`
without that inset — a real, separate gap from the (already-fixed) stroke deflate, not a duplicate of it.

**Independent corroboration, same page, different platform.** This file's own cross-platform pass
(2026-07-31, earlier the same day) already measured and fixed the Android twin of this exact gap: *"Measured
on `border_stroke`, MAUI's orange content edge sits `+0.50` pt inward of the port's at both T=1 and T=5 —
the same constant as the stroke — so `border_content_inner_path_points`'s non-round-rect branch is inset
too."* This Windows change is that fix's Windows twin, on the same page, same measured offset.

**The apparent conflict, and why it isn't one.** The comment directly above `inner_round_rectangle_path`
says "DO NOT extend `shape_self_inset` to this clip," measured on `border_resize_content`'s Ellipse
"circle+image" cell (114 px residual, all clip-edge AA, no directional bias). That measurement is real but
scoped ONLY to the round-rect branch's own `InnerPathForBounds` — a genuinely different C# code path from
the non-round-rect branch this fix touches. Re-checked directly here: `border_resize_content`'s row0/col0
Ellipse cell (translucent-red Label content, NOT the noisier image cell) shows, at its top edge (col 265,
row 56): MAUI `(172,193,159)`, cpp `(216,91,75)` — **cpp blends too**, just at a different value, unlike
border_stroke's EXACT zero. Curved boundaries land at a fractional pixel position at nearly every scan
angle regardless of any inset bug, so they show SOME coverage gradation either way — a radius-residual
check (as the prior measurement used) is not sensitive enough to distinguish "correctly inset" from
"missing a 0.5 DIP inset" on a curve the way a straight axis-aligned edge makes obvious (zero vs nonzero
coverage, unambiguous). The two findings do not contradict: one says the ROUND-RECT branch is fine (still
true, unchanged by this fix), the other exposes that the NON-round-rect branch — which `border_resize_content`
also exercises via its Ellipse and Polygon cells — was never measured for this specific defect at all.

**T-invariance, checked properly.** All 12 content-clip-seam pixels (T=1/5/10, both grids, top AND bottom
edges) read the EXACT SAME MAUI value, `(251,133,92)`, bit-for-bit — not merely similar, identical to the
byte. A 3-colour least-squares solve (`RED=(255,0,0)`, `ORANGE=(255,165,0)`, plus a page-background third
term) fits this consistently across all six edges with the same coefficients every time, which is the same
signature the doc's own stroke-centroid table used to conclude "a fixed 0.5 DIP/side inset, not a scale
error, not noise" — a quantity that depended on `thickness` (which varies 1/5/10) would not generically
produce an identical byte-for-byte MAUI pixel at every T by coincidence.

**The draft fix.** `apply_content_clip()`'s non-round-rect branch: wrap `path_size` in
`maui::core::shape_self_inset(path_size, thickness)` before calling `spec.shape->path_for_bounds(...)`,
mirroring `update_border`'s existing stroke-geometry call three lines up in the same file. This was written,
reviewed, and then reverted — see below.

**Why it was not shipped.** The cpp side of the seam is not merely "close to" the hard endpoint colour, it
is BIT-IDENTICAL to it (`(255,165,0)`, exactly the interior orange, zero measurable admixture of red or
background). Two different explanations produce that same observation, and the committed 8-bit captures
cannot distinguish them:

  (a) the geometry is missing the 0.5 DIP inset and consequently lands EXACTLY on an integer pixel row,
      which has nothing fractional to blend — the draft fix's premise. Adding the inset moves the edge to
      a genuinely fractional position and the seam should soften.
  (b) the geometry is already fractional (possibly even already off by roughly 0.5 DIP, undistinguishable
      from (a) at this point), but something in the `apply_content_clip` → Direct2D → `CompositionGeometricClip`
      pipeline SNAPS the rasterized clip edge to the nearest device pixel before compositing. If so, adding
      0.5 DIP either lands on the SAME integer after snapping (no visible change) or the ADJACENT one (the
      cutoff moves one row, not softens) — in neither case does the fix do what it is meant to.

These are NOT distinguishable from the PNG alone: 8-bit quantization means "coverage = 100.0%" and
"coverage = 99.6%, rounds identically at 8 bits" render pixel-identical either way, so "cpp shows zero
blend" cannot itself prove "cpp's edge sits at an exact integer" strongly enough to rule out (b). The
STROKE path (`path.Data`, a WinUI `Shapes::Path`) is proven NOT to snap — its own 0.5 DIP inset produces a
measured `+0.517` DIP sub-pixel centroid that blends correctly, matching MAUI bit-for-bit (row 75). But the
content clip is a structurally different pipeline from that same file's own header comment: Direct2D
geometry → `IGeometrySource2DInterop` → `compositor.CreateGeometricClip`, not a `Shapes::Path` fill/stroke.
"The sibling code path doesn't snap" does not prove this one doesn't; it is exactly the kind of
platform-rasterizer question the `varied_size_selector` investigation (elsewhere in this doc) also hit and
correctly declined to guess past. Shipping the inset without settling (a) vs (b) risks either a no-op or a
same-magnitude one-row-displaced cutoff dressed up as "reference noise" (this doc's own recorded
`border_stroke` cross-run swing is 4104 MAUI px / +0.50pp with the port BYTE-IDENTICAL both times — a wrong
fix could hide inside that band for a whole cycle before anyone notices it didn't work).

**What would settle it, cheaply, on the guest (no code change needed for the check itself):** apply the
`shape_self_inset` change alone, capture `border_stroke` and `alignment`, and read the SIGN and MAGNITUDE
of the movement. Per this doc's own reference-noise section, a diff_pct move under ~0.5pp on either page is
within measured cross-run noise and inconclusive either way — but the underlying pixel VALUES resolve it
regardless of magnitude: if the seam row's cpp value now shows genuine fractional coverage (anything other
than exactly one of the two flanking hard colours), hypothesis (a) is confirmed and the fix works; if it is
still bit-identical to a hard colour (whichever row it is now on), hypothesis (b) is confirmed and the fix
needs a different mechanism (likely: whatever in the D2D/Composition clip chain is snapping, not the
geometry feeding it). That one-line diff plus a single capture is the whole remaining task.

### border_playground: a DIFFERENT mechanism — port already antialiases here, just to a different value

`border_playground`'s Border (`Stroke="#CAC531"`, `StrokeDashArray="1,1"`, `LinearGradientBrush`
Background, asymmetric `RoundRectangle CornerRadius="20,0,0,12"`) shows NO child content with its own
BackgroundColor — the Label inside has none — so `apply_content_clip` never paints a visible seam here
regardless of this fix. Its board-visible diff (1.11%/1.26%, `DIFF_THRESHOLD=25` metric) is dominated by
two ~6-row bands at the shape's own top/bottom edges (cols 24-999, i.e. spanning the full width, not
localized to the rounded corners). Sampled at col 500/700, row 48: MAUI `(223,220,147)` solves to α≈0.50
coverage of the stroke color over background; cpp `(240,240,218)` solves to α≈0.10 — **the port DOES blend
here, just at roughly 1/5th the coverage MAUI shows**, the opposite signature from border_stroke's exact
zero. This points at a dash-phase / gradient-sampling / RoundRectangle-corner position mismatch in the
Border's own stroke+fill rendering (`path.Stroke`/`path.Fill`, NOT `apply_content_clip`) — an unrelated,
unfixed, architecturally separate question. Raw (non-thresholded) diff on this page is ~22% of pixels, but
almost all of it is sub-threshold (±1-3 unit) gradient-interpolation rounding noise unrelated to either
mechanism above.

**Prediction if the draft fix above is ever applied: `border_playground` must NOT move.** It never touches
`apply_content_clip`'s non-round-rect branch (StrokeShape is `RoundRectangle`, taking the untouched branch)
and has no visible content-clip seam regardless. A movement here on a future capture would mean the fix has
a side effect not accounted for in this section.

### varied_size_selector: outside this mechanism entirely — the prior diagnosis stands

`varied_size_selector`'s cells set `BackgroundColor="Wheat"` on the BORDER itself (not a child), which
`border_platform::update_background` routes to the STROKE PATH's `Fill` (`path.Fill()`), per this file's
own header note on Background routing — never `apply_content_clip`. Nothing in this investigation touches
that path. The existing diagnosis (this doc, "varied_size_selector dark 3.05%: edge ANTIALIASING, not
geometry") — a CollectionView-cell fractional-positioning question, narrowed to "real MAUI hosts each cell
in a native ListViewItem/ContentPresenter... this port realizes cells by direct placement on a bare
Canvas," needing live Windows inspection or a native-item-container architecture change — is unchanged and
still correctly unfixed. It shares no code path, and by extension no fix, with either of the other two pages.

### Falsifiable predictions, IF the draft `shape_self_inset` fix is applied

Not shipped in this pass — recorded so whoever applies it (after the live check above) knows what to expect:

  border_stroke              SHOULD show genuine fractional coverage at the seam row (settles hypothesis
                              (a) vs (b) above) and the diff_pct SHOULD drop from 1.49%/1.51%. Not expected
                              to reach 0.00% — a small clip-edge-AA residual (curved/diagonal-adjacent
                              rounding, cf. border_resize_content's 114 px) is plausible even once the
                              position is right. A diff_pct move under ~0.5pp is within this doc's own
                              measured reference-capture noise floor (border_stroke swung 4104 MAUI px /
                              +0.50pp between runs with the port's code and captures byte-identical) and
                              should not be over-read — read the PIXEL VALUES at the seam, not just diff_pct.
  alignment                  SHOULD improve slightly (currently green, 0.17%/0.19%) — stays green either way.
  border_resize_content      MAY move slightly (currently green, 0.45%/0.46%) — its Ellipse and Polygon
                              cells (4 of its 6 Border instances) go through the changed branch. Expected
                              small if any; the RoundRectangle cells (2 of 6) are untouched. This is the
                              page the fix's own author could not fully resolve (see the conflict discussion
                              above) — check it first, not last.
  border_playground          should NOT move (different mechanism, different code path — see above).
  varied_size_selector       should NOT move (different mechanism, different code path — see above).
  border, border_layout,     should NOT move — all explicit-RoundRectangle StrokeShape (round-rect branch,
    border_clip_playground,   untouched) or no colored child content reaching the inner edge, or
    border_styles,             StrokeThickness=0 (shape_self_inset's own gate, `borderless`).
    borderless, containers,
    swipe_view_shadow
  carousel_page/carousel_view, invalidate_shadow_host, radio_template_from_style: default-Rectangle Border,
    but no colored child content (carousel's Label has none) or content color ≈ identical to the stroke
    color (`radio_template_from_style`'s `#F3F2F1` stroke AND fill) — should NOT move visibly.

### Verified vs assumed

VERIFIED (by pixel measurement on committed captures, no compiler needed): the row-75/row-76 coverage
arithmetic on `border_stroke` (both by row-scan and column-scan, exactly zero cpp-side coverage at all 12
edges); the `alignment.xaml` 8-edge corroboration (identical hard-cutoff-vs-blend signature, different
page, different colors); the `border_resize_content` Ellipse cell showing PARTIAL (not zero) coverage at
its own seam, which is why it does not contradict border_stroke's finding despite sharing the same code
branch; the T-invariance of MAUI's blend value across T=1/5/10 (bit-identical, not merely similar);
`path_for_bounds`/`shape_self_inset` signatures match the draft call site
(`maui::graphics::rect shape_self_inset(const maui::graphics::rect&, double)`,
`maui::graphics::path_f path_for_bounds(const maui::graphics::rect&) const` — confirmed against
`include/maui/graphics/i_shape.hpp` and `include/maui/core/border_handler.hpp`); the full sweep of all 17
`<Border`-using page XAMLs in `port/maui-reference/pages/` for StrokeShape + colored-child-content
combinations (table above); `check_winrt_includes.py` clean (`checked 31 file(s): 0 problem(s)`, run against
the working tree with no code changes present, since none were kept).

NOT SHIPPED, and why: a draft one-line fix (wrap `path_size` in `maui::core::shape_self_inset(path_size,
thickness)` in `apply_content_clip()`'s non-round-rect branch, mirroring `update_border`'s existing stroke
call three lines up) was written and then reverted. It is well-derived from the C# oracle and corroborated
by an already-landed, already-measured Android fix on this SAME page — but this investigation could not
rule out, using only the committed 8-bit captures, that the content-clip's Direct2D/`CompositionGeometricClip`
pipeline (a structurally different path from the proven-correct `Shapes::Path` stroke rendering) snaps its
rasterized edge to the nearest device pixel, which would make the fix a no-op or a same-magnitude
one-row-displaced cutoff rather than a real improvement — see "Why it was not shipped" above for the full
reasoning and the specific one-capture check that would settle it. `git diff` on this pass touches only this
doc file; `src/platform/windows/border_handler.cpp` is back at its pre-investigation state.

---

## The 4 "diff<=1%, SSIM<0.98" pages: FOUR separate mechanisms, not one (2026-07-31)

Investigated the remaining Windows yellow cluster (`pickers`, `header_footer_grid_horizontal`,
`selection_synchronization`, `search_bar` — all pass the diff gate comfortably, all miss SSIM). The brief's
own steer ("3 of 4 names suggest CollectionView") does NOT hold up: `pickers` and `search_bar` contain no
CollectionView at all, and their dominant defect turns out to be shared with each other, not with the two
CollectionView pages. **Verdict: four pages, at least four distinct mechanisms, none safely fixable from
this Mac this pass.** No code shipped. `check_winrt_includes.py`: `checked 31 file(s): 0 problem(s)`
(baseline — no source files touched).

Method: for each page/theme, computed the same `diff`/`ssim()` this repo's `pixel_score.py` uses, then
read off exactly which rows/columns the low-SSIM windows and diff-mask pixels sit in (Pillow + numpy,
committed captures only), and visually cropped every hot region to identify what control/glyph is there.
Pixel accounting below sums to (or very near) 100% of each page's diff-mask population, not just the worst
few windows, so the attributions are not cherry-picked outliers.

### `pickers` (light 0.59%/0.9681, dark 0.64%/0.9625): ~80-90% the known focus-visual noise floor — AND a
### direct counterexample to this doc's own "verified: the C++ side needs no change" claim

Region accounting (light): rows 50-95 (a focus rectangle around the "Pick a room" `Picker`) = 4096 of 4869
diff px (84%); rows 190-215 (a text-content difference, below) = 773 px (16%); rows 0-25/30-50 = 0. Dark:
same two regions = 4100+797 of 5269 (93%), plus a ~98px dark-only titlebar cluster and a ~274px "Pick a
room" text-color artifact (next section).

The rows 50-95 rectangle IS the same "MAUI's own CollectionView captures carry a ~0.50pp focus-visual noise
floor" / "22 of 24 pages" mechanism this doc already extensively documents (2px near-black 26,26,26
rectangle, ~1008px-wide top/bottom edges, ~4px-wide left/right edges) — **but on `pickers_light` it sits on
the CPP column, not MAUI's.** Measured directly: `cpp/pickers_light.png` shows the "Pick a room" `Picker`
(a `ComboBox`) with a full keyboard-focus outline AND a text-insertion caret; `maui/pickers_light.png` shows
the same control with neither. `pickers_dark` has the band on the opposite (MAUI) column instead — same
page, same control, two themes, two different sides. This directly contradicts the "Focus-visual
suppression... In-process suppression IMPLEMENTED" entry's claim: *"The C++ side needed no change, and this
was verified... host_run.cpp has no initial-focus `Focus()` call anywhere in the Windows backend."* That
statement was true of the SAMPLE it was measured against (this doc's own repeated cross-run tests never
showed the band on `cpp`) but is not true in general — the port's own WinUI `ComboBox` can and does end up
keyboard-focused at capture time on at least one page. Operationally this matters: `MAUI_SUPPRESS_FOCUS_VISUAL`
only patches `port/maui-reference/app/App.xaml.cs` (the MAUI reference app). If the C++ gallery app can also
carry the band, that suppression is one-sided and the doc's own prescribed two-halves acceptance test ("a
page already at 0.00 stays 0.00 on cpp too") will eventually catch a page where it doesn't. NOT a port
rendering bug (nothing about `pickers`' layout or paint is wrong) — it's a capture-time input-focus race,
same family as the existing mechanism, just not proven one-directional. No action taken; flagging so the
suppression fix (or its verification pass) accounts for both columns, not just MAUI's.

Rows 190-215 (the "No room on ... at ..." readout Label, 16% light / 15% dark): MAUI shows "No room on
(no date) at (no time)"; cpp shows "No room on 7/31/2026 at 09:00" — cpp's `DatePicker`/`TimePicker`
propagate their resting default value into the bound readout at init, MAUI's Windows render does not. This
is the SAME shape of quirk ruling 10(c) already covers for Mac Catalyst ("picker default-value propagation
at init... MAUI Mac Catalyst does not fire a DatePicker/TimePicker's default through its change event at
first layout... whereas iOS/Android (and the port on all backends) propagate the default... the port's
fuller render is CORRECT"), just now observed on **Windows**, a platform ruling 10(c) doesn't currently
list. Plausibly the same underlying MAUI behavior (binding-update timing at first layout), not something
Mac-Catalyst-specific — but that's an inference, not confirmed against another Windows page exercising the
same bindings, so recording as a candidate ruling-10(c) extension rather than assuming it. Either way: not
a port bug, the port's fuller readout matches iOS/Android/the port's own other backends, nothing to fix.

### `search_bar` (light 0.24%/0.9927, dark 0.44%/0.9739): same focus-visual floor (light: 98% of the diff),
### plus a NEW MAUI dark-theme quirk, not previously documented

Light: rows 109-110 (the accent-color focus underline WinUI draws under a focused `TextBox`-derived control
— the SearchBar variant of the same mechanism as `pickers`' rectangle, different WinUI control template)
account for 1946 of 1990 diff px (98%); everything else is single-digit-px noise. This time the band sits on
the MAUI column (matches the doc's usual direction). Not actionable, same reasoning as above.

Dark: the focus band (rows 105-113, 1946px, 54%) plus a **second, NOT previously documented artifact**
(rows 215-260, 773px, 21%): the "Italic 24pt" `SearchBar`'s own entered text. `maui/search_bar_dark.png`
renders that text at luma ~5-52 (i.e. barely distinguishable from its own ~52-value dark fill — effectively
invisible); `cpp/search_bar_dark.png` renders the identical string at full light-theme-appropriate contrast
(up to 255). Critically: in the LIGHT-theme capture of the SAME element, the two columns are **byte-identical**
(max abs diff = 0 across the whole text region) — cpp's renderer is already provably correct there. So this
is not "cpp guesses the wrong color" — cpp already matches MAUI exactly in light theme; only MAUI's own
DARK capture goes wrong. The value MAUI shows in dark (luma ~5-52) is suspiciously close to what a
LIGHT-theme foreground color would look like sitting on a dark fill — i.e. this reads like a text-color
brush that got resolved once (at light-theme values) and never re-evaluated when the app's dark theme
applied, a stuck/stale `ThemeResource` rather than a deliberate render. No SearchBar `TextColor` is set in
the source XAML (`<SearchBar Text="Italic 24pt" FontSize="24" FontAttributes="Italic" />`), so there's no
explicit-color-vs-unset-sentinel confusion to blame on the port; the port isn't in this pipeline at all —
this is 100% a `port/maui-reference` capture artifact (or a genuine upstream MAUI Windows dark-theme bug).
Per ruling 3 ("New MAUI imperfections -> flag, don't act, pause for a user ruling"): flagged here, not
fixed, not matched. The port's brighter/legible rendering is very likely the ONE that's actually correct;
matching MAUI's near-invisible text would make the port worse, not better.

### `pickers` dark, same signature: "Pick a room" `Picker.Title` header text (rows 30-50, 274px, 5% of
### the page total)

Same shape as the `search_bar` finding immediately above, on a different control: `maui/pickers_dark.png`'s
"Pick a room" header (the `Picker.Title`, mapped to the `ComboBox`'s Header) renders at luma 5-52; light
theme, same element, is byte-identical between columns (max abs diff = 0). Recording as the SAME candidate
mechanism as the SearchBar text above — two independent controls, two independent pages, identical
"stuck-at-light-theme-color, only in dark, only for this one element while the rest of the page correctly
follows the theme" signature — worth a single ruling covering both rather than two.

### `selection_synchronization` (light 0.38%/0.9689, dark 0.40%/0.9551): the previously-documented
### "checkbox mid-text" defect is REAL, but is TWO compounding, differently-sized defects, not one —
### drafted a one-constant fix, then DISCONFIRMED it by measurement and did NOT ship it

100% of the light diff (3151/3151 px) and 97% of dark (3173/3271) sit in cols 0-90 — the multi-select
`CheckBox` glyph `paint_selection_checkbox` hand-draws into every `SelectionMode="Multiple"` CollectionView
cell (`src/platform/windows/collection_view_handler.cpp`). This confirms the existing entry ("selection_
synchronization — CollectionView checked-cell layout, NOT fixed (reserved territory)... adjacent to
border_handler.cpp/view_chrome_ops.cpp, which this task was explicitly told not to touch"). This pass WAS
allowed to touch it (no such restriction in this brief) and went looking for the actual fix.

**First measurement, which looked like a clean single-constant bug.** The checkbox's own bounding box
(found by isolating its solid accent-blue fill, `(0,103,192)`, unambiguous — not contaminated by adjacent
text anti-aliasing) is **exactly 20px wide in both columns** (matches `k_selection_checkbox_size = 20`,
correct) but its LEFT edge is a constant **4px too far left in cpp**, confirmed on 3 independent controls:
  `selection_synchronization` row 136/161 (checked): maui 27-46, cpp 23-42 (slot.x=13, `Margin="5,2,5,5"`)
  `multiple_bound_selection`  row 140/149 (checked): maui 22-41, cpp 18-37 (slot.x=8, no margin)
Both give `maui_left - cpp_left = 4` exactly, at two different `slot.x` values, on a page that's currently
GREEN (`multiple_bound_selection`, 0.17%/0.19%) precisely because it only has 4 checkbox rows vs.
`selection_synchronization`'s 9 CollectionViews' worth — same underlying 4px error, ~9x the row count, well
past the SSIM gate. This pointed straight at `k_selection_checkbox_left_inset = 10` (line ~379): change it
to `14`. Drafted the one-line fix.

**Then applied the discriminator a reviewer flagged before shipping: measure the LABEL's own trailing
glyphs, well clear of the box, not just the box.** This is the part that changes the conclusion. On
`selection_synchronization` row 136 ("Item 2", checked), the "2" digit — fully clear of either column's box
— sits at col **85-89 in maui** vs. col **44-50 in cpp**: a **40px** gap, not 4px. On the unchecked "Item 1"
row (98-121), the "1" digit sits at col **74-75 maui** vs. **46-47 cpp**: a **28px** gap. Both far exceed
the clean, reproducible 4px the box itself carries, and — this is the key tell — **the size of the gap
differs by row/content (40px vs 28px) while the box's own 4px offset does not.** A single mispositioned
`slot.x` feeding both the box and the label would move both by the SAME amount; it doesn't. There are two
independent, compounding defects: (1) the checkbox is a flat 4px too far left (real, confirmed, small), and
(2) something in this same cell's Label content — likely its own measured/available width, or a second,
separate x-origin — sits well further left in cpp than in maui, by an amount that is NOT constant. Visually
this is exactly what the crops show: cpp's `"It[box]2"` isn't just "the box is 4px left of where it should
be", the whole `"Item N"` string reads MORE COMPRESSED than MAUI's `"[box]tem N"` — MAUI's checkbox+text
together span roughly 75-89px of "Item 2"/"Item 1", cpp's span roughly 42-51px, a ~2x difference that a
20px, 4px-off checkbox cannot produce on its own.

**Did NOT ship the `k_selection_checkbox_left_inset` constant change.** The 4px box-only offset reproduces
cleanly on two samples, but its SIGN is only trustworthy once defect (2) is ruled out: if the real cause
turns out to be a too-narrow measured cell content width (the leading hypothesis below), the checkbox is
merely inheriting that wrong slot, `left_inset = 10` is already correct relative to it, and moving it to 14
would overshoot once (2) is fixed — this doc's own next paragraph says exactly that. So the 4px is a
symptom of unknown standing, not a confirmed independent bug to correct in isolation. Shipping it alone
would also, at best, fix a 4-in-~30px sliver of a much larger gap, very likely leaving
`selection_synchronization` yellow anyway, while adding an unverifiable (no Windows compiler on this Mac)
change whose interaction with the larger, undiagnosed defect is unknown. This is the same judgment call this file has made before (the `apply_content_clip` inset draft,
written and reverted for a related reason: two plausible mechanisms, no way to distinguish them from 8-bit
captures alone) — write it down precisely enough for whoever has Windows compile access to pick straight up,
don't ship a fix proven insufficient by the same measurement that found it.

**A third data point that does NOT fit a constant model either, recorded but not chased:**
`cv_visual_states` (currently green, 0.9859L/0.9905D) shows the same qualitative "checkbox eats into the
label" pattern on its `FontSize="Large"` multi-select rows, but the box-left gap there reads closer to
**9px**, not 4px (noisier measurement — Large-font glyph anti-aliasing overlaps the box-detection heuristic
more than the default-size rows above, so treat 9 as approximate, not exact). A 4px-vs-9px spread across
default vs. Large font strengthens the case for "font/content-dependent," i.e. defect (2), not a single
mis-set constant. `cv_visual_states` is not in scope for this pass and was only checked for regression
safety (see prediction below); its own resolution is left to whoever picks up defect (2).

**Falsifiable prediction for whoever ships the real fix:** the true fix must move the LABEL's own glyph
positions right by a page/content-dependent amount (not a constant), in addition to (or possibly instead
of, if defect (2) turns out to subsume defect (1) — e.g. if both trace to the same cell content-arrange
call computing too-small an available width for the `HorizontalStackLayout`-equivalent content, which would
also explain why the checkbox — sized/positioned off that same too-small slot — lands 4px short). Check
that hypothesis FIRST on the real guest: instrument or log the cell's computed content width against MAUI's
own `ItemsRepeater`/`ListView` cell width for the identical row, before assuming two unrelated bugs. If
confirmed to be one underlying cause (a too-narrow measured cell content width), `k_selection_checkbox_
left_inset` should NOT be touched at all — it would already be correct once the content width is fixed, and
this pass's proposed 10->14 change would then OVERSHOOT.

**Guard/regression pages if this is picked up:** `multiple_bound_selection` (green, dark SSIM 0.9831 — only
0.0031 of margin, must stay green), `cv_visual_states` (green, light SSIM 0.9859), `preselected_items`
(green — GridItemsLayout branch, uses `k_selection_checkbox_margin` not `_left_inset`, must NOT move),
`preselected_item` (green, singular — `SelectionMode="Single"`, no checkbox at all, must NOT move).

### `header_footer_grid_horizontal` (light 0.47%/0.9846, dark 0.61%/0.9799 — misses dark by 0.0001): a
### REAL layout defect, but cross-platform-suspect, so NOT Windows-surgical — not fixed

Region accounting (dark): rows 220-231 (a `HorizontalGrid, 3` `CollectionView` column's item text) = 1600px
(rows 220-235) + 853px (rows 310-320) = 49%; rows 430-450 (the rotated "This Is A Footer" label) = 1014px
(20%); remainder scattered. Visually confirmed by cropping: MAUI wraps `"Vegetables.jpg, 3"` and
`"FlowerBuds.jpg, 5"` onto two lines inside their grid cell; cpp fits the identical strings on one line —
cpp is computing (or being handed) a WIDER per-column width than MAUI for this `HorizontalGrid` `Span="3"`
CollectionView (10 items, `<Label Text="{Binding .}" Margin="6" />` cells, no explicit item width). This is
a genuine port_diff, not noise or a capture artifact — the two renders show materially different column
widths, not a sub-pixel or antialiasing difference.

**Not fixed, and not attempted, because it is not Windows-exclusive:** `header_footer_grid_horizontal` is
independently YELLOW on iOS (0.9748L/0.9745D) and Mac Catalyst (0.9786L/0.9774D) too — only Android is
green. Column-width computation for `GridItemsLayout` is either shared cross-platform code or independently
buggy on 3 of 4 backends with different magnitudes; either way, a Windows-only change can't be verified not
to leave iOS/Catalyst's (possibly-different-cause) yellows untouched or to avoid a regression there that
this Mac has no way to check in the same pass. `grid_items_layout.cpp` itself (the shared descriptor file,
`span`/`vertical_item_spacing`/`horizontal_item_spacing` properties) is a 34-line property holder with no
sizing algorithm in it — the actual column-width logic lives per-backend, so this needs an agent with
reason to touch iOS/Catalyst captures too, not a Windows-scoped one.

The rows 430-450 rotated-footer-text residual (20% of dark) is visually an antialiasing-softness difference
at the glyph edges of a `Rotation="10"` Label, not a position/content difference (both columns clearly read
"A Footer" at the same size and angle) — almost certainly the same class of thing as this doc's own
`varied_size_selector` finding (edge AA under a transform is architectural, not a targeted-fixable bug).
Not chased further; it's a fifth of one page's dark diff, not the dominant term.

**The dark-miss-by-0.0001 the brief flagged is fully explained by "not quite enough of the above resolved,"
not a separate near-miss mechanism** — no single component here is anywhere near the gate on its own; it is
the combination of the grid-wrap defect (real, cross-platform, not fixed) and the rotation-AA residual
(architectural, not fixable) landing just under threshold together.

### Answering the brief directly: one cause or four

**Four**, not one — matching the brief's own caution against forcing a unification, and its steer ("3 of 4
suggest CollectionView") turned out to be the wrong axis to split on. The actual split is by MECHANISM,
which cuts across the CollectionView/non-CollectionView line the brief proposed:
  1. Reference-focus-visual capture noise (session-level, can land on EITHER column) — `pickers` light+dark,
     `search_bar` light. Already documented elsewhere in this file; this pass adds the cpp-side
     counterexample. Not actionable from source.
  2. A new "stuck light-theme text color in dark captures" MAUI-reference quirk — `pickers` dark (Picker
     header), `search_bar` dark (SearchBar text). Not previously documented. Not a port bug (port's
     dark-theme rendering is provably correct in the one case with a byte-identical light-theme control:
     matches MAUI exactly there). Flagged per ruling 3, not fixed.
  3. `HorizontalGrid` column-width mismatch — `header_footer_grid_horizontal` only. Real port_diff (or
     shared-code diff), but touches 3 backends independently, out of this pass's safe verification range.
  4. CollectionView multi-select cell content layout — `selection_synchronization` only. Two compounding
     defects (a confirmed, small, constant checkbox-position error + a larger, content-dependent label-
     width error). Drafted and DISCONFIRMED a partial fix before shipping it; documented precisely enough
     to resume without redoing the measurement work.
  5. (minor, non-dominant) Rotation-transform edge AA — `header_footer_grid_horizontal` dark only, ~20% of
     that one theme's diff, architectural, same family as the already-documented `varied_size_selector`
     finding.

**Worktree note, unrelated to any finding above but worth recording for whoever runs here next:**
mid-investigation, three consecutive `git log -1` calls in this same session returned `a734782658`, then
`bcb41ea90a`, then `cd3752dd71` — different HEAD hashes with no action taken on this side. At the middle
observation, `docs/comparison/captures/windows/` (1094 files) and every `"windows"` key in
`comparison.json` were absent from the working tree; both were back and byte-consistent with the brief's
original four scores by the third observation. No rebase markers were present at any point, and no local
modifications were involved — this settled on its own, evidently from another session rewriting history on
this shared branch/worktree mid-flight. All four target pages' numbers were re-verified against the settled
tree (`cd3752dd71`) and match the brief exactly, and this doc's findings are keyed to that settled state —
but the next agent here should check `git log -1` before trusting anything on disk, not assume the tree is
static just because a task brief pins a hash.

**Nothing shipped this pass.** `check_winrt_includes.py`: `checked 31 file(s): 0 problem(s)` (no source
files were touched — this is the baseline, not evidence of a fix). `tools/dev.sh` was not run: no
cross-platform code changed, and the headless preset does not compile `src/platform/windows/*` anyway, so
it would provide no signal on the one file this investigation centered on. No Windows build or run was
performed or claimed anywhere in this entry. All 10 guard pages (`border`,
`border_layout`, `borderless`, `shapes`, `alignment`, `border_stroke`, `border_playground`, `clip`,
`clip_gallery`, `clip_corner_radius`) verified present in `comparison.json` with a `windows` platform entry
and confirmed still green — unaffected, as expected, since no code changed. Regression-relevant page names
used above (`multiple_bound_selection`, `cv_visual_states`, `preselected_items`, `preselected_item`,
`picker`) individually verified present in `comparison.json` with a `windows` entry before being cited.

**What in the original brief turned out to be wrong:** the CollectionView-shared-cause steer (addressed
above — it's a coincidence of naming, not a shared mechanism); and implicitly, the assumption that a
"pixel-forensics-confirmed, single-constant" fix is safe to ship once measured — the `selection_
synchronization` checkbox constant looked exactly like that after the FIRST measurement (two independent
controls, same 4px, clean) and was wrong to ship anyway once a THIRD, better-chosen measurement (the
label's own trailing glyphs, not the box) was taken. Worth keeping as a general lesson for this file: a
constant that reproduces cleanly on 2 samples is necessary, not sufficient — check a measurement the fix
does NOT touch before trusting it explains the whole gap.

---

## Both remaining actionable Windows defects SHIPPED (2026-07-31, follow-up pass): the HorizontalGrid
## column-width mechanism, and the selection-checkbox measurement resolved differently than either prior
## pass concluded

Picked up the two pages the immediately-preceding entry explicitly separated from the other three
(Win2D/architectural/MAUI-quirk scope cuts): `header_footer_grid_horizontal` and `selection_
synchronization`. Both got a real, shipped fix this pass — `src/platform/windows/collection_view_handler.
cpp` only, commit `826c197b3a`. `check_winrt_includes.py`: `checked 31 file(s): 0 problem(s)`. `dev.sh` NOT
run: `collection_view_handler.cpp`'s windows partial is only added to `MAUI_CONTROLS_ITEMS_PLATFORM_SOURCES`
when `MAUI_BACKEND STREQUAL "windows"` (`CMakeLists.txt:1131`) — the headless preset dev.sh drives never
compiles this file, so it provides no signal and running it would just restate the existing 3775/3775
baseline. No shared/cross-platform code was touched by either fix. UNVERIFIED BY BUILD (no Windows
toolchain on this Mac) — everything below is a pixel-measurement-and-source-reading argument, not a
compiled-and-run result.

### `header_footer_grid_horizontal`: the prior entry's own diagnosis was right, and DID need fixing —
### it just needed a Windows-scoped mechanism, not the cross-platform one the brief assumed

The immediately-preceding entry found the real defect (MAUI wraps `"Vegetables.jpg, 3"` onto 2 lines, cpp
renders it on 1) and traced it to a `HorizontalGrid` column-width mismatch, but declined to fix it because
it is ALSO yellow on iOS/Mac Catalyst and "the actual column-width logic lives per-backend... this needs an
agent with reason to touch iOS/Catalyst captures too." That per-backend claim is confirmed correct by
reading all four backends' `collection_view_handler` sources this pass (android/apple/ios/windows each
implement their own independent grid column-sizing code — `grid_items_layout.cpp`, the one truly shared
file, is a 34-line property holder with no sizing algorithm at all). That means the CONVERSE also holds: a
Windows-only fix cannot regress and does not need to touch iOS/Catalyst, whose own yellow scores on this
page have their own (unfixed, out-of-scope-for-this-pass) causes. **The brief's premise — "the defect is
probably NOT Windows-specific... a fix may be cross-platform" — does not hold**; it's a coincidence of
symptom (all three backends happen to under-wrap the same page), not a shared mechanism, matching this same
doc's own general lesson from the neighboring `pickers`/`search_bar` entry ("3 of 4 suggest CollectionView"
was also the wrong axis to split on there).

**Mechanism, with the oracle:** `src/Controls/src/Core/Platform/Windows/CollectionView/FormsGridView.cs`
`UpdateItemSize()` — for a `GridItemsLayout` with `Orientation="Horizontal"` (this page's `"HorizontalGrid,
3"`), sets `_wrapGrid.ItemHeight = ActualHeight / Span` but leaves `ItemWidth` **unset**. An unset
`ItemsWrapGrid.ItemWidth` locks the WHOLE panel to a uniform column width derived from the FIRST realized
item, not a per-column auto-fit (the same principle this file's own `first_cell_cross` comment already
documented for the CROSS axis, just never applied to the MAIN axis in the actual arrange loop). Confirmed
by measurement, not just the oracle: `header_footer_grid_horizontal_light.png`'s 4 column text-run x-origins
(ink-measured) are 21, 109, 199, 288 — three consecutive deltas of 88/90/89, i.e. one uniform ~89px pitch
across all 4 columns, not four independently-sized ones; column 0's own natural width (its longest
un-wrapped item, `"cover1.jpg, 0"`) measures ~86px, matching that pitch within capture/AA noise. cpp's prior
code computed `row_extent` (the column's main-axis extent, for a Horizontal grid) as an independent running
max PER column-group, so a column holding a longer string got its own wider slot instead of being forced to
wrap inside column 0's real width.

**The fix:** lock `row_extent` to the FIRST column-group's value for every later group, gated on
`platform->grid && !vertical` (grep over `port/maui-reference/pages` confirms `header_footer_grid_horizontal`
is the ONLY page in this gallery using `HorizontalGrid` — every `VerticalGrid` page, e.g. `grid_grouping`/
`header_footer_grid`/`preselected_items`/`staggered_layout`, is untouched). No change needed to the earlier
per-cell probe `measure()` call (still unbounded-width) that establishes this value from column 0:
`arrange_realized_view` already re-measures each cell's content at the FINAL arranged width via its own
`view->measure(frame.width, frame.height)` call before `arrange()`, so once `row_extent` is locked to the
real value, a later column's Label wraps correctly on its own. Verified the wrap will actually occur, not
just narrow-then-clip: `Label.LineBreakModeProperty` defaults to `WordWrap` in both the C# oracle
(`Label.cs:103`) and the port's own `label.cpp:90`, and this page's item template does not override it, so
`label_handler.cpp`'s `apply_line_break_mode` maps it to `TextWrapping::Wrap` (not `NoWrap`) by default —
narrowing the arrange width reflows text across lines rather than truncating it. Also verified `label_
handler.cpp`'s `platform_arrange` unconditionally re-stamps `host.Width(frame.width)` on every arrange
regardless of what an earlier `measure()` constraint was, so there is no stale-pin risk from the two-phase
measure/arrange split.

**Falsifiable prediction:** `header_footer_grid_horizontal`'s wrap-defect share of the diff (this doc's own
prior accounting: rows 220-235/310-320 = 49% of the DARK diff) should collapse toward 0 on recapture; the
separate, architectural rotation-AA residual on the footer label (~20% of dark, the same family as the
`varied_size_selector` finding) is untouched and should survive. Whether the page fully clears the SSIM gate
or lands closer-but-still-yellow depends on how much of the remaining ~30% (scattered, not attributed to
either named mechanism) moves with it — a real open question, not a confident "now green" claim. iOS and
Mac Catalyst captures of this page are UNTOUCHED (their own per-backend grid code was not modified) and
should show byte-identical scores to before on next recapture; if they move at all, that would falsify this
entry's "independently implemented" finding above and is worth flagging loudly.

### `selection_synchronization` / `multiple_bound_selection` / `cv_visual_states`: ONE constant checkbox/
### content-overlap defect, not the two content-dependent ones the immediately-preceding pass drafted

The immediately-preceding entry found a real 4px checkbox-position error (`k_selection_checkbox_left_inset`
should be 14, not 10) but declined to ship it, having discovered what looked like a SECOND, larger,
content-dependent defect (the label's own trailing glyphs — the "2" in `"Item 2"` vs the "1" in `"Item 1"`
— measured 40px vs 28px off, a row/content-dependent gap a single mispositioned constant couldn't produce)
and correctly refused to ship a fix proven insufficient by its own measurement.

**That second measurement does not reproduce.** Re-measured `selection_synchronization_light.png` with a
full-glyph-HEIGHT dark-pixel run scan (not a single fixed y-scanline, which is what the prior pass used):
the "2" in a CHECKED `"Item 2"` row and the "1" in an UNCHECKED `"Item 1"` row both start their digit at
x=73 (68-79 span, depending on the individual digit glyph's own width) — the SAME position, not 85-89 vs
74-75. A single scanline crosses a glyph at an arbitrary height, and a digit's ink-bearing at that height is
shape-dependent ("1" is a narrow vertical stroke, "2" is wide and rounded) independent of any real per-row
position difference — that shape noise, not a real content-dependent shift, produced the 40-vs-28 reading.
Confirmed the label's content position is genuinely constant (not content/state-dependent) two more ways:
(a) the checkbox+"Item" glyph run measures identically, x=27-67, for BOTH the checked and unchecked row on
the same page; (b) a second, larger-font page (`cv_visual_states_light.png`, "Multi Selection" section)
shows the identical qualitative pattern — checkbox ~22-40, "Item" glyphs resume flush against its right
edge on every row regardless of checked state, no font-dependent variation in the OVERLAP mechanism itself
(the checkbox is a fixed ~20px glyph, not font-scaled).

**The real, single mechanism:** the label's content is not, and never was, positioned relative to the
checkbox at all — it renders at its raw, checkbox-oblivious cell position (matching `paint_selection_
checkbox`'s own pre-existing header comment, which already flagged this and shipped nothing: "the checkmark
glyph visibly overlaps the label's leading character... content is NOT inset to make room for the
checkbox"). Real MAUI's content starts flush against the checkbox's OWN right edge (`checkbox_left +
checkbox_size`, no additional gap) — not co-located with the checkbox's left edge, and not shifted by any
row/content-dependent amount.

**The fix, two parts, shipped together (the immediately-preceding pass explicitly reserved the right to
ship both once this ambiguity resolved):**
1. `k_selection_checkbox_left_inset`: 10 -> 14, matching the already-measured, already-drafted correction
   (checkbox's own solid-fill bounding box sits at absolute x=27 on a row whose slot is x=13).
2. New `k_selection_checkbox_content_inset` (`= k_selection_checkbox_left_inset + k_selection_checkbox_size`
   = 34): the cell's CONTENT arrange rect (not its selection-chrome slot rect, which stays whole-cell for the
   fill/indicator/checkbox) is shifted right and narrowed by this amount, LIST cells only (`!platform->grid
   && platform->allows_multiple_selection`) — a GRID cell's checkbox sits in the TOP-RIGHT corner
   (`paint_selection_checkbox`'s `grid` branch), nowhere near where grid content starts, so `preselected_
   items` (GRID, Multiple selection, green) is untouched by construction, verified by reading its own
   checkbox-positioning branch (`k_selection_checkbox_margin`, a different constant this fix never touches).

**Per-page falsifiable predictions, computed from each page's OWN pre-fix pixel measurement, not just
asserted:**
  - `selection_synchronization` (target): pre-fix cpp slot origin ~13 (checkbox at 23 = 13+10). Post-fix
    checkbox = 13+14 = **27**, exactly matching MAUI's measured 27. Post-fix content start = 13+34 = **47**,
    within 1px of MAUI's ~48 (derived from the digit-position argument above). Should clear the SSIM gate.
  - `multiple_bound_selection` (GUARD, dark SSIM 0.9831 — only 0.0031 of margin, the tightest guard in this
    fix's blast radius): pre-fix cpp slot origin ~12 (checkbox at 18 = 12+~6, some capture-noise slop already
    present). Post-fix checkbox = 12+14 = **26** vs MAUI's measured 22 — a **4px residual**, same direction
    and magnitude as this fix's other predicted residuals, expected because the correction constant is
    calibrated globally, off `selection_synchronization`'s own slot, not per-page. Post-fix content =
    12+34 = **46** vs MAUI's ~42 (derived the same way) — also a ~4px residual. Both residuals are a small
    fraction of the CURRENT >20px gap this page already tolerates at 0.9831 — predict this page's dark SSIM
    IMPROVES and stays green, not regresses; would be a real problem (and a real prediction failure) if it
    instead moved yellow.
  - `cv_visual_states` (green, light SSIM 0.9859 — flagged by the immediately-preceding entry as relevant to
    "whoever picks up defect (2)," not one of this task's originally-named guards): pre-fix cpp slot origin
    ~8 (checkbox at 15 = 8+~7). Post-fix checkbox = 8+14 = **22**, exactly matching MAUI's measured 22.
    Post-fix content = 8+34 = **42**, within 0-2px of MAUI's ~40-42. The cleanest prediction of the three —
    should move measurably closer to 0 diff, not just stay green.
  - `preselected_item` (GUARD, singular, `SelectionMode="Single"` — no checkbox drawn at all,
    `allows_multiple_selection` false) and `preselected_items` (GUARD, plural, GRID — the `!platform->grid`
    gate excludes it): both structurally untouched by either changed constant; predict BYTE-IDENTICAL
    captures, not just "still green."

Regression pages named above (`selection_synchronization`, `multiple_bound_selection`, `cv_visual_states`,
`preselected_item`, `preselected_items`) individually verified present in both `port/cpp/examples/gallery/
pages/` and `comparison.json` with a `windows` platform entry before being cited. Full accounting of every
`SelectionMode="Multiple"` page in `port/maui-reference/pages/*.xaml` (4 total: the 3 above plus
`preselected_items`) confirms no other page is affected either way.

**What in the immediately-preceding entries turned out to be wrong:** the brief's premise that `header_
footer_grid_horizontal`'s defect is likely shared/cross-platform code (it's independently implemented per
backend — confirmed by reading all four `collection_view_handler` sources, not inferred); and the "two
compounding, content-dependent defects" reading of the selection-checkbox gap (it's one constant defect,
the apparent content-dependence was digit-glyph-shape measurement noise from a single-scanline scan). Both
corrections came from applying this doc's own recurring lesson — re-measure with a DIFFERENT method before
trusting a clean-looking number — one level deeper than the passes before them did.

---

## `generic.xaml` is READABLE ON THE GUEST — use it as a first-class oracle

The single most useful tooling discovery of this pass. WinUI's full default theme dictionary ships as
plain XAML inside the Windows App SDK NuGet package on the VM:

```
C:\Users\Testings-VM\.nuget\packages\microsoft.windowsappsdk\1.7.250606001\
    lib\net6.0-windows10.0.18362.0\Microsoft.WinUI\Themes\generic.xaml
```

Grep it with `Select-String` over SSH. It resolves every `{ThemeResource ...}` key the port has been
hand-approximating, INCLUDING per-theme colour values (the Light dictionary and the Dark dictionary each
redefine the same keys; a third HighContrast block uses `#FF0000` placeholders — do not mistake it for a
real value). This directly settled four separate questions in one pass that had previously been guessed:

| question | key | light | dark |
|---|---|---|---|
| checked glyph colour | `ListViewItemCheckBrush` -> `TextOnAccentFillColorPrimary` | `#FFFFFF` | **`#000000`** |
| unchecked square fill | `ListViewItemCheckBoxBrush` -> `ControlAltFillColorSecondary` | `#06000000` | `#19000000` |
| unchecked square border | `ListViewItemCheckBoxBorderBrush` -> `ControlStrongStrokeColorDefault` | `#72000000` | `#8BFFFFFF` |
| check glyph size | `MultiSelectCheck` FontIcon (:15762) | `16` | `16` |

**Two traps this exposed, both of which had already cost real errors:**

1. **Theme-INVERTING keys.** `TextOnAccentFillColorPrimary` is white in light and BLACK in dark. Any port
   code that hardcodes "white glyph on accent" is right in one theme and wrong in the other, and a
   light-only measurement will never catch it. Check both dictionaries for every key.
2. **Deliberately TRANSLUCENT keys.** `ControlAltFillColorSecondary` is `#06000000` / `#19000000` — 2%
   and 10% black. Compositing these down to an opaque RGB (which the port had done) discards the entire
   point of them: what shows THROUGH. The unchecked selection square is translucent enough that the
   label's leading glyph is legible through it.

**Scope limit — read this before citing it.** MAUI's CollectionView renders through the NATIVE
`ListViewItemPresenter`, which rasterises the check glyph and the selection indicator internally. So
generic.xaml is authoritative for the BRUSHES (the presenter reads the same resource keys) but has NO
geometry for those two visuals: the indicator gets only a brush (:2319) and `CornerRadius` 1.5 (:2304),
never a width or height. Where geometry is needed, ruling 11 applies — the render decides the value, and
generic.xaml's sibling XAML template is corroboration, not citation.

## Open item for a user ruling: `selection_synchronization` and the SSIM floor

The page is **pixel-converged and still yellow**, and no further pixel work will change that.

- diff: **53px light / 63px dark out of 819200 = 0.01%**, clearing the `<=1.0%` bar by two orders of magnitude
- SSIM: **0.9816 light (passes) / 0.9696 dark (fails the 0.98 bar)**

SSIM is barely tracking pixel count on this page: it moved +0.0016 while the diff fell 235px->137px, and
+0.0017 more while it fell 137px->63px. Extrapolating, even reaching zero diff would not obviously clear
0.98. This is the same structural-SSIM behaviour already recorded for the four-SSIM-page cluster — SSIM's
windowed response punishes compact, high-contrast, localized differences far out of proportion to area.

Everything mechanically identifiable on this page has been fixed and verified MAUI-exact: checkbox box
geometry, glyph size, glyph colour per theme, translucent unchecked fill, content offset, selection
indicator extent and position. The known residual is ~36px where MAUI's unchecked square border does not
occupy x46 (its label's leading "I" shows there instead) plus pill corner-radius antialiasing.

**Ruling requested:** treat this page as converged and exempt on the dark SSIM bar, or keep grinding a
0.01%-diff page? Recommend the former — the remaining delta is below the level at which the metric is
meaningfully discriminating.

---

## `image`: Win2D landed and WORKS; the remaining defect is a MEASURE bug, not a raster bug

Win2D is fully integrated (Microsoft.Graphics.Win2D 1.3.2, provisioned + projected + activated
registration-free) and `render_font_glyph` faithfully reproduces
`FontImageSourceService.Windows.cs:57-97`. It demonstrably rasterizes. The page is still 5.44%, and
FIVE hypotheses died on measurements before the real cause surfaced. Recording all of them so the next
pass does not re-run them.

**Verified working, end to end** (env-gated `MAUI_WINUI_LOG` diagnostics, now plumbed by
`run_comparison.py`):

```
font_glyph: family='Ionicons' glyph_bytes=3 size=90.0 bounds=78.75x98.09 px=81x100 ink=3898
font_copy:  copied=32400 ink=3898 solid(a>=250)=3405 max_a=255 peak=[255 255 255 255]
apply_source: kind='font' applied=1 px=81x100 opacity=1.00 vis=1 stretch=2
```

i.e. the font family resolves, Win2D draws 3,405 FULLY OPAQUE WHITE pixels, the memcpy lands every one
of 32,400 bytes in the WriteableBitmap's own buffer, and `Image.Source` reads back as a visible 81x100
BitmapSource at opacity 1. Nothing throws anywhere.

**Hypotheses REFUTED by measurement (do not retry):**

1. *Win2D activation failed / DLL missing.* No — `Microsoft.Graphics.Canvas.dll` and `ionicons.ttf` are
   both beside `gallery.exe`, and the render logs prove activation succeeded.
2. *The page passes an empty glyph.* No — `glyph_bytes=3` (U+F30C in UTF-8), matching the twin's
   `Glyph="&#xf30c;"`.
3. *A later mapper pass clears Source.* No — `clear_source` logs `had_source=0`, so those clears belong
   to other Image controls on the page.
4. *`Invalidate()` runs before the bitmap is attached, so it pushes to no composition surface.*
   Plausible, and REFUTED: re-invalidating after `image.Source(...)` changed nothing. Reverted.
5. *Premultiplied-vs-straight alpha, or a pixel-format mismatch.* No — the ink is `[255 255 255 255]`,
   valid premultiplied opaque white.

**THE DECISIVE EXPERIMENT.** Filling the same buffer with OPAQUE RED instead of the glyph rendered
**90,528 red pixels — the ENTIRE band**, exactly the area the green background had occupied. So the
WriteableBitmap plumbing is perfect and the bitmap IS displayed. That single result partitions the
problem away from rasterization entirely.

**THE ACTUAL CAUSE.** `stretch=2` (`Stretch.Uniform`) plus a band that the bitmap fills edge-to-edge
means the Image is scaled by WIDTH and OVERFLOWS: 81x100 -> 984x1215, of which only the top ~92 rows
(≈7.6 SOURCE rows) are ever on screen. And the glyph's ink does not start until source row 14:

```
row_ink[0..13] = 0,0,0,0,0,0,0,0,0,0,0,0,0,0    (the 81x100, size-90 bitmap)
row_ink[0..13] = 0,0,0,8,11,9,11,12,14,14,...   (the 20x24, size-20 bitmap)
```

The visible slice is rows 0-7, which are empty — hence a band of pure `(0,128,0)`. The ink is not
missing; it is BELOW the fold.

That empty top margin is CORRECT and oracle-faithful: `LayoutBounds` (which the oracle uses verbatim,
:82-84 and :89-93) includes the line box's leading above the glyph, so `-LayoutBounds.Y + 1` places the
LAYOUT top at row 1 and the ink lower. MAUI produces the same bitmap.

So the divergence is that MAUI's Image FITS its row while the port's overflows it. **The remaining bug
is in the Image measure/constraint path** (`image_handler.cpp`'s note 2 area —
`update_platform_max_constraints` / the unconstrained cross-axis), NOT in the glyph rasterizer. Note
that `image_handler.cpp`'s header still asserts the overflow happens "in BOTH the MAUI capture and, now,
this port's" — that was written when the stand-in was a SQUARE transparent bitmap; with a real 81x100
glyph the aspect differs and the claim no longer holds. Fix that comment when fixing the measure.

### RESOLVED 2026-08-01 — it was the RASTERIZER after all (canvas box), not the measure path

The section above is correct that the Image overflows its row and that only the top ~92 output rows are
ever visible. Its **conclusion is wrong** on one point: *"That empty top margin is CORRECT and
oracle-faithful … MAUI produces the same bitmap."* MAUI does not.

`src/`'s `FontImageSourceService.Windows.cs` is a POST-10.0.71 revision. The board renders against
shipped **10.0.71** (`port/maui-reference/app/MauiReference.csproj:18`), and the two differ in exactly
three lines — `:80`, `:83-84`, `:90-91`:

| | `src/` snapshot | shipped 10.0.71 |
|---|---|---|
| layout box | `CanvasTextLayout(.., fontSize, fontSize)` | `CanvasTextLayout(.., 0, 0)` |
| canvas size | `LayoutBounds.{Width,Height} + 2` | `DrawBounds.{Width,Height} + 2` |
| draw offset | `-LayoutBounds.{X,Y} + 1` | `-DrawBounds.{X,Y} + 1` |

`LayoutBounds` is the LINE BOX; `DrawBounds` is the INK box. Measured on the guest, before → after:

```
size=20  bounds=17.50x21.80  px=20x24  ink=241     ->  bounds=17.50x17.50  px=20x20  ink=241
size=90  bounds=78.75x98.09  px=81x100 ink=3898    ->  bounds=78.75x78.75  px=81x81  ink=3898
```

Same ink, tighter canvas. The widths were never the problem (`DrawBounds.Width == LayoutBounds.Width`:
this glyph fills its advance); the line box's ~20% vertical leading was, because the page's ~50x
`Stretch=Uniform` blow-up turns a 3-row top margin on a 24-row canvas into ~148 output rows — more than
the entire 92-row visible band. Dumping the new bitmap shows alpha ink at rows **0**-17, cols 1-18: the
antialiased raster spills across the nominal 1px pad at the top, which is why MAUI (and now the port)
has saturated ink in the band's very first pixel row.

Per **parity ruling 11** (render wins) the port follows the shipped revision, documented in
`image_source_services.cpp`'s `render_font_glyph`. `update_platform_max_constraints` and the rest of the
measure path were NOT touched — they were never at fault. Result: `image` windows
**5.44% / SSIM 0.967 (yellow) -> 0.00% / SSIM 0.9993 (green)**, light and dark, `cpp` and `xaml`; band
rows 700-791 now read green=50301 / row700=626 / row790=435 in all three columns, identical.

---

## RULING REQUESTED (ruling 3): MAUI drops the `context_flyout` 🆒 FontImageSource; the port renders it

**The port is arguably the CORRECT one here, so this is not something to "fix" without a ruling.**

Both the shared twin (`port/maui-reference/pages/context_flyout.xaml:22-26`) and the code-first page
(`context_flyout_page.hpp:259-262`) author the same element:

```xml
<Image MaximumHeightRequest="200" MaximumWidthRequest="200">
  <Image.Source>
    <FontImageSource Glyph="🆒" FontFamily="Arial" Color="MediumPurple" Size="50" />
  </Image.Source>
</Image>
```

MAUI renders NOTHING for it — the row collapses. The port renders it correctly. Measured:

| column | Bing WebView starts | 🆒 glyph |
|---|---|---|
| maui | y=234 | absent (0 px) |
| cpp  | y=434 | 6,472 px, y229-537 |

exactly the 200px `MaximumHeightRequest`. The port's rasterization is demonstrably right — guest diag
says `family='Arial' glyph_bytes=4 size=50.0 bounds=52.78x52.78 px=55x55 ink=1114` with
`peak=[216 112 147 255]`, i.e. BGRA for **MediumPurple**, precisely what the markup asks for.

Why MAUI drops it is NOT established. The most likely mechanism is that its Win2D rasterization of
this emoji throws and `FontImageSourceService.Windows.cs:47-51` LOGS AND RETHROWS, so the image
source fails and the Image gets no source at all. The port deliberately swallows and degrades there
(documented deviation — a rethrow would kill the gallery mid-capture), which is why it survives and
renders. That is a hypothesis, not a finding: it cannot be confirmed without instrumenting MAUI.

**This is exactly ruling 3's shape** — a new MAUI-side imperfection not covered by rulings 2/7/8/9/10.
Per that ruling it is flagged here and NOT acted on. Making the port drop a glyph it renders correctly,
purely to reproduce a MAUI defect, would mean deliberately breaking a working feature; that is the
user's call, not an implementer's.

**It does not affect the board either way.** `context_flyout` cannot go green regardless: with the
200px offset removed by translation, the page drops 59.88% -> 3.49% over the overlapping region, but
the **Bing band alone still differs by 22.76%** because both columns load live `https://bing.com`
seconds apart. That is ~13% of the frame against a 1.0% bar. The page is genuinely unscoreable and
the standing exemption is correct — now with a number behind it rather than an assumption.

---

## CORRECTION: the border RenderTransform did NOT regress `date_picker`

Two commits on this branch (`dbd5657d31`, `65591f8c95`) state that `date_picker` regressed from SSIM
0.9974 to 0.9838 "under the border RenderTransform change". **That attribution is wrong.** Measured
directly against the pre-change capture (`a54f91a896~1`):

```
port BEFORE the border fix  vs  port NOW :     0 px changed
port BEFORE                 vs  maui     : 4996 px
port NOW                    vs  maui     : 4996 px
```

The port's render of this page is BYTE-IDENTICAL before and after every commit in that batch. The
gradient box measures x21-124 (w104) in both, with the same solid pixels at x34/x37. Nothing the
border change did touched this page.

What actually moved was the MAUI reference capture. Early in this session `import_run_captures.py`
was rewriting files under `captures/windows/maui/` before that hazard was noticed; the 0.9974 reading
was taken against a MAUI capture that has since been replaced. Ruling 6 exists precisely to stop
this, and it is now applied after every import — but the two commit messages above were written
before the correction and should be read with this note.

## RETRACTED: the `date_picker` "8px defect" was a MIDNIGHT DATE ROLLOVER, not a port bug

0.61% light / 0.62% dark, SSIM 0.9838/0.9841 — passing, but nearest the 0.98 bar of any page. Two
distinct differences, both long-standing and neither caused by recent work:

1. **The gradient box is 8px too wide.** MAUI paints x21-116 (w96); the port paints x21-124 (w104).
   Same left edge, so this is a WIDTH/measure difference, not a position one.
2. **MAUI renders inner detail the port does not.** At y=148 MAUI reads (0,0,163) at x34 and
   (0,0,73) at x37 — dark pixels antialiased over the blue, i.e. real content inside the box — while
   the port is flat (0,0,255) across the whole span.

Three gradient rows (blue y133-163, green y196-226, blue y259-289) plus a grey band at y747-759 carry
essentially all 4,996 differing pixels. Whoever picks this up should start from the DatePicker's own
width measurement rather than the border handler — the border geometry is proven identical.

### Retraction detail (the section above is superseded)

The "gradient box is 8px too wide" and "MAUI renders inner detail the port does not" findings were
BOTH artifacts of the two columns being captured on different calendar days. A DatePicker shows
today's date: MAUI's baseline read `8/1/2026`, the port's `7/31/2026`. `7/31/2026` is a wider string,
which is the entire 8px, and the "inner detail at x34/x37" was simply different digit glyphs.

Recapturing BOTH columns in ONE run settles it — the guest clock read 2026-08-01 04:54 +03:00:

```
same-run date_picker light diff : 4996 px -> 606 px
maui coloured span              : x21-116 (w96)
cpp  coloured span              : x21-116 (w96)   <- identical
```

Scored: `date_picker` cpp 0.61%/SSIM 0.9838 -> **0.07%/0.9973** (both themes); the xaml column is now
**1.0000/0.00%**. `time_picker` likewise 0.05%/0.06%, xaml 1.0000. The page was never defective.

**The general hazard, worth remembering:** any page rendering a live date or time is only comparable
when both columns come from the SAME run. A board assembled from captures spanning midnight will show
a stable, plausible, entirely fake diff on those pages — and it looks exactly like a layout bug,
because a different date string really is a different width. Three separate wrong conclusions were
drawn from this one artifact (a border regression that never happened, an 8px measure bug, and
missing inner detail) before anyone simply cropped the region and read the numbers on screen.

---

## BLOCKER: the MAUI Android reference cannot render DARK — its dark column is not ground truth

Measured, with the capture script's own component
(`dev.mauicpp.mauireference/crc64f234786c1765579a.MainActivity`), force-stopping between launches:

```
MAUI_THEME=Light  -> body mean 137.7
MAUI_THEME=Dark   -> body mean 139.3      <- no theme change whatsoever
port (cpp/xaml)   -> light 136.1, dark 81.1   <- a real theme change
```

The intent extra reaches the app (the sibling `MAUI_COMPARE_PAGE` extra, read on the same line of the
same `CreateWindow`, correctly selects the page), but `UserAppTheme = Dark` produces no visible change.
`Platforms/Android/Resources/` contains only `values/` — there is no `values-night/` — though the
activity config itself looks standard (`Maui.SplashTheme`, `ConfigChanges.UiMode` present). Root cause
not yet isolated.

**Why this surfaced only now, and why the old green was false.** The Android C++ columns were never
sent an appearance at all (the old path passed only `MAUI_THEME`, which the C++ apphost does not read —
it reads `MAUI_APPEARANCE`), so BOTH columns rendered light and the dark slot scored green by
coincidence: light-vs-light. Sending the appearance correctly made the port render genuinely dark,
which exposed that the reference does not.

**So the current Android dark result — 166 red of 172 — is NOT a port regression.** It is the port
rendering dark correctly against a reference that renders light. Reading those reds as port defects
would be exactly backwards: on this axis the port is right and the ground truth is broken. Android dark
cannot be meaningfully scored until the reference app renders dark.

## AppKit is captured but NOT pixel-scored, by design — it cannot fill 🟢/🟡/🔴 cells

`capture_appkit.py`'s own header states the intent: AppKit "can't pixel-match MAUI/Catalyst (different
UI framework — NSViews vs UIKit)", so the requirement there is COMPLETENESS — every element specified
in the code/XAML must be PRESENT, and `appkit_cpp` must not differ from `appkit_xaml`.

Confirmed in the data: `comparison.json`'s maccatalyst entry carries `pixel` and `pixel_xaml` only —
there is no `pixel_appkit*` key — and `pixel_score.py` contains no appkit handling at all. The two
AppKit columns appear as screenshots in the board and are reviewed, not scored.

Consequence for the requested summary table: a per-platform 🟢/🟡/🔴/⬛/⏳ count can only be built from
pixel-scored columns, so the macOS row must mean CATALYST. AppKit's two columns are a separate
completeness dimension and would need their own metric (element presence, and cpp-vs-xaml agreement)
before they could contribute counts. Inventing 🟢/🟡/🔴 numbers for them would fabricate a
pixel-parity claim the capture path explicitly disclaims.

## ✅ Cross-column identical captures are PIXEL-PERFECT PARITY, not a defect (2026-08-04)

`check_capture_integrity.py` exits 1 on the committed tree with 395 "CROSS-COLUMN" findings. **They are
not a data-integrity failure.** An earlier note here claimed the board tallies were uncitable; that was
wrong and is retracted.

**Evidence that these are independent captures, not one file copied into three slots:**

* **mtimes differ per column, in capture-run order.** `ios/box_view_light.png`: maui `08-01 18:43:38`,
  cpp `08-03 16:35:09`, xaml `08-03 17:09:09` — three separate passes. A propagated frame would land at
  one moment, not spread across two days.
* **light and dark still differ** in every one of the 172 cross-column cases (ios 45, android 69,
  windows 58 pages; **0** with `light == dark`). The real 2026 incident this check was written for had
  `cpp/{light,dark}` and `maui/dark` all one file — that signature is absent.
* Byte-identical output **is achievable here**: same simulator/emulator, same `screencapture`, same
  resolution, frozen status-bar clock (9:41), deterministic PNG encoder. Identical pixels ⇒ identical
  bytes. My earlier "two independent processes cannot produce byte-identical PNGs" was simply false for
  this pipeline.
* **maccatalyst has 0** — consistent, because a Catalyst window capture carries live menu-bar chrome
  and a window shadow, so it can never be byte-stable.

**So the board tallies stand**: ios 162/10/0, maccatalyst 163/7/2, android 160/10/2, windows 171/0/1.
The duplicates are the strongest possible parity result — the port's output is *literally the same
bytes* as MAUI's on 172 page/platform combinations.

**Action on the tool, not on the data:** `check_capture_integrity.py`'s cross-column rule was written
before the port could hit pixel-perfection, and it now produces false positives that will only grow.
It should be narrowed to the signature that is actually always wrong — cross-column identity **combined
with** `light == dark` in a column, or with a mismatch against the run manifest — rather than
cross-column identity alone. Until then its non-zero exit on this tree is expected, and that is worth a
line in its docstring so the next reader does not repeat this mistake.

---

## iOS `clip`: the code-first gallery page renders 60pt of content the ground truth never authored
### 2026-08-06 — needs a user ruling (scope), not a code fix I should make unilaterally

**What the board shows.** On iOS, `clip` scores `cpp = red` and `cpp_xaml = green`. The XAML column
renders the same shared twin MAUI does, through the port's loader, and matches it. Only the code-first
column diverges — so this is not a framework defect, it is a page-authoring difference.

**Measured, not inferred.** Driving the page on the simulator through idb, four SLOW fling-free drags so
each column pins at its own maximum content offset (alignment error 0.00 px in every measurement — the
two frames are a pure vertical shift of one another, i.e. identical content):

| column | max content offset |
|---|---|
| maui (shared twin)      | **361.0 pt** |
| cpp (code-first)        | **421.0 pt** |
| difference              | **60.0 pt** |

A fast swipe reproduces the same 60.0 pt gap (maui 361.0, cpp 421.0), and a slow 208 pt drag that
reaches neither end gives **0.0 pt** difference. So the geometry above the fold is identical and the
divergence is entirely trailing extent. Both columns are deterministic here: each measured 0.00%
against itself across two runs, which is what makes the 30.03% cross-column difference a finding rather
than sampling noise (see the NON_REPRODUCIBLE_DRIVE note in tools/parity/lib/motion_score.py).

**Cause, located.** `port/cpp/examples/gallery/pages/clip_page.hpp:123-130` appends a status label
("Clipped") and a real Button ("Toggle clip on/off") after the four clipped images. Its own header says
why: *"The XAML has no interaction (no code-behind), but the gallery convention is an observable
readout"*. That is deliberate, and it is the whole 60 pt.

**Neither oracle authors it.** `port/maui-reference/pages/clip.xaml` has exactly four `<Label>`s
(RectangleGeometry / EllipseGeometry / GeometryGroup / PathGeometry) and no button; the ORIGINAL
`src/Controls/samples/Controls.Sample/Pages/Core/ClipPage.xaml` has the same four and no button. So
ruling 12 does NOT apply — the shared twin is a faithful copy, not a degraded one, and there is no
original content for the code-first page to be preserving.

**Why it hid until now.** The extra rows sit BELOW the fold. At rest the two columns are pixel-identical
(the driven run's frame 0 reads 0.00%), so no still comparison could ever see it. It only surfaces once
the board drives a scroll and the columns clamp at different bottoms.

**Why this is a user decision.** Under ruling 1 MAUI's render is ground truth for page CONTENT, which
says remove the readout. But the readout is a deliberate gallery convention for making an otherwise
inert page observable, and the driven-scenario work depends on readouts elsewhere to witness
interaction. Deleting it silently would trade a parity diff for a hole in the motion evidence. The
options are: (a) drop the two rows from the code-first page and accept that `clip` cannot witness its
own toggle; (b) add the same two rows to the shared twin so all three columns agree (this invalidates
the MAUI column for this page and needs a recapture); (c) exempt the page and record it.

**Scope.** `grep` for this convention across `port/cpp/examples/gallery/pages/` matches ONE page —
`clip`. So this is a single-page issue, not a class.

**NOT explained by this.** The other four iOS scroll reds are a different shape and remain open:
`box_view` (cpp red / xaml yellow), `clip_gallery` (cpp yellow / xaml red),
`path_gallery` (cpp red / xaml yellow), `selection_synchronization` (both red). Their column patterns do
not match `clip`'s, so the same cause cannot simply be assumed.
