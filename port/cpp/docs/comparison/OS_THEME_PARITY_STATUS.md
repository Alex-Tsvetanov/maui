# System-wide theme parity — status

Tracks the campaign to make the board prove that **both** frameworks follow the **operating system's**
theme, rather than each being pinned by a per-column environment variable.

## Why this exists

The port derives its theme from the OS as of `2a3a4eb090` (the `application` ctor seeds
`PlatformAppTheme` from `AppInfo.RequestedTheme`, Application.cs:61). But the harness still forced the
theme on both sides — `MAUI_APPEARANCE` for the C++ galleries, `MAUI_THEME` for MauiReference — and both
of those set `UserAppTheme`, which **overrides** the OS by design. So a green board said nothing about
system-wide theming.

Coverage before this campaign: the OS-dark / no-env case was measured on all four platforms; the
OS-light / no-env case only on AppKit; Catalyst never, in either theme.

## Phase 0 — capture the OS theme, not an app-level override

| step | state |
|---|---|
| (a) `maui-reference/app/App.xaml.cs` sets `UserAppTheme` only when `MAUI_THEME` is present | **done** |
| (b) `tools/parity/device_state.py` OS-theme setters, read-back verified, returning the previous value | **done** |
| (c) wire the setters into every capture path; stop passing the per-column theme env | **done** |

**Phase 0 is complete and every mechanism is measured, not assumed.** The last open assumption — that
MAUI *itself* follows the device theme once `UserAppTheme` is left Unspecified — is now confirmed on
Android with the rebuilt APK and no `MAUI_THEME` extra: device-dark renders the page at body mean 21.2
(red dark-slot text, teal DarkPrimaryColor), device-light at 251.9.

### (c) progress

| lane | path | state |
|---|---|---|
| Catalyst / AppKit / Windows | `run_comparison.py` `capture.system_theme = true` | done — theme hoisted to the outermost loop (one flip per theme instead of ~1000), restore on the normal and abort paths |
| iOS | `capture_ios_clean.py` (default; `--app-theme-env` restores the old behaviour) | done — theme hoisted outermost, appearance + status bar both restored |
| Android | `build_android_apphost*.sh`, `capture_all_csharp_android.sh` | done — theme extras dropped from both columns; night mode restored to the value found, not forced to light |

`capture_ios_clean.py --app cpp --only app_theme_binding --themes light,dark` renders light under
OS-light (bg mean 249.4) and dark under OS-dark (5.0), and both frames are **0 px different** from the
committed env-driven ones — the switch to OS-driven capture is a no-op on the output, which is the
result to want: it says the port already followed the OS and the existing iOS baseline is sound.

### (b) — verified per lane

| lane | mechanism | verified |
|---|---|---|
| Android | `adb shell cmd uimode night yes\|no` | set both ways, read back, previous value correct |
| macOS VM (Catalyst + AppKit) | System Events `set dark mode` | set both ways **and** end-to-end: gallery launched with NO env renders light under OS-light (page mean 251.7) and dark under OS-dark (42.1) |
| Windows VM | `AppsUseLightTheme` registry | proven 2026-08-03: OS dark + no env → app renders dark |
| iOS | `simctl ui <UDID> appearance` | setter written, **not yet exercised** (simulator was shut down) |

### The trap `defaults` set for us

The obvious macOS implementation is `defaults write -g AppleInterfaceStyle Dark` /
`defaults delete -g AppleInterfaceStyle`. **It does not work over SSH and it fails silently.** Measured
on this VM: `defaults read -g AppleInterfaceStyle` reported the key ABSENT (i.e. light) at the same
moment System Events reported `dark mode = true`, and a `defaults delete` left the GUI session dark. A
setter built on `defaults` passes its own read-back, reports success, and captures an entire board in
the wrong theme — invisible to every check except looking at a frame. `set_macos_theme` therefore drives
System Events, and reads it back through System Events too.

## Phase 1 — full recapture, both system themes, sequentially

Order: iOS → macOS Catalyst → macOS AppKit → Android → Windows. Never in parallel: concurrent runs
starve the VMs and bank stale or splash frames. AppKit runs `--env macos-appkit`, never concurrently
with `macos-arm64` (one desktop, one `scratch/shot.png`).

**BLOCKED on macOS (Catalyst + AppKit).** The VM's display no longer offers a 1512-wide mode — UTM
regenerates the guest's mode list when its window is resized, and every remaining mode is HiDPI, so
captures come back at 2x (960x1504 where the AppKit baseline is 480x752). A failed `set-resolution` is
now FATAL rather than a warning, so this aborts instead of burning a run. Resolve by either restoring
the UTM window to its former size (cheap — existing baselines stay valid) or re-pinning
`[environments.*.display]` to a mode that exists (rebaselines both macOS platforms, and Catalyst's
scenario tap calibration is keyed to width 1512).

