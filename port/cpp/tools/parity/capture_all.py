#!/usr/bin/env python3
"""Unified native-resolution parity capture for BOTH stacks on ONE simulator instance.

Supersedes capture_all_cpp.py + capture_all_csharp.py for the pixel-perfect rebuild:
  - Captures the C++ gallery (dev.maui-cpp.ios-gallery) AND the real-.NET-MAUI baseline
    (com.companyname.mauicompare) on the SAME booted sim, at NATIVE resolution (NO sips downsize),
    into docs/comparison/captures/{cpp,maui}_{light,dark}/<key>.png.
  - For the animated pages, records a short mp4 via `simctl io recordVideo` and converts it to a
    paletted GIF via ffmpeg, into the same dirs as <key>.{mp4,gif}.

Both apps MUST be installed on the sim first (the C++ gallery rebuilt from the current port; maui-compare
rebuilt on the pinned stable MAUI with UILaunchScreen). This script does NOT install.

Keys come from tools/parity/page_keys.txt (the 172-key both-apps-supported set). Status-bar/clock is NOT
stripped here — the diff script masks it.

Usage:
  python3 capture_all.py [--apps cpp,maui] [--themes light,dark] [--mode static|animated|both]
                         [--only key1,key2] [--record-secs 4.0]
"""
import argparse
import os
import signal
import subprocess
import time

HERE = os.path.dirname(os.path.abspath(__file__))
CMP = os.path.normpath(os.path.join(HERE, "..", "..", "docs", "comparison"))
CAP = os.path.join(CMP, "captures")
UDID = os.environ.get("MAUI_SIM_UDID", "C4926671-2FA7-428E-B4A4-480692EE742B")

# Per-app launch contract: bundle id + the SIMCTL_CHILD_* env keys for page + theme, and a settle time.
APPS = {
    "cpp": {
        "bundle": "dev.maui-cpp.ios-gallery",
        "page_env": "SIMCTL_CHILD_MAUI_SAMPLE_PAGE",
        "theme_env": "SIMCTL_CHILD_MAUI_APPEARANCE",
        "theme_val": {"light": "light", "dark": "dark"},
        "settle": float(os.environ.get("MAUI_SETTLE_CPP", "3.0")),
    },
    "maui": {
        "bundle": "com.companyname.mauicompare",
        "page_env": "SIMCTL_CHILD_MAUI_COMPARE_PAGE",
        "theme_env": "SIMCTL_CHILD_MAUI_THEME",
        "theme_val": {"light": "Light", "dark": "Dark"},
        "settle": float(os.environ.get("MAUI_SETTLE_MAUI", "4.5")),
    },
}

# Animated pages (🎬) — a single still can't represent them. Auto-animating ones record meaningfully;
# the purely-interactive ones (gestures/swipe/pan/pointer) capture only the idle state (annotated as such
# in the README) unless input injection is added later.
ANIMATED = {
    "activity_indicator", "animation", "carousel_page", "swipe_refresh", "empty_view_load_simulate",
    "swipe_gesture", "swipe_item_position", "gestures", "pan_gesture_events", "pointer_gesture",
    "ios_pan_gesture", "ios_swipe_transition", "ios_blur_effect", "chrome",
}


def load_keys():
    with open(os.path.join(HERE, "page_keys.txt")) as fh:
        return [ln.strip() for ln in fh if ln.strip()]


def run(*a):
    return subprocess.run(a, capture_output=True, text=True)


def launch(app, key, theme):
    """Terminate + relaunch `app` showing `key` in `theme`; returns after launch (caller settles)."""
    spec = APPS[app]
    run("xcrun", "simctl", "terminate", UDID, spec["bundle"])
    env = dict(os.environ, **{spec["page_env"]: key, spec["theme_env"]: spec["theme_val"][theme]})
    subprocess.run(["xcrun", "simctl", "launch", "--terminate-running-process", UDID, spec["bundle"]],
                   env=env, capture_output=True, text=True)


def shoot_static(app, key, theme):
    launch(app, key, theme)
    time.sleep(APPS[app]["settle"])
    out = os.path.join(CAP, f"{app}_{theme}", f"{key}.png")
    run("xcrun", "simctl", "io", UDID, "screenshot", "--type=png", out)
    return out


def shoot_animated(app, key, theme, record_secs):
    """Launch, settle, record a short mp4, convert to a paletted GIF (downscaled for the README)."""
    launch(app, key, theme)
    time.sleep(APPS[app]["settle"])
    mp4 = os.path.join(CAP, f"{app}_{theme}", f"{key}.mp4")
    gif = os.path.join(CAP, f"{app}_{theme}", f"{key}.gif")
    proc = subprocess.Popen(["xcrun", "simctl", "io", UDID, "recordVideo", "--codec=h264", "--force", mp4],
                            stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    time.sleep(record_secs)
    proc.send_signal(signal.SIGINT)  # simctl finalizes the mp4 on SIGINT
    try:
        proc.wait(timeout=20)
    except subprocess.TimeoutExpired:
        proc.kill()
    # mp4 -> paletted gif (~12fps, width 400) for crisp inline README playback.
    run("ffmpeg", "-y", "-i", mp4,
        "-vf", "fps=12,scale=400:-1:flags=lanczos,split[s0][s1];[s0]palettegen[p];[s1][p]paletteuse",
        gif)
    return gif


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--apps", default="cpp,maui")
    ap.add_argument("--themes", default="light,dark")
    ap.add_argument("--mode", choices=["static", "animated", "both"], default="both")
    ap.add_argument("--only", default="")
    ap.add_argument("--record-secs", type=float, default=4.0)
    args = ap.parse_args()

    apps = [a for a in args.apps.split(",") if a in APPS]
    themes = [t for t in args.themes.split(",") if t in ("light", "dark")]
    keys = load_keys()
    if args.only:
        want = set(args.only.split(","))
        keys = [k for k in keys if k in want]
    for app in apps:
        for theme in themes:
            os.makedirs(os.path.join(CAP, f"{app}_{theme}"), exist_ok=True)

    static_keys = [k for k in keys if args.mode != "animated" or k not in ANIMATED]
    if args.mode == "animated":
        static_keys = []
    elif args.mode == "static":
        static_keys = keys
    else:  # both -> still-shoot every key (animated pages also get a still frame)
        static_keys = keys
    anim_keys = [k for k in keys if k in ANIMATED] if args.mode in ("animated", "both") else []

    total = len(apps) * len(themes) * (len(static_keys) + len(anim_keys))
    done = 0
    for app in apps:
        for theme in themes:
            for key in static_keys:
                shoot_static(app, key, theme)
                done += 1
                print(f"[{done}/{total}] static {app} {theme} {key}", flush=True)
            for key in anim_keys:
                shoot_animated(app, key, theme, args.record_secs)
                done += 1
                print(f"[{done}/{total}] gif    {app} {theme} {key}", flush=True)
    print("CAPTURE_ALL_DONE", flush=True)


if __name__ == "__main__":
    main()
