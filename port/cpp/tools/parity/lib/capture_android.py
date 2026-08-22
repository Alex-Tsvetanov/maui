#!/usr/bin/env python3
"""Android GIF capture — the motion pass for the animated pages.

The STILL pass is the three build+install+capture scripts next to this file; they own the APK
pipeline and are what the board has always been captured with. This module only adds what they cannot
do: a burst of frames for the handful of pages a single still cannot represent.

It runs AFTER the still pass for a theme, and that pass leaves the device in a state a recording
cannot use — so this module restores BOTH things itself and puts them back afterwards:
  * night mode, which the scripts' exit trap has already reverted (the port reads Configuration.uiMode,
    not the intent extra, so a "dark" GIF recorded without this would be a light one);
  * the ANIMATION SCALES, which device_state.pin_android() sets to 0 so that stills are deterministic.
    A GIF recorded under that pin is a dozen copies of one frozen frame — measured: an entire board
    pass produced 84 empty GIFs this way, and because the board prefers .gif over .png they shadowed
    every still they were named after.

It also owns the lane's only INTERACTION path (`run_steps` / `input_argv` below). Nothing else on
Android touches the UI — the still pass launches the app and screencaps — so a page that only changes
under a tap or a drag is photographed at rest, and its "animated" GIF is twelve copies of one idle
frame. `capture_gif(steps=…)` drives the page while the burst shoots it.

The per-page determinism mirrors the shell scripts exactly — force-stop, wait for the process to be
GONE, clear logcat, `am start -W`, poll for THIS launch's Displayed marker, dismiss any ANR dialog —
because a recording that starts on the previous page's frame is worse than no recording.

The burst is also the lane's MOTION-SCORING EVIDENCE, when `capture_gif(run_dir=…)` asks for it — see
the "run-unit evidence" section below for the shape, the step-naming scheme, and the equal-duration
assumption that scheme both encodes and enforces.
"""
from __future__ import annotations

import json
import os
import re
import shutil
import subprocess
import tempfile
import threading
import time
from datetime import datetime
from pathlib import Path

import gif as gifmod
from device_state import set_android_theme

SERIAL = os.environ.get("MAUI_ANDROID_SERIAL", "emulator-5554")
ADB = os.environ.get("MAUI_ADB", "adb")
HERE = os.path.dirname(os.path.abspath(__file__))
CPP = os.path.abspath(os.path.join(HERE, "..", "..", ".."))
# OPEN BLOCKER (2026-08-22) — ANDROID DARK RECAPTURE IS NOT CURRENTLY TRUSTWORTHY.
#
# The dark page background of SCROLLVIEW-ROOTED pages moves between (18,18,18) = #121212 and
# (47,47,47) = #2F2F2F depending on emulator state, and the two columns do not always move together.
# Measured this session, all on AVD `maui-test`, same APKs throughout:
#
#   pipeline recapture 03:59   box_view      maui 18            (agreed; cell scored GREEN)
#   snapshot-loaded boot       box_view      maui 47 / port 18  (manual probe, force-stop + relaunch)
#   clean -no-snapshot-load    box_view      maui 18            (manual probe — the wash GONE)
#   pipeline recapture 05:57   box_view      maui 47 / port 47  (AGREE at 47; still green)
#   pipeline recapture 05:54   date_picker   maui 47 / port 18  (DISAGREE -> 85% diff, would be RED)
#   pipeline recapture 05:57   label         maui 18 / port 18  (non-ScrollView: unaffected either way)
#
# NARROWED FURTHER, by bisecting the pipeline's device state by hand. A MANUAL probe — force-stop,
# `am start`, sleep, `adb exec-out screencap -p` — reads (18,18,18) for BOTH apps on BOTH box_view and
# time_picker, under EVERY combination tried:
#     plain dark                                  maui 18 / port 18
#     dark + animation scales pinned to 0         maui 18 / port 18
#     dark + animations 0 + SystemUI demo mode    maui 18 / port 18
# So demo mode and the animation pinning are BOTH ruled out, and so is the capture primitive: the
# pipeline shoots the same `adb exec-out screencap -p` this probe does. The wash has never once been
# reproduced outside a pipeline run.
#
# Still unexplained, and the remaining differences to bisect: the pipeline flips night mode ONCE PER
# THEME PASS and then launches each of the three apps in turn (so an app can start while the
# configuration change is still propagating, which no settle here covers); the GIF pass turns animations
# back ON mid-run (recapture.py lane_android_gifs) and box_view/date_picker are both DRIVEN pages that
# get one; and the build stage reinstalls the port APKs between passes.
#
# CONSEQUENCE: do not commit an android dark recapture of a ScrollView-rooted page without checking the
# dominant background of BOTH columns first. Four cells (date_picker/time_picker x pixel,pixel_xaml)
# were about to be committed RED purely on this. They were restored.
#
# NOT the cause, each ruled out by measurement rather than argument: the MauiReference rebuild (the
# 03:59 captures already used the new APK and read 18); the local-time fix in 6a06e704d2 (the LIGHT
# captures improved to 0.62%/0.55% and both columns read 8/22/2026 — the fix is verified working); the
# safe-area producer (label is unaffected, and where box_view moved BOTH columns moved).
# Related and NOT the same thing: the older "dark #2F2F2F was a capture artifact" note refers to a
# whole-frame wash both apps hit; this one is per-page and column-asymmetric.

COMP_CAP = os.path.join(CPP, "docs", "comparison", "captures", "android")

# package + page-extra per column. MauiReference reads MAUI_COMPARE_PAGE, the C++ app hosts read
# MAUI_SAMPLE_PAGE; both theme extras are sent every time (each family ignores the other's).
# `dir` is the board's captures/android/<dir>/ folder; `col` is the RUNNER column the same cell is
# called in a run directory (motion_score.FW_TO_COL is the same mapping read the other way round —
# note xaml -> cpp_xaml, which is the one that does not simply repeat itself).
APPS = {
    "maui": {"pkg": "dev.mauicpp.mauireference", "page": "MAUI_COMPARE_PAGE",
             "dir": "maui", "col": "maui_xaml"},
    "cpp": {"pkg": "dev.mauicpp.apphost", "page": "MAUI_SAMPLE_PAGE",
            "dir": "cpp", "col": "cpp"},
    "xaml": {"pkg": "dev.mauicpp.apphost.xaml", "page": "MAUI_SAMPLE_PAGE",
             "dir": "xaml", "col": "cpp_xaml"},
}
COLUMNS = {spec["col"] for spec in APPS.values()}


def adb(*args, **kw):
    return subprocess.run([ADB, "-s", SERIAL, *args], **kw)


def out_path(app: str, key: str, theme: str) -> str:
    return os.path.join(COMP_CAP, APPS[app]["dir"], f"{key}_{theme}.gif")


def still_path(app: str, key: str, theme: str) -> str:
    """The board PNG the STILL pass published for this cell (the shell scripts next to this file)."""
    return os.path.join(COMP_CAP, APPS[app]["dir"], f"{key}_{theme}.png")


