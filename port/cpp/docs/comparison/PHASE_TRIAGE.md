# PHASE_TRIAGE — which motion/phase yellows hide a real render defect

**Scope.** The board carries 97 yellow cells. 53 of them are yellow because of the MOTION layer, not
the static one: their review string is one of `!! PHASE ONLY, NOT DECIDABLE ON THIS LANE`,
`MOTION <n> frames paired by step …`, or `(single frame only) — NOT motion-scored`. The other 44 are
ordinary static yellows and are out of scope here. This file is the per-cell verdict for those 53.

**Method.** Every verdict below is measured from artifacts already on disk — the published at-rest
stills under `docs/comparison/captures/`, and the full-resolution per-step frames under the run
directories the reviews name (all six of which still exist: `2026-08-19-01_32_37`,
`2026-08-19-08_27_20`, `2026-08-19-17_17_18`, `2026-08-19-22_33_30`, `2026-08-21-01_57_59`,
`2026-08-21-09_13_47`). No device, simulator or VM was touched; no lane was recaptured.

Read the numbers with two facts in mind:

* **The board's green bar is conjunctive** — `SSIM >= 0.98 AND diff <= 1.0%` (`motion_score.GREEN_SSIM`
  / `GREEN_DIFF`). Both halves must pass.
* **On a motion-scored cell the colour comes from the WORST PAIRED FRAME, not from the still.**
  `pixel_score.classify` receives `ssim`/`diff_pct` that `motion_score.score_cell` has already
  replaced with the worst frame's. So a cell can hold a byte-identical at-rest still and still be
  yellow. That is the mechanism behind 46 of these 53 cells: only 7 of the 53 have a still that fails the bar on its own (D3, D4 x4, D5 x2).

---

## 1. Headline results

| finding | count |
|---|---|
| REAL DEFECT — the difference is what makes the cell yellow | 15 cells / 8 page-platform units |
| SCORING ARTIFACT — stills equivalent; the colour is a phase sample | 32 cells |
| ↳ …of which 7 still carry a real, separately-described defect | D6, D7, D8, D10 |
| CAPTURE CORRUPTION — a column never banked its action frame | 6 cells (3 pages), **+1 green cell** (§5b) |
| STALE VERDICT — needs a targeted recapture | **0 cells** (see §4) |

**"Why is this cell yellow" and "is there a defect on this page" are different questions, and
this file answers both separately.** A cell is REAL DEFECT only when the difference is what the
board is actually colouring. Twelve pages carry a defect; on four of them (D6 `android/entry`,
D7 `android/picker`, D8 `android/empty_view_rtl`, D10 `ios/swipe_refresh`) the at-rest still passes
the green bar and the yellow comes from a phase sample — so those cells are SCORING ARTIFACT, and
fixing the port will not green them. They stay in the ranked list below because they are genuine
findings; they are just not the explanation for the colour. D10 in particular has **byte-identical
stills in both themes**, which is this task's stated scoring-artifact condition.

Two negative results that are worth as much as the positives:

* **No cell in scope is stale behind the deterministic-scroll fix.** `342236e316` changed *scroll*
  injection. Every android page whose scenario contains a `scroll`/`swipe` action — `box_view`,
  `carousel_page`, `clip`, `path_gallery`, `scroll_view`, `selection_synchronization` — was already
  recaptured at that commit. Every android cell still sitting on the older `06dcfcef48`
  (`activity_indicator`, `clip_views`, `editor`, `empty_view_rtl`, `entry`, `ios_picker`, `picker`,
  `search_bar`, `semantics`) has a `click`/`type` scenario or no scenario at all, so a scroll fix
  cannot move it. **Do not schedule a recapture on the strength of the commit date.**
  The claim is scoped to what was actually checked. `git log --since=2026-08-19` over the *capture*
  surface (`capture_android.py`, `capture_ios.py`, `vm_agent_windows.py`, `vm_agent_macos.py`,
  `run_comparison.py`, `recapture.py`) returns five commits: `342236e316` (the scroll fix, handled
  above), `7479d2ee25` (the `image` download race — `image` is a static yellow, out of scope),
  `c008cb88e1` (Catalyst geometry measurement + a display race), `d112964f98` (one-writer-per-
  destination, whose capture half is **iOS-only** — it removes the `promote_reference_captures` hop
  and has nothing to do with the maccatalyst frame gap in §5), and `91fec35eb9`, which is a *scoring*
  change and therefore applies retroactively to existing frames — scoring changes cannot create
  staleness. So no in-scope cell is behind a capture fix either.
* **No duplicate-frame corruption touches any in-scope cell**, and there are no 0-byte or truncated
  GIFs/PNGs anywhere under `captures/` (`find captures -name '*.gif' -size 0` → 0;
  `-size -1k` → 0). The cross-key duplicate sweep (hashed *within* one platform+column, never across
  columns — a maui-vs-port byte-identical pair is exact parity, not corruption) found 58 duplicate
  groups, of which all but one involve page keys that are not on the board at all (`table_view`,
  `carousel_view`, `gap_*`, `horizontal_stack` vs `horizontal_stack_layout`, …). The exception is
  noted in §5.

---

## 2. REAL DEFECT — ranked, hand these to the platform owners

Ranked by how much a platform agent can act on them immediately.

### D1 — windows: `Clip` is not applied to `Image` for GeometryGroup / PathGeometry
**Cells:** `windows/clip/{pixel,pixel_xaml}`, `windows/clip_gallery/{pixel,pixel_xaml}` (4 cells).
**Why the board could not see it:** the affected controls are **below the fold**. The published
at-rest still (`clip` 0.04%, `clip_gallery` 0.00%) only photographs the top of the page, where the
`EllipseGeometry` / `RectangleGeometry` clips *do* work. The defect only enters frame in the
`scrolled-down` step.

**Evidence.** Run `2026-08-19-17_17_18`, step `scrolled-down`, light and dark alike:

* `clip` — 19669 px differ, bbox **x8–207, y291–782** of the 1024x800 frame. Mean colour over the
  differing pixels: MAUI `rgb(210,209,211)` (page background), port `rgb(89,28,174)` (the purple
  submarine artwork). MAUI clips the image to the geometry; **the port renders the full unclipped
  bitmap.** Visible on both the "Clipped Image using GeometryGroup" and "Clipped Image using
  PathGeometry" rows.
* `clip_gallery` — 18896 px differ, single row band **y613–791**, the "Clipped Image using
  GeometryGroup" row. MAUI shows the pug photo confined to the geometry with grey background around
  it; the port shows the whole photo edge to edge.

