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
| (c) wire the setters into every capture path; stop passing the per-column theme env | *in progress* |

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

Not started.

## Phase 2 — rebuild the board · Phase 3 — commit and push · Phase 4 — drive every cell to 100%

Not started.