# ------------------------------------------------------------- run-unit evidence (motion scoring)
# The board kept only the published still and the 400px GIF; this burst went into a TemporaryDirectory
# and vanished with it. motion_score.py scores an animated page FRAME BY FRAME at full resolution, and
# the only thing it can read is a RUN DIRECTORY — so `capture_gif(run_dir=…)` now also drops the burst
# there, in exactly the shape run_comparison.py writes on the VM lanes:
#
#     <run_dir>/<key>/android/<column>/NNNN.png  +  NNNN.json {tag, platform, column, theme, step,
#                                                              frame, commit, captured_at}
#
# STEP NAMES, AND WHY THEY ARE NOT AN INDEX WEARING A NAME
# --------------------------------------------------------
# motion_score._pair joins the two columns BY STEP NAME precisely so that a dropped frame cannot
# silently re-align the tail of a sequence onto earlier moments. A VM step name is a discrete scenario
# step; an Android burst has NO step boundaries at all — `screencap` shoots on a fixed schedule while
# the scenario (if any) runs on a concurrent thread. What the two columns do share is that schedule:
# the same page, the same scenario, `frame_count` samples over the same nominal `secs`. So the k-th
# sample is the same NOMINAL MOMENT in both columns, and THAT — not its position in the surviving
# list — is what the name encodes: a screencap that comes back short leaves a GAP in the names rather
# than promoting every later frame to an earlier moment.
#
# NOMINAL is the honest word, and it is this scheme's ceiling. `interval = secs/(frame_count-1)` is the
# REQUESTED schedule; each screencap costs ~0.13s of its own, and that drift does not accumulate
# identically in the MAUI app and in the port. The name pins the schedule the two columns were ASKED to
# share, never a measured timestamp. Frames that are nominally the same moment can therefore be tens of
# milliseconds apart in wall clock — which is why the number this produces is a frame-by-frame parity
# signal, not a timing measurement.
#
# THE EQUAL-DURATION ASSUMPTION IS ENFORCED, NOT ASSUMED. `secs` and `frame_count` are IN every name,
# so two recordings made with different geometry produce DISJOINT name sets: _pair returns nothing and
# score_cell refuses the cell with its existing "NO step name occurs in both … a comparison by frame
# index would be a guess. Re-capture this page". All-or-nothing, with no edit to motion_score — an
# elapsed-milliseconds name would still pair the t=0 frame and score a confident single-frame "motion".
#
# The `gif` PREFIX is load-bearing too: recapture.burst_frames treats a unit whose steps are all
# `initial`/`gif*` as UNDRIVEN and drops the at-rest frame from the burst. That is exactly right here.
# Frame 0001 is a BYTE COPY of the still the board published, which came from a different launch
# entirely (the still pass, with the animation scales pinned to 0). It is in the unit only as
# motion_score._is_published_run's provenance witness — the byte-identical twin that proves these
# frames belong to the run behind the board — and must never be scored as a frame of this recording.
# Its sidecar carries THIS pass's commit/captured_at while its pixels are the still pass's; the step
# name `initial` is what says so.
RUN_PLAT_DIR = "android"          # the <plat_dir> segment motion_score looks under


def _git_commit() -> str:
    r = subprocess.run(["git", "-C", CPP, "rev-parse", "--short", "HEAD"],
                       capture_output=True, text=True)
    return r.stdout.strip() or "unknown"


def step_name(sample: int, secs: float, frame_count: int) -> str:
    """The name of the `sample`-th (1-based) burst sample of a `frame_count`-over-`secs` recording."""
    return f"gif{sample:02d}@{secs:g}s/{frame_count}f"


def write_run_unit(run_dir: str, column: str, app: str, key: str, theme: str,
                   samples: list[tuple[int, str]], secs: float, frame_count: int,
                   at_rest: str | None = None) -> str:
    """Persist one (page, column, theme) burst as the run unit motion_score.py reads. Returns its path.

    `samples` is [(nominal sample number, png path)] — see the naming note above; the number, never the
    list position, is what becomes the step name. `run_dir` is the run ROOT, because motion_score pairs
    two columns that must live under ONE run.
    """
    if column not in COLUMNS:
        # A column nothing recognises writes frames no scorer will ever look for: the page would read
        # as "no run directory" forever, which is precisely the silent nothing this lane must not do.
        raise ValueError(f"unknown runner column {column!r}: expected one of {sorted(COLUMNS)}")
    unit = Path(run_dir) / key / RUN_PLAT_DIR / column
    unit.mkdir(parents=True, exist_ok=True)

    # A RE-CAPTURE of this cell into the SAME run supersedes the earlier attempt. Without this both
    # attempts' frames would sit in the unit, and _pair (keyed by name + Nth occurrence) would pair
    # column A's first attempt against column B's — which may be different attempts of the two. Only
    # THIS theme is cleared: light and dark share the unit dir and _shots tells them apart by sidecar.
    highest = 0
    for sidecar in sorted(unit.glob("*.json")):
        try:
            same_theme = json.loads(sidecar.read_text()).get("theme") == theme
        except (OSError, json.JSONDecodeError):
            same_theme = False                  # unreadable: leave it, but never reuse its number
        if same_theme:
            sidecar.with_suffix(".png").unlink(missing_ok=True)
            sidecar.unlink()
        elif sidecar.stem.isdigit():
            highest = max(highest, int(sidecar.stem))

    commit, when = _git_commit(), datetime.now().astimezone().isoformat()
    n = highest

    def put(src: str, step: str) -> None:
        nonlocal n
        n += 1                                  # NNNN, zero-padded: lexical order IS capture order
        shutil.copyfile(src, unit / f"{n:04d}.png")
        (unit / f"{n:04d}.json").write_text(json.dumps(
            {"tag": key, "platform": RUN_PLAT_DIR, "column": column, "theme": theme,
             "step": step, "frame": n, "commit": commit, "captured_at": when}, indent=2))

    # THE BEFORE FRAME. Prefer the one the BURST shot, not the board still.
    #
    # The still comes from the MAIN pass and is shot under different device state: the burst runs after
    # pin_android + set_theme, the still does not. Splicing it in as `initial` therefore compares two
    # columns' frames that were never taken under the same conditions. MEASURED on run
    # 2026-08-07-05_47_52, hit_testing/dark: MAUI's spliced `initial` has mean luma 66.4 against its own
    # burst's 41.6, and pairing it with the port's (41.6) reports 89.63% of pixels differing — reddening
    # a page whose motion is IDENTICAL in both columns (1134 px each). Five pages went green->red that
    # way the moment the frame was actually used, which is why motion_score still refuses to trust it.
    #
    # An at-rest frame shot INSIDE the burst has none of that: same theme, same demo mode, same
    # animation scales, microseconds before the first gesture. This is the ordering 89261d905a fixed on
    # iOS, applied to the lane that still had it backwards.
    # THE PUBLISHED STILL STAYS THE `initial` FRAME, and that is NOT negotiable: motion_score
    # .find_frames (:346) accepts a run only when its frames match the published still BYTE-FOR-BYTE.
    # `initial` is the PROVENANCE WITNESS tying this unit to the board, not merely a BEFORE.
    #
    # Substituting the burst's at-rest shot here — which is otherwise the better BEFORE, and is
    # measurably so (see capture_gif) — breaks that tie: find_frames then REJECTS this run and falls
    # back to an older one whose `initial` still matches, scoring the very frames the change removed.
    # Measured: android 269g/58y/17r -> 255g/57y/32r, WORSE than the 259g the previous attempt cost.
    #
    # Doing this properly means carrying BOTH: the still as the witness, the burst at-rest under its
    # own step name, with the motion selector preferring the latter. `at_rest` is still captured and
    # still passed in, so that change is a naming decision rather than a capture one.
    still = still_path(app, key, theme)
    if os.path.isfile(still):
        put(still, "initial")                   # the provenance witness — copied bytes, not a shot
    else:
        # Not fatal — the GIF is still a valid board artifact — but the unit cannot prove which run the
        # board's still came from, so motion_score will refuse it. Say that here rather than let it
        # surface hours later as an unexplained "frames do not match captures/ byte-for-byte".
        print(f"      !! {key} ({app}/{theme}): no published still at {still} — run unit written "
              f"WITHOUT its provenance frame, so this cell stays NOT motion-scored until the still "
              f"pass runs", flush=True)
    # THE BURST'S OWN BEFORE, carried ALONGSIDE the provenance copy rather than replacing it. Both are
    # needed and they are not the same thing:
    #   `initial`  bytes of the published still, from the STILL pass — motion_score._is_published_run's
    #              byte-identical twin, the only thing tying this unit to the board. Shot under
    #              DIFFERENT device state (no pin_android, no set_theme), so it is not a usable BEFORE.
    #   `at-rest`  shot inside THIS burst, after the settle and before the first gesture — same theme,
    #              same demo mode, same animation scales. The only honest BEFORE a time-labelled burst
    #              can have, because its frames are named gif01..gifNN whatever the driver does.
    #
    # Substituting one for the other was tried twice and reverted twice (ccb057fcca, 3b46bce76c): using
    # the still as the BEFORE reds pages on a device-state mismatch (89.63% "differ" on hit_testing/dark),
    # and using the at-rest shot as the witness makes find_frames reject the whole run and score an older
    # one. Carrying both costs one PNG per unit and ends that.
    if at_rest and os.path.isfile(at_rest):
        put(at_rest, "at-rest")
    for sample, png in samples:
        put(png, step_name(sample, secs, frame_count))
    return str(unit)