**This is not a landing offset.** Both columns scroll *exactly* 380 px (measured by aligning each
column's own `initial` onto its own `scrolled-down`; MAUI −380, port −380, delta 0), and
`_drive_shift`'s ±48 px search correctly finds `dy=0` — no vertical translation improves the match,
because there is nothing translated.

**And it is not a stale Windows guest.** D1, D2 and D3 all come from the single run
`2026-08-19-17_17_18` at commit `06dcfcef48`, and `C:/maui-src` is a tarball copy rather than a
checkout, so this had to be ruled out. `git log 06dcfcef48..HEAD -- port/cpp/src/platform/windows/`
returns **one** commit, `eada050996`, and it is a documentation rename — no Windows backend behaviour
has changed since the capture, so the Windows agent is not being sent after something already fixed.
The last functional change in that directory before the run was `face7f7c72` (2026-08-18,
font_image_source); nothing has touched clip or geometry there since `24db16875a` (2026-08-05), so
even a guest tree a few days behind would render the same clip code. `lane_status.toml` declares no
lane stale, and records the 2026-08-08 verification that retracted the previous Windows declaration.

### D2 — windows: the `Clip` is dropped when an Entry repaints on focus
**Cells:** `windows/clip_views/{pixel,pixel_xaml}` (2 cells).
**Evidence.** Run `2026-08-19-17_17_18`, step `typed`, bbox **x1–339, y25–302**, 8936 px.
At `initial` both columns are identical: the Entry is clipped to the page's red swoosh shape. After
the click+type, MAUI keeps the swoosh clip and draws only the text and the blue focus underline
inside it; **the port replaces the swoosh with a full, unclipped white rounded TextBox** plus the
focus underline. Same class as D1 but triggered by a visual-state repaint rather than by geometry
kind — the clip is applied once and not re-applied when the background layers are rebuilt. Compare
`resize_background_layers` / the `Border.UpdateStrokeShape` latch.
Corroborated by the review's own `!! SELF-MOTION ASYMMETRY 12x` (MAUI 725 px changed, port 8591 px):
MAUI's change is the glyphs, the port's is the glyphs *plus* the whole field chrome.

### D3 — windows: `selection_synchronization` pre-selects items the ground truth does not
**Cells:** `windows/selection_synchronization/pixel` (1 cell). **This one the still can see:** dark
SSIM **0.9675** (< 0.98) at 0.19% — the only in-scope windows cell whose at-rest still fails the
green bar.
**Evidence.** Published stills, **both themes**. bbox light **x7–66, y105–490**; dark **x7–66,
y80–515**; ~1540 px. In both of the page's two sections ("Set SelectedItems then ItemsSource" and
"Set ItemsSource then SelectedItem") MAUI renders every CheckBox **unchecked**; the port renders
**items 2 and 3 checked** (filled blue `#0067C0`-family checkmark). The diff percentage is tiny only
because a checkbox glyph is small; the SSIM penalty is what exposes it.
Note this is the *inverse* of android, where the same page's selection highlight matches MAUI
exactly — so it is a Windows-side handler issue, not a shared-core one.

### D4 — ios: RadioButton content rows sit 6–12 px lower than MAUI's
**Cells:** `ios/radio_button_content/{pixel,pixel_xaml}`, `ios/radio_content_properties/{pixel,pixel_xaml}`
(4 cells). **Both fail the green bar on the still alone** — 0.9762/1.35% and 0.9697/1.31% — so the
motion string on these four is *not* load-bearing. Static scoring alone would yield the same yellow.
**Evidence.** Published stills, 1206x2622, identical in light and dark. Every differing band is a
**pure vertical offset**: shifting the port's band down by the offset below drops the residual to
near zero, and the best *horizontal* shift is always 0 (the ring x-extent and the text start x are
byte-identical — measured `(69,131)` for the ring and `(153,179)` for the first glyph run in *both*
columns).

| page | band (rows) | port offset | residual after shift / before |
|---|---|---|---|
| radio_button_content | y341–375, y594–630 | **+6 px** | 1.7 / 9.6 |
| radio_button_content | y1266–1297 | **+7 px** | 1.0 / 15.6 |
| radio_button_content | 13 further bands | +1 px | 0.0 / 3.4–21.8 |
| radio_content_properties | y509–540 | **+8 px** | 2.1 / 5.4 |
| radio_content_properties | y1086–1124, y1328–1366, y1620–1658, y1862–1900, y1985–2023 | **+12 px** | 3.9 / 24.3 |

The +12 px bands are all the "It's a button inside a button" rows — RadioButtons whose `Content` is a
`ContentView`. The offset is **not cumulative page drift** (later bands return to +1), so it is a
per-row vertical placement/centring difference, not an accumulating measure error. Same family as the
already-known iOS radio measurement work (`radio_button_border` 2.76% and
`radio_button_group_gallery` 2.31% are static yellows sitting right beside these) — start from the
21 pt ring / `get_desired_size` calibration rather than re-deriving.
**Possibly already in flight:** at the time this file was written
`port/cpp/src/platform/ios/radio_button_handler.mm` carried 87 uncommitted insertions in the shared
worktree. Check with the iOS owner before starting; these four cells may move on their own.

### D5 — android: `clip_views` loses the Entry/Editor underline in dark
**Cells:** `android/clip_views/{pixel,pixel_xaml}` (2 cells). **Fails the green bar on the still:**
dark SSIM **0.9770**.
**Evidence.** Published dark stills, bbox **x13–877, y350–1072**, 14894 px. MAUI draws the Material
bottom underline under both the `Entry` and the `Editor` rows (a 1–2 px near-white rule spanning the
field width); the port draws **no underline at all**. The `Editor` placeholder colour also differs
(MAUI near-white, port mid-grey). Light theme is milder (0.9862/0.58%) but the same rows.

### D6 — android: `entry` ignores `HorizontalTextAlignment="End"`, and password bullets are spaced wrong
**Cells:** `android/entry/{pixel,pixel_xaml}` (2 cells). *The still passes the green bar*
(0.9872/0.50%), so this does **not** explain the yellow — but the difference is real, not noise.
**Evidence.** Published dark still, bbox **x0–1080, y797–1501**, 9887 px.
1. The Entry captioned "This should be on the end" is **right-aligned in MAUI and left-aligned in the
   port**. That is the whole of the large left-hand band in the bbox.
2. The password Entry renders `•••••` tightly spaced in MAUI and `• • • • •` widely spaced in the
   port — a password-transformation / letter-spacing difference.

### D7 — android: `picker` loses the Picker underline and paints buttons darker in dark theme
**Cells:** `android/picker/{pixel,pixel_xaml}` (2 cells). Still passes the green bar
(0.9899/0.29% L, 0.9861/0.42% D) — again a real difference that does not explain the yellow.
**Evidence.** Published dark still, bbox **x24–1055, y1295–2121**, 10042 px. (a) the yellow Picker at
the top of the band has a white underline in MAUI and none in the port — same missing-underline
family as D5; (b) the "Clear Items" / "Add Items" / "Replace Items" buttons are lighter in MAUI
(≈`#E0E0E0`) than in the port (≈`#C6C6C6`).

### D8 — android: `empty_view_rtl` Picker shows the SelectedItem where MAUI shows the Title
**Cells:** `android/empty_view_rtl/pixel` (1 cell). Still passes the green bar (0.9969/0.14%).
**Evidence.** Published stills, both themes, bbox **x14–240, y174–211**, 3398 px. MAUI renders the
Picker's `Title` — "FlowDirection", in the muted placeholder grey. The port renders the resolved
value — "Left to Right", in the full-strength foreground colour. Reproduced identically in the run
frames, so it is not a drive artifact.
**The discriminator that localises it:** `pixel_xaml` for the same page/platform is **0.00% in both
themes** — the XAML twin matches MAUI exactly. So this is the **code-first page** setting a
`SelectedIndex`/`SelectedItem` the XAML twin leaves unset, not a Picker handler bug. Start in
`examples/.../empty_view_rtl` page construction, not in `picker_handler`.