iOS, Android and Windows are unblocked.

### Phase 4 — `ios/image`: the port does not composite animated-GIF frames

`ios/image` is light **3.91%** / dark **0.13%**, and the asymmetry is misleading: the affected element
is a GIF whose own canvas is BLACK, so in dark mode the defect is invisible against a dark page and
scores ~0. It is equally wrong in both themes.

Scanning every dark box in the light frames, all match except one:

| | box | height |
|---|---|---|
| MAUI | y2301-2621 | **321px** |
| port | y2501-2621 | **121px** |

Same bottom edge, port 200px shorter, and everything above and below matches exactly — so the layout is
right and the IMAGE is short. The port also renders no heart where MAUI shows one mid-animation.

**CORRECTION — "the port does not composite GIF frames" was stated too confidently.** The asset does use
partial frames (inspected: frame 0 is the full 400×300 canvas, frames 1-15 are ~44×46 rects around
(183,127)), but the oracle does not composite them either. `ImageAnimationHelper.ToConsistentImageArray`
(`src/Core/src/ImageSources/iOS/ImageAnimationHelper.cs:113-130`) only repeats frames by
`delay/gcd` to fit GIF per-frame durations into UIImage's single `duration` — it is about TIME, not
size — and `image_source_services.mm:215-250` ports that faithfully, `CGImageSourceCreateImageAtIndex`
per frame and all. So the decode is not obviously divergent and a "composite the frames" fix would be
aimed at behaviour MAUI does not have.

What IS established:

* the asset's frames really are partial rects (inspected directly)
* the port's render is DETERMINISTIC — re-captured both columns, score identical to two decimals, files
  verifiably rewritten. A GIF actually animating would almost certainly land on a different phase, so
  the port's image appears not to be animating while MAUI's is (MAUI shows the heart mid-animation)
* the port's decode faithfully mirrors the oracle's algorithm

**ON-DEVICE PROBE SETTLES IT: the decode is CORRECT and the GIF theory is dead.** Instrumenting
`animated_image_from_source` and reading the simulator log:

```
MauiGifProbe frame 0 size 400x300
MauiGifProbe frame 1 size 400x300
... all 16 frames 400x300
```

ImageIO returns every frame as the **full, already-composited canvas** — the partial rects visible in
the file never reach the port. So there is no compositing bug, no partial-frame bug, and nothing to fix
in the image decoder. (Probe reverted; the file is unmodified.)

**What remains is a 200px LAYOUT offset of that one cell.** MAUI's GIF canvas starts at y2301 and shows
the heart (y2520 = `(252,205,64)`); the port's starts at y2501 with white above it. Both extend past the
screen bottom, so both are clipped, and everything above y2301 matches pixel-for-pixel.

**STOPPING, same as `android/border`.** Two theories tested and killed on this page — animation-phase
noise (disproved by a byte-identical re-capture) and GIF frame compositing (disproved by the oracle and
then by the device). One page at 3.91%, whose remaining symptom is a single element sitting 200px low
with no second instance to compare against, does not justify further speculative work on shared image
or layout code.

### Phase 4 — NEXT LEAD: Android centred content sits 34px low (the `border*` family)

The `border*` pages are the largest remaining yellow family — 7 cells across Android, iOS and Catalyst
(`android/border` 3.55%, `android/border_stroke` 3.07%, `maccatalyst/border_stroke` 2.02%,
`android/border_clip_playground` 2.02%, `ios/border_clip_playground` 1.81%,
`android/border_resize_content` 1.71%, `android/border_playground` 1.65%).

MEASURED on `android/border_light` — a page that is a single `Border` with `VerticalOptions="Center"`:

| | MAUI | port |
|---|---|---|
| box | y952-1387 (436px), x157-922 | y985-1424 (440px), x155-924 |
| stroke at mid-height | 13px | 14px |
| box centre y | 1169.5 | 1204.5 |

The port's box is 34px LOW, 4px larger on each axis, with a 1px thicker stroke.

**The centring arithmetic identifies the mechanism.** Top-anchored pages are unaffected — `label` and
`button` match at *exactly* +0 top and +0 bottom — and the navigation bar is pixel-identical in both
frames (black from y≈2276). Given content top 64 and nav-bar top 2276:

* centre of [64, **2275**] = **1169.5** — MAUI's box centre, exactly
* centre of [64, **2340**] = 1202 — the port's, within rounding of 1204.5

