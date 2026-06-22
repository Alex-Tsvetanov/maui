#!/usr/bin/env python3
"""Re-capture every C++ gallery page (light+dark) into docs/comparison/cpp_ios_{light,dark}/.

Drives the already-INSTALLED dev.maui-cpp.ios-gallery on the booted iPhone-17 sim, one page per
relaunch (SIMCTL_CHILD_MAUI_SAMPLE_PAGE / MAUI_APPEARANCE), settles, screenshots, downsizes to the
README row height basis (sips -Z 1100). Order = gen_parity_readme.KEYS (README/FIX_ORDER order).
"""
import importlib.util, os, subprocess, sys, time

HERE = os.path.dirname(os.path.abspath(__file__))
CMP = os.path.normpath(os.path.join(HERE, "..", "..", "docs", "comparison"))
spec = importlib.util.spec_from_file_location("g", os.path.join(CMP, "gen_parity_readme.py"))
g = importlib.util.module_from_spec(spec); spec.loader.exec_module(g)
KEYS = g.KEYS
UDID = os.environ.get("MAUI_SIM_UDID", "C4926671-2FA7-428E-B4A4-480692EE742B")
BUNDLE = "dev.maui-cpp.ios-gallery"
SETTLE = float(os.environ.get("MAUI_SETTLE", "3.0"))

def run(*a):
    return subprocess.run(a, capture_output=True, text=True)

total = len(KEYS) * 2
done = 0
for key in KEYS:
    for theme in ("light", "dark"):
        run("xcrun", "simctl", "terminate", UDID, BUNDLE)
        env = dict(os.environ, SIMCTL_CHILD_MAUI_SAMPLE_PAGE=key, SIMCTL_CHILD_MAUI_APPEARANCE=theme)
        subprocess.run(["xcrun", "simctl", "launch", "--terminate-running-process", UDID, BUNDLE],
                       env=env, capture_output=True, text=True)
        time.sleep(SETTLE)
        out = os.path.join(CMP, f"cpp_ios_{theme}", f"{key}.png")
        run("xcrun", "simctl", "io", UDID, "screenshot", "--type=png", out)
        run("sips", "-Z", "1100", out)
        done += 1
        print(f"[{done}/{total}] {key} {theme}", flush=True)
print("CAPTURE_ALL_DONE", flush=True)