### D9 — android: `carousel_page` rests without MAUI's peek after paging, and hard-edges the card border
**Cells:** `android/carousel_page/{pixel,pixel_xaml}` (2 cells). Still passes the green bar
(0.9914/0.49% L, 0.9935/0.49% D) — but this cell has **two** distinct differences, and the larger one
is only visible after the swipe.
**Evidence, post-swipe (the one that matters).** Run `2026-08-21-09_13_47`, frames
`gif03…gif12@4s/12f`, a **flat 2.19–2.21% across nine consecutive settled frames** — a resting state,
not a transient. bbox x0–1080, 52032 px. MAUI rests showing a ~220 px **peek of Card 1** at the left
edge with Card 2 beginning at x≈220; the port rests with **Card 2 filling the frame edge to edge and
no peek**. Self-motion MAUI 2.2421% vs port 1.8730% (20% apart, which is why `phase_only`'s 10%
tolerance correctly refuses to forgive this one). Likely the RecyclerView snap position /
`PeekAreaInsets` — plausibly already tracked with the android carousel RecyclerView work; confirm
before re-deriving.
**Evidence, at rest (secondary, sub-threshold).** The card's `#800080` border differs in *stroke
antialiasing*: at row 1200 MAUI reads `x1=(176,96,176) x2..x5=#800080 x6=(144,32,144)` — a ~5.7 px
stroke with antialiased edges — while the port reads `x1..x5=#800080, x6=white` — a hard-edged 5 px
stroke. Identical on the right edge (MAUI x1073–1078 feathered, port x1074–1078 hard) and the top
(MAUI y138–142, port y138–141). This is the `shape_self_inset` / 0.5 DIP deflate family; 11726 px,
identical count in both themes.

### D10 — ios: `RefreshView` completes its refresh cycle faster than MAUI's
**Cells:** `ios/swipe_refresh/{pixel,pixel_xaml}` (2 cells). **At-rest stills are BYTE-IDENTICAL in
both themes**, so the end state is exact parity and no rendering change can green this cell.
**Evidence.** Run `2026-08-19-01_32_37`, frame `gif02000` (and `gif02333`, `gif02667`): MAUI is still
mid-refresh — spinner visible at ~y240, content pushed down, status label reads "Ready" — while the
port has already finished: no spinner, content back at the top, status reads "Refreshed x1". Diff
1.29%, unimprovable by any vertical shift (best `dy=0` over ±200 px), because the columns are in
different *states*, not at different offsets.
Listed as a defect rather than an artifact only because the duration difference is a genuine
behavioural divergence; it is **not** a content difference and it is **not** what the board should be
red about. Whoever picks this up: the cell cannot be greened by a render fix, only by scoring the
settled frame (see §6).

---

## 3. Full verdict table

`still` = published at-rest PNG, MAUI vs the column, Android cropped 140 rows for the status bar
(`pixel_score.score_images` `crop_top`). `worst frame` = the value the cell's colour is actually
taken from, light / dark. `[ident]` marks a byte-identical still.