So the port's available height appears to include the bottom navigation bar, where MAUI's excludes it.
Only centred and bottom-anchored content can show this, which is why the rest of the Android board is
unaffected.

**Where it is NOT.** `MauiHostActivity.usableContentHeightPx()` does subtract both insets, and
`app_host.cpp` does prefer it over the legacy path — both hosts, verified. So the mechanism is present
and the bug is not a missing call.

**HYPOTHESIS TESTED AND DISPROVED (2026-08-03).** A temporary probe in
`MauiHostActivity.usableContentHeightPx()`, on the live emulator, logged:

```
bounds=2340  systemBars(top=136, bottom=66)  statusBars=136  navBars=66  caption=0  displayCutout=136
```

The helper is correct and timing-safe: it returns 2138 = 2340 − 136 − 66, subtracting BOTH insets at
mount time. The comment's claim holds. (Probe reverted; the file is unmodified.)

**The top inset is also right, and that was my second wrong guess.** `statusBars()` reads 136 because
the emulator reports a 136px display cutout — and the status bar really is 136px tall in the capture
(grey #BDBDBD from y0 to y≈136). Top-anchored pages confirm it: `label` and `button` place first ink at
y=249 in BOTH columns.

**So the discrepancy is at the BOTTOM.** Solving the centring both ways against the measured box
centres:

| | top inset | bottom inset | predicted centre | measured |
|---|---|---|---|---|
| port | 136 | 66 (nav bar) | 1205 | 1204.5 ✓ |
| MAUI | 136 | **~137** | 1169.5 | 1169.5 ✓ |

MAUI reserves ~137px at the bottom where the port reserves 66 — roughly the navigation bar *doubled*,
or equivalently the same inset it applies at the top. The nav bar is visually identical in both frames
(black from y≈2276), so this is reserved layout space, not painted chrome.

**THAT INFERENCE IS ALSO DISPROVED (2026-08-03).** `borderless` pins the content area directly — it is a
two-row `*,*` Grid whose cells are FILLED Borders, so the cell edges are the content bounds with nothing
to solve for:

```
maui  pink(top cell) y136-1204   red(bottom cell) y1205-2273   -> content [136, 2273], height 2138
cpp   pink(top cell) y136-1204   red(bottom cell) y1205-2273   -> content [136, 2273], height 2138
```

**Identical**, and exactly the 2138 that `usableContentHeightPx()` returns. So the port's content area is
correct, MAUI uses the same one, and the whole inset theory — top, bottom, and helper alike — is
exonerated. Three hypotheses tested, three disproved.

**What is actually true on the `border` page:** centring in the now-KNOWN area [136, 2273] predicts a box
top of 985 for the port's 440px box — and the port measures 985. The port centres correctly. MAUI's box
sits at 952, which is 35px ABOVE the centre of that same area. So MAUI is not centring the box in the
full content area on that page, and the divergence is in MAUI's layout of this specific page, not in the
port's insets.

**FOURTH HYPOTHESIS ALSO DISPROVED.** "A non-filling child gets a different arrange rect" predicts that
every vertically-centred page is off. It is not: on Android `animation` and `basic_swipe` — both
`VerticalOptions="Center"` — score **0.00%, SSIM 1.0000**, and `borderless`, `border_layout` and
`adaptive_collection` are green too. Only `border` diverges.

**Narrowed to a single page, with the port behaving correctly.** Measuring the full box (fill + stroke,
not just the red stroke):

| | box ink | height | centre |
|---|---|---|---|
| MAUI | y951-1388 | 438px | 1169.5 |
| port | y985-1424 | 440px | 1204.5 |

Sizes agree to 2px (the shape-deflate residual). The content area is [136, 2273] with centre 1204.5 —
the port's box centre exactly. MAUI places its box 35px ABOVE the centre of the area it demonstrably
lays out into.

`border` is the only page in the corpus whose ContentPage child is BOTH non-filling and page-level, so
there is no second instance to generalise from. Four hypotheses have now been tested and disproved here
with zero code changed, and each would have modified correct code. **Recommendation: leave it.** One
page at 3.55% is not worth a fifth speculative change to shared layout code; revisit only if a second
page with this shape appears, which would give the second data point every attempt so far has lacked.

The 4px box / 1px stroke deltas are a separate, smaller matter (the shape-deflate family) and should
not be conflated with this.

### Phase 4 — the iOS radio family (2026-08-03)

Four defects found and fixed, each root-caused from the oracle or from a controlled on-page comparison.
All four pages improved substantially; none reached green, so the board did not move.

| page | before | after |
|---|---|---|
| `radio_button_border` | 5.70% | **2.76%** |
| `radio_button_group_gallery` | 3.97% | **2.31%** |
| `radio_content_properties` | 2.05% | **1.31%** |
| `radio_button_content` | 2.09% | **1.35%** |

1. **Indicator was an SF symbol at its natural size** — ~16.7pt/1.33pt stroke vs the oracle's 21pt/2pt
   (`RadioButton.cs:546-556`). Now drawn.
2. **The button's image slot was 2pt too short**, clipping even a correct indicator. `get_desired_size`
   used 14pt total vertical chrome while `contentEdgeInsets` spent 16. Found by measuring BOTH axes —
   23.0pt wide × 19.0pt tall is anisotropic, which proves a clip rather than a scale.
3. **`gap` was 8, the template's `ColumnSpacing` is 6** (`RadioButton.cs:536`).
4. **`BorderWidth` grows the control; the port shrank the fill inward.** Isolated by a controlled
   comparison: that page's Option 2 (fill only) is 35.0pt in both columns, Option 1 (same fill +
   `BorderWidth=4`) is 45.0pt reference vs 27.0pt port — and 27 = 35 − 2×4 exactly.

