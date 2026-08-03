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

**Still open, and deliberately not guessed at:** a bordered radio measures 43.0pt in the port against
45.0pt in the reference. 43 = 35 + 2×4 is exactly what the template arithmetic predicts, so MAUI is
growing by 5pt per side for a 4pt border and the extra 1pt is unexplained. Also unexplained: the
horizontal chrome is 7pt as shipped where `Border(6) + Grid(2)` declares 8 — the same 8-vs-7
disagreement as the vertical chrome.

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