| platform | tag | column | current score (worst frame L/D) | at-rest still L/D | VERDICT | evidence |
|---|---|---|---|---|---|---|
| ios | radio_content_properties | pixel | 0.9697@1.31% / 0.9698@1.26% | 0.9697/1.31% · 0.9698/1.26% | **REAL DEFECT** | D4 — rows +8/+12 px low; still fails the bar on its own |
| ios | radio_content_properties | pixel_xaml | 0.9697@1.31% / 0.9698@1.26% | 0.9697/1.31% · 0.9698/1.26% | **REAL DEFECT** | D4 |
| ios | radio_button_content | pixel | 0.9762@1.35% / 0.9774@1.18% | 0.9762/1.35% · 0.9774/1.18% | **REAL DEFECT** | D4 — rows +6/+7 px low |
| ios | radio_button_content | pixel_xaml | 0.9762@1.35% / 0.9774@1.18% | 0.9762/1.35% · 0.9774/1.18% | **REAL DEFECT** | D4 |
| windows | selection_synchronization | pixel | 0.9816@0.19% / 0.9675@0.19% | 0.9816/0.19% · **0.9675**/0.19% | **REAL DEFECT** | D3 — items 2,3 checked in the port, unchecked in MAUI, both themes |
| android | clip_views | pixel | 0.9135@4.37% / 0.8247@7.86% | 0.9862/0.58% · **0.9770**/0.63% | **REAL DEFECT** | D5 — missing Entry/Editor underline (dark); the frame numbers themselves are phase (§4) |
| android | clip_views | pixel_xaml | 0.9138@4.35% / 0.8247@7.86% | 0.9862/0.58% · **0.9770**/0.63% | **REAL DEFECT** | D5 |
| windows | clip | pixel | 0.9726@2.40% / 0.9725@2.40% | 0.9992/0.04% · 0.9992/0.04% | **REAL DEFECT** | D1 — below the fold; still cannot see it |
| windows | clip | pixel_xaml | 0.9726@2.40% / 0.9725@2.40% | 0.9992/0.04% · 0.9992/0.04% | **REAL DEFECT** | D1 |
| windows | clip_gallery | pixel | 0.9823@2.31% / 0.9833@2.30% | 1.0000/0.00% · 1.0000/0.00% | **REAL DEFECT** | D1 — GeometryGroup row, y613–791 |
| windows | clip_gallery | pixel_xaml | 0.9823@2.31% / 0.9833@2.30% | 1.0000/0.00% · 1.0000/0.00% | **REAL DEFECT** | D1 |
| windows | clip_views | pixel | 0.9874@1.09% / 0.9911@1.08% | 0.9983/0.10% · 0.9987/0.09% | **REAL DEFECT** | D2 — clip dropped on focus repaint |
| windows | clip_views | pixel_xaml | 0.9874@1.09% / 0.9911@1.08% | 0.9983/0.10% · 0.9987/0.09% | **REAL DEFECT** | D2 |
| android | carousel_page | pixel | 0.9534@2.19% / 0.9612@2.21% | 0.9914/0.49% · 0.9935/0.49% | **REAL DEFECT** | D9 — flat 2.19% over 9 settled frames: MAUI rests with a 220 px peek of Card 1, the port does not |
| android | carousel_page | pixel_xaml | 0.9534@2.20% / 0.9611@2.21% | 0.9914/0.49% · 0.9935/0.49% | **REAL DEFECT** | D9 |
| android | entry | pixel | 0.9758@1.17% / 0.9868@0.64% | 0.9872/0.50% · 0.9871/0.42% | SCORING ARTIFACT *(defect D6)* | still passes the bar; yellow is IME phase. D6 is real but sub-threshold |
| android | entry | pixel_xaml | 0.9739@1.11% / 0.9868@0.64% | 0.9872/0.50% · 0.9871/0.42% | SCORING ARTIFACT *(defect D6)* | as above |
| android | picker | pixel | 0.9508@1.60% / 0.9483@1.73% | 0.9899/0.29% · 0.9861/0.42% | SCORING ARTIFACT *(defect D7)* | still passes the bar; yellow is popover phase |
| android | picker | pixel_xaml | 0.9508@1.60% / 0.9483@1.73% | 0.9899/0.29% · 0.9861/0.42% | SCORING ARTIFACT *(defect D7)* | as above |
| android | empty_view_rtl | pixel | 0.9809@0.79% / 0.9782@0.64% | 0.9969/0.14% · 0.9969/0.15% | SCORING ARTIFACT *(defect D8)* | still passes the bar; D8 is a 3398 px Picker-text difference, code-first page only |
| ios | swipe_refresh | pixel | 0.9663@1.31% / 0.9655@1.31% | 1.0000/0.00% [ident] · 1.0000/0.00% [ident] | SCORING ARTIFACT *(defect D10)* | stills BYTE-IDENTICAL both themes — the scoring-artifact condition; refresh duration only |
| ios | swipe_refresh | pixel_xaml | 0.9691@1.21% / 0.9759@1.12% | 1.0000/0.00% [ident] · 1.0000/0.00% [ident] | SCORING ARTIFACT *(defect D10)* | as above |
| maccatalyst | check_box | pixel | 0.9972@0.11% (light only) | 0.9972/0.11% · 0.9973/0.11% | **CAPTURE CORRUPTION** | §5 — MAUI banked `[initial]`, port banked `[initial, checked]` (dark) |
| maccatalyst | check_box | pixel_xaml | 0.9965@0.12% (light only) | 0.9965/0.12% · 0.9966/0.11% | **CAPTURE CORRUPTION** | §5 |
| maccatalyst | slider | pixel | 0.9978@0.08% (dark only) | 0.9978/0.09% · 0.9978/0.08% | **CAPTURE CORRUPTION** | §5 — MAUI banked `[initial]`, port `[initial, dragged-right]` (light) |
| maccatalyst | slider | pixel_xaml | 0.9966@0.11% (dark only) | 0.9965/0.12% · 0.9966/0.11% | **CAPTURE CORRUPTION** | §5 |
| maccatalyst | ios_date_picker | pixel | n/a — no frames | 0.9978/0.09% · 0.9978/0.08% | **CAPTURE BUG — FIXED `0e97fa9652`** | present sized the POPOVER, not the page; see the ios_date_picker section below |
| maccatalyst | ios_date_picker | pixel_xaml | n/a — no frames | 0.9965/0.12% · 0.9966/0.11% | **CAPTURE BUG — FIXED `0e97fa9652`** | as above |
| android | box_view | pixel | 0.9772@1.74% / 0.9762@1.31% | 1.0000/0.00% [ident] · 1.0000/0.00% | SCORING ARTIFACT | single-frame spike at burst frame 3, tail 0.00% (§4) |
| android | box_view | pixel_xaml | 0.9146@5.95% / 0.8368@14.79% | 1.0000/0.00% [ident] · 1.0000/0.00% | SCORING ARTIFACT | per-frame `0/0/14.79/0.03/0.53/0…0`; self-motion 36.31% vs 36.29% |
| android | path_gallery | pixel | 0.9771@1.69% / 0.9995@0.03% | 1.0000/0.00% [ident] · 1.0000/0.00% | SCORING ARTIFACT | per-frame `0/1.38/1.69/0…0` |
| android | path_gallery | pixel_xaml | 0.9771@1.69% / 0.8871@4.44% | 1.0000/0.00% [ident] · 1.0000/0.00% | SCORING ARTIFACT | per-frame `0/0.89/4.44/0.03/0…0`; self-motion 16.29% vs 15.78% |
| android | scroll_view | pixel | 0.9104@2.64% / 0.9822@0.69% | 1.0000/0.00% [ident] · 1.0000/0.00% | SCORING ARTIFACT | tail all 0.00%; `_drive_shift` already corrects −11/+11 px |
| android | scroll_view | pixel_xaml | 0.9126@2.63% / 0.9797@0.25% | 1.0000/0.00% [ident] · 1.0000/0.00% | SCORING ARTIFACT | as above |
| android | selection_synchronization | pixel | 0.9643@2.34% / 0.7068@20.92% | 0.9878/0.73% · 0.9873/0.80% | SCORING ARTIFACT | per-frame `0.80/1.85/20.92/0.93/0.67×9`; self-motion 33.03% vs 33.08% |
| android | selection_synchronization | pixel_xaml | 0.9689@1.82% / 0.9696@1.85% | 0.9878/0.73% · 0.9873/0.80% | SCORING ARTIFACT | same page, same run |
| android | clip | pixel | 0.9728@1.76% / 0.9928@0.34% | 0.9935/0.31% · 0.9934/0.31% | SCORING ARTIFACT | in-flight sample; end state agrees |
| android | activity_indicator | pixel | 0.9902@1.35% / 0.9894@0.77% | 0.9985/0.09% · 0.9984/0.10% | SCORING ARTIFACT | spinner rotation phase; self-motion 1.68% vs 1.53% |
| android | activity_indicator | pixel_xaml | 0.9891@1.53% / 0.9886@0.83% | 0.9985/0.09% · 0.9984/0.10% | SCORING ARTIFACT | as above |
| android | editor | pixel | 0.9854@1.18% / 0.9978@0.43% | 0.9997/0.47% · 0.9981/0.21% | SCORING ARTIFACT | IME slide-in phase |
| android | editor | pixel_xaml | 0.9855@1.19% / 0.9978@0.43% | 0.9997/0.47% · 0.9981/0.21% | SCORING ARTIFACT | as above |
| android | search_bar | pixel | 0.9556@2.07% / 0.9919@0.34% | 0.9927/0.52% · 0.9928/0.30% | SCORING ARTIFACT | still diff is glyph antialiasing on the ✕ marks + 1 px row rules |
| android | search_bar | pixel_xaml | 0.9598@1.83% / 0.9919@0.34% | 0.9927/0.52% · 0.9928/0.30% | SCORING ARTIFACT | as above |
| android | ios_picker | pixel | 0.9473@1.95% / 0.9471@1.98% | 0.9999/0.00% · 0.9996/0.00% | SCORING ARTIFACT | stills differ in **zero** pixels above threshold; pure popover-open phase |
| android | ios_picker | pixel_xaml | 0.9473@1.95% / 0.9471@1.98% | 0.9999/0.00% · 0.9996/0.00% | SCORING ARTIFACT | as above |
| android | semantics | pixel | 0.9792@1.18% / 0.9975@0.26% | 0.9987/0.27% · 0.9978/0.04% | SCORING ARTIFACT | still diff is 947 px in one glyph box, x953–1039 y1049–1133 |
| android | empty_view_rtl | pixel_xaml | 0.9036@7.49% / 0.9812@0.49% | 1.0000/0.00% · 1.0000/0.00% | SCORING ARTIFACT | IME-up transient in the unpaired head; see §4 |
| ios | box_view | pixel | 0.9854@0.79% / 0.9736@1.85% | 1.0000/0.00% · 1.0000/0.00% [ident] | SCORING ARTIFACT | landing offset +4/−45 px; after the −45 px correction the 57359 residual px sit ENTIRELY in rows 0–152 of 2577 — the non-scrolling header a global shift must misalign |
| ios | box_view | pixel_xaml | 0.9785@1.32% / 0.9808@1.17% | 1.0000/0.00% · 1.0000/0.00% [ident] | SCORING ARTIFACT | as `ios/box_view/pixel` |
| ios | scroll_view | pixel | 0.9651@1.91% / 0.9759@0.98% | 1.0000/0.00% · 1.0000/0.00% [ident] | SCORING ARTIFACT | landing offset −31/+19 px; `initial` is 0.00% |
| ios | scroll_view | pixel_xaml | 0.9613@2.25% / 0.9751@1.03% | 1.0000/0.00% · 1.0000/0.00% [ident] | SCORING ARTIFACT | as above |
| ios | carousel_page | pixel | 0.9581@2.24% / 0.9573@2.24% | 1.0000/0.00% · 1.0000/0.00% | SCORING ARTIFACT | per-frame alternates 0.00 / 2.18; self-motion 2.30% vs 2.35% |
| ios | carousel_page | pixel_xaml | 0.9581@2.24% / 0.9572@2.24% | 1.0000/0.00% · 1.0000/0.00% | SCORING ARTIFACT | as above |

---

## 4. Why the SCORING ARTIFACT cells cannot be greened by a port change

Every cell above marked SCORING ARTIFACT has an at-rest still that passes **both** halves of the
board's green bar, and ten of them are byte-identical. The whole of the yellow is the worst *paired
frame*, and in each case the frame series shows the same shape: one spike where a sample lands
mid-motion, then a settled tail at or near zero. Three representative series, all from the
post-deterministic-scroll run `2026-08-21-09_13_47`:

```
android/box_view/xaml   dark   0.00 0.00 14.79 0.03 0.53 0.00 …0     self-motion 36.31% vs 36.29%
android/path_gallery/x  dark   0.00 0.89  4.44 0.03 0.00 0.00 …0     self-motion 16.29% vs 15.78%
android/selection_sync  dark   0.80 1.85 20.92 0.93 0.67 …0.67       self-motion 33.03% vs 33.08%
```

