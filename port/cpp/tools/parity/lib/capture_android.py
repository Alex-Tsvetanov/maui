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

The per-page determinism mirrors the shell scripts exactly — force-stop, wait for the process to be
GONE, clear logcat, `am start -W`, poll for THIS launch's Displayed marker, dismiss any ANR dialog —
because a recording that starts on the previous page's frame is worse than no recording.
"""
from __future__ import annotations

import os
import subprocess
import tempfile
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


def capture_gif(app: str, key: str, theme: str, secs: float = 4.0, settle: float = 2.0,
                frame_count: int = 12) -> str | None:
    """Launch the page and record `secs` of it. Returns the GIF path, or None if nothing usable."""
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
    with tempfile.TemporaryDirectory() as tmp:
        for i in range(frame_count):
            png = adb("exec-out", "screencap", "-p", capture_output=True).stdout
            if png and len(png) > 1000:
                f = os.path.join(tmp, f"{i:03d}.png")
                with open(f, "wb") as fh:
                    fh.write(png)
                frames.append(f)
            time.sleep(interval)
        # frames_to_gif refuses a single frame, and _ffmpeg deletes a GIF whose frames are all
        # identical — so a page that genuinely does not move ends up with its still and no GIF.
        ok = gifmod.frames_to_gif(frames, out, fps=max(1, min(10, round(1 / max(interval, 0.1)))))
    return out if ok else None
