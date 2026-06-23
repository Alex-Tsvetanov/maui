#!/usr/bin/env python3
"""Re-capture every maui-compare (real .NET MAUI) page (light+dark) into
docs/comparison/csharp_ios_{light,dark}/ — the MAUI baseline twin of capture_all_cpp.py.

Drives the already-INSTALLED com.companyname.mauicompare on the booted iPhone-17 sim, one page per
relaunch (SIMCTL_CHILD_MAUI_COMPARE_PAGE selects the page, SIMCTL_CHILD_MAUI_THEME=Dark|Light forces
appearance — App.xaml.cs reads both), settles, screenshots at NATIVE resolution (1206x2622; no
downsize — the existing csharp baseline format; the montage/README normalize by height).

Keys = the basenames already present in docs/comparison/csharp_ios_light/ — i.e. exactly the
maui-compare-supported pages that already have a baseline (avoids the App.xaml.cs "unknown key ->
controls_stack" default silently overwriting a good baseline with the wrong page).

IMPORTANT: the maui-compare app MUST declare UILaunchScreen in Platforms/iOS/Info.plist, else iOS
runs it in legacy compatibility mode (letterboxed, laid out ~320pt and upscaled ~1.25x) — which made
every MAUI control render ~1.25x larger than the native-rendering C++ gallery. This script does NOT
(re)install; install a UILaunchScreen-enabled build first, then run this.
"""
import glob
import os
import subprocess
import time

HERE = os.path.dirname(os.path.abspath(__file__))
CMP = os.path.normpath(os.path.join(HERE, "..", "..", "docs", "comparison"))
UDID = os.environ.get("MAUI_SIM_UDID", "C4926671-2FA7-428E-B4A4-480692EE742B")
BUNDLE = "com.companyname.mauicompare"
SETTLE = float(os.environ.get("MAUI_SETTLE", "4.5"))

keys = sorted(
    os.path.splitext(os.path.basename(p))[0]
    for p in glob.glob(os.path.join(CMP, "csharp_ios_light", "*.png"))
)


def run(*a):
    return subprocess.run(a, capture_output=True, text=True)


total = len(keys) * 2
done = 0
for key in keys:
    for theme in ("light", "dark"):
        run("xcrun", "simctl", "terminate", UDID, BUNDLE)
        env = dict(
            os.environ,
            SIMCTL_CHILD_MAUI_COMPARE_PAGE=key,
            SIMCTL_CHILD_MAUI_THEME=("Dark" if theme == "dark" else "Light"),
        )
        subprocess.run(
            ["xcrun", "simctl", "launch", "--terminate-running-process", UDID, BUNDLE],
            env=env, capture_output=True, text=True,
        )
        time.sleep(SETTLE)
        out = os.path.join(CMP, f"csharp_ios_{theme}", f"{key}.png")
        run("xcrun", "simctl", "io", UDID, "screenshot", "--type=png", out)
        done += 1
        print(f"[{done}/{total}] {key} {theme}", flush=True)
print("CSHARP_CAPTURE_ALL_DONE", flush=True)