**Still open, and deliberately not guessed at.** All four rows of `radio_button_border`, measured with
the non-white band method (the reliable one — see the lesson below):

| row | MAUI | port |
|---|---|---|
| Opt 1 — `BorderWidth=4` + fill | 45.0pt | 43.0pt |
| Opt 2 — fill only | 35.0pt | 35.0pt |
| Opt 3 — neither | 21.0pt | 21.0pt |
| Opt 4 — `BorderWidth=4`, no fill | 44.3pt | 43.0pt |

Opt 3 reading 21.0pt in both is the interpretive key: with no background there is no box to see, so that
figure is the RING, not the layout row. The visible box in Opt 2 (35pt) is the background extent, i.e.
the control's layout box.

So a 4pt border grows the control 35 → 45, **+10pt**, where the port now adds +8 (= 2×4, exactly the
template arithmetic: ring 21 + 2×Grid.Padding(2) + 2×Border.Padding(6) + 2×stroke(4) = 45 explains the
BORDERED case exactly, yet the same formula predicts 37 for the unbordered case where the render is 35).

One hypothesis IS ruled out by the data: if MAUI's stroke straddled the box edge, the overhang would be
w/2 = 2pt per side and the ink would be 47pt. It is 45, so the overhang is 1pt per side and does not
scale with w — at least at w=4.