Both columns travel the same distance to within 0.1–3%, the tail agrees, and the disagreement lives
in exactly the one or two samples taken while the content is in flight. That is the burst's sampling
grid, not the rendering.

**One correction to the review prose, which the orchestrator should schedule.** The
`!! PHASE ONLY` sentence still argues from `adb shell input swipe` and the 11.57% fling
self-variance. On the six android pages recaptured at `342236e316` that justification is obsolete —
those runs use the deterministic `input motionevent` path, whose self-variance is 0.0000–0.0136%.
The *verdict* is still correct (the residual is burst sampling phase, which the deterministic
gesture does not remove), but the *stated evidence* now describes a mechanism that is no longer in
use. Rewriting that sentence is a text change in `motion_score`; it changes no colour. I have not
made it, because it would need a full re-measure to land in `comparison.json` and other agents are
re-scoring individual tags concurrently.

**A second, narrower observation, reported not acted on.** `phase_only` gates on
`plat_dir in NON_REPRODUCIBLE_DRIVE`, i.e. the whole android lane. It therefore forgives pages whose
scenario contains no gesture at all — `clip_views`, `editor`, `entry`, `picker`, `search_bar`,
`semantics` are `click`/`type` only, and `activity_indicator` has no scenario. Those cells *do* have
a genuinely non-reproducible phase (the IME slide-in, a spinner's rotation), so the exemption lands
on the right answer; but it lands there for a reason the code does not state. Tightening the gate
would turn cells **red**, which is an adjudication for the user, not a triage decision.

---

## 5. CAPTURE CORRUPTION — the maccatalyst `unpairable` trio

Run `2026-08-19-08_27_20` holds these steps (verified by reading every sidecar):

| page | maui_xaml banked | cpp banked | consequence |
|---|---|---|---|
| check_box | light `initial`, light `checked`, dark `initial` | all four | **dark**: MAUI has no action frame |
| slider | light `initial`, dark `initial`, dark `dragged-right` | all four | **light**: MAUI has no action frame |
| ios_date_picker | light `initial`, dark `initial` | light `initial`, dark `initial` | **both themes, both columns**: the `opened` step never fired |

The step names that *did* land agree exactly between columns; nothing is misnamed. What is missing is
the action frame. `recapture.burst_frames` then infers "undriven" from the surviving names, drops the
at-rest frame as a non-burst sample, and the column contributes **zero** frames — which is why the
review reported `0 MAUI frames` while simultaneously blaming a step-name mismatch.

**Landed fix (message only, no colour moves):** `motion_score.score_cell`'s unpairable diagnostic now
names the real cause and lists the steps each column banked. Verified with
`pixel_score.py --verify --platform maccatalyst --only check_box,slider,ios_date_picker` →
*6 scored, 0 cell(s) differ*. Both selftests pass. The new text will appear in `comparison.json`
only on the next re-measure of those tags.

**Request to the orchestrator:** a *targeted* recapture of `check_box`, `slider` and
`ios_date_picker` on maccatalyst only. `ios_date_picker` needed no aim change: the aim is correct and the click fires. Its `opened` step
produced no frame in *either* column because `present` resized the POPOVER — root-caused and fixed in
`0e97fa9652`, see the ios_date_picker section below. (`at_macos-arm64 = [0.036, 0.0488]` in
`scenarios/ios_date_picker.toml` is CORRECT — do not retune it.)
Their stills are 0.08–0.12%, so no midnight-rollover date skew is present in these pairs.

### 5b. A SEVENTH cell sits on the same capture gap — and it is scoring GREEN

The yellow-only scan could not see it. `grep "NO step name occurs in both" comparison.json` returns
**seven** cells, not six: the extra one is **`maccatalyst/radio_content_properties/pixel`, status
green**, whose light theme reads *"run 2026-08-19-08_27_20 has 2 MAUI and 0 C++ frames"* — the port
column banked nothing. Add it to the §5 recapture request.

**Why the INVALID cap did not catch it, which is a scorer bug and is REPORTED, NOT FIXED HERE.**
In `pixel_score.classify`:

```python
frozen_both = bool(have) and any(v.get("both_frozen") for v in have.values())
```

Every sibling exemption in that same function requires **all** themes to agree, and the block's own
comment says so in as many words — *"ALL scored themes must agree it was undriven — one theme
carrying a real not-driven result still caps the cell, so a half-authored scenario cannot buy a
green."* This line uses `any`. On `radio_content_properties/maccatalyst` the **dark** theme is a
legitimate symmetric both-frozen (0 px vs 0 px), and that single theme lifts the cap off the
**light** theme's genuine missing evidence. The cell is green off one still.

Changing `any` to `all` would make the board **stricter**, never laxer, so it does not run into the
no-weakening rule. It is left undone for two other reasons: it changes cell colours, and the blast
radius has to be bounded by a board-wide `pixel_score.py --verify` — a ~12-minute grind that would
race the concurrent per-tag re-scores. The bound to expect: 24 green cells currently hold an INVALID
verdict with `why != no-scenario`; 23 of them are `not-driven`, which is exactly what USER RULING
2026-08-16 says should be green, so the fix must be checked not to sweep those up. Only the one
`unpairable` cell above is an unambiguous leak.

**Incidental duplicate finding, out of scope, worth a separate look:**
`captures/windows/cpp/swipe_refresh_{light,dark}.png` is byte-identical to
`captures/windows/cpp/table_view_{light,dark}.png`, and the same three-way identity holds in
`captures/windows/xaml/` across `carousel_view`, `swipe_refresh` and `table_view` — while the MAUI
column's stills for those keys are all distinct. `table_view` and `carousel_view` are not board
pages, and `windows/swipe_refresh/pixel` currently scores **green**, so nothing in this triage
depends on it; but one frame standing in for three keys in one column is the corruption signature and
should be confirmed rather than assumed benign.

---

## 6. What would actually move these cells

**One exemption WAS proposed, measured, and rejected — do not re-propose it from the numbers alone.**
Every maccatalyst cell carries a constant ~771 px (0.09%) mismatch in the WINDOW TITLE BAR, because the
three columns are three apps with three titles. It is genuinely chrome rather than app content, so a
mask looks like the Android `crop_top` carve-out — and it is not. The three reasons it fails, with the
measurements, are recorded in `tools/parity/lib/pixel_score.py` beside `crop_top` itself (short version:
13 pages carry real content in that box, `gap_title_bar` among them; the box is x 96-262 and is easy to
mis-size from one column; and the only 4 cells it moves are hair's-breadth crossings on pages with known
real defects). Read that note before spending time on it again.

Nothing here proposes relaxing a threshold, widening a tolerance, or loosening an alignment. The
`phase_only` and `INVALID` caps are adjudicated policy (USER RULING 2026-08-10 / 2026-08-16) and are
left exactly as they are.

* **The 15 REAL DEFECT cells** move when the port is fixed: D3/D4/D5 register on the still, D1/D2/D9
  only on the driven frames.
* **The 32 SCORING ARTIFACT cells** cannot be moved by any port change — including the 7 that carry a
  real defect (D6, D7, D8, D10). Fixing those is still worth doing; it just will not repaint the cell.
  If the board is ever to green them, the change is on the capture side — sample the burst relative to the gesture rather
  than to wall-clock, or take the verdict on the settled tail — and that is a scoring-policy decision
  for the user, not a triage one. Note that `_align`'s ±3-sample window already absorbs a uniform
  drift; what it cannot absorb is a single frame that lands *inside* the transition while its partner
  lands outside it.
* **The 6 CAPTURE CORRUPTION cells**, plus the green one in §5b, move only with the targeted
  recapture in §5.

---

## Structurally unwinnable cells — the honest ceiling (added 2026-08-22)

The board's yellow count is not a count of fixable defects. Two groups were filed here as unwinnable **by
construction** — but ONE OF THE TWO HAS SINCE BEEN SOLVED (`maccatalyst/ios_date_picker`, `0e97fa9652`),
so treat "unwinnable by construction" in this file as a claim to re-test, not a settled fact. Each was
each established by measurement rather than by giving up on them.

### maccatalyst/ios_date_picker — 2 cells — ~~CAPTURE-SIDE LIMITATION~~ **SOLVED 2026-08-22 (`0e97fa9652`)**

**This section's original verdict was WRONG and is retained only so the reasoning can be audited.** It
said "no port change can green these 2 cells; only a different way of presenting or capturing that
popover could." The second half was right, the conclusion drawn from it was not: the capture path had an
ordinary bug, and finding it took reading the run log rather than the summary.

What the original got right: the aim IS correct (`at_macos-arm64 = [0.036, 0.0488]`), the click DOES
fire, and `present` IS what fails, identically in the GROUND-TRUTH column. What it got wrong was calling
that failure structural. `run_comparison.py` reports the generic `no window to capture` line, but the
AGENT's own error, in `_recapture_logs/2026-08-19-013237-macos-catalyst--capture.log`, is specific and
names the cause twice per column:

```
  ~ present failed (window did not reach target: got 139x174, want 1024x800) — resolution-toggle self-heal, retrying
  ~ present failed (window did not reach target: got 139x174, want 1024x800) — resolution-toggle self-heal, retrying
  ! ios_date_picker/cpp/light#2: DROPPED — present failed after self-heal (no window to capture)
```

139x174 is the popover. `cmd_present` drove `set position/size of window 1`, and System Events orders a
process's windows FRONT TO BACK — so the moment the click opens the compact `UIDatePicker`
(`DatePickerHandler.MacCatalyst.cs`), that popover BECOMES window 1. present then tried to resize the
popover, missed the target by construction, self-healed, failed again, and the frame was dropped. All 6
action frames, all three columns, both themes — deterministic, which is exactly why it read as structural.

**Fix (`0e97fa9652`): address the LARGEST window instead of `window 1`.** A no-op on the ~170 pages that
only ever have one window, and it also stops present from resizing popover chrome. Measured on the VM,
all three columns, before → after:

```
before   present(opened) -> error "window did not reach target: got 139x174, want 1024x800"
after    present(opened) -> rect 128,30,1024,800, popover still up (count of windows = 2)
```

The CAPTURE side needed no change: `_window_info_quartz` already picks the largest layer-0 window, and
`screencapture -l <page id>` composites the popover into the page window's own backing store — verified
by eye, the opened December-2020 calendar is in the resulting frame.

**⚠ 1px of headroom, watch this in the recapture.** The popover sits at x=125 while the page sits at
x=128, so the `opened` frame comes back **1027x800**, not 1024x800. `run_comparison`'s size guard is
`abs(w - 1024) > 4`, so 1027 passes with ONE pixel to spare. All three columns measured 1027 (they agree),
but if any column's popover ever lands at a different x, that column's frames drop and the cell is
unpairable again — with no error message pointing here. If that happens, the fix is to crop the shot to
the page window's own Quartz bounds, not to widen the guard.

**Status: the fix is committed and verified at the agent level; the 2 cells are still yellow pending a
targeted recapture** (`--platforms macos --examples ios_date_picker --frameworks maui_xaml,cpp,cpp_xaml
--no-measure --skip-build`).

### android PHASE-ONLY — ~20 cells — CAP STANDS, BUT THE 12/16 DID NOT MEASURE WHAT IT CLAIMED

**RETRACTED 2026-08-22.** This section previously read: "`--stability --platform android --runs 4` →
**12 of 16 cells disagreed across runs** … The same build scores these cells differently on consecutive
runs". **The second sentence is false as written, and the first does not support the conclusion drawn
from it.** It was not the same build.

`--stability` selected run directories by RECENCY (`_run_dirs`, newest first) with no commit filter, and
every android run directory on disk carries a DIFFERENT commit:

| run | commit | | run | commit |
|---|---|---|---|---|
| `2026-08-22-09_21_55` | `a9e594470f` | | `2026-08-22-03_27_31` | `e115eafe6a` |
| `2026-08-22-05_56_05` | `716da124e5` | | `2026-08-22-02_07_26` | `1812647b56` |
| `2026-08-22-03_51_29` | `c511b85708` | | `2026-08-21-09_13_47` | `342236e316` |

So the four runs compared per cell were four different builds of the port, captured while the android
rendering agent was actively landing fixes. A verdict that changes across them is the board doing its
job. The 12/16 is **uninterpretable as a drive-reproducibility measurement** — it is not evidence the
drive is noisy, and it is not evidence the drive is clean.

**The tool defect is fixed.** `stability()` now groups runs BY THE COMMIT recorded in their sidecars and
compares only within a group; a cell with no two runs at one commit is reported under "no two runs AT ONE
COMMIT (not a stability measurement)" rather than silently folded in as agreement. An absent measurement
must not read as a passing one.

**First same-commit control, from frames already on disk.** `title_bar`, three runs at `716da124e5`
(`05_32_17` / `05_40_00` / `05_50_30`), no rebuild between them — the only capped page with a
same-commit repeat in the corpus. **All 4 cells (cpp/xaml × light/dark) held the SAME verdict in all
three runs.** The inputs behind the verdict did move:

```
title_bar/cpp/light   INCONCLUSIVE ×3   shift 0/-1/0   worst 4.32% / 3.58% / 7.90%
title_bar/cpp/dark    PASS         ×3   shift 0/ 0/0   worst 0.06% / 0.07% / 0.06%
```

Light spikes at burst frame 3 and its `_align` offset flips between 0 and −1; dark never spikes, and its
self-motion agrees to six significant figures (20.9439 / 20.9441 / 20.9441). Same page, same runs, same
build. So the variance is not a property of the lane or of the page — it is whether a burst sample
happens to land mid-transition.

### THE CLEAN MEASUREMENT — 4 same-commit repeats, 6 pages, 24 cells

Run 2026-08-22, repeats `16_24_40` / `17_19_32` / `17_41_01` / `18_00_44`. **Validity rests on binary
identity, not on HEAD**: all three `base.apk` md5s were byte-identical before and after the whole run
(`4a78781e…` / `2c6c1df3…` / `256326a9…`), sampled again between every repeat. HEAD moved during the
window — other lanes were committing — and that is deliberately treated as informational, because a
commit that does not touch Android changes the label without changing the artefact. Three run dirs are
excluded with cause (see `EXCLUDE` in the analysis): one contaminated by the board measure's TTFF stage
running 20 cold app launches on the same emulator, two truncated by a killed capture process.

**RESULT: 19 stable, 5 FLIPPED.** So the android verdict IS irreproducible at a constant binary, on
about 21% of cells — and the cap's *conclusion* survives even though its *justification* was void.

**WHY a cell flips, exactly.** A cell flips **iff** the board's green conjunction
(`ssim >= 0.98 AND diff <= 1.0`) is unstable across repeats — **5 of 5 flipped, 0 of 19 stable**. No
exceptions. The flips are not a different kind of page; they are the cells sitting close enough to the
bar that ordinary sampling variation carries them across it.

**AND THE MECHANISM IS NOT WHAT THE PAGE LIST SUGGESTS.** All 5 flips land on caret-blink pages
(`title_bar`, `search_bar`, `clip_views`) and none on the scroll pages — which invites the conclusion
that a blinking caret is the cause. It is not. Worst-SSIM RANGE across the 4 repeats, by page class:

    caret   n=11   median range 0.0201   max 0.0919
    fling   n= 8   median range 0.0744   max 0.1405     <- the LARGEST variation
    picker  n= 4   median range 0.0189   max 0.0222

The scroll pages vary nearly **4x more** run-to-run than the caret pages and never flip, because they
sit far outside the green region in every repeat. Sampling-phase variation is GENERAL and largest on
the fling pages; the caret pages merely happen to sit near the threshold. Reading the page list without
this table gives a confident wrong answer.

**WHAT IS ACTUALLY IRREPRODUCIBLE — the correspondence, not the drive.** Median relative run-to-run
spread over the 24 cells:

    each column's OWN self-motion (THE DRIVE):        0.65%
    cross-column worst-frame diff (CORRESPONDENCE):  53.3%

An 80x separation. `clip_views/*/dark` and `path_gallery/*/dark` repeat their self-motion to
0.00-0.06%. The gesture lands the same distance every time; what varies is WHICH MOMENT of that motion
each column's burst photographs, and therefore which offset `_align` picks — and the offset determines
the verdict. Worked example, `title_bar/cpp/light`:

    16_24_40  PASS          shift=-2  worst 0.61% @f3
    17_19_32  INCONCLUSIVE  shift= 0  worst 1.74% @f3   <- only run above the 1.0% bar
    17_41_01  PASS          shift= 0  worst 0.82% @f3
    18_00_44  PASS          shift=-1  worst 0.51% @f4

Two honest exceptions: `picker/*/dark` self-motion swings 33.7% (one repeat read 53.62 against ~80.8
elsewhere — the dialog did not open the same way) and `search_bar/xaml/dark` 10.6%. So "the drive is
reproducible" holds for most pages, not all.

**AND THE CAP DOES NOT CONTAIN ALL OF IT — 2 of the 5 flips can still go RED.** The verdict sequences:

    title_bar/cpp/light    PASS -> INCONCLUSIVE -> PASS -> PASS
    title_bar/xaml/light   PASS -> INCONCLUSIVE -> PASS -> PASS
    clip_views/cpp/light   PASS -> INCONCLUSIVE -> INCONCLUSIVE -> PASS
    search_bar/cpp/dark    INCONCLUSIVE -> FAIL -> PASS -> PASS     <- RED on one repeat
    search_bar/xaml/dark   PASS -> FAIL -> PASS -> PASS             <- RED on one repeat

A THIRD boundary is straddled, and it is not one of the green pair. In repeat `17_19_32` MAUI's OWN
self-motion on `search_bar`/dark read 23.20 against ~20.77 in the other three repeats — a spread of
10.6-10.7%, just over `PHASE_SELF_MOTION_TOL = 0.10`. `phase_only` therefore cannot fire, the cell
falls through to `FAIL/frames-disagree`, and the board shows RED on a coin flip **despite** the cap.
Both FAILs are in a clean repeat, so this is not the contaminated unit; and it means the honest claim
is narrower than "these cells are correctly capped yellow". The cap catches the flips whose self-motion
spread stays inside 10% and does NOT catch the rest.

It also punctures "the drive is reproducible" as a universal: MAUI's own gesture on that page produced
20.74 / 23.20 / 20.79 / 20.77 across four identical runs. Mostly reproducible, not always.

**THE CAP STANDS** — `NON_REPRODUCIBLE_DRIVE={"android"}` is unchanged, now on a valid measurement
rather than the void one. But its NAME and its stated reason are both wrong: the drive is not the
irreproducible part, and the `input swipe` fling story in the review text cites a mechanism no longer
in use AND the wrong pages. The tractable follow-up is the CORRESPONDENCE — score the settled tail, or
compare each column's own trajectory (self-motion is already computed, already in the review string,
and reproducible to 0.65% median) rather than frame-index-matched cross-column SSIM.

