#!/usr/bin/env python3
"""Capture the Mac Catalyst 3-way parity surface: MAUI | C++ builder | C++ & XAML.

For each gallery page key, launches each app's macabi .app DIRECTLY with the page/theme env, brings it
frontmost (screencapture -l window capture is broken on this macOS — ScreenCaptureKit restriction — so we
must region-capture the frontmost window), grabs its window rect, and saves a PNG into the canonical
layout captures/maccatalyst/<framework>/<key>_<theme>.png (framework in maui/cpp/xaml), the form
build_comparison_json.py + gen_readme.py read.

NOTE: this steals window focus ~once per (app,page) while it runs — it's the one disruptive phase; judging
and fixing afterward are GUI-free. Re-run with explicit keys to re-capture only pages you changed.

Prereqs: the three apps built (examples/build-maccatalyst + ~/maui-compare maccatalyst) and ad-hoc signed.

Usage:
  python3 tools/parity/capture_maccatalyst.py [--theme light|dark] [--dry-run] [key ...]
"""
import os, sys, subprocess, time, glob

import comparison_paths as cp

ROOT = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", ".."))
MC = os.path.join(ROOT, "examples", "build-maccatalyst")
HOME = os.path.expanduser("~")
PLATFORM = "maccatalyst"

# Twin keys with NO hand-written page in the builder gallery (gallery_host.hpp MAUI_GALLERY_PAGES) -> the
# builder gallery falls back to value_controls for these, so the "cpp" column is meaningless. Skip it (the
# XAML twin still renders them). All 6 are also in the no-MAUI set, so these become XAML-only captures.
NON_BUILDER = {"carousel_view", "frame", "grid_definitions", "grid_layout",
               "horizontal_stack_layout", "vertical_stack_layout"}

APPS = {
    "maui": {
        "bin": f"{HOME}/maui-compare/bin/Debug/net10.0-maccatalyst/maccatalyst-arm64/MauiCompare.app/Contents/MacOS/MauiCompare",
        "proc": "MauiCompare",
        "rect": "221,33,1024,768",
        "env": lambda key, theme: {"MAUI_COMPARE_PAGE": key, "MAUI_THEME": "Light" if theme == "light" else "Dark"},
    },
    "cpp": {
        "bin": f"{MC}/gallery/gallery.app/Contents/MacOS/gallery",
        "proc": "gallery",
        "rect": "244,59,1024,768",
        "env": lambda key, theme: {"MAUI_SAMPLE_PAGE": key, "MAUI_APPEARANCE": theme},
    },
    "xaml": {
        "bin": f"{MC}/gallery_xaml/gallery_xaml.app/Contents/MacOS/gallery_xaml",
        "proc": "gallery_xaml",
        "rect": "244,59,1024,768",
        "env": lambda key, theme: {"MAUI_SAMPLE_PAGE": key, "MAUI_APPEARANCE": theme},
    },
}


def osa(script):
    return subprocess.run(["osascript", "-e", script], capture_output=True, text=True).stdout.strip()


def window_rect(pid, default):
    # name-by-pid, then tell-process-by-name (the only form that coerces point/size reliably); fall back.
    name = osa(f'tell application "System Events" to get name of (first process whose unix id is {pid})')
    if name:
        r = osa(
            'tell application "System Events" to tell process "%s"\n'
            "set p to position of window 1\nset s to size of window 1\n"
            'return ((item 1 of p) as string) & "," & ((item 2 of p) as string) & "," & '
            '((item 1 of s) as string) & "," & ((item 2 of s) as string)\nend tell' % name
        )
        if r.count(",") == 3:
            return r
    return default