def _component(pkg: str) -> str:
    out = adb("shell", "cmd", "package", "resolve-activity", "-c",
              "android.intent.category.LAUNCHER", pkg, capture_output=True, text=True).stdout
    for line in out.splitlines():
        line = line.strip()
        if line.startswith("name="):
            return f"{pkg}/{line[5:].strip()}"
    raise RuntimeError(f"could not resolve launcher activity for {pkg} (installed?)")


def _wait_gone(pkg: str) -> None:
    for _ in range(40):
        if not adb("shell", "pidof", pkg, capture_output=True, text=True).stdout.strip():
            return
        time.sleep(0.25)


def _wait_ready(pkg: str) -> None:
    for _ in range(60):
        if f"Displayed {pkg}/" in adb("logcat", "-d", capture_output=True, text=True).stdout:
            return
        acts = adb("shell", "dumpsys", "activity", "activities", capture_output=True, text=True).stdout
        if "ResumedActivity" in acts and f"{pkg}/" in acts:
            return
        time.sleep(0.25)


def foreground_package() -> str:
    """The package owning the RESUMED activity, or "" if the device would not say.

    Returning "" on an unreadable dumpsys is deliberate: the caller treats it as "cannot assert" and
    proceeds, because a guard that fails CLOSED on a parsing quirk would wedge the whole lane. The
    failure this exists to catch is loud and unambiguous — a DIFFERENT package in front — not a
    missing reading."""
    out = adb("shell", "dumpsys", "activity", "activities", capture_output=True, text=True).stdout or ""
    # MEASURED against this emulator's real output rather than the documented spelling, because the
    # documented spelling does not appear on it. API 34 prints BOTH of:
    #     topResumedActivity=ActivityRecord{1f1c698 u0 dev.mauicpp.apphost.xaml/.MauiHostActivity …}
    #       ResumedActivity: ActivityRecord{1f1c698 u0 dev.mauicpp.apphost.xaml/.MauiHostActivity …}
    # and NEITHER is `mResumedActivity`, the field name every guide greps for — a regex written from
    # that name matches nothing, returns "", and leaves this guard permanently inert while looking
    # correct. (`cmd activity get-foreground-activity` does not exist here either.) So match the
    # optional `top` prefix and either separator, and anchor on the `uN ` that precedes the package.
    m = re.search(r"(?:top)?ResumedActivity[=:][^\n]*?\bu\d+\s+([A-Za-z0-9_.]+)/", out)
    return m.group(1) if m else ""


def assert_foreground(pkg: str, where: str) -> None:
    """Refuse to bank a frame when `pkg` is not the app on screen.

    WHY THIS EXISTS, measured 2026-08-11. The port's Android WebView had no WebViewClient, so
    context_flyout's `<WebView Source="https://bing.com">` handed its redirect to the ActivityManager,
    which fired ACTION_VIEW and launched CHROME over the gallery. Chrome then stayed foreground, and
    every page captured after it in that column recorded Chrome's first-run interstitial instead of the
    app: 20 files (the light+dark pairs of ten gap_* pages) were committed to the board showing a
    FOREIGN APP, byte-identical to each other, and they read as port bugs to any reviewer.
    Ten of them were still 100% identical to that Chrome frame when this guard was written.

    The handler bug is fixed, but that is not what makes this a lane defect. The lane ACCEPTED a
    foreign window silently and banked it as the port's render — the same failure family as every other
    fabricated capture this board has produced (a month-stale MauiReference, a --skip-build freeze,
    mid-edit binaries). A capture is only evidence of the app that was actually on screen, and until
    now nothing checked which app that was.

    Raises rather than returning a flag: the caller's contract is "a step runs or it raises", and a
    dropped frame is always better than a wrong one that scores."""
    fg = foreground_package()
    if fg and fg != pkg:
        raise RuntimeError(
            f"{where}: foreground is {fg!r}, expected {pkg!r} — refusing to capture another app's "
            f"window. If a page navigated away (a WebView escaping to a browser is the known cause), "
            f"fix that rather than relaxing this check")


ANIM_KEYS = ("window_animation_scale", "transition_animation_scale", "animator_duration_scale")


def animations() -> str:
    """The device's current window_animation_scale, so the caller can put back what it found."""
    out = adb("shell", "settings", "get", "global", ANIM_KEYS[0], capture_output=True, text=True).stdout
    return (out or "").strip() or "1"


def set_animations(on: bool | str) -> None:
    """Turn the device's animation scales on for a recording, off again afterwards.

    device_state.pin_android() sets all three to 0 for the STILL pass — a frame caught mid-animation
    is nondeterministic by construction. That pin is also why a GIF recorded without this call is a
    dozen copies of one frozen frame: the spinner genuinely is not moving."""
    value = on if isinstance(on, str) else ("1" if on else "0")
    for k in ANIM_KEYS:
        adb("shell", "settings", "put", "global", k, value,
            stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)


def set_theme(theme: str) -> str:
    """Device night mode for this pass; returns the previous value so the caller can restore it."""
    return set_android_theme(theme, SERIAL)


