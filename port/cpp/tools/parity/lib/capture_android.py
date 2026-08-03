#!/usr/bin/env python3
"""Android GIF capture — the motion pass for the animated pages.

The STILL pass is the three build+install+capture scripts next to this file; they own the APK
pipeline and are what the board has always been captured with. This module only adds what they cannot
do: `adb shell screenrecord` for the handful of pages a single frame cannot represent.

It runs AFTER the still pass for a theme, which means the device night mode those scripts set has
already been RESTORED by their exit trap — so this pass sets it again itself (the port reads
Configuration.uiMode, not the intent extra, so without this the "dark" GIF would be a light one).

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
REMOTE_MP4 = "/sdcard/parity_gif.mp4"

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


def set_theme(theme: str) -> str:
    """Device night mode for this pass; returns the previous value so the caller can restore it."""
    return set_android_theme(theme, SERIAL)


def capture_gif(app: str, key: str, theme: str, secs: float = 4.0, settle: float = 2.0) -> str | None:
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

    adb("shell", "rm", "-f", REMOTE_MP4, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    # --time-limit is an INTEGER number of seconds and screenrecord blocks for the whole window.
    adb("shell", "screenrecord", "--time-limit", str(int(round(secs))), "--bit-rate", "4000000",
        REMOTE_MP4, capture_output=True)
    local = os.path.join(tempfile.gettempdir(), f"parity_android_{app}_{key}_{theme}.mp4")
    adb("pull", REMOTE_MP4, local, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    adb("shell", "rm", "-f", REMOTE_MP4, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    # screenrecord fails by producing a 0-byte file rather than by erroring — video_to_gif checks size.
    ok = gifmod.video_to_gif(local, out)
    if os.path.exists(local):
        os.remove(local)
    return out if ok else None
