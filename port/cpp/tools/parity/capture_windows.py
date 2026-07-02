#!/usr/bin/env python3
"""Capture the Windows (WinUI 3) 3-way parity surface: MAUI | C++ builder | C++ & XAML.

The Windows twin of capture_maccatalyst.py. For each gallery page key, launches each app's exe
DIRECTLY with the page/theme env, waits for the window, brings it to the foreground, grabs its
CLIENT area (no titlebar/chrome — the fair-comparison region; both apps host the page in a
480x800 client), and saves a PNG into the canonical layout
captures/windows/<framework>/<key>_<theme>.png (framework in maui/cpp/xaml), the form
build_comparison_json.py + gen_readme.py read.

NOTE: this steals window focus ~once per (app,page) while it runs — the one disruptive phase;
judging and fixing afterward are GUI-free. Re-run with explicit keys to re-capture only pages you
changed.

Prereqs: pillow (`pip install pillow`); the apps built:
  cpp/xaml — examples/build-windows (gallery / gallery_xaml, unpackaged WinUI 3)
  maui     — %USERPROFILE%\\maui-compare (dotnet build -f net10.0-windows10.0.19041.0)

Usage:
  python tools/parity/capture_windows.py [--theme light|dark] [--framework cpp[,maui,xaml]]
                                         [--dry-run] [key ...]
"""
import ctypes
import ctypes.wintypes as wt
import os
import subprocess
import sys
import time

from PIL import ImageGrab

import comparison_paths as cp

ROOT = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", ".."))
WINBUILD = os.path.join(ROOT, "examples", "build-windows")
HOME = os.path.expanduser("~")
PLATFORM = "windows"

# Twin keys with NO hand-written page in the builder gallery (gallery_host.hpp MAUI_GALLERY_PAGES) ->
# the builder gallery falls back to value_controls, so the "cpp" column is meaningless (the XAML twin
# still renders them). Mirrors capture_maccatalyst.py.
NON_BUILDER = {"carousel_view", "frame", "grid_definitions", "grid_layout",
               "horizontal_stack_layout", "vertical_stack_layout"}

APPS = {
    "maui": {
        "bin": os.path.join(HOME, "maui-compare", "bin", "Debug",
                            "net10.0-windows10.0.19041.0", "win-x64", "MauiCompare.exe"),
        "env": lambda key, theme: {"MAUI_COMPARE_PAGE": key,
                                   "MAUI_THEME": "Light" if theme == "light" else "Dark"},
    },
    "cpp": {
        "bin": os.path.join(WINBUILD, "gallery", "gallery.exe"),
        "env": lambda key, theme: {"MAUI_SAMPLE_PAGE": key, "MAUI_APPEARANCE": theme},
    },
    "xaml": {
        "bin": os.path.join(WINBUILD, "gallery_xaml", "gallery_xaml.exe"),
        "env": lambda key, theme: {"MAUI_SAMPLE_PAGE": key, "MAUI_APPEARANCE": theme},
    },
}

user32 = ctypes.windll.user32
user32.SetProcessDpiAwarenessContext(ctypes.c_void_p(-4))  # PER_MONITOR_AWARE_V2: true pixel rects


def find_main_window(pid: int, timeout: float = 15.0) -> int:
    """The pid's first visible, titled, top-level window (polled until the app shows one)."""
    hwnd_found = wt.HWND(0)

    @ctypes.WINFUNCTYPE(wt.BOOL, wt.HWND, wt.LPARAM)
    def enum_cb(hwnd, _lparam):
        wnd_pid = wt.DWORD(0)
        user32.GetWindowThreadProcessId(hwnd, ctypes.byref(wnd_pid))
        if wnd_pid.value == pid and user32.IsWindowVisible(hwnd) and user32.GetWindowTextLengthW(hwnd) >= 0:
            # skip zero-size helper windows
            rect = wt.RECT()
            user32.GetWindowRect(hwnd, ctypes.byref(rect))
            if rect.right - rect.left > 100 and rect.bottom - rect.top > 100:
                hwnd_found.value = hwnd
                return False
        return True

    deadline = time.time() + timeout
    while time.time() < deadline:
        user32.EnumWindows(enum_cb, 0)
        if hwnd_found.value:
            return hwnd_found.value
        time.sleep(0.25)
    return 0


def client_rect_on_screen(hwnd: int) -> tuple[int, int, int, int]:
    rect = wt.RECT()
    user32.GetClientRect(hwnd, ctypes.byref(rect))
    origin = wt.POINT(0, 0)
    user32.ClientToScreen(hwnd, ctypes.byref(origin))
    return (origin.x, origin.y, origin.x + rect.right, origin.y + rect.bottom)


def shot(app_key: str, key: str, theme: str, out_png: str) -> bool:
    a = APPS[app_key]
    if not os.path.exists(a["bin"]):
        print(f"  ! missing binary for {app_key}: {a['bin']}")
        return False
    env = dict(os.environ, **a["env"](key, theme))
    proc = subprocess.Popen([a["bin"]], env=env, cwd=os.path.dirname(a["bin"]),
                            stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    try:
        hwnd = find_main_window(proc.pid)
        if not hwnd:
            print(f"  {app_key:5} {key:28} FAIL (no window)")
            return False
        time.sleep(2.0)  # first layout + WinUI composition settle
        user32.SetForegroundWindow(hwnd)
        time.sleep(0.6)
        bbox = client_rect_on_screen(hwnd)
        os.makedirs(os.path.dirname(out_png), exist_ok=True)
        ImageGrab.grab(bbox=bbox, all_screens=True).save(out_png)
        ok = os.path.exists(out_png)
        print(f"  {app_key:5} {key:28} {'ok' if ok else 'FAIL'} ({bbox[2]-bbox[0]}x{bbox[3]-bbox[1]})")
        return ok
    finally:
        proc.kill()
        try:
            proc.wait(timeout=3)
        except Exception:
            pass
        time.sleep(0.3)


def main() -> int:
    args = sys.argv[1:]
    theme = "light"
    dry_run = False
    only_fw = None
    keys: list[str] = []
    i = 0
    while i < len(args):
        if args[i] == "--theme":
            theme = args[i + 1]; i += 2
        elif args[i] == "--dry-run":
            dry_run = True; i += 1
        elif args[i] == "--framework":
            only_fw = set(f.strip() for f in args[i + 1].split(",") if f.strip()); i += 2
        else:
            keys.append(args[i]); i += 1

    if not keys:
        keys = cp.load_keys()
    frameworks = [f for f in cp.PLATFORM_FW[PLATFORM] if only_fw is None or f in only_fw]

    failed: list[str] = []
    for key in keys:
        for fw in frameworks:
            if fw == "cpp" and key in NON_BUILDER:
                continue
            out = cp.capture_path(PLATFORM, fw, key, theme)
            if dry_run:
                print(f"  would capture {fw}/{key}_{theme} -> {out}")
                continue
            if not shot(fw, key, theme, out):
                failed.append(f"{fw}/{key}")
    if failed:
        print(f"\n{len(failed)} captures FAILED: {', '.join(failed[:20])}")
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