**THE PROPOSED FIX WAS DESIGNED, PRE-REGISTERED, AND MEASURED WORSE — held OFF.**
`motion_score.TRAJECTORY_SCORING` (one line, default False) scores three phase-free quantities instead
of the worst cross-column paired frame: the at-rest frame, the settled last frame, and each column's
OWN travel distance. Predicted before running: 16 PASS / 8 FAIL / **0 FLIP**. Measured on the same
4-repeat data: **8 PASS / 10 FAIL / 6 FLIP** — one flip WORSE than the paired path it would replace,
and `title_bar/cpp/dark` (a stable PASS today) starts flipping.

The error is worth recording. The design rested on "self-motion is reproducible, 0.65% median", which
is true and irrelevant to the clause built on it: the travel clause compares MAUI's travel TO THE
PORT'S, and the gap between two independently wobbling quantities is far less stable than either.

    title_bar/cpp/dark   MAUI 20.72 / 20.74 / 20.66 / 20.75
                         port 20.73 / 20.73 / 23.12 / 20.76      gap 0.05 / 0.05 / 10.6 / 0.05 %

One column takes a single-run ~11% excursion and the gap crosses the tolerance. **The median hid the
tail.** The 80x separation was a statement about typical runs and said nothing about the worst case,
which is exactly what a threshold meets.

