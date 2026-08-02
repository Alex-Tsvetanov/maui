#!/usr/bin/env python3
"""WS-E clean iOS capture — dodges the SpringBoard "back to <app>" overlay artifact.

The overlay (a "◀ <previous-app>" affordance in a SEPARATE window) leaks into
`simctl io screenshot` ONLY on the first launch after switching FROM a different app.
A same-app relaunch (--terminate-running-process on an already-foreground bundle) does
NOT re-trigger it. So the flow is: launch the target bundle ONCE to make it foreground
(warm-up, discarded), then for each (key, theme) relaunch the SAME bundle + settle +
screenshot. No idb / XCUITest tap needed (SIGSTOP/SIGCONT was flaky — see iter35).

Apps (bundle + env contract):
  maui -> dev.mauicpp.mauireference   MAUI_COMPARE_PAGE / MAUI_THEME(Light|Dark)  -> port/maui-reference/captures/ios/<key>_<theme>.png
  cpp  -> dev.maui-cpp.ios-gallery     MAUI_SAMPLE_PAGE  / MAUI_APPEARANCE(light|dark) -> docs/comparison/captures/ios/cpp/<key>_<theme>.png
  xaml -> dev.maui-cpp.ios-gallery-xaml MAUI_SAMPLE_PAGE / MAUI_APPEARANCE -> docs/comparison/captures/ios/xaml/<key>_<theme>.png

Does NOT build or install — `ninja` the gallery / `dotnet build` MauiReference and
`simctl install` first. Usage:
  python3 capture_ios_clean.py --app cpp --themes light,dark --only switch,ios_scroll_view
  python3 capture_ios_clean.py --app maui --themes light,dark            # all shared pages
"""
import argparse
from capture_guard import is_splash  # reject .NET startup-splash frames: see capture_guard.py
from device_state import clear_ios, pin_ios, set_ios_theme  # fixed status bar + system appearance
import os
import shutil
import subprocess
import sys
import tempfile
import time

UDID = os.environ.get("MAUI_SIM_UDID", "C4926671-2FA7-428E-B4A4-480692EE742B")
HERE = os.path.dirname(os.path.abspath(__file__))
CPP = os.path.abspath(os.path.join(HERE, "..", ".."))          # port/cpp
PORT = os.path.abspath(os.path.join(CPP, ".."))                 # port
PAGES = os.path.join(PORT, "maui-reference", "pages")
REF_CAP = os.path.join(PORT, "maui-reference", "captures", "ios")
COMP_CAP = os.path.join(CPP, "docs", "comparison", "captures", "ios")

APPS = {
    "maui": {"bundle": "dev.mauicpp.mauireference", "page": "MAUI_COMPARE_PAGE",
             "theme": "MAUI_THEME", "theme_val": lambda t: "Dark" if t == "dark" else "Light",
             "out": lambda k, t: os.path.join(REF_CAP, f"{k}_{t}.png")},
    "cpp": {"bundle": "dev.maui-cpp.ios-gallery", "page": "MAUI_SAMPLE_PAGE",
            "theme": "MAUI_APPEARANCE", "theme_val": lambda t: t,
            "out": lambda k, t: os.path.join(COMP_CAP, "cpp", f"{k}_{t}.png")},
    "xaml": {"bundle": "dev.maui-cpp.ios-gallery-xaml", "page": "MAUI_SAMPLE_PAGE",
             "theme": "MAUI_APPEARANCE", "theme_val": lambda t: t,
             "out": lambda k, t: os.path.join(COMP_CAP, "xaml", f"{k}_{t}.png")},
}


def all_keys():
    # the board pages = shared XAML minus the deliberately-broken gap_*.xaml corpus.
    return sorted(f[:-5] for f in os.listdir(PAGES)
                  if f.endswith(".xaml") and not f.startswith("gap_"))