# ----------------------------------------------------------------- interaction (adb shell input)
# These execute the SAME step dicts the scenario TOMLs (docs/comparison/scenarios/*.toml) hand the
# macOS/Windows drivers — name / action / at / to / text / dy / direction / distance / duration — so
# one scenario file can describe an interaction for every platform.
#
# COORDINATE SPACE — read this before authoring an `at`/`to` that has to work on Android:
#   * A pair whose |x| and |y| are BOTH <= 1.0 is a FRACTION of the display (0,0 = top-left,
#     1,1 = bottom-right), scaled here by `adb shell wm size`. PREFER THIS FORM. It is the only one
#     that means the same thing on a 1080x2340 emulator and on the 1512x950 Mac display, i.e. the only
#     one that lets a single scenario drive all four platforms.
#   * Anything larger is ABSOLUTE PIXELS OF THIS DEVICE — not of the Mac screen. The checked-in
#     scenarios (button/entry/scroll_view) are calibrated for a 1512-wide Mac display and their
#     numbers are meaningless here, which is why a start point outside the display is a hard error
#     rather than a tap that quietly lands nowhere.
#   * To author a fraction: open the page's board still, captures/android/<col>/<key>_<theme>.png —
#     the Android stills are raw full-screen `screencap` output, uncropped, exactly the framing the
#     burst shoots (the 140-row status-bar crop lives in pixel_score.py, not in the capture) — read
#     the pixel you want to hit and divide by that image's own width/height.
#   * The status bar is rows 0..~135, i.e. y < ~0.06: a gesture STARTING inside that strip pulls the
#     notification shade down instead of touching the page.
#   * `dy` follows the same rule on the height axis (|dy| <= 1 = fraction), and keeps the macOS sign
#     convention: NEGATIVE dy reveals lower rows. On a touch screen that is the finger moving UP, so
#     the swipe ends at y + dy.
#
# VERBS — the same set the desktop agents implement (see run_comparison.py's scenario doc block):
#   click at                                  -> input tap
#   type text                                 -> input text (into whatever has focus)
#   scroll at, dy                             -> motionevent drag (dy along y; see the sign note above)
#   drag/swipe at, to | direction[, distance] -> motionevent drag; `steps` = how many MOVEs to emit
#   hover at                                  -> SKIPPED, loudly. There is no pointer to park.
# `duration` is in SECONDS, like every other time key in a scenario (`settle`); on this lane it now only
# bounds the per-call adb timeout, since the gesture's pacing comes from the MOVE sequence itself.
_HOVER_SKIP = ("hover: Android has no pointer, so there is nothing to hover with — and a tap is a "
               "different gesture, not a substitute")
# EVERY PAN VERB RELEASES AT ZERO VELOCITY. `input swipe` interpolates linearly and lets go at full
# speed, handing Android's VelocityTracker ~1300 px/s, so the list coasts to a resting offset that is
# NOT repeatable run to run — MAUI's own column differed from ITSELF by up to 11.57% on the same page
# while byte-stable at rest. `input motionevent` sends DOWN/MOVE/UP separately, so the gesture can
# DWELL on its final coordinate and drain the tracker before lifting: measured 0.0000-0.0136% across
# three identical runs. scroll, drag and swipe all take that path (see input_argv) -- so a driven
# frame IS a usable oracle here, not merely a motion witness.
_SWIPE_SECS = 0.8
_SWIPE_DIRECTIONS = {"up": (0, -1), "down": (0, 1), "left": (-1, 0), "right": (1, 0)}
# The shared vocabulary's default swipe distance is 300 px at the Mac's 1024x800 capture geometry — a
# quarter of the axis is that same board-scale flick expressed in something this display can honour.
_SWIPE_FRACTION = 0.25
# `adb shell` hands the argv to a remote sh -c, so shell metacharacters in a `type` step are eaten
# rather than typed. Reject them instead of typing something else — same doctrine as the rest of this
# lane: drop the frame, never fabricate one. ('%' is excluded too: it is input's own escape prefix,
# and a LEADING '-' because `input text -x` reads as a flag rather than as the string to type.)
_TEXT_SAFE = re.compile(r"^(?!-)[A-Za-z0-9 _.,:@/+-]*$")

_size_cache: tuple[int, int] | None = None


def device_size(refresh: bool = False) -> tuple[int, int]:
    """The display size in PIXELS, from `adb shell wm size` (cached — it cannot change mid-run).

    `wm size` prints `Physical size: 1080x2340`, plus an `Override size:` line when the display has
    been resized. The LAST match wins: that is the resolution the frames come back at, and therefore
    the one a fractional coordinate has to be scaled by."""
    global _size_cache
    if _size_cache is None or refresh:
        out = adb("shell", "wm", "size", capture_output=True, text=True).stdout or ""
        found = None
        for line in out.splitlines():
            m = re.search(r"(\d+)x(\d+)", line)
            if m:
                found = (int(m.group(1)), int(m.group(2)))
        if found is None:
            raise RuntimeError(f"could not read `wm size` from {SERIAL}: {out.strip()!r}")
        _size_cache = found
    return _size_cache


def _scale(v: float, span: int) -> int:
    """One axis: |v| <= 1 is a FRACTION of `span`, anything larger is already device pixels."""
    return round(v * span) if abs(v) <= 1.0 else round(v)


def to_pixels(at, size: tuple[int, int]) -> tuple[int, int]:
    """Resolve a scenario `at`/`to` pair to device pixels. See the COORDINATE SPACE note above."""
    x, y = float(at[0]), float(at[1])
    frac = [abs(v) <= 1.0 for v in (x, y)]
    if any(frac) and not all(frac):
        raise ValueError(f"mixed coordinate {list(at)!r}: both values must be fractions (<=1.0) or "
                         f"both device pixels — a mixed pair would scale one axis and not the other")
    return _scale(x, size[0]), _scale(y, size[1])


