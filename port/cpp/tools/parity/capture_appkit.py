#!/usr/bin/env python3
"""Capture the AppKit (native macOS NSView) gallery — builder vs XAML — for COMPLETENESS parity.

AppKit can't pixel-match MAUI/Catalyst (different UI framework — NSViews vs UIKit), but the requirement
is: every element specified in the code/XAML must be PRESENT in the render, and the C++ builder and
C++ & XAML columns must NOT differ from each other. This captures both apple-backend galleries
(examples/build-apple, plain executables opening a centered ~480x720 NSWindow) into
docs/comparison/maccatalyst/appkit_{cpp,xaml}/<theme>/ plus a side-by-side montage under
montages_appkit/. Sibling of capture_maccatalyst.py; kept separate so the Catalyst harness is untouched.

Usage: python3 tools/parity/capture_appkit.py [--theme light|dark] [key ...]
Prereqs: examples/build-apple built (gallery + gallery_xaml).
"""
import os, sys, subprocess, time, glob

ROOT = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", ".."))
AP = os.path.join(ROOT, "examples", "build-apple")
OUT = os.path.join(ROOT, "docs", "comparison", "maccatalyst")
HELV = "/System/Library/Fonts/Helvetica.ttc"
# Builder gallery lacks these 6 page keys -> falls back to value_controls; capture only the XAML column.
NON_BUILDER = {"carousel_view", "frame", "grid_definitions", "grid_layout",
               "horizontal_stack_layout", "vertical_stack_layout"}

APPS = {
    "appkit_cpp": {"bin": f"{AP}/gallery/gallery", "proc": "gallery"},
    "appkit_xaml": {"bin": f"{AP}/gallery_xaml/gallery_xaml", "proc": "gallery_xaml"},
}
DEFAULT_RECT = "516,63,480,752"  # apple host centers a 480x720 window; queried per-launch, this is fallback


def osa(s):
    return subprocess.run(["osascript", "-e", s], capture_output=True, text=True).stdout.strip()


def window_rect(pid):
    name = osa(f'tell application "System Events" to get name of (first process whose unix id is {pid})')
    if name:
        r = osa('tell application "System Events" to tell process "%s"\n'
                "set p to position of window 1\nset s to size of window 1\n"
                'return ((item 1 of p) as string) & "," & ((item 2 of p) as string) & "," & '
                '((item 1 of s) as string) & "," & ((item 2 of s) as string)\nend tell' % name)
        if r.count(",") == 3:
            return r
    return DEFAULT_RECT


def shot(app_key, key, theme, out_png):
    a = APPS[app_key]
    if not os.path.exists(a["bin"]):
        print(f"  ! missing {a['bin']}")
        return False
    env = dict(os.environ, MAUI_SAMPLE_PAGE=key, MAUI_APPEARANCE=theme)
    # Run from the binary's own directory: the apple plain-exe gallery resolves from_file() resource paths
    # (dotnet_bot.png, oasis.jpg, ...) against the CWD, and maui_add_app copies the assets next to the binary.
    proc = subprocess.Popen([a["bin"]], env=env, cwd=os.path.dirname(a["bin"]),
                            stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    try:
        time.sleep(3.5)
        osa(f'tell application "System Events" to set frontmost of (first process whose unix id is {proc.pid}) to true')
        time.sleep(1.2)
        rect = window_rect(proc.pid)
        os.makedirs(os.path.dirname(out_png), exist_ok=True)
        subprocess.run(["killall", "UserNotificationCenter"], stderr=subprocess.DEVNULL)
        rc = subprocess.run(["screencapture", "-x", f"-R{rect}", out_png]).returncode
        ok = rc == 0 and os.path.exists(out_png)
        print(f"  {app_key:12} {key:28} {'ok' if ok else 'FAIL'} ({rect})")
        return ok
    finally:
        proc.kill()
        try:
            proc.wait(timeout=3)
        except Exception:
            pass
        time.sleep(0.3)


def montage(key, theme):
    cols = []
    for ak, label in (("appkit_cpp", "AppKit C++"), ("appkit_xaml", "AppKit C++ & XAML")):
        p = os.path.join(OUT, ak, theme, f"{key}.png")
        if os.path.exists(p):
            cols += ["-label", label, p]
    if len(cols) < 6:  # need both columns (3 tokens each)
        return
    m = os.path.join(OUT, "montages_appkit", theme, f"{key}.png")
    os.makedirs(os.path.dirname(m), exist_ok=True)
    subprocess.run(["magick", "montage", *cols, "-tile", "2x1", "-geometry", "480x740+8+8",
                    "-background", "white", "-font", HELV, "-pointsize", "20", m])
    print(f"  montage {key}")


def main():
    theme = "light"
    keys = []
    args = sys.argv[1:]
    i = 0
    while i < len(args):
        if args[i] == "--theme":
            theme = args[i + 1]; i += 2
        else:
            keys.append(args[i]); i += 1
    if not keys:
        keys = sorted(os.path.splitext(os.path.basename(f))[0]
                      for f in glob.glob(os.path.join(ROOT, "examples", "gallery_xaml", "Views", "*.xaml")))
    print(f"appkit capture theme={theme} pages={len(keys)}")
    for key in keys:
        shot("appkit_xaml", key, theme, os.path.join(OUT, "appkit_xaml", theme, f"{key}.png"))
        if key not in NON_BUILDER:
            shot("appkit_cpp", key, theme, os.path.join(OUT, "appkit_cpp", theme, f"{key}.png"))
        montage(key, theme)
    for a in APPS.values():
        subprocess.run(["pkill", "-f", a["bin"]], stderr=subprocess.DEVNULL)
    print("done.")


if __name__ == "__main__":
    main()