**So the real residual is not the correspondence alone** — it is self-motion divergence between the two
columns. **CORRECTION: these are not one phenomenon, and the largest one is not an excursion at all.**
The list first written here (search_bar/dark 10.6-10.7%, picker/dark 22.5%, clip_views/dark 75.3%) came
from a script taking `max(gap)` ACROSS repeats, which cannot tell a one-run spike from a constant
offset. Read per-repeat:

    search_bar/dark    MAUI 20.74 / 23.20 / 20.79 / 20.77     one-run EXCURSION
    title_bar/cpp/dark port 20.73 / 20.73 / 23.12 / 20.76     one-run EXCURSION
    clip_views/dark    MAUI 96.43 / 96.42 / 96.43 / 96.37     STABLE, every run
                       port 26.25 / 23.83 / 23.80 / 23.83     STABLE, every run

### clip_views/dark 75.3% — SOLVED, and it is the #2F2F2F capture wash

Not scorer noise and not a port defect. Measured on the empty page background (x100-900, y1150-1250),
run `2026-08-22-17_41_01`:

    maui_xaml at-rest   RGB [47 47 47]      <- #2F2F2F, the Android dark-wash artifact
    maui_xaml settled   RGB [18 18 18]
    cpp       at-rest   RGB [18 18 18]
    cpp       settled   RGB [18 18 18]

MAUI's AT-REST BURST FRAME ALONE carries the wash; its own settled frames do not, and the port's frames
never do. Delta 29 against `pixel_score.DIFF_THRESHOLD = 25` — just over, so it counts. That is the
documented artifact whose signature is exactly this: environmental drift both apps hit, which becomes a
signal only because the three columns are captured in three SEPARATE PASSES.

So MAUI's 96.4% self-motion is "keyboard + the whole background clearing 47->18" while the port's 23.8%
is "keyboard" — same page, same gesture, one contaminated frame.

**AND IT COSTS TWO RED CELLS.** Cross-column, in that run:

    TRUE at-rest   93.49% differ (SSIM 0.7437)     <- entirely the wash
    TRUE settled    0.43% differ (SSIM 0.9889)     <- the columns AGREE