def shot(app_key, key, theme, out_png):
    a = APPS[app_key]
    if not os.path.exists(a["bin"]):
        print(f"  ! missing binary for {app_key}: {a['bin']}")
        return False
    env = dict(os.environ, **a["env"](key, theme))
    proc = subprocess.Popen([a["bin"]], env=env, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    try:
        time.sleep(3.5)
        osa(f'tell application "System Events" to set frontmost of (first process whose unix id is {proc.pid}) to true')
        time.sleep(1.2)
        rect = window_rect(proc.pid, a["rect"])
        # Clear any crash dialog before grabbing the region: ReportCrash generates it, UserNotificationCenter
        # OWNS the visible window (killing ReportCrash alone doesn't dismiss it). SIGKILL teardown prevents new
        # ones from our apps, but a page that crashes on load can still raise one.
        subprocess.run(["killall", "-9", "ReportCrash"], stderr=subprocess.DEVNULL)
        subprocess.run(["killall", "UserNotificationCenter"], stderr=subprocess.DEVNULL)
        os.makedirs(os.path.dirname(out_png), exist_ok=True)
        rc = subprocess.run(["screencapture", "-x", f"-R{rect}", out_png]).returncode
        ok = rc == 0 and os.path.exists(out_png)
        print(f"  {app_key:5} {key:28} {'ok' if ok else 'FAIL'} ({rect})")
        return ok
    finally:
        proc.kill()  # SIGKILL, NOT SIGTERM — SIGTERM makes the .NET MAUI app crash-report (lingering dialog)
        try:
            proc.wait(timeout=3)
        except Exception:
            pass
        time.sleep(0.4)


def main():
    args = sys.argv[1:]
    theme = "light"
    dry_run = False
    keys = []
    i = 0
    while i < len(args):
        if args[i] == "--theme":
            theme = args[i + 1]; i += 2
        elif args[i] == "--dry-run":
            dry_run = True; i += 1
        else:
            keys.append(args[i]); i += 1

    # The keys that have a hand-written XAML twin (examples/gallery_xaml/Views/*.xaml) — the ONLY keys the
    # C++ & XAML column can render. The MAUI + C++ (builder) columns cover the full 172 (the builder gallery
    # renders every MAUI_GALLERY_PAGES key, exactly as the iOS capture does).
    xaml_twins = set(os.path.splitext(os.path.basename(f))[0]
                     for f in glob.glob(os.path.join(ROOT, "examples", "gallery_xaml", "Views", "*.xaml")))
    builtins = {"controls_stack", "alignment", "shapes", "border", "collectionview", "fonts", "grid", "gradient"}
    def snake(p):
        import re
        return re.sub(r"([a-z0-9])([A-Z])", r"\1_\2", p).lower()
    maui_keys = builtins | {snake(os.path.basename(f)[:-7])  # strip "Page.cs"
                            for f in glob.glob(f"{HOME}/maui-compare/Pages/*Page.cs")}
    if not keys:
        # The full 172-example key list (the same page_keys.txt the iOS capture uses) so the macOS board
        # mirrors iOS row-for-row — the C++ mirror is cross-platform, so every iOS page gets a macOS render.
        with open(os.path.join(ROOT, "tools", "parity", "page_keys.txt"), encoding="utf-8") as kf:
            keys = [line.strip() for line in kf if line.strip()]

    print(f"theme={theme}  pages={len(keys)}  (dry_run={dry_run})")

    # DRY-RUN: print the canonical output path each (app, key) WOULD write, no GUI/capture. Applies the
    # same maui_keys / NON_BUILDER / xaml_twins gating the real run does, so it shows exactly what lands.
    if dry_run:
        for key in keys:
            for app_key in ("maui", "cpp", "xaml"):
                if app_key == "maui" and key not in maui_keys:
                    continue
                if app_key == "cpp" and key in NON_BUILDER:
                    continue
                if app_key == "xaml" and key not in xaml_twins:
                    continue
                print(cp.rel_capture(PLATFORM, app_key, key, theme, "png"))
        print("DRY_RUN_DONE", flush=True)
        return

    # Suppress the macOS crash-reporter dialog for the run: SIGTERM'ing the .NET MAUI app makes it
    # crash-report, and that dialog lingers center-screen polluting later region captures. Save/restore.
    prev_dialog = subprocess.run(["defaults", "read", "com.apple.CrashReporter", "DialogType"],
                                 capture_output=True, text=True).stdout.strip()
    subprocess.run(["defaults", "write", "com.apple.CrashReporter", "DialogType", "none"])
    subprocess.run(["killall", "-9", "ReportCrash"], stderr=subprocess.DEVNULL)  # clear any existing dialogs

    for key in keys:
        if key in NON_BUILDER:  # drop any stale builder fallback (value_controls) capture for these
            stale = cp.capture_path(PLATFORM, "cpp", key, theme, "png")
            if os.path.exists(stale):
                os.remove(stale)
        for app_key in ("maui", "cpp", "xaml"):
            if app_key == "maui" and key not in maui_keys:
                continue  # no MAUI counterpart -> don't capture MAUI's ControlsStack fallback
            if app_key == "cpp" and key in NON_BUILDER:
                continue  # builder gallery has no page for this key -> would fall back to value_controls
            if app_key == "xaml" and key not in xaml_twins:
                continue  # C++ & XAML column only exists for the hand-written XAML twins
            shot(app_key, key, theme, cp.capture_path(PLATFORM, app_key, key, theme, "png"))
    # cleanup
    for a in APPS.values():
        subprocess.run(["pkill", "-f", a["bin"]], stderr=subprocess.DEVNULL)
    if prev_dialog:  # restore the user's crash-reporter setting
        subprocess.run(["defaults", "write", "com.apple.CrashReporter", "DialogType", prev_dialog])
    else:
        subprocess.run(["defaults", "delete", "com.apple.CrashReporter", "DialogType"], stderr=subprocess.DEVNULL)
    print("done.")


if __name__ == "__main__":
    main()