def input_argv(step: dict, size: tuple[int, int]) -> list[str] | None:
    """The `adb` argv for ONE scenario step, resolved against `size`. Pure: builds, never runs.

    None means there is nothing for adb to do — either the step has no `action` (the plain screenshot
    step every scenario opens with) or it is a `hover`, which run_steps() reports SKIPPED. A hover is
    never quietly turned into a tap: that is a different gesture and would fake a reaction this
    platform cannot produce. An unknown action raises, exactly as run_comparison.py's CoordinateDriver
    does — a typo in a scenario must not read as a page that declined to move."""
    action = step.get("action")
    if not action or action == "hover":
        return None
    if action == "type":
        text = str(step["text"])
        if not _TEXT_SAFE.match(text):
            raise ValueError(f"unsafe text {text!r}: `adb shell input text` goes through a remote "
                             f"shell, so only [A-Za-z0-9 _.,:@/+-] survives it verbatim")
        return ["shell", "input", "text", text.replace(" ", "%s")]  # a bare space splits the argv
    # PER-LANE OVERRIDE, the same mechanism the VM lanes get from run_comparison.for_lane — promoted
    # here rather than there because this lane never goes through that runner.
    #
    # WHY IT IS NEEDED: `at` is shared by iOS and Android (neither has an ENV name, so neither can use
    # run_comparison's `at_<env>` keys), and the two lanes do not always agree on where a control is.
    # stepper is the case that forced it: the portable 0.21,0.13 lands on the "+" half on iOS, where the
    # cell is GREEN, and on the "-" half on Android, where a tap at Value==Minimum is a clamped no-op
    # with no repaint — 0 changed pixels, scored INVALID/`not-driven`. Without this key the only way to
    # fix Android was to break iOS.
    #
    # Keyed `at_android` / `to_android` by BOARD PLATFORM, matching motion_score's `roi_<platform>`.
    # A step with no such key is returned untouched, so every existing scenario behaves exactly as before.
    step = {**step, **{k: step[f"{k}_android"] for k in ("at", "to") if f"{k}_android" in step}}
    w, h = size
    x, y = to_pixels(step["at"], size)
    if not (0 <= x < w and 0 <= y < h):
        raise ValueError(f"step {step.get('name', action)!r} starts at ({x},{y}), outside the {w}x{h} "
                         f"display: Android coordinates are THIS device's pixels, and the checked-in "
                         f"scenarios are calibrated for a 1512x950 Mac. Use a 0..1 fraction instead")
    if action == "click":
        return ["shell", "input", "tap", str(x), str(y)]
    if action in ("scroll", "swipe", "drag"):
        # ONE GESTURE SHAPE SERVES ALL THREE: a press-move-release IS a pan/drag, and Android has no
        # separate scroll or drag injection. The vocabulary's `steps` maps directly onto the number of
        # MOVE events emitted below, which is what the shared vocabulary always meant by it.
        if action == "scroll":
            x2, y2 = x, y + _scale(float(step["dy"]), h)
        elif "to" in step:
            x2, y2 = to_pixels(step["to"], size)
        else:
            direction = str(step.get("direction", "")).lower()
            if direction not in _SWIPE_DIRECTIONS:
                raise ValueError(f"step {step.get('name', action)!r}: needs to = [x, y] or "
                                 f"direction = one of {sorted(_SWIPE_DIRECTIONS)} "
                                 f"(got {step.get('direction')!r})")
            dx, dy = _SWIPE_DIRECTIONS[direction]
            span = w if dx else h
            dist = _scale(float(step["distance"]), span) if "distance" in step \
                else round(_SWIPE_FRACTION * span)
            x2, y2 = x + dx * dist, y + dy * dist
        # The END is clamped, not rejected: a drag running off the edge is a SHORTER drag, which is
        # what a real finger does. Only the START is an authoring error, because it decides what the
        # gesture touches.
        x2, y2 = max(0, min(w - 1, x2)), max(0, min(h - 1, y2))
        if (x2, y2) == (x, y):
            # Press-hold-release at one point is a CLICK. `input swipe` would run, adb would report
            # success, and the frame would come back identical — the silent no-op this whole section
            # exists to prevent, wearing a different hat.
            raise ValueError(f"step {step.get('name', action)!r}: zero-length {action} at ({x},{y}) "
                             f"is a click, not a pan (clamped to the {w}x{h} display?)")
        # SCROLL TAKES THE DETERMINISTIC PATH TOO. It used to return `input swipe` here, on the grounds
        # that a scroll is only a MOTION WITNESS for the GIF and "nothing downstream reads a scrolled
        # still as an oracle". That last clause is false: motion_score pairs the frames of a driven
        # sequence and scores them, so a fling lands the two columns at different points of the same
        # motion and the cell reds on WHEN rather than on whether. box_view is the worked example --
        # light capped "PHASE ONLY, NOT DECIDABLE ON THIS LANE", dark carrying a 3x self-motion
        # asymmetry (MAUI 38.01% vs C++ 95.56%) purely because each column coasted a different
        # distance. Scroll is still a motion witness; it is now a REPRODUCIBLE one.
        # A DRAG/SWIPE/SCROLL RELEASES AT ZERO VELOCITY, so where it settles is reproducible.
        #
        # The header above says "there is no fling-free `input swipe`", and that is true OF THAT
        # COMMAND and false of Android. `input swipe` interpolates and lets go at full speed, handing
        # the VelocityTracker ~1300 px/s, so a snapping container coasts to a resting offset that is
        # NOT repeatable — measured on this lane, MAUI's own column differs from ITSELF by up to
        # 11.57% across two runs of the same page while byte-stable at rest. That single fact is what
        # capped ~22 android cells at "PHASE ONLY, NOT DECIDABLE ON THIS LANE".
        #
        # `input motionevent` (API 29+; this lane's emulator is 34) sends DOWN / MOVE / UP as separate
        # events, so the gesture can DWELL on its final coordinate before lifting. A few repeated MOVEs
        # at the same point drain the VelocityTracker, the release carries no fling, and the container
        # snaps to the nearest boundary every time.
        #
        # MEASURED, three identical runs of carousel_page/android/cpp with the shape below:
        #     run1 vs run2  0.0000%      run1 vs run3  0.0136%      run2 vs run3  0.0136%
        # against 11.57% for `input swipe` — and it genuinely paged (the settled frame reads "Card 2"),
        # so this is a deterministic PAGE, not a stable no-op.
        #
        # `steps` STOPS BEING IGNORED. It was accepted-and-dropped because the driver interpolated for
        # us; here it is the number of MOVE events we emit ourselves, which is exactly what the shared
        # vocabulary always meant by it. Scenarios that never set it keep the same gesture they had.
        moves = max(2, int(step.get("steps", 12)))
        pts = [(round(x + (x2 - x) * i / moves), round(y + (y2 - y) * i / moves))
               for i in range(1, moves + 1)]
        seq = [["shell", "input", "motionevent", "DOWN", str(x), str(y)]]
        seq += [["shell", "input", "motionevent", "MOVE", str(px), str(py)] for px, py in pts]
        # THE DWELL. Three repeats measured sufficient; it is the whole point of this path, so it is
        # not configurable — a scenario that could set it to 0 could silently reintroduce the fling.
        seq += [["shell", "input", "motionevent", "MOVE", str(x2), str(y2)]] * 3
        seq.append(["shell", "input", "motionevent", "UP", str(x2), str(y2)])
        return seq
    raise ValueError(f"unknown scenario action: {action!r}")


def run_step(step: dict, size: tuple[int, int] | None = None) -> str:
    """Perform one scenario step on the device; returns (and prints) its status."""
    size = size or device_size()
    name = step.get("name", step.get("action", "?"))
    argv = input_argv(step, size)                 # raises on an authoring error; run_steps reports it
    if argv is None:
        status = "idle" if not step.get("action") else f"SKIPPED ({_HOVER_SKIP})"
    else:
        # ONE argv, or a SEQUENCE of them: the motionevent drag path (input_argv) has to emit
        # DOWN/MOVE.../UP as separate adb calls, because that separation is the entire mechanism —
        # it is what lets the gesture dwell before lifting. Normalised here so every other caller
        # keeps seeing "a step runs and either works or raises".
        batch = argv if argv and isinstance(argv[0], list) else [argv]
        # Bounded so a wedged adb can never outlive the burst it is driving (see capture_gif's join).
        # The budget is PER CALL; a drag is now many short calls rather than one blocking swipe, and
        # each is far under it.
        for one in batch:
            r = adb(*one, capture_output=True, text=True,
                    timeout=float(step.get("duration", _SWIPE_SECS)) + 20)
            if r.returncode != 0:
                raise RuntimeError(f"adb {' '.join(one)} -> rc={r.returncode} {r.stderr.strip()[:120]}")
        status = "ok"
    print(f"      step {name}: {status}", flush=True)
    return status