def launch(spec, key, theme, system_theme: bool):
    env = dict(os.environ)
    env[f"SIMCTL_CHILD_{spec['page']}"] = key
    # Under system_theme the SIMULATOR's appearance is the source and the per-app env var is deliberately
    # NOT set: MAUI_APPEARANCE / MAUI_THEME both map to UserAppTheme, which OVERRIDES the OS, so setting
    # one would mean the capture proves the override works rather than that the app follows the system.
    if not system_theme:
        env[f"SIMCTL_CHILD_{spec['theme']}"] = spec["theme_val"](theme)
    subprocess.run(["xcrun", "simctl", "launch", "--terminate-running-process", UDID, spec["bundle"]],
                   env=env, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, check=True)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--app", required=True, choices=list(APPS))
    ap.add_argument("--themes", default="light,dark")
    ap.add_argument("--only", default="")
    ap.add_argument("--settle", type=float, default=4.0)
    ap.add_argument("--app-theme-env", action="store_true",
                    help="legacy: force the theme with the per-app env var instead of the SIMULATOR's "
                         "system appearance. Off by default — the board measures system-wide theming.")
    args = ap.parse_args()
    system_theme = not args.app_theme_env

    spec = APPS[args.app]
    themes = [t.strip() for t in args.themes.split(",") if t.strip()]
    keys = [k.strip() for k in args.only.split(",") if k.strip()] or all_keys()

    # Pin the status bar for the WHOLE run. iOS captures are full-screen, so the clock, battery and
    # signal bars sit inside every frame; unpinned they differ between the reference pass and the port
    # pass and score as a diff on every single page. Pinned to the SAME values by both, they cancel.
    # Pinning THIS udid specifically (the one this script drives) — see device_state._ios_udid.
    pin_ios(UDID)
    restore_appearance = None

    n = 0
    # THEME OUTERMOST. It used to be the inner loop, which was free when the theme was an env var handed
    # to each launch. Under system_theme it is a property of the SIMULATOR, so the inner ordering would
    # flip the device appearance once per page — ~364 flips on a full board — instead of once per theme.
    for theme in themes:
        if system_theme:
            previous = set_ios_theme(theme, UDID)
            if restore_appearance is None:
                restore_appearance = previous  # only the FIRST pass records the pre-run state
        # Warm-up: bring the bundle to foreground ONCE so subsequent same-app relaunches
        # don't trigger the SpringBoard back-to-app overlay. Discard this frame. Re-done per theme
        # because the appearance change itself can put SpringBoard in front.
        launch(spec, keys[0], theme, system_theme)
        time.sleep(args.settle + 2.0)
        for key in keys:
            out = spec["out"](key, theme)
            os.makedirs(os.path.dirname(out), exist_ok=True)
            launch(spec, key, theme, system_theme)
            time.sleep(args.settle)
            # simctl (a subprocess) can't write into ~/Documents (macOS TCC), so screenshot to a
            # /tmp staging path, then copy into the repo with Python file I/O (which is permitted).
            # Retry: screenshot can transiently fail if it fires mid display-transition.
            stage = os.path.join(tempfile.gettempdir(), f"wse_{args.app}_{key}_{theme}.png")
            for attempt in range(4):
                r = subprocess.run(["xcrun", "simctl", "io", UDID, "screenshot", "--type=png", stage],
                                   stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
                if r.returncode == 0 and os.path.exists(stage):
                    break
                time.sleep(1.0)
            else:
                print(f"  WARN: screenshot failed after retries: {key} {theme}")
                continue
            # A screenshot can succeed and still be the WRONG THING: a .NET app that has not finished
            # starting shows the purple ".NET" splash. That is not a degraded frame, it is a frame of a
            # different screen, and it scores as an enormous port defect on a page the port may render
            # perfectly. 18 such frames are committed in this very directory from one earlier run
            # (contiguous: clipping..empty_view_swap), all captured while three other platforms were
            # building on the same host — the failure is LOAD-dependent, so no fixed settle fixes it.
            # Re-launch and wait longer, escalating; drop the frame rather than bank a known-bad one.
            shutil.copyfile(stage, out)
            os.remove(stage)
            if is_splash(out):
                banked = False
                for extra in (4.0, 8.0, 16.0):
                    print(f"  ~ splash on {key} {theme} — relaunching, +{extra:.0f}s settle")
                    launch(spec, key, theme, system_theme)
                    time.sleep(args.settle + extra)
                    r = subprocess.run(["xcrun", "simctl", "io", UDID, "screenshot", "--type=png", stage],
                                       stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
                    if r.returncode != 0 or not os.path.exists(stage):
                        continue
                    shutil.copyfile(stage, out)
                    os.remove(stage)
                    if not is_splash(out):
                        banked = True
                        break
                if not banked:
                    os.remove(out)
                    print(f"  ! DROPPED (still splash after retries): {key} {theme}")
                    continue
            n += 1
            print(f"[{n}] {args.app} {theme} {key} -> {os.path.relpath(out, PORT)}")
    # Put the simulator's appearance back where we found it — same reasoning as the status-bar restore
    # below, plus: the NEXT run's "previous" reading would otherwise record our leftover value as the
    # device's real pre-run state.
    if restore_appearance:
        set_ios_theme(restore_appearance, UDID)
    # Restore the simulator's real status bar. Leaving it pinned is harmless for captures but
    # confusing for anyone using the sim afterwards, and a pin left on across a REBOOT would silently
    # expire, so pin/clear is kept per-run rather than assumed sticky.
    clear_ios(UDID)
    print(f"CLEAN_CAPTURE_DONE ({n} shots)")


if __name__ == "__main__":
    sys.exit(main())
