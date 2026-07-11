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

Deps on the VM: `brew install cliclick displayplacer` + `pip3 install pyobjc-framework-Quartz`
(pyobjc is only needed for window-id lookup and scroll). See tools/README_e2e.md.
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


def _window_info(pid: int, retries: int, delay: float):
    import Quartz  # optional dep; only needed here + scroll
    for _ in range(retries):
        info = Quartz.CGWindowListCopyWindowInfo(
            Quartz.kCGWindowListOptionOnScreenOnly, Quartz.kCGNullWindowID)
        cands = [w for w in (info or [])
                 if w.get("kCGWindowOwnerPID") == pid and w.get("kCGWindowLayer") == 0]
        if cands:
            def area(w):
                b = w.get("kCGWindowBounds", {})
                return b.get("Width", 0) * b.get("Height", 0)
            w = max(cands, key=area)
            b = w.get("kCGWindowBounds", {})
            return (int(w["kCGWindowNumber"]),
                    [int(b.get("X", 0)), int(b.get("Y", 0)), int(b.get("Width", 0)), int(b.get("Height", 0))])
        time.sleep(delay)
    return None


def cmd_window_id(a) -> int:
    info = _window_info(a.pid, a.retries, a.delay)
    if info is None:
        return _emit(ok=False, error="no on-screen window for pid")
    win, bounds = info
    return _emit(id=win, bounds=bounds)


def cmd_click(a) -> int:
    rc = subprocess.run([CLICLICK, f"c:{a.x},{a.y}"], capture_output=True, text=True)
    return _emit(ok=rc.returncode == 0, stderr=rc.stderr.strip()[:400])


def cmd_type(a) -> int:
    rc = subprocess.run([CLICLICK, f"t:{a.text}"], capture_output=True, text=True)
    return _emit(ok=rc.returncode == 0, stderr=rc.stderr.strip()[:400])


def cmd_scroll(a) -> int:
    # cliclick has no scroll — move the pointer to the target, then post a Quartz scroll-wheel event.
    import Quartz
    Quartz.CGWarpMouseCursorPosition((a.x, a.y))
    ev = Quartz.CGEventCreateScrollWheelEvent(None, Quartz.kCGScrollEventUnitPixel, 1, int(a.dy))
    Quartz.CGEventPost(Quartz.kCGHIDEventTap, ev)
    return _emit(x=a.x, y=a.y, dy=a.dy)


def cmd_shot(a) -> int:
    os.makedirs(os.path.dirname(a.out) or ".", exist_ok=True)
    if a.window:
        # -x no sound, -o omit shadow padding (tight window crop), -l capture that window's own backing
        # store (no other window can composite in; needs no focus). Mirrors e2e.py::_maccat_shot.
        cmd = [SCREENCAPTURE, "-x", "-o", "-l", str(a.window), a.out]
    else:
        cmd = [SCREENCAPTURE, "-x", a.out]  # fallback: whole main display
    rc = subprocess.run(cmd, capture_output=True, text=True)
    ok = rc.returncode == 0 and os.path.isfile(a.out)
    return _emit(ok=ok, out=a.out, window=a.window, stderr=rc.stderr.strip()[:400])


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
    s.add_argument("--retries", type=int, default=15); s.add_argument("--delay", type=float, default=0.3)
    s.set_defaults(fn=cmd_window_id)

    s = sub.add_parser("click"); s.add_argument("x", type=int); s.add_argument("y", type=int)
    s.set_defaults(fn=cmd_click)

    s = sub.add_parser("type"); s.add_argument("text"); s.set_defaults(fn=cmd_type)

    s = sub.add_parser("scroll"); s.add_argument("x", type=int); s.add_argument("y", type=int)
    s.add_argument("dy", type=int); s.set_defaults(fn=cmd_scroll)

    s = sub.add_parser("shot"); s.add_argument("out"); s.add_argument("--window", type=int, default=0)
    s.set_defaults(fn=cmd_shot)

    s = sub.add_parser("stop"); s.add_argument("pid", type=int); s.set_defaults(fn=cmd_stop)

    a = p.parse_args(argv)
    try:
        return a.fn(a)
    except Exception as e:  # never crash the SSH call without a parseable result
        return _emit(ok=False, error=f"{type(e).__name__}: {e}")


if __name__ == "__main__":
    sys.exit(main())