**Not fixed, on purpose.** `+2×(w+1)` fits this measurement, and the corpus has no second RadioButton
border width to validate it (`styles.xaml`'s `BorderWidth="2"` is on a Button). Fitting a formula to a
single data point is precisely the blind calibration that misfired twice in this file already. The same
unexplained +1pt appears in the horizontal chrome — 7pt as shipped where `Border(6) + Grid(2)` declares
8 — and two +1pt discrepancies in one control look like one mechanism worth finding, not two constants
worth nudging.

**A measurement lesson worth keeping.** Two methods disagreed on that 2pt: a colour-classified vertical
slice said the heights matched, a non-white band scan said 45 vs 43. The slice was wrong — its "white"
test required >240 on all channels, so it silently discarded the antialiased edge pixels. When two
measurements disagree, zoom in on the pixels; do not pick the one that suits the story.

### Windows — DONE (2026-08-03), and it FOUND SOMETHING

1085 frames, 0 drops, and exactly **two** OS theme switches plus a restore for the whole board — the
outermost-theme-loop design doing its job (the inner ordering would have been ~1000 switches).

Windows **171/0/1 → 168/2/2** on both columns. **This is not a port regression.** The MAUI reference was
rebuilt, and under system-driven theming *real MAUI renders differently — and correctly* where the
forced override had been rendering wrong:

`radio_button_content`, dark, the ControlTemplate'd RadioButton:

| | rendering |
|---|---|
| MAUI **before** (`UserAppTheme = Dark` via the env override) | a WHITE box, an invisible grey dot, **no text at all** |
| MAUI **now** (system dark, `UserAppTheme` Unspecified) | dark box, visible radio, its text shown |
| the **port**, now | the WHITE box — i.e. it replicates MAUI's OLD, BROKEN behaviour |

So `UserAppTheme = Dark` never propagated into that ControlTemplate's resources on Windows, while the
system theme does. The board had been comparing the port against a reference that was itself wrong, and
scoring the pair as a match. Three cells moved for this reason:

| page | delta | |
|---|---|---|
| `radio_button_content` | dark 8.53% | **new red** — the templated-RadioButton gap above |
| `containers` | dark 7.49% | new yellow |
| `hybrid_web_view` | light 2.07% | new yellow |

`context_flyout` stays red in both columns: live external content, explicitly exempt.

These three are Phase 4 work — genuine port gaps that only system-wide theming could reveal, because
both sides were previously pinned by an override that masked them.

### iOS — DONE (2026-08-03)

All three columns recaptured under system-wide light and dark: 182+182 each, 0 drops, 0 splash frames.
Prerequisite: MauiReference rebuilt for iOS **and** `gallery_xaml` rebuilt — the latter was stale from
the previous evening and predated the theme fix, so that column would have captured old behaviour.

Result: iOS cpp **161/11/0 → 162/10/0**, xaml 162/10/0 unchanged. **Zero reds.**

**A MISSING TOOL WAS THE REAL FIND.** The reference captures live in two roots —
`port/maui-reference/captures/<platform>/` (fresh, what the capture tools write) and
`docs/comparison/captures/<platform>/maui/` (what the scoring tools read) — and NOTHING copied between
them. A full 364-frame reference sweep had just run and the board's column was still the 2-day-old set,
so the port would have been scored against a reference captured under a *different theme mechanism*.
`tools/parity/promote_reference_captures.py` is that importer; it refuses to promote a splash frame, a
blank frame, or an empty source directory.

Two capture flakes were caught and repaired before scoring, both would have read as port regressions:

| frame | symptom | cause |
|---|---|---|
| `cpp` + `xaml` `image_light` | 65.19% differ | the `UriSource` NETWORK image again — same flake as Android |
| `xaml/border_stroke_dark` | 33.15% differ | frame rendered **entirely black** (body mean 0.0) |

A full sweep of all 1092 iOS frames for flat / wrong-theme / splash found 45 suspicious, of which only
**6 were also changed vs HEAD** — the other 39 are pre-existing flat pages that match their reference.
Cross-checking suspicion against "did this frame actually change" is what keeps a verification pass from
reporting 45 fictional regressions.

### Android — DONE (2026-08-03)

All three columns recaptured under system-wide light and dark: maui 172+172, cpp 174+174, xaml 196+196.

**The switch to OS-driven theming is a no-op on the output**, which is the result to want. Of ~1084
frames only 10 differ from HEAD, and of those:

| frames | cause |
|---|---|
| `date_picker`, `pickers`, `clip_views` (both themes, ≤0.10%) | the midnight date rollover — the rendered date string, not the port. Why all three columns must be captured in ONE session. |
| 2 × `gap_*` at 0 px | metadata-only rewrite |
| `xaml/image_dark` at 66% | **a broken frame in the COMMITTED board, now repaired** |

Dropping `--es MAUI_THEME Dark` changed nothing at all, which empirically confirms the long-standing
note that `UserAppTheme = Dark` was a no-op on Android: the device night mode was already doing 100% of
the work. The column was OS-driven by accident; it is now OS-driven by design.

Score effect: Android xaml 159/10/3 → **160/10/2** (board xaml total 655/27/6 → **656/27/5**), entirely
from the repaired `image` frame.

**KNOWN NONDETERMINISM — the `image` page.** Its `UriSource` is a NETWORK image, so a capture can land
before it loads; the layout then collapses upward and ~66% of pixels differ. This is not theoretical:
the committed `xaml/image_dark` was carrying such a failure, and a fresh capture of `cpp/image_light`
reproduced it once before succeeding on retry. All six Android `image` frames are now verified loaded
(UriSource band > 5000 distinct colours). Re-check that band after any `image` recapture.

**Board scoring is a SEPARATE step.** `build_comparison_json.py` CARRIES OVER pixel scores; it cannot
compute them. The chain is import → build_comparison_json → `pixel_score.py --platform <p>` →
gen_readme. Skipping the scorer produces a byte-identical board and a cheerful success line while the
verdicts stay stale — which is exactly what happened here until the repaired `image` page was still
reading "61.63% pixels differ".

**Rebuild before capturing:** the MauiReference APK/app must be rebuilt anywhere the App.xaml.cs change
has not landed — an older binary still forces Light when no `MAUI_THEME` extra is present, so a dark
pass renders LIGHT and reads as an enormous regression. Android's is rebuilt and installed
(2026-08-03); iOS/Catalyst/Windows references are NOT yet.

## Phase 2 — rebuild the board · Phase 3 — commit and push · Phase 4 — drive every cell to 100%

Not started.
