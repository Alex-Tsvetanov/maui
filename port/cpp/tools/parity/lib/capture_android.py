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
"""
from __future__ import annotations

import os
import re
import subprocess
import tempfile
import threading
import time

import gif as gifmod
from device_state import set_android_theme

SERIAL = os.environ.get("MAUI_ANDROID_SERIAL", "emulator-5554")
ADB = os.environ.get("MAUI_ADB", "adb")
HERE = os.path.dirname(os.path.abspath(__file__))
CPP = os.path.abspath(os.path.join(HERE, "..", "..", ".."))
COMP_CAP = os.path.join(CPP, "docs", "comparison", "captures", "android")

# package + page-extra per column. MauiReference reads MAUI_COMPARE_PAGE, the C++ app hosts read
# MAUI_SAMPLE_PAGE; both theme extras are sent every time (each family ignores the other's).
APPS = {
    "maui": {"pkg": "dev.mauicpp.mauireference", "page": "MAUI_COMPARE_PAGE", "dir": "maui"},
    "cpp": {"pkg": "dev.mauicpp.apphost", "page": "MAUI_SAMPLE_PAGE", "dir": "cpp"},
    "xaml": {"pkg": "dev.mauicpp.apphost.xaml", "page": "MAUI_SAMPLE_PAGE", "dir": "xaml"},
}


def adb(*args, **kw):
    return subprocess.run([ADB, "-s", SERIAL, *args], **kw)


def out_path(app: str, key: str, theme: str) -> str:
    return os.path.join(COMP_CAP, APPS[app]["dir"], f"{key}_{theme}.gif")


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
#   scroll at, dy                             -> input swipe (dy along y; see the sign note above)
#   drag/swipe at, to | direction[, distance] -> input swipe; `steps` is accepted and ignored
#   hover at                                  -> SKIPPED, loudly. There is no pointer to park.
# `duration` is in SECONDS, like every other time key in a scenario (`settle`); it is converted to the
# milliseconds `input swipe` wants.
_HOVER_SKIP = ("hover: Android has no pointer, so there is nothing to hover with — and a tap is a "
               "different gesture, not a substitute")
# A drag long enough not to read as a fling. `input swipe` interpolates linearly and releases at full
# velocity, so a fast swipe hands Android's VelocityTracker ~1300 px/s and the list keeps coasting: the
# resting offset is NOT repeatable run to run. A longer press-move-release shrinks that but cannot
# remove it (there is no fling-free `input swipe`), so treat a driven scroll as a motion witness for
# the GIF, never as a pixel oracle for a still.
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
    w, h = size
    x, y = to_pixels(step["at"], size)
    if not (0 <= x < w and 0 <= y < h):
        raise ValueError(f"step {step.get('name', action)!r} starts at ({x},{y}), outside the {w}x{h} "
                         f"display: Android coordinates are THIS device's pixels, and the checked-in "
                         f"scenarios are calibrated for a 1512x950 Mac. Use a 0..1 fraction instead")
    if action == "click":
        return ["shell", "input", "tap", str(x), str(y)]
    if action in ("scroll", "swipe", "drag"):
        # ONE `input swipe` serves all three: a slow press-move-release IS a pan/drag, and Android has
        # no separate scroll or drag injection. The vocabulary's `steps` (how many move events the
        # desktop agents synthesise) has no analogue here — the driver interpolates for us — so it is
        # accepted and IGNORED rather than rejected, which keeps one scenario file valid everywhere.
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
        ms = max(1, round(float(step.get("duration", _SWIPE_SECS)) * 1000))
        return ["shell", "input", "swipe", str(x), str(y), str(x2), str(y2), str(ms)]
    raise ValueError(f"unknown scenario action: {action!r}")


def run_step(step: dict, size: tuple[int, int] | None = None) -> str:
    """Perform one scenario step on the device; returns (and prints) its status."""
    size = size or device_size()
    name = step.get("name", step.get("action", "?"))
    argv = input_argv(step, size)                 # raises on an authoring error; run_steps reports it
    if argv is None:
        status = "idle" if not step.get("action") else f"SKIPPED ({_HOVER_SKIP})"
    else:
        # Bounded so a wedged adb can never outlive the burst it is driving (see capture_gif's join).
        r = adb(*argv, capture_output=True, text=True,
                timeout=float(step.get("duration", _SWIPE_SECS)) + 20)
        if r.returncode != 0:
            raise RuntimeError(f"adb {' '.join(argv)} -> rc={r.returncode} {r.stderr.strip()[:120]}")
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
                frame_count: int = 12, steps: list[dict] | None = None) -> str | None:
    """Launch the page and record `secs` of it. Returns the GIF path, or None if nothing usable.

    `steps` are the page's scenario steps (see run_steps). They run on a background thread started
    WITH the burst, which is the one mechanism that covers both "before" and "during": a tap is
    instantaneous, so it lands in the first frame or two and the rest of the burst records whatever it
    set off; a drag is an `input swipe` that BLOCKS for its whole duration, so running the steps ahead
    of the burst would record only the resting end state — the twelve-identical-frames outcome the
    burst exists to avoid. There is deliberately no before/during switch to get wrong."""
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

    # A BURST OF STILLS, not `screenrecord`. On this emulator screenrecord returns an mp4 carrying a
    # single frame with no timebase at all (r_frame_rate=1/0, duration=0), which ffmpeg's fps filter
    # turns into zero output frames — measured on both a static and an animating page. `screencap` is
    # ~0.13s per shot here, fast enough to catch a spinner moving, and it is the same path the still
    # pass already trusts.
    frames = []
    interval = max(0.0, secs / max(frame_count - 1, 1))
    statuses: list[str] = []
    # Resolve the display HERE, on the main thread: an emulator that cannot answer `wm size` is a lane
    # failure the caller must see, not a step failure reported from inside a thread.
    size = device_size() if steps else None
    driver = (threading.Thread(target=lambda: statuses.extend(run_steps(steps, size)), daemon=True)
              if steps else None)
    with tempfile.TemporaryDirectory() as tmp:
        if driver:
            driver.start()                   # with the burst, not before it — see the docstring
        for i in range(frame_count):
            png = adb("exec-out", "screencap", "-p", capture_output=True).stdout
            if png and len(png) > 1000:
                f = os.path.join(tmp, f"{i:03d}.png")
                with open(f, "wb") as fh:
                    fh.write(png)
                frames.append(f)
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
    print("capture_android selftest: coordinate scaling + adb argv OK")


if __name__ == "__main__":
    _selftest()
