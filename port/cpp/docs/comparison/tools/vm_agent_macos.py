#!/usr/bin/env python3
"""macOS guest agent for the E2E visual-comparison runner.

This file is copied to a test VM's staging dir and invoked one subcommand at a time by the host
orchestrator (run_comparison.py) over SSH. It wraps the macOS primitives the runner needs:
resolution, staging cleanup, app launch, window lookup, interaction, capture, teardown.

It is the per-OS seam: a future Windows/Linux target is a sibling vm_agent_<os>.py exposing the
SAME subcommands, so the host orchestrator never changes.

Every external tool is invoked by ABSOLUTE PATH — a non-interactive SSH shell has a minimal PATH
(no /opt/homebrew/bin), so bare `cliclick`/`displayplacer` would fail. Defaults below match a
stock Homebrew install; override per-tool via env (MAUI_E2E_CLICLICK, MAUI_E2E_DISPLAYPLACER, ...).

Each subcommand prints ONE JSON line to stdout: {"ok": bool, ...}. The host parses the last line.

Deps on the VM: `brew install cliclick displayplacer`. pyobjc is OPTIONAL — with it, window capture uses
the tight per-window `screencapture -l <id>`; without it, the window rect comes from AppleScript (System
Events) and capture uses `-R <x,y,w,h>`. Scroll uses ctypes→CoreGraphics (no pyobjc). See README_e2e.md.
"""
from __future__ import annotations

import argparse
import json
import os
import shutil
import subprocess
import sys
import time

# Absolute tool paths (Homebrew/system defaults; env-overridable).
CLICLICK = os.environ.get("MAUI_E2E_CLICLICK", "/opt/homebrew/bin/cliclick")
DISPLAYPLACER = os.environ.get("MAUI_E2E_DISPLAYPLACER", "/opt/homebrew/bin/displayplacer")
SCREENCAPTURE = os.environ.get("MAUI_E2E_SCREENCAPTURE", "/usr/sbin/screencapture")
OPEN = os.environ.get("MAUI_E2E_OPEN", "/usr/bin/open")


def _emit(**kw) -> int:
    kw.setdefault("ok", True)
    print(json.dumps(kw))
    return 0 if kw["ok"] else 1


def _pgrep(proc: str) -> set[str]:
    return set(subprocess.run(["/usr/bin/pgrep", "-x", proc], capture_output=True, text=True).stdout.split())


def cmd_set_resolution(a) -> int:
    # Find the main display's persistent id, then request the mode. Best-effort: the VM must offer WxH.
    listing = subprocess.run([DISPLAYPLACER, "list"], capture_output=True, text=True).stdout
    disp_id = None
    for line in listing.splitlines():
        line = line.strip()
        if line.lower().startswith("persistent screen id:"):
            disp_id = line.split(":", 1)[1].strip()
            break
    if not disp_id:
        return _emit(ok=False, error="no display id from `displayplacer list`", raw=listing[:400])
    rc = subprocess.run([DISPLAYPLACER, f"id:{disp_id} res:{a.width}x{a.height}"],
                        capture_output=True, text=True)
    return _emit(ok=rc.returncode == 0, display=disp_id, width=a.width, height=a.height,
                 stderr=rc.stderr.strip()[:400])


def cmd_clean(a) -> int:
    shutil.rmtree(a.dir, ignore_errors=True)
    os.makedirs(a.dir, exist_ok=True)
    return _emit(dir=a.dir)


def cmd_launch(a) -> int:
    # `open -g -n --env ...`: background (no focus theft), new instance. Find the new pid by diffing
    # pgrep before/after (mirrors e2e.py::_launch_background).
    before = _pgrep(a.proc)
    cmd = [OPEN, "-g", "-n"]
    for kv in a.env or []:
        cmd += ["--env", kv]
    cmd.append(a.bundle)
    rc = subprocess.run(cmd, capture_output=True, text=True)
    if rc.returncode != 0:
        return _emit(ok=False, error="open failed", stderr=rc.stderr.strip()[:400])
    for _ in range(40):
        fresh = _pgrep(a.proc) - before
        if fresh:
            return _emit(pid=int(next(iter(fresh))))
        time.sleep(0.25)
    return _emit(ok=False, error="process did not register after launch")


def _osa(script: str) -> str:
    return subprocess.run(["/usr/bin/osascript", "-e", script], capture_output=True, text=True).stdout.strip()


def _window_info_quartz(pid: int):
    """Preferred: the pid's largest normal-layer window as (cgwindowid, [x,y,w,h]), or None.
    Needs pyobjc-framework-Quartz — OPTIONAL; ImportError means fall back to AppleScript."""
    import Quartz  # optional dep — see _window_rect_applescript for the no-pyobjc path
    info = Quartz.CGWindowListCopyWindowInfo(
        Quartz.kCGWindowListOptionOnScreenOnly, Quartz.kCGNullWindowID)
    cands = [w for w in (info or [])
             if w.get("kCGWindowOwnerPID") == pid and w.get("kCGWindowLayer") == 0]
    if not cands:
        return None

    def area(w):
        b = w.get("kCGWindowBounds", {})
        return b.get("Width", 0) * b.get("Height", 0)

    w = max(cands, key=area)
    b = w.get("kCGWindowBounds", {})
    return (int(w["kCGWindowNumber"]),
            [int(b.get("X", 0)), int(b.get("Y", 0)), int(b.get("Width", 0)), int(b.get("Height", 0))])


