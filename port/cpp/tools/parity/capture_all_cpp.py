#!/usr/bin/env python3
"""Re-capture every C++ gallery page (light+dark) into docs/comparison/captures/ios/cpp/.

Drives the already-INSTALLED dev.maui-cpp.ios-gallery on the booted iPhone-17 sim, one page per
relaunch (SIMCTL_CHILD_MAUI_SAMPLE_PAGE / MAUI_APPEARANCE), settles, screenshots at NATIVE resolution
into the canonical layout captures/ios/cpp/<key>_<theme>.png (the form build_comparison_json.py +
gen_readme.py read). This is the single-app C++-only twin of capture_all.py (`--apps cpp`); prefer
capture_all.py when capturing MAUI + C++ together on one sim. Keys come from page_keys.txt.

Usage:  python3 capture_all_cpp.py [--only k1,k2] [--dry-run]
"""
import argparse
import os
import shutil
import subprocess
import tempfile
import time

import comparison_paths as cp

PLATFORM, FRAMEWORK = "ios", "cpp"
UDID = os.environ.get("MAUI_SIM_UDID", "C4926671-2FA7-428E-B4A4-480692EE742B")
BUNDLE = "dev.maui-cpp.ios-gallery"
SETTLE = float(os.environ.get("MAUI_SETTLE", "3.0"))


def run(*a):
    return subprocess.run(a, capture_output=True, text=True)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--only", default="", help="comma-separated keys to capture (default: all)")
    ap.add_argument("--dry-run", action="store_true",
                    help="print the output path for each (key,theme) WITHOUT booting the sim, then exit")
    args = ap.parse_args()

    keys = cp.load_keys()
    if args.only:
        want = set(k.strip() for k in args.only.split(",") if k.strip())
        keys = [k for k in keys if k in want]

    if args.dry_run:
        for key in keys:
            for theme in ("light", "dark"):
                print(cp.rel_capture(PLATFORM, FRAMEWORK, key, theme, "png"))
        print("DRY_RUN_DONE", flush=True)
        return

    total, done = len(keys) * 2, 0
    for key in keys:
        for theme in ("light", "dark"):
            run("xcrun", "simctl", "terminate", UDID, BUNDLE)
            env = dict(os.environ, SIMCTL_CHILD_MAUI_SAMPLE_PAGE=key, SIMCTL_CHILD_MAUI_APPEARANCE=theme)
            subprocess.run(["xcrun", "simctl", "launch", "--terminate-running-process", UDID, BUNDLE],
                           env=env, capture_output=True, text=True)
            time.sleep(SETTLE)
            out = cp.capture_path(PLATFORM, FRAMEWORK, key, theme, "png")
            os.makedirs(os.path.dirname(out), exist_ok=True)
            # `simctl io screenshot` runs as a CoreSimulator process that lacks macOS TCC permission to
            # write under ~/Documents (NSCocoaErrorDomain 513 "Operation not permitted"), so shoot into a
            # temp dir it CAN write and move the file into the repo from THIS process (which has access).
            tmp = os.path.join(tempfile.gettempdir(), f"maui_{PLATFORM}_{FRAMEWORK}_{key}_{theme}.png")
            r = run("xcrun", "simctl", "io", UDID, "screenshot", "--type=png", tmp)
            if os.path.exists(tmp):
                shutil.move(tmp, out)
            else:
                print(f"  ! screenshot failed for {key} {theme}: {r.stderr.strip()[:120]}", flush=True)
            done += 1
            print(f"[{done}/{total}] {key} {theme}", flush=True)
    print("CAPTURE_ALL_DONE", flush=True)


if __name__ == "__main__":
    main()