`phase_only` requires `at_rest_diff <= PHASE_AT_REST_PCT (1.0)`. At 93.49% it cannot fire, so
`clip_views/{cpp,xaml}/dark` fall through to `FAIL/frames-disagree` with worst-frame SSIM 0.82-0.91 at
8.55-24.15% and colour RED — on a page whose PUBLISHED STILLS are clean and agree. Verified: the
published `clip_views_dark.png` reads [18 18 18] in all three columns, so the still pass is unaffected
and only the burst carries it. This is the "cell is red and the artifact a human can open shows nothing
wrong" failure mode again, and the fix is a recapture of that page's GIF pass, not a port change.

**A SECOND, SEPARATE BUG — in the analysis, not the scorer.** The rest/peak/tail table above took
"rest" as `per-frame[0]`. After `_align` shifts by a nonzero offset, `pairs[0]` is no longer at-rest vs
at-rest: at offset -1 it is MAUI's gif01 against the port's at-rest. So every cell with a nonzero offset
had a wrong "rest" figure (clip_views/dark was reported 3.60%; it is really 93.49%), and the
8/10/6 trajectory prediction is built on partly wrong inputs and MUST be recomputed against true
at-rest frames before it is quoted. `trajectory_score` itself matches on `rest_step` and uses the
correct frame — the code is right, the analysis of it was not.

### The remaining excursions — SOLVED for two of three, and they refute the trajectory design's premise

The wash is REFUTED here: at-rest backgrounds are byte-stable across all four runs (search_bar 24.3,
title_bar 18.4, picker 63.0). The cause is in the LATER frames:

    picker/maui/dark     0.0 80.9 53.3 53.3 ...   transient CAUGHT   -> travel 80.86
                         0.0  0.0 53.6 53.3 ...   transient MISSED   -> travel 53.62
    title_bar/cpp/dark   0.0  0.1 23.1 20.6 ...   transient CAUGHT   -> travel 23.12
                         0.0  0.0 20.8 20.6 ...   transient MISSED   -> travel 20.76

Both pairs SETTLE identically. The dialog scrim and the IME slide OVERSHOOT their settled state, so
whether the burst samples during the overshoot is a coin flip.

**That refutes the load-bearing claim of the trajectory design.** It asserted that each column's own
travel is "phase-invariant by construction" because it is a max over the sequence. A max is
phase-invariant only for a MONOTONIC trajectory, and these are not monotonic. So `travel` samples the
same race as the correspondence it was meant to replace, one layer down.

**RE-MEASURED BY RUNNING THE CODE** (the first evaluation reimplemented the clauses by hand and used
the wrong "rest" frame):

    TRAJECTORY:  PASS 7 / FAIL 10 / FLIP 7      predicted 16 / 8 / 0      paired path: 19 stable / 5 flipped

The correction did NOT rescue it — 7 flips against the paired path's 5. The design fails, now
established on correct inputs and with the reason understood rather than guessed.

**Direction for a next attempt:** compare the SETTLED value (the last frame) rather than the MAX. That
is monotonic-free by construction. Deliberately not built here — it is a different design and deserves
its own pre-registration rather than being retro-fitted onto this one's.

**Still unexplained, and NOT a transient:** `search_bar/maui_xaml/dark` SETTLES at 23.2% in run
`17_19_32` and 20.7% in the other three — a genuine end-state difference across runs of one binary.

**STATUS OF THE FIX: DESIGNED, HELD OFF, AND NOT WORKING AS SPECIFIED.** The cause is localised (`_align`'s offset selection, driven by
which moment each burst samples) and the remedy is specified above, but neither is built. Three gates
now known to be straddled — `GREEN_SSIM`, `GREEN_DIFF`, `PHASE_SELF_MOTION_TOL` — and moving any of
them would be threshold-tuning to hide noise, which is forbidden. The fix has to change WHAT is
compared, not where the bar sits.

**REPRODUCED BY THE PRODUCTION TOOL**, not only by the bespoke analysis:

    motion_score.py --stability --platform android --only <the 6 pages> --runs 5
    24 cell(s) scored across up to 5 runs each; 0 had no two runs AT ONE COMMIT; 5 disagreed

Same five cells, same verdict sequences. This matters because the earlier bespoke script and the
shipped tool could have disagreed and nobody would have known which to believe.

**A COMMIT LABEL CANNOT GROUP THESE RUNS, and the reason is sharper than first thought.**
`write_run_unit` calls `_git_commit()` PER UNIT, so HEAD moving during a ~20-minute repeat is recorded
inside the run: `16_24_40` alone carries four distinct commits across its 468 sidecars, `17_19_32`
seven. A run does not HAVE a commit. Sidecars now also record `apk_md5` (the installed binary), and
`stability()` groups on that in preference to the commit — these four runs were backfilled with the
md5s measured on-device before, between and after the capture, all byte-identical.

The key uses BOTH columns' binaries. `score_cell`'s `evidence` is built from the MAUI column's sidecar,
so a key from it alone would identify the GROUND-TRUTH binary — which does not change when the port is
rebuilt, so two runs straddling a port rebuild would group as comparable. That is the original defect
one column over, and the first cut of this fix had it: the production run above printed
`@apk:4a78781e…`, the MauiReference md5.

**Population limit, stated next to the result:** 6 of the 19 capped pages, chosen for phase-shape
coverage. A 2-repeat pass over the remaining 13 would cheaply falsify the generalisation.

### What this means for reading the board

Subtract these ~20 before treating a yellow count as a work queue — ~20, not the ~22 this file and
STATUS.md originally said: `maccatalyst/ios_date_picker`'s 2 cells were NOT unwinnable and have been
fixed (`0e97fa9652`), leaving only the android PHASE-ONLY group. They are correctly yellow — the board is
saying "not established", which is true — but they are not defects awaiting a fix, and a plan that budgets
time for them is budgeting for something unreachable.


---

## Correction: the "0px face" discriminator on the iOS radio cells was a BAD MEASUREMENT (2026-08-22)

This document, and the briefs derived from it, told successive agents that `ios/radio_button_content` and
`ios/radio_content_properties` showed content offsets of **+6/+7/+8/+12 px**, and that a `(row - line)/2`
title-rise model was dead because it predicted **7px against a measured 0px** for Baskerville Italic 14 —
a contradiction no constant could survive.

**The disk does not show that.** Re-measured on the current build: the residuals are **0px** (system 14),
**+2px** (Baskerville Italic 14), **-5px** (system 14, wrapped 2-line) and **+6px** (Arial Bold 12).
Baskerville needs **+2px**, not 0. The font-table arithmetic predicts 1px there — a **1px miss**, not the
7-vs-0 contradiction that was recorded as fatal.

So the model was never wrong. It was **uniformly 0.45-1.2px optimistic across all four faces**, because
font tables do not expose `UILabel`'s intrinsic-height rounding. It reproduces the earlier agent's
Arial-Bold-12 figure (10.8 predicted vs 12 measured) exactly. **Dead-end #1 was the right idea killed by
the wrong instrument.**

The real fix does not need the model at all: `RadioButtonHandler.iOS.cs` hosts a bare `ContentView` whose
content column is a Fill/Fill `ContentPresenter` (`RadioButton.cs:569-573`) holding the `ContentLabel`
`ContentConverter.ConvertToLabel` builds (`ContentConverter.cs:47-66`). That Label fills the Grid row and
draws at `TextAlignment.Start` — the TOP. The port's `UIButton` CENTERS `titleLabel`, so the title sits
`(row - line)/2` low. `MauiIosRadioButton` now overrides `-titleRectForContentRect:` and pins super's rect
to `contentRect.origin.y`, which is exact by construction and needs no font arithmetic.

**The lesson for this document:** a recorded "measured 0px" killed a correct hypothesis for two passes. When
a single data point is the sole reason a model is rejected, re-measure it before writing the model off —
and record HOW it was measured, not just the number.
