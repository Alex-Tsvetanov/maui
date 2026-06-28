#!/usr/bin/env python3
"""Capture the example apps (builder vs compile-time-XAML twins) on iOS, light + dark.

Feeds the 3-column comparison in docs/comparison/EXAMPLES_XAML.md:
    MAUI | C++ (builder) | C++ & XAML        horizontally
    light / dark                              vertically

The "C++" column is the hand-written maui::ui builder example (e.g. examples/counter); the "C++ & XAML"
column is its #embed/build_page twin (examples/counter_xaml). They are proven tree-identical by the
in-tree parity tests; this captures them on a REAL simulator so the compile-time-XAML path is verified
visually, not just structurally. The MAUI column is supplied separately (the examples are not part of the
172-page maui-compare set; see the doc).

Each example app is built by examples/build-ios (MAUI_BACKEND=ios) as dev.maui-cpp.examples.<name>.app.
Theme is the SYSTEM appearance (simctl ui appearance) — the port follows the trait, no per-app env needed.

Usage:  MAUI_SIM_UDID=<udid> python3 tools/parity/capture_examples.py [pair ...]
        default pairs: hello_world counter
Each <pair> P captures app P (C++ column) and P_xaml (C++ & XAML column).
"""
import os
import subprocess
import sys
import time

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.normpath(os.path.join(HERE, "..", ".."))
BUILD = os.path.join(ROOT, "examples", "build-ios")
OUT = os.path.join(ROOT, "docs", "comparison", "examples_ios")
UDID = os.environ.get("MAUI_SIM_UDID", "booted")
SETTLE = float(os.environ.get("MAUI_SETTLE", "2.5"))


def run(*a, **kw):
    return subprocess.run(a, capture_output=True, text=True, **kw)


def appearance(theme):
    run("xcrun", "simctl", "ui", UDID, "appearance", theme)


def app_dir(name):
    # maui_add_app emits <build>/<name>/<name>.app on iOS.
    return os.path.join(BUILD, name, f"{name}.app")


def bundle_id(name):
    return f"dev.maui-cpp.examples.{name}"


# --- the MAUI reference column: the maui-compare app (real .NET MAUI), page-selected by MAUI_COMPARE_PAGE,
# themed by MAUI_THEME. Its pages (Pages/<Pascal>Page.cs) mirror the C++ examples 1:1. ---
MAUI_COMPARE_APP = os.path.expanduser(
    "~/maui-compare/bin/Debug/net10.0-ios/iossimulator-arm64/MauiCompare.app")
MAUI_COMPARE_BUNDLE = "com.companyname.mauicompare"


def capture(name, column):
    """Install + launch app `name`, screenshot light & dark into examples_ios/<theme>/<column>.png."""
    appdir = app_dir(name)
    if not os.path.isdir(appdir):
        print(f"  SKIP {name}: {appdir} not built", flush=True)
        return False
    bid = bundle_id(name)
    run("xcrun", "simctl", "install", UDID, appdir)
    for theme in ("light", "dark"):
        appearance(theme)
        run("xcrun", "simctl", "terminate", UDID, bid)
        run("xcrun", "simctl", "launch", "--terminate-running-process", UDID, bid)
        time.sleep(SETTLE)
        dst_dir = os.path.join(OUT, theme)
        os.makedirs(dst_dir, exist_ok=True)
        out = os.path.join(dst_dir, f"{column}.png")
        run("xcrun", "simctl", "io", UDID, "screenshot", "--type=png", out)
        run("sips", "-Z", "1400", out)
        print(f"  [{theme}] {name} -> examples_ios/{theme}/{column}.png", flush=True)
    return True


def capture_maui(key, column):
    """Launch the maui-compare app on page `key`, screenshot light & dark into examples_ios/<theme>/<column>.png.
    The page is selected by MAUI_COMPARE_PAGE; theme is forced both system-wide (for native chrome) and via
    MAUI_THEME (the app's UserAppTheme override)."""
    if not os.path.isdir(MAUI_COMPARE_APP):
        print(f"  SKIP MAUI {key}: {MAUI_COMPARE_APP} not built", flush=True)
        return False
    run("xcrun", "simctl", "install", UDID, MAUI_COMPARE_APP)
    for theme in ("light", "dark"):
        appearance(theme)
        run("xcrun", "simctl", "terminate", UDID, MAUI_COMPARE_BUNDLE)
        env = dict(os.environ, SIMCTL_CHILD_MAUI_COMPARE_PAGE=key,
                   SIMCTL_CHILD_MAUI_THEME=("Dark" if theme == "dark" else "Light"))
        subprocess.run(["xcrun", "simctl", "launch", "--terminate-running-process", UDID, MAUI_COMPARE_BUNDLE],
                       env=env, capture_output=True, text=True)
        time.sleep(SETTLE)
        dst_dir = os.path.join(OUT, theme)
        os.makedirs(dst_dir, exist_ok=True)
        out = os.path.join(dst_dir, f"{column}.png")
        run("xcrun", "simctl", "io", UDID, "screenshot", "--type=png", out)
        run("sips", "-Z", "1400", out)
        print(f"  [{theme}] MAUI {key} -> examples_ios/{theme}/{column}.png", flush=True)
    return True


def main():
    pairs = sys.argv[1:] or ["hello_world", "counter"]
    for pair in pairs:
        print(f"== {pair} ==", flush=True)
        capture_maui(pair, f"{pair}__maui")       # MAUI reference column
        capture(pair, f"{pair}__cpp")             # C++ builder column
        capture(f"{pair}_xaml", f"{pair}__xaml")  # C++ compile-time-XAML column
    appearance("light")  # leave the sim in light
    print("CAPTURE_EXAMPLES_DONE", flush=True)


if __name__ == "__main__":
    main()