def _window_rect_applescript(proc: str):
    """Fallback (NO pyobjc): the process's window 1 rect [x,y,w,h] via System Events, or None. Needs the
    Accessibility grant (already required for cliclick). Mirrors e2e.py's fixed-rect fallback."""
    out = _osa(
        f'tell application "System Events" to tell process "{proc}"\n'
        "set p to position of window 1\nset s to size of window 1\n"
        'return ((item 1 of p) as string) & "," & ((item 2 of p) as string) & "," & '
        '((item 1 of s) as string) & "," & ((item 2 of s) as string)\nend tell')
    parts = out.split(",")
    if len(parts) == 4:
        try:
            return [int(float(p)) for p in parts]
        except ValueError:
            return None
    return None


def cmd_window_id(a) -> int:
    # Preferred pyobjc/Quartz path yields a CGWindowID (tight `-l` capture, no compositing). Falls back to
    # an AppleScript window rect (`-R` region capture) when pyobjc is absent — poll for the window to show.
    for _ in range(a.retries):
        try:
            info = _window_info_quartz(a.pid)
        except ImportError:
            break  # pyobjc missing — skip straight to the AppleScript fallback
        if info is not None:
            win, bounds = info
            return _emit(id=win, bounds=bounds)
        time.sleep(a.delay)
    if a.proc:
        for _ in range(a.retries):
            rect = _window_rect_applescript(a.proc)
            if rect and rect[2] > 0 and rect[3] > 0:
                return _emit(rect=",".join(map(str, rect)), bounds=rect)  # no id -> caller uses -R
            time.sleep(a.delay)
    return _emit(ok=False, error="no window found (Quartz absent or empty; AppleScript found none)")


def cmd_present(a) -> int:
    """Bring the process's window to the FRONT (key window → colored, not greyed, traffic lights) and set it
    to an EXPLICIT position+size so every column captures at the SAME rect. Returns the ACTUAL resulting rect.

    Two empirically-verified facts (contrary to the README's earlier note that Catalyst windows can't be
    externally sized): (1) `set position`/`set size` of window 1 DO take on Mac Catalyst apps — both the C++
    gallery and the C# MauiReference obey them; height clamps to the screen's max usable height, which is the
    SAME for every app, so a common target yields an identical rect. (2) The traffic lights only draw colored
    while the window is key AT CAPTURE TIME, and any System Events call between this present and the shot (e.g.
    a separate window-id query) steals key focus back — so the caller must `shot -R <this rect>` IMMEDIATELY,
    with no intervening agent call. `--zoom` is accepted for back-compat (ignored; explicit sizing supersedes)."""
    x, y, w, h = a.x, a.y, a.w, a.h
    out = _osa(
        'with timeout of 10 seconds\n'
        f'tell application "System Events" to tell process "{a.proc}"\n'
        '  set frontmost to true\n'
        '  try\n'
        f'    set position of window 1 to {{{x}, {y}}}\n'
        f'    set size of window 1 to {{{w}, {h}}}\n'
        '  end try\n'
        '  set p to position of window 1\n  set s to size of window 1\n'
        '  return (item 1 of p as string) & "," & (item 2 of p as string) & "," & '
        '(item 1 of s as string) & "," & (item 2 of s as string)\n'
        'end tell\nend timeout')
    parts = out.split(",")
    if len(parts) == 4:
        try:
            rect = [int(float(p)) for p in parts]
            return _emit(ok=True, proc=a.proc, rect=",".join(map(str, rect)), bounds=rect)
        except ValueError:
            pass
    return _emit(ok=False, proc=a.proc, error=out[:200] or "no rect from System Events")


def cmd_click(a) -> int:
    rc = subprocess.run([CLICLICK, f"c:{a.x},{a.y}"], capture_output=True, text=True)
    return _emit(ok=rc.returncode == 0, stderr=rc.stderr.strip()[:400])


def cmd_type(a) -> int:
    rc = subprocess.run([CLICLICK, f"t:{a.text}"], capture_output=True, text=True)
    return _emit(ok=rc.returncode == 0, stderr=rc.stderr.strip()[:400])