def run_steps(steps, size: tuple[int, int] | None = None) -> list[str]:
    """Perform a scenario's steps in order, stopping at the first failure.

    Stopping matters: steps are a SEQUENCE (focus the field, THEN type into it), so carrying on past a
    failure types into whatever happens to have focus and produces a frame nobody can explain. Nothing
    is raised — this runs on a background thread, where an escaping exception would print a bare
    traceback and lose the page it belongs to; the caller reads the returned statuses instead."""
    size = size or device_size()
    out: list[str] = []
    for step in steps or ():
        try:
            out.append(run_step(step, size))
        except Exception as exc:
            print(f"      step {step.get('name', '?')}: FAILED ({exc})", flush=True)
            out.append(f"FAILED ({exc})")
            break
    return out


def capture_gif(app: str, key: str, theme: str, secs: float = 4.0, settle: float = 2.0,
                frame_count: int = 12, steps: list[dict] | None = None,
                run_dir: str | None = None, column: str | None = None) -> str | None:
    """Launch the page and record `secs` of it. Returns the GIF path, or None if nothing usable.

    `steps` are the page's scenario steps (see run_steps). They run on a background thread started
    WITH the burst, which is the one mechanism that covers both "before" and "during": a tap is
    instantaneous, so it lands in the first frame or two and the rest of the burst records whatever it
    set off; a drag is an `input swipe` that BLOCKS for its whole duration, so running the steps ahead
    of the burst would record only the resting end state — the twelve-identical-frames outcome the
    burst exists to avoid. There is deliberately no before/during switch to get wrong.

    `run_dir` is the run ROOT (docs/comparison/<YYYY-MM-DD-HH_MM_SS>): give it, and the full-resolution
    burst is ALSO kept as <run_dir>/<key>/android/<column>/NNNN.png + sidecars — the evidence
    motion_score.py needs to score this page frame by frame instead of from one resting still. `column`
    is the RUNNER column (maui_xaml/cpp/cpp_xaml) and defaults from `app`. With `run_dir` None (the
    default) NOTHING changes: the burst lives and dies in the TemporaryDirectory, exactly as before,
    and the GIF is the only artifact either way."""
    spec = APPS[app]
    pkg = spec["pkg"]
    out = out_path(app, key, theme)
    os.makedirs(os.path.dirname(out), exist_ok=True)
    gifmod.drop_stale(out)

    adb("shell", "am", "force-stop", pkg, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    _wait_gone(pkg)
    adb("logcat", "-c", stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    adb("shell", "am", "start", "-W", "-n", _component(pkg),
        "--es", spec["page"], key,
        "--es", "MAUI_THEME", "Dark" if theme == "dark" else "Light",
        "--es", "MAUI_APPEARANCE", theme,
        stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    _wait_ready(pkg)
    adb("shell", "am", "broadcast", "-a", "android.intent.action.CLOSE_SYSTEM_DIALOGS",
        stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    time.sleep(settle)
    # AFTER the settle, not before it. _wait_ready returns on the `Displayed` logcat marker, which
    # says the app DID come up — it cannot say the app is still in front once the page has run for
    # `settle` seconds, and that gap is exactly where a page navigating away (Chrome, over the
    # gallery) put a foreign window on screen. Assert what is actually being photographed.
    assert_foreground(pkg, f"{app}/{key}/{theme}")

    # A BURST OF STILLS, not `screenrecord`. On this emulator screenrecord returns an mp4 carrying a
    # single frame with no timebase at all (r_frame_rate=1/0, duration=0), which ffmpeg's fps filter
    # turns into zero output frames — measured on both a static and an animating page. `screencap` is
    # ~0.13s per shot here, fast enough to catch a spinner moving, and it is the same path the still
    # pass already trusts.
    frames = []
    moments: list[int] = []      # each kept frame's NOMINAL sample number — see the run-unit note
    interval = max(0.0, secs / max(frame_count - 1, 1))
    statuses: list[str] = []
    # Resolve the display HERE, on the main thread: an emulator that cannot answer `wm size` is a lane
    # failure the caller must see, not a step failure reported from inside a thread.
    size = device_size() if steps else None
    driver = (threading.Thread(target=lambda: statuses.extend(run_steps(steps, size)), daemon=True)
              if steps else None)
    with tempfile.TemporaryDirectory() as tmp:
        # THE AT-REST FRAME, shot HERE: after the settle, before a single gesture, in exactly the device
        # state the burst runs in (theme set, demo mode pinned, animation scales as configured). This is
        # the only BEFORE a time-labelled burst can have — the frames themselves are named gif01..gifNN
        # whatever the driver does, so nothing downstream can pick one out. write_run_unit prefers it
        # over the main pass's still, which is shot under DIFFERENT state; see its comment for the
        # 89.63%-differing measurement that cost five green cells.
        #
        # It is NOT added to `frames`: the GIF is unchanged by this, and a page that turns out not to
        # move still yields the same animation it always did.
        at_rest = None
        if driver:
            shot = adb("exec-out", "screencap", "-p", capture_output=True).stdout
            if shot and len(shot) > 1000:
                at_rest = os.path.join(tmp, "at_rest.png")
                with open(at_rest, "wb") as fh:
                    fh.write(shot)
            else:
                print(f"      !! {key} ({app}/{theme}): at-rest screencap failed — the unit falls back "
                      f"to the main pass's still, whose device state does not match the burst",
                      flush=True)
        if driver:
            driver.start()                   # with the burst, not before it — see the docstring
        for i in range(frame_count):
            png = adb("exec-out", "screencap", "-p", capture_output=True).stdout
            if png and len(png) > 1000:
                f = os.path.join(tmp, f"{i:03d}.png")
                with open(f, "wb") as fh:
                    fh.write(png)
                frames.append(f)
                moments.append(i + 1)         # 1-based, and of the SCHEDULE, not of `frames`
            time.sleep(interval)
        if driver:
            # A step thread that outlives this call would inject into the NEXT page's app — a phantom
            # tap that reads as a random parity regression on a page nothing touched. Every adb call
            # inside it is individually timed out, so this join is a backstop; if it still trips, or
            # if a step failed, DROP the GIF: frames of a page that never reacted are an idle still
            # wearing an animation's name, and the still from the main pass already tells that truth.
            driver.join(timeout=secs + 60)
            if driver.is_alive():
                print(f"      !! {key} ({app}/{theme}): interaction thread still running after the "
                      f"burst — GIF dropped, and the next page may see a stray gesture", flush=True)
                return None
            if any(s.startswith("FAILED") for s in statuses):
                print(f"      !! {key} ({app}/{theme}): interaction failed — GIF dropped (the page "
                      f"never reacted, so its frames would misreport it as static)", flush=True)
                return None
        # PERSIST THE EVIDENCE — deliberately AFTER the two drops above and BEFORE the GIF.
        #
        # After the drops, because both of them mean the frames misreport the page: a failed step
        # photographs a page that never reacted, and a thread that outlived the burst was still
        # gesturing while the last frames were taken. Neither leaves a run unit at all, so
        # motion_score.find_frames reports the honest "no run directory has frames for both columns"
        # instead of scoring a capture failure as a parity finding.
        #
        # Before the GIF, and INDEPENDENT of its verdict, because gifmod deletes a GIF whose frames are
        # all identical — and "nothing moved" is a FINDING, not an absence. That is motion_score's
        # `both_frozen` verdict, and its mismatch check needs the frozen column's frames to state it.
        # Persisting only when a GIF survived would delete exactly the evidence for the one result this
        # pass exists to surface: MAUI animates, the port is frozen.
        #
        # Under 2 usable frames is the exception: _self_motion returns 0.0 for a single frame, so a
        # unit of one would read as a rock-solid "FROZEN" and could force a red on what is really a
        # screencap failure. Same threshold frames_to_gif already refuses at; write nothing, say so.
        if run_dir:
            if len(frames) < 2:
                print(f"      !! {key} ({app}/{theme}): only {len(frames)} usable frame(s) from a "
                      f"{frame_count}-frame burst — NO run unit written (a one-frame sequence would "
                      f"score as FROZEN and blame the port for a failed screencap)", flush=True)
            else:
                unit = write_run_unit(run_dir, column or APPS[app]["col"], app, key, theme,
                                      list(zip(moments, frames)), secs, frame_count, at_rest=at_rest)
                before = "burst at-rest" if at_rest else "still"
                print(f"      frames {len(frames)}/{frame_count} + {before} -> {unit}", flush=True)
        # frames_to_gif refuses a single frame, and _ffmpeg deletes a GIF whose frames are all
        # identical — so a page that genuinely does not move ends up with its still and no GIF.
        ok = gifmod.frames_to_gif(frames, out, fps=max(1, min(10, round(1 / max(interval, 0.1)))))
    return out if ok else None


def _selftest() -> None:
    """Device-free check of the two things that silently drive the WRONG PIXEL if they break: the
    fraction->pixel scaling, and the adb argv built from a step. Every command is BUILT, never run."""
    size = (1080, 2340)                       # this emulator; every expected value below is derived

    assert to_pixels([0.5, 0.2], size) == (540, 468)          # fraction of the display
    assert to_pixels([540, 468], size) == (540, 468)          # already device pixels
    assert to_pixels([1.0, 1.0], size) == (1080, 2340)        # the 1.0 boundary is a fraction
    for bad in ([0.5, 300], [300, 0.5]):                      # one axis scaled, one not
        try:
            to_pixels(bad, size)
            raise AssertionError(f"mixed coordinate {bad} was accepted")
        except ValueError:
            pass

    tap = input_argv({"name": "after-tap", "action": "click", "at": [0.5, 0.2]}, size)
    assert tap == ["shell", "input", "tap", "540", "468"], tap
    # The full command the adb() helper would run — the argv the device actually sees.
    assert [ADB, "-s", SERIAL, *tap] == [ADB, "-s", SERIAL, "shell", "input", "tap", "540", "468"]

    # scroll: negative dy reveals lower rows, i.e. the finger travels UP (y decreases).
    sc = input_argv({"action": "scroll", "at": [0.5, 0.5], "dy": -0.25}, size)
    assert sc == ["shell", "input", "swipe", "540", "1170", "540", "585", "800"], sc
    # ...and a dy that runs off the top clamps to row 0 rather than failing: a shorter drag, like a
    # real finger, is a truer answer than no gesture at all.
    assert input_argv({"action": "scroll", "at": [0.5, 0.1], "dy": -0.9}, size)[5:7] == ["540", "0"]

    sw = input_argv({"action": "swipe", "at": [0.8, 0.5], "to": [0.2, 0.5], "duration": 1.5}, size)
    assert sw == ["shell", "input", "swipe", "864", "1170", "216", "1170", "1500"], sw
    # The axis form, and `steps` (a desktop-agent knob) accepted and ignored rather than rejected.
    ax = input_argv({"action": "swipe", "at": [0.5, 0.5], "direction": "left", "steps": 10}, size)
    assert ax == ["shell", "input", "swipe", "540", "1170", "270", "1170", "800"], ax
    assert input_argv({"action": "drag", "at": [0.5, 0.5], "direction": "up",
                       "distance": 0.1}, size)[5:7] == ["540", "936"]
    for bad in ({"action": "swipe", "at": [0.5, 0.5]},                       # no `to`, no direction
                {"action": "swipe", "at": [0.5, 0.5], "direction": "sideways"},
                {"action": "drag", "at": [0.5, 0.0], "direction": "up"}):    # clamps to a no-op
        try:
            input_argv(bad, size)
            raise AssertionError(f"malformed drag {bad!r} was accepted")
        except ValueError:
            pass

    assert input_argv({"action": "type", "text": "MAUI test"}, size) == \
        ["shell", "input", "text", "MAUI%stest"], "spaces must be escaped, not argv-split"
    for bad in ("a; rm -rf /",                # shell metacharacter — eaten by the remote sh -c
                "-x"):                        # leading dash — `input text -x` reads as a flag
        try:
            input_argv({"action": "type", "text": bad}, size)
            raise AssertionError(f"unsafe text {bad!r} was accepted")
        except ValueError:
            pass

    assert input_argv({"name": "initial"}, size) is None                       # plain screenshot step
    assert input_argv({"action": "hover", "at": [0.5, 0.5]}, size) is None     # reported SKIPPED
    try:
        input_argv({"action": "pinch", "at": [0.5, 0.5]}, size)
        raise AssertionError("unknown action was accepted")
    except ValueError:
        pass
    try:                                      # a Mac-calibrated x, wider than this display
        input_argv({"name": "mac-coords", "action": "click", "at": [1400, 171]}, size)
        raise AssertionError("off-display start point was accepted")
    except ValueError:
        pass
    # But note the limit of that guard: button.toml's [756, 171] IS inside 1080x2340, so it builds a
    # perfectly valid tap on the wrong widget. Nothing can detect that — which is the whole argument
    # for authoring fractions.
    assert input_argv({"action": "click", "at": [756, 171]}, size)[-2:] == ["756", "171"]

    # run_step reports a hover instead of silently dropping it (and never touches adb to do so).
    assert run_step({"name": "peek", "action": "hover", "at": [0.5, 0.5]}, size).startswith("SKIPPED")
    assert run_steps([{"name": "initial"}], size) == ["idle"]
    # PER-LANE OVERRIDE. `at_android` must beat the portable `at`, and its ABSENCE must change nothing.
    # The defect this guards is the one that motivated the key: stepper's portable point is correct on
    # iOS (green) and lands on the wrong half of the control on Android, so a lane-specific value is the
    # only fix that does not break the other lane. A silent failure to promote would look exactly like a
    # scenario nobody had corrected yet.
    size = (1080, 2340)
    plain = {"name": "t", "action": "click", "at": [0.21, 0.13]}
    # 0.21*1080 = 226.8 -> 227: to_pixels ROUNDS. Worth pinning, because the crosshair sheet used
    # to locate these targets TRUNCATES, so a by-eye measurement and the driver can differ by a
    # pixel — harmless here (the plate is ~220px wide) and not harmless on a 16px radio ring.
    assert input_argv(plain, size) == ["shell", "input", "tap", "227", "304"], input_argv(plain, size)
    over = {**plain, "at_android": [0.367, 0.13]}
    assert input_argv(over, size) == ["shell", "input", "tap", "396", "304"], input_argv(over, size)
    assert input_argv(plain, size) != input_argv(over, size), "the override made no difference"
    # …and a DRAG promotes both ends independently.
    drag = {"name": "d", "action": "swipe", "at": [0.5, 0.5], "to": [0.9, 0.5],
            "at_android": [0.1, 0.2], "to_android": [0.8, 0.2]}
    argv = input_argv(drag, size)
    assert argv[:3] == ["shell", "input", "swipe"] and argv[3:7] == ["108", "468", "864", "468"], argv

    print("capture_android selftest: coordinate scaling + adb argv OK")


def _run_unit_selftest() -> None:
    """Device-free check that the unit this module WRITES is the one motion_score.py READS.

    Asserted against the real reader (motion_score.score_cell), never against key names copied out of
    it: the defect being guarded is a contract drift that leaves every Android animated page silently
    back on "NOT motion-scored", which no test of our own field names would notice."""
    global COMP_CAP                                    # noqa: PLW0603  redirected to a scratch board

    import shutil as _shutil  # noqa: PLC0415  selftest-only
    import tempfile as _tempfile  # noqa: PLC0415

    import motion_score  # noqa: PLC0415  pulls pixel_score/recapture — the real readers
    from PIL import Image  # noqa: PLC0415

    def png(path, x):
        """A 120x160 white page with a 20x20 black box at x — big enough for the 11x11 SSIM window."""
        im = Image.new("RGB", (120, 160), "white")
        for dx in range(20):
            for dy in range(20):
                im.putpixel((x + dx, 10 + dy), (0, 0, 0))
        im.save(path)
        return str(path)

    STILL = {"ssim": 1.0, "diff_pct": 0.0}
    real_cap = COMP_CAP
    with _tempfile.TemporaryDirectory() as tmp:
        comp = Path(tmp)
        run = comp / "2026-08-05-01_02_29"             # the run-dir name shape motion_score globs for
        shots = comp / "shots"
        shots.mkdir()
        COMP_CAP = str(comp / "captures" / RUN_PLAT_DIR)     # where a "published" still lives
        try:
            def publish(app, key, theme, x):
                p = Path(still_path(app, key, theme))
                p.parent.mkdir(parents=True, exist_ok=True)
                return png(p, x)

            def unit(app, key, theme, samples, frame_count=3, secs=3.0):
                return write_run_unit(str(run), APPS[app]["col"], app, key, theme,
                                      [(n, png(shots / f"{key}_{app}_{theme}_{n}.png", x))
                                       for n, x in samples], secs, frame_count)

            # (1) HAPPY PATH — two equal recordings pair, and the copied-in still is NOT scored as a
            #     frame of the burst (4 frames written, 3 paired).
            publish("maui", "same", "light", 10)
            publish("cpp", "same", "light", 10)
            um = unit("maui", "same", "light", [(1, 10), (2, 40), (3, 70)])
            unit("cpp", "same", "light", [(1, 10), (2, 40), (3, 70)])
            r = motion_score.score_cell("same", RUN_PLAT_DIR, "cpp", "light", 0, STILL, comp)
            assert "MOTION 3 frames paired by step" in r["detail"], r["detail"]
            assert "NOT motion-scored" not in r["detail"], r["detail"]
            assert (r["ssim"], r["mismatch"], r["both_frozen"]) == (1.0, False, False), r
            # …and the published-still bytes really are in the unit, which is what let it be found.
            assert Path(um, "0001.png").read_bytes() == \
                Path(still_path("maui", "same", "light")).read_bytes()
            assert json.loads(Path(um, "0001.json").read_text())["step"] == "initial"

            # (2) A DROPPED SCREENCAP LEAVES A GAP, not a shift. cpp misses sample 2, so its surviving
            #     frame 3 must pair with MAUI's frame 3 (identical -> SSIM 1.0). Index pairing would
            #     put it against MAUI's frame 2 and score a difference that never happened.
            publish("maui", "gap", "light", 10)
            publish("cpp", "gap", "light", 10)
            unit("maui", "gap", "light", [(1, 10), (2, 40), (3, 70)])
            unit("cpp", "gap", "light", [(1, 10), (3, 70)])
            r = motion_score.score_cell("gap", RUN_PLAT_DIR, "cpp", "light", 0, STILL, comp)
            assert "MOTION 2 frames" in r["detail"] and r["ssim"] == 1.0, r["detail"]
            assert "1 frame(s) had no partner" in r["detail"], r["detail"]

            # (3) UNEQUAL RECORDINGS REFUSE. Same three samples, but cpp recorded a 12-frame burst:
            #     the k-th sample is no longer the same nominal moment, the names are disjoint, and
            #     the cell must fall back to the labelled still rather than pair anything.
            publish("maui", "geom", "light", 10)
            publish("cpp", "geom", "light", 10)
            unit("maui", "geom", "light", [(1, 10), (2, 40), (3, 70)], frame_count=3)
            unit("cpp", "geom", "light", [(1, 10), (2, 40), (3, 70)], frame_count=12)
            r = motion_score.score_cell("geom", RUN_PLAT_DIR, "cpp", "light", 0, STILL, comp)
            assert "NO step name occurs in both" in r["detail"], r["detail"]
            assert "MOTION" not in r["detail"] and (r["ssim"], r["diff_pct"]) == (1.0, 0.0), r

            # (4) BOTH THEMES SHARE ONE UNIT DIR, and a re-capture REPLACES its own theme only.
            publish("cpp", "themes", "light", 10)
            publish("cpp", "themes", "dark", 10)
            u = Path(unit("cpp", "themes", "light", [(1, 10), (2, 40)]))
            unit("cpp", "themes", "dark", [(1, 10), (2, 40)])
            assert sorted(p.name for p in u.glob("*.png")) == \
                ["0001.png", "0002.png", "0003.png", "0004.png", "0005.png", "0006.png"]
            unit("cpp", "themes", "light", [(1, 10), (2, 40), (3, 70)])   # re-capture, same run
            themes = [json.loads(p.read_text())["theme"] for p in sorted(u.glob("*.json"))]
            assert themes.count("light") == 4 and themes.count("dark") == 3, themes

            # (5) NO PUBLISHED STILL: the burst is still kept (it is real), but the provenance frame
            #     cannot be, and nothing pretends otherwise.
            u = Path(unit("cpp", "orphan", "light", [(1, 10), (2, 40)]))
            steps = [json.loads(p.read_text())["step"] for p in sorted(u.glob("*.json"))]
            assert steps == [step_name(1, 3.0, 3), step_name(2, 3.0, 3)], steps

            # (6) An unknown column would write frames no scorer ever looks for.
            try:
                write_run_unit(str(run), "cpp_appkit_typo", "cpp", "x", "light", [], 3.0, 3)
                raise AssertionError("unknown column was accepted")
            except ValueError:
                pass
        finally:
            COMP_CAP = real_cap
            _shutil.rmtree(shots, ignore_errors=True)
    print(f"capture_android selftest: run unit reads back through motion_score "
          f"(steps {step_name(1, 4.0, 12)} … {step_name(12, 4.0, 12)}) OK")


if __name__ == "__main__":
    _selftest()
    _run_unit_selftest()
