#!/usr/bin/env python3
"""iOS simulator capture engine (was capture_ios_clean.py — the WS-E clean capture).

recapture.py owns the per-example loop; this module owns the iOS-specific traps, which are the whole
reason it is not three lines of `simctl`:

  * SpringBoard "◀ back to <app>" overlay — leaks into `simctl io screenshot` ONLY on the first launch
    after switching FROM a different app. A same-app relaunch does not re-trigger it, so every theme
    pass starts with a discarded warm-up launch (`warmup()`).
  * THEME IS A DEVICE SETTING, not an app env var: MAUI_APPEARANCE / MAUI_THEME both map to
    UserAppTheme, which OVERRIDES the OS — setting one would prove the override works rather than that
    the app follows the system. So `set_theme()` drives the simulator and the env var is NOT sent.
    That also makes theme the OUTER loop for the caller: flipping it per page would be ~364 device
    appearance changes on a full board.
  * STATUS BAR pinned for the whole run (`pin()` / `unpin()`): iOS captures are full-screen, so an
    unpinned clock/battery/signal scores as a diff on every single page.
  * TCC: a simctl subprocess cannot write into ~/Documents, so every shot stages in /tmp and Python
    copies the bytes into the repo.
  * .NET SPLASH frames: a screenshot can succeed and still be a picture of the WRONG SCREEN. Escalating
    relaunch, and the frame is DROPPED rather than banked (a splash scores as an enormous port defect
    on a page the port may render perfectly).

Apps (bundle + env contract):
  maui -> dev.mauicpp.mauireference     MAUI_COMPARE_PAGE -> port/maui-reference/captures/ios/<key>_<theme>.png
  cpp  -> dev.maui-cpp.ios-gallery      MAUI_SAMPLE_PAGE  -> docs/comparison/captures/ios/cpp/<key>_<theme>.png
  xaml -> dev.maui-cpp.ios-gallery-xaml MAUI_SAMPLE_PAGE  -> docs/comparison/captures/ios/xaml/<key>_<theme>.png

Does NOT build or install.
"""
import os
import shutil
import signal
import subprocess
import tempfile
import time

import gif as gifmod
from capture_guard import is_splash
from device_state import clear_ios, pin_ios, set_ios_theme

UDID = os.environ.get("MAUI_SIM_UDID", "C4926671-2FA7-428E-B4A4-480692EE742B")
HERE = os.path.dirname(os.path.abspath(__file__))
CPP = os.path.abspath(os.path.join(HERE, "..", "..", ".."))     # port/cpp
PORT = os.path.abspath(os.path.join(CPP, ".."))                 # port
REF_CAP = os.path.join(PORT, "maui-reference", "captures", "ios")
COMP_CAP = os.path.join(CPP, "docs", "comparison", "captures", "ios")

APPS = {
    "maui": {"bundle": "dev.mauicpp.mauireference", "page": "MAUI_COMPARE_PAGE",
             "out": lambda k, t, e: os.path.join(REF_CAP, f"{k}_{t}.{e}")},
    "cpp": {"bundle": "dev.maui-cpp.ios-gallery", "page": "MAUI_SAMPLE_PAGE",
            "out": lambda k, t, e: os.path.join(COMP_CAP, "cpp", f"{k}_{t}.{e}")},
    "xaml": {"bundle": "dev.maui-cpp.ios-gallery-xaml", "page": "MAUI_SAMPLE_PAGE",
             "out": lambda k, t, e: os.path.join(COMP_CAP, "xaml", f"{k}_{t}.{e}")},
}


def out_path(app: str, key: str, theme: str, ext: str = "png") -> str:
    return APPS[app]["out"](key, theme, ext)


def pin(udid: str = UDID) -> None:
    pin_ios(udid)


def unpin(udid: str = UDID) -> None:
    clear_ios(udid)


def set_theme(theme: str, udid: str = UDID) -> str:
    """Set the SIMULATOR's appearance; returns the previous value (restore it when the run ends)."""
    return set_ios_theme(theme, udid)


def launch(app: str, key: str, udid: str = UDID) -> None:
    env = dict(os.environ)
    env[f"SIMCTL_CHILD_{APPS[app]['page']}"] = key
    subprocess.run(["xcrun", "simctl", "launch", "--terminate-running-process", udid, APPS[app]["bundle"]],
                   env=env, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, check=True)


def warmup(app: str, key: str, settle: float, udid: str = UDID) -> None:
    """Foreground the bundle once per theme pass and DISCARD the frame — kills the SpringBoard overlay."""
    launch(app, key, udid)
    time.sleep(settle + 2.0)


def _screenshot(stage: str, udid: str) -> bool:
    # Retry: a screenshot fired mid display-transition fails transiently.
    for _ in range(4):
        r = subprocess.run(["xcrun", "simctl", "io", udid, "screenshot", "--type=png", stage],
                           stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        if r.returncode == 0 and os.path.exists(stage):
            return True
        time.sleep(1.0)
    return False


def capture_still(app: str, key: str, theme: str, settle: float, udid: str = UDID) -> str | None:
    """Launch + settle + screenshot. Returns the written path, or None if the frame was DROPPED."""
    out = out_path(app, key, theme, "png")
    os.makedirs(os.path.dirname(out), exist_ok=True)
    stage = os.path.join(tempfile.gettempdir(), f"parity_{app}_{key}_{theme}.png")
    launch(app, key, udid)
    time.sleep(settle)
    if not _screenshot(stage, udid):
        return None
    shutil.copyfile(stage, out)
    os.remove(stage)
    if not is_splash(out):
        return out
    for extra in (4.0, 8.0, 16.0):
        launch(app, key, udid)
        time.sleep(settle + extra)
        if not _screenshot(stage, udid):
            continue
        shutil.copyfile(stage, out)
        os.remove(stage)
        if not is_splash(out):
            return out
    os.remove(out)   # still a splash — drop it rather than bank a known-bad frame
    return None


def capture_gif(app: str, key: str, theme: str, settle: float, record_secs: float = 4.0,
                udid: str = UDID) -> str | None:
    """Record a short mp4 and convert it to a paletted GIF — for pages a single still cannot represent.

    The GIF is not a nicety: comparison_paths.find_capture() and build_comparison_json.py both prefer
    `.gif` over `.png`, so refreshing an animated page's PNG alone leaves the board rendering the
    PREVIOUS run's GIF behind a green log line.
    """
    gif = out_path(app, key, theme, "gif")
    os.makedirs(os.path.dirname(gif), exist_ok=True)
    gifmod.drop_stale(gif)
    mp4 = os.path.join(tempfile.gettempdir(), f"parity_{app}_{key}_{theme}.mp4")
    launch(app, key, udid)
    time.sleep(settle)
    proc = subprocess.Popen(["xcrun", "simctl", "io", udid, "recordVideo", "--codec=h264", "--force", mp4],
                            stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    time.sleep(record_secs)
    proc.send_signal(signal.SIGINT)   # simctl finalizes the mp4 on SIGINT
    try:
        proc.wait(timeout=20)
    except subprocess.TimeoutExpired:
        proc.kill()
    ok = gifmod.video_to_gif(mp4, gif)
    if os.path.exists(mp4):
        os.remove(mp4)
    return gif if ok else None