def cmd_scroll(a) -> int:
    # cliclick has no scroll. Move the pointer to the target (cliclick), then post a pixel scroll-wheel
    # event via CoreGraphics through ctypes — NO pyobjc needed. Needs Accessibility (same as cliclick).
    import ctypes
    subprocess.run([CLICLICK, f"m:{a.x},{a.y}"], capture_output=True, text=True)
    cg = ctypes.CDLL("/System/Library/Frameworks/ApplicationServices.framework/ApplicationServices")
    cg.CGEventCreateScrollWheelEvent.restype = ctypes.c_void_p
    cg.CGEventCreateScrollWheelEvent.argtypes = [ctypes.c_void_p, ctypes.c_uint32, ctypes.c_uint32, ctypes.c_int32]
    cg.CGEventPost.argtypes = [ctypes.c_uint32, ctypes.c_void_p]
    ev = cg.CGEventCreateScrollWheelEvent(None, 1, 1, int(a.dy))  # units=1 kCGScrollEventUnitPixel, 1 wheel
    if not ev:
        return _emit(ok=False, error="CGEventCreateScrollWheelEvent returned null")
    cg.CGEventPost(0, ev)  # tap = 0 kCGHIDEventTap
    cf = ctypes.CDLL("/System/Library/Frameworks/CoreFoundation.framework/CoreFoundation")
    cf.CFRelease.argtypes = [ctypes.c_void_p]
    cf.CFRelease(ev)
    return _emit(x=a.x, y=a.y, dy=a.dy)


def cmd_shot(a) -> int:
    os.makedirs(os.path.dirname(a.out) or ".", exist_ok=True)
    if a.window:
        # -x no sound, -o omit shadow padding (tight crop), -l capture that window's own backing store
        # (no other window can composite in; needs no focus). Mirrors e2e.py::_maccat_shot. Needs pyobjc.
        cmd = [SCREENCAPTURE, "-x", "-o", "-l", str(a.window), a.out]
    elif a.rect:
        cmd = [SCREENCAPTURE, "-x", f"-R{a.rect}", a.out]  # region x,y,w,h (no-pyobjc AppleScript path)
    else:
        cmd = [SCREENCAPTURE, "-x", a.out]  # last resort: whole main display
    rc = subprocess.run(cmd, capture_output=True, text=True)
    ok = rc.returncode == 0 and os.path.isfile(a.out)
    return _emit(ok=ok, out=a.out, window=a.window, rect=a.rect, stderr=rc.stderr.strip()[:400])


def cmd_stop(a) -> int:
    subprocess.run(["/bin/kill", "-9", str(a.pid)], capture_output=True)
    return _emit(pid=a.pid)


def main(argv=None) -> int:
    p = argparse.ArgumentParser(description="macOS guest agent for the E2E comparison runner")
    sub = p.add_subparsers(dest="cmd", required=True)

    s = sub.add_parser("set-resolution"); s.add_argument("width", type=int); s.add_argument("height", type=int)
    s.set_defaults(fn=cmd_set_resolution)

    s = sub.add_parser("clean"); s.add_argument("dir"); s.set_defaults(fn=cmd_clean)

    s = sub.add_parser("launch")
    s.add_argument("--bundle", required=True); s.add_argument("--proc", required=True)
    s.add_argument("--env", action="append", help="K=V, repeatable"); s.set_defaults(fn=cmd_launch)

    s = sub.add_parser("window-id"); s.add_argument("pid", type=int)
    s.add_argument("--proc", default="", help="process name (for the no-pyobjc AppleScript fallback)")
    s.add_argument("--retries", type=int, default=15); s.add_argument("--delay", type=float, default=0.3)
    s.set_defaults(fn=cmd_window_id)

    s = sub.add_parser("present"); s.add_argument("--proc", required=True)
    s.add_argument("--x", type=int, default=128); s.add_argument("--y", type=int, default=30)
    s.add_argument("--w", type=int, default=1024); s.add_argument("--h", type=int, default=800)
    s.add_argument("--zoom", action="store_true", help="(ignored; explicit --x/--y/--w/--h supersede)")
    s.set_defaults(fn=cmd_present)

    s = sub.add_parser("click"); s.add_argument("x", type=int); s.add_argument("y", type=int)
    s.set_defaults(fn=cmd_click)

    s = sub.add_parser("type"); s.add_argument("text"); s.set_defaults(fn=cmd_type)

    s = sub.add_parser("scroll"); s.add_argument("x", type=int); s.add_argument("y", type=int)
    s.add_argument("dy", type=int); s.set_defaults(fn=cmd_scroll)

    s = sub.add_parser("shot"); s.add_argument("out"); s.add_argument("--window", type=int, default=0)
    s.add_argument("--rect", default="", help="x,y,w,h region capture (no-pyobjc fallback)")
    s.set_defaults(fn=cmd_shot)

    s = sub.add_parser("stop"); s.add_argument("pid", type=int); s.set_defaults(fn=cmd_stop)

    a = p.parse_args(argv)
    try:
        return a.fn(a)
    except Exception as e:  # never crash the SSH call without a parseable result
        return _emit(ok=False, error=f"{type(e).__name__}: {e}")


if __name__ == "__main__":
    sys.exit(main())
