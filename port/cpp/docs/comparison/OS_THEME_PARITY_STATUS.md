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
