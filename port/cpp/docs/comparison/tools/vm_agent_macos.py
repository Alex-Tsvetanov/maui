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
import signal
import subprocess
import sys
import time

# Absolute tool paths (Homebrew/system defaults; env-overridable).
CLICLICK = os.environ.get("MAUI_E2E_CLICLICK", "/opt/homebrew/bin/cliclick")
DISPLAYPLACER = os.environ.get("MAUI_E2E_DISPLAYPLACER", "/opt/homebrew/bin/displayplacer")
SCREENCAPTURE = os.environ.get("MAUI_E2E_SCREENCAPTURE", "/usr/sbin/screencapture")
OPEN = os.environ.get("MAUI_E2E_OPEN", "/usr/bin/open")


# cliclick's own wording when the build does not have a verb (measured on 5.1, 2022-08-14). Matched
# case-insensitively against BOTH streams: 5.1 prints it to STDERR here, other reports have it on
# STDOUT, and getting the stream wrong turns the `dm:` probe below into a silent no-op again.
UNKNOWN_VERB = "unrecognized action shortcut"


def _emit(**kw) -> int:
    kw.setdefault("ok", True)
    print(json.dumps(kw))
    return 0 if kw["ok"] else 1


# ---------------------------------------------------------------- pointer hygiene
#
# POINTER CONTAMINATION. The pointer is MACHINE-global: it outlives the cliclick process, the agent
# process, and the app relaunch between columns. A pointer left sitting on a control keeps that
# control in its hover / PointerOver / tooltip state for every later frame of every later PAGE, and
# that reads on the board as a port defect rather than as tooling — the same "plausible but wrong
# capture" shape this file's header warns about throughout. So every verb that MOVES the pointer as a
# means to an end puts it back afterwards.
#
# cmd_hover is the deliberate exception: parking IS the gesture there.


def _pointer_pos() -> list[int] | None:
    """Where the pointer is RIGHT NOW as [x, y], or None if cliclick could not say.

    `cliclick p:.` prints "x,y" (verified on 5.1). None is not fatal — the gesture still runs — but it
    means the restore below cannot happen, so callers report `pointer_restored` rather than pretending."""
    rc = subprocess.run([CLICLICK, "p:."], capture_output=True, text=True)
    parts = rc.stdout.strip().split(",")
    if rc.returncode == 0 and len(parts) == 2:
        try:
            return [int(float(parts[0])), int(float(parts[1]))]
        except ValueError:
            return None
    return None


def _restore_pointer(saved: list[int] | None) -> bool:
    """Put the pointer back where `saved` says it was. False if it could not be done — which the
    caller must EMIT, not swallow: an un-restored pointer contaminates every later frame silently.

    Both failure branches say so on stderr. A missing `saved` is the sneakier one: the restore is then
    skipped ENTIRELY while the gesture still reports ok, i.e. this fix quietly not running — the very
    shape it exists to eliminate, aimed at itself."""
    if not saved:
        print("[agent] pointer NOT restored: `cliclick p:.` never gave a position to restore TO, so "
              "the pointer is left wherever this gesture put it", file=sys.stderr)
        return False
    rc = subprocess.run([CLICLICK, f"m:{saved[0]},{saved[1]}"], capture_output=True, text=True)
    if rc.returncode != 0:
        print(f"[agent] pointer NOT restored to {saved}: {rc.stderr.strip()[:200]}", file=sys.stderr)
        return False
    return True


def _pgrep(proc: str) -> set[str]:
    return set(subprocess.run(["/usr/bin/pgrep", "-x", proc], capture_output=True, text=True).stdout.split())


def cmd_set_resolution(a) -> int:
    # Find the main display's persistent id + its available modes, then request WxH. If that exact mode is
    # gone (UTM regenerates the mode list when its window is resized — this bit us once, silently leaving a
    # wrong resolution), fall back to ANY same-WIDTH mode (the scenario x-calibration only needs the width),
    # preferring the height closest to the request. Read back + return the ACTUAL current mode so the caller
    # can trust the geometry. Fail loudly if no same-width mode exists.
    #
    # ONLY 1x MODES ARE ELIGIBLE. UTM offers most sizes TWICE — plain and `scaling:on` (HiDPI/Retina) — and
    # the HiDPI twin captures at 2x: on a 1512x950 HiDPI guest, `screencapture -R 0,0,1024,800` returns a
    # 2048x1600 PNG (measured), which the runner's +/-4px size guard then drops, every frame. The twins are
    # indistinguishable by (w, h), so matching on size alone can select the 2x one, and a readback that
    # compares only (w, h) still calls that a success. Hence: consider unscaled modes only, ask for
    # `scaling:off` explicitly, and reject a HiDPI landing in the readback.
    import re
    listing = subprocess.run([DISPLAYPLACER, "list"], capture_output=True, text=True).stdout
    disp_id = None
    modes = []       # (w, h) available on the main display at 1x — the only ones we may select
    hidpi = []       # (w, h) offered as scaling:on; kept solely to explain a failure
    for line in listing.splitlines():
        s = line.strip()
        if s.lower().startswith("persistent screen id:") and disp_id is None:
            disp_id = s.split(":", 1)[1].strip()
        m = re.search(r"res:(\d+)x(\d+)", s)
        if m and "mode" in s:
            (hidpi if "scaling:on" in s else modes).append((int(m.group(1)), int(m.group(2))))
    if not disp_id:
        return _emit(ok=False, error="no display id from `displayplacer list`", raw=listing[:400])
    want = (a.width, a.height)
    if want in modes:
        target = want
    else:
        same_w = sorted((h for (w, h) in modes if w == a.width), key=lambda h: abs(h - a.height))
        if not same_w:
            # Name the HiDPI case explicitly: "no 1x mode at this size" is a different problem from
            # "this size does not exist", and only the former is fixed by resizing the UTM window.
            twins = sorted(set(m for m in hidpi if m[0] == a.width))
            why = (f"no 1x mode at {a.width} wide — it exists ONLY as HiDPI {twins}, which captures at 2x "
                   f"and would get every frame dropped by the size guard" if twins
                   else f"no {a.width}-wide mode available")
            return _emit(ok=False, error=why, available=sorted(set(modes)), hidpi_only=twins)
        target = (a.width, same_w[0])
    # Toggle through a different mode first, THEN set the target. Re-setting the CURRENT mode is a no-op and
    # does NOT re-sync a WindowServer that got confused about display geometry (seen after a UTM window resize
    # + lid events: apps then open with bogus bounds and their windows are invisible / not AX-enumerable).
    # A real mode change forces the session to re-apply, which fixes it. Pick the most-different-size mode.
    others = [m for m in dict.fromkeys(modes) if m != target]
    if others:
        scratch = max(others, key=lambda m: abs(m[0] - target[0]) + abs(m[1] - target[1]))
        subprocess.run([DISPLAYPLACER, f"id:{disp_id} res:{scratch[0]}x{scratch[1]}"], capture_output=True, text=True)
        time.sleep(1.0)
    subprocess.run([DISPLAYPLACER, f"id:{disp_id} res:{target[0]}x{target[1]} scaling:off"],
                   capture_output=True, text=True)
    time.sleep(1.0)
    cur = subprocess.run([DISPLAYPLACER, "list"], capture_output=True, text=True).stdout
    actual, actual_hidpi = None, False
    for s in cur.splitlines():
        if "current mode" in s.lower():
            m = re.search(r"res:(\d+)x(\d+)", s)
            if m:
                actual = [int(m.group(1)), int(m.group(2))]
                actual_hidpi = "scaling:on" in s
    ok = actual == list(target) and not actual_hidpi
    err = None
    if actual != list(target):
        err = f"display did not switch to {target[0]}x{target[1]} (it is {actual})"
    elif actual_hidpi:
        err = (f"display is {actual[0]}x{actual[1]} but HiDPI (scaling:on): captures come back at 2x and "
               f"every frame would be dropped by the size guard")
    return _emit(ok=ok, error=err, display=disp_id, requested=[a.width, a.height], set=list(target),
                 actual=actual, hidpi=actual_hidpi)


def cmd_clean(a) -> int:
    shutil.rmtree(a.dir, ignore_errors=True)
    os.makedirs(a.dir, exist_ok=True)
    return _emit(dir=a.dir)


def _bundle_id(bundle: str) -> str | None:
    plist = os.path.join(bundle, "Contents", "Info.plist")
    rc = subprocess.run(
        ["/usr/libexec/PlistBuddy", "-c", "Print :CFBundleIdentifier", plist], capture_output=True, text=True
    )
    return rc.stdout.strip() or None if rc.returncode == 0 else None


def _clear_saved_state(bundle: str) -> str | None:
    """Drop macOS's window-restoration state for `bundle`.

    Catalyst apps opt into NSWindow restoration: on relaunch AppKit reopens the window the app had last
    time and restores its page, IGNORING the MAUI_SAMPLE_PAGE env var the runner passes. That silently
    captures the WRONG PAGE — and it is not a visibly broken capture, it is a plausible screenshot of some
    other page, which then gets scored against the right MAUI reference. It bit hardest right after
    `reboot_before_run` (a reboot is exactly when macOS re-opens everything it had), but nothing about it
    is reboot-specific, so clear it on EVERY launch rather than once per run.
    """
    bid = _bundle_id(bundle)
    if not bid:
        return None
    state = os.path.expanduser(f"~/Library/Saved Application State/{bid}.savedState")
    shutil.rmtree(state, ignore_errors=True)
    # …and stop it being rewritten on the next quit, so a launch never inherits a sibling run's window.
    subprocess.run(
        ["/usr/bin/defaults", "write", bid, "NSQuitAlwaysKeepsWindows", "-bool", "false"], capture_output=True
    )
    return bid


def _launch_plain_executable(a) -> int:
    """Launch a PLAIN UNIX EXECUTABLE (the AppKit galleries), which `open` cannot start.

    `open` resolves its argument through LaunchServices, which has no handler for a bare Mach-O — on this
    VM it hands the file to Terminal.app, so `open -g -n <exe>` returns 0 having started a TERMINAL, and
    the pgrep-diff below then waits 10s for a process that will never appear under that name.

    Two things the `open` path gets for free and this one must do explicitly:
      cwd  — the apple-backend gallery resolves from_file() asset paths (dotnet_bot.png, oasis.jpg, …)
             against the CWD, and maui_add_app copies those assets next to the binary. Without cwd every
             image page renders empty.
      env  — `open --env` REPLACES the environment; Popen inherits ours, so start from os.environ and
             overlay the --env pairs (the agent's own PATH etc. must survive for the app to run).

    Output goes to a log file, never DEVNULL and never an inherited fd: host_run writes its "[host_run]
    mounted app window" trace to stderr, which is the only evidence available when a launch silently
    fails — and an inherited SSH pipe would keep the connection open past the agent's exit, hanging the
    caller's 120s timeout.
    """
    exe = a.bundle
    if os.path.isdir(exe):  # a deployed build directory: the binary inside is named after the process
        exe = os.path.join(exe, a.proc)
    if not os.path.isfile(exe) or not os.access(exe, os.X_OK):
        return _emit(ok=False, error=f"not an executable: {exe}")
    env = dict(os.environ)
    for kv in a.env or []:
        key, _, value = kv.partition("=")
        env[key] = value
    log_path = f"/tmp/maui_e2e_{a.proc}.log"
    try:
        with open(log_path, "wb") as log:
            proc = subprocess.Popen([exe], cwd=os.path.dirname(exe), env=env,
                                    stdout=log, stderr=subprocess.STDOUT,
                                    start_new_session=True)  # survive the agent's exit
    except OSError as exc:
        return _emit(ok=False, error=f"exec failed: {exc}")
    time.sleep(0.5)  # long enough for an immediate crash (bad dylib, missing asset) to be visible
    if proc.poll() is not None:
        tail = ""
        try:
            with open(log_path, "r", errors="replace") as log:
                tail = log.read()[-400:]
        except OSError:
            pass
        return _emit(ok=False, error=f"process exited immediately (rc={proc.returncode})", stderr=tail)
    return _emit(pid=proc.pid, launched="exec", log=log_path)


def cmd_launch(a) -> int:
    # A plain executable cannot go through `open` (see _launch_plain_executable). Branch on the artifact
    # shape, NOT on the column name, so the .app path below is untouched for every existing column.
    if not a.bundle.endswith(".app"):
        return _launch_plain_executable(a)
    # `open -g -n --env ...`: background (no focus theft), new instance. Find the new pid by diffing
    # pgrep before/after (mirrors e2e.py::_launch_background).
    cleared = _clear_saved_state(a.bundle)
    # Reap instances left over from earlier pages BEFORE launching a new one.
    #
    # Finding a window is pid-safe (Quartz, _window_info_quartz). PRESENTING it is not: cmd_present sizes
    # the window with AppleScript `window 1 of application process <name>`, keyed on the PROCESS NAME,
    # which cannot tell two live instances apart. So one leaked instance silently captures every present
    # from then on and the real window is never framed — the run reports "present failed after self-heal
    # (no window to capture)" and DROPS the frame. It does not fail loudly and it does not fake a picture;
    # it just quietly stops producing motion frames for that column.
    #
    # On 2026-08-13 two instances leaked at 19:07/19:08 on the catalyst lane and cost 125 dropped frames
    # across ~33 pages before anyone looked. `open -n` always starts a fresh instance, so anything already
    # running under this name is by definition stale and safe to kill.
    stale = _pgrep(a.proc)
    for pid in stale:
        try:
            os.kill(int(pid), signal.SIGKILL)
        except (OSError, ValueError):
            pass
    if stale:
        time.sleep(0.3)  # let them leave the process table, or the pgrep diff below re-finds them as "fresh"
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
            return _emit(pid=int(next(iter(fresh))), cleared_saved_state=cleared)
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
    # Mac Catalyst sometimes ignores the FIRST `set size` (it restores a remembered window size and only
    # accepts the resize on a later attempt), which left some columns at their restored height (e.g. 548) while
    # others hit the target (800) — non-reproducible captures. So retry position+size until the window actually
    # reaches the target (within a few pt) or we run out of tries, then return the ACTUAL rect regardless.
    # Patient loop: ~20 tries x 0.4s = 8s. On a freshly-restarted / loaded VM Catalyst can take several
    # seconds to actually apply the resize, and a short 2s loop timed out and returned the WRONG (restored,
    # e.g. 548-tall) size. The caller treats a short return as a failure and self-heals (see below), so it
    # is better to wait here than to bounce through a resolution toggle.
    out = _osa(
        'with timeout of 30 seconds\n'
        f'tell application "System Events" to tell process "{a.proc}"\n'
        '  set frontmost to true\n'
        '  repeat 20 times\n'
        '    try\n'
        f'      set position of window 1 to {{{x}, {y}}}\n'
        f'      set size of window 1 to {{{w}, {h}}}\n'
        '    end try\n'
        '    delay 0.4\n'
        '    set s to size of window 1\n'
        f'    if (item 1 of s) > {w - 5} and (item 2 of s) > {h - 5} then exit repeat\n'
        '  end repeat\n'
        '  set p to position of window 1\n  set s to size of window 1\n'
        '  return (item 1 of p as string) & "," & (item 2 of p as string) & "," & '
        '(item 1 of s as string) & "," & (item 2 of s as string)\n'
        'end tell\nend timeout')
    parts = out.split(",")
    if len(parts) == 4:
        try:
            rect = [int(float(p)) for p in parts]
        except ValueError:
            return _emit(ok=False, proc=a.proc, error=out[:200] or "no rect from System Events")
        # FAIL if the window never reached the target size. Returning it as a "success" is how ~800 short
        # frames got banked across two sweeps: present handed back e.g. 1024x548, the shot captured the
        # window at that size, and only a downstream size sweep noticed. A short return here instead routes
        # the caller (shoot_presented) into its resolution-toggle self-heal + retry, which re-syncs a
        # desynced / load-stalled WindowServer session — the actual recovery, not a silent bad capture.
        if rect[2] < w - 5 or rect[3] < h - 5:
            return _emit(ok=False, proc=a.proc, error=f"window did not reach target: got {rect[2]}x{rect[3]}, "
                                                      f"want {w}x{h}", rect="", bounds=rect)
        # …and the window's CGWindowID, so the caller can shoot THAT WINDOW rather than the screen region
        # it happens to occupy. `screencapture -R` photographs whatever is on top of the rect: a lingering
        # window from a previous page composites straight into the shot, and the result looks perfectly
        # legitimate — a full board sweep captured the C++ gallery's window into the MAUI reference column
        # for scroll_view and scored it as a 96% "diff". `-l <id>` reads the window's own backing store and
        # cannot be occluded. Looked up via QUARTZ, deliberately: it is a read-only window-server query, so
        # unlike a System Events / AX call it does NOT steal key focus back and grey the traffic lights,
        # which is why the earlier window-id lookup had to be removed.
        # Retry the lookup: the window is up as far as System Events is concerned (we just positioned it),
        # but Quartz's ON-SCREEN list can lag by a beat, and a miss here is not harmless — the caller would
        # fall back to a rect shot and silently photograph whatever occupies that region. A whole sweep lost
        # 40 alphabetically-contiguous pages that way, capturing the previous column's window into the MAUI
        # reference column.
        window_id = None
        quartz_bounds = None
        if a.pid:
            for _ in range(10):
                try:
                    found = _window_info_quartz(int(a.pid))
                except Exception:  # noqa: BLE001 — pyobjc missing/odd state; report it rather than guess
                    break
                if found:
                    window_id, quartz_bounds = found
                    break
                time.sleep(0.2)

        # Atomic present-and-capture. present and shot used to be two SEPARATE SSH round-trips, and a
        # heavier app (the .NET MauiReference, the compile-time-XAML gallery_xaml) re-lays-out its window
        # to CONTENT size in the ~300ms gap: present confirmed 1024x800, the app shrank the window back to
        # 1024x548, and the -l shot captured 548. The lean cpp gallery fills its window so never shrinks —
        # which is why only the two MAUI columns dropped, a per-app symptom that looked like a settle bug.
        # Capturing HERE, in the same process the instant the size is confirmed, closes the gap. Quartz's
        # freshly-read bounds are the last-moment size check: if the app already shrank, fail so the caller
        # self-heals instead of banking a short frame.
        if a.shot and window_id is not None:
            if quartz_bounds is not None and (quartz_bounds[2] < w - 5 or quartz_bounds[3] < h - 5):
                return _emit(ok=False, proc=a.proc, rect="", bounds=rect,
                             error=f"window shrank before capture: {quartz_bounds[2]}x{quartz_bounds[3]}")
            os.makedirs(os.path.dirname(a.shot) or ".", exist_ok=True)
            rc = subprocess.run([SCREENCAPTURE, "-x", "-o", "-l", str(window_id), a.shot],
                                capture_output=True, text=True)
            if rc.returncode != 0 or not os.path.isfile(a.shot):
                return _emit(ok=False, proc=a.proc, rect="", bounds=rect,
                             error=f"capture failed: {rc.stderr.strip()[:120]}")
            return _emit(ok=True, proc=a.proc, rect=",".join(map(str, rect)), bounds=rect,
                         window=window_id, shot=a.shot)
        return _emit(ok=True, proc=a.proc, rect=",".join(map(str, rect)), bounds=rect, window=window_id)
    return _emit(ok=False, proc=a.proc, error=out[:200] or "no rect from System Events")


def cmd_click(a) -> int:
    """Click at (x, y) and PUT THE POINTER BACK (see the POINTER CONTAMINATION note above).

    `-r` ("restore initial mouse location when finished") does it inside the same cliclick process, so
    the click lands at (x, y) and nothing is parked on the clicked control afterwards — no second
    invocation, no saved position to lose."""
    rc = subprocess.run([CLICLICK, "-r", f"c:{a.x},{a.y}"], capture_output=True, text=True)
    return _emit(ok=rc.returncode == 0, x=a.x, y=a.y, stderr=rc.stderr.strip()[:400])


def cmd_type(a) -> int:
    rc = subprocess.run([CLICLICK, f"t:{a.text}"], capture_output=True, text=True)
    return _emit(ok=rc.returncode == 0, stderr=rc.stderr.strip()[:400])


def cmd_hover(a) -> int:
    """Park the pointer at (x, y) and LEAVE it there.

    That is the whole gesture: a hover-reactive page (PointerGestureRecognizer entered/moved, a
    VisualState PointerOver, a tooltip) only changes while the pointer is inside it, so the pointer
    must still be there when the shot is taken — which is why this is the ONE verb that deliberately
    does not restore the pointer (see POINTER CONTAMINATION above; click/scroll/drag all do).

    EFFICACY ON MAC CATALYST IS UNVERIFIED. `m:` posts a kCGEventMouseMoved at the HID tap, which is
    the only pointer-move primitive cliclick offers. AppKit turns that into NSTrackingArea
    enter/exit, but Catalyst's hover comes from UIKit's indirect-pointer path, and whether a
    SYNTHETIC cursor move reaches it cannot be settled off-guest. So the first hover scenario on
    maccatalyst must be checked against the MAUI COLUMN reacting too: if BOTH columns are unchanged
    that is this tooling failing to deliver the event, and it must be reported as a tooling
    limitation — never as a port finding. Two identical non-reactions look like agreement.
    (The Windows agent's cmd_hover has no such doubt: SetCursorPos -> WM_MOUSEMOVE -> XAML
    PointerEntered is the real mechanism.)

    The flip side, for scenario authors: the pointer is MACHINE-global and survives the app relaunch
    between columns, so a parked pointer bleeds into every later frame of the run. End a
    pointer-moving scenario with a hover to a neutral point (see the syntax block in
    run_comparison.py's load_scenario)."""
    rc = subprocess.run([CLICLICK, f"m:{a.x},{a.y}"], capture_output=True, text=True)
    return _emit(ok=rc.returncode == 0, x=a.x, y=a.y, stderr=rc.stderr.strip()[:400])


def _drag_points(x1: int, y1: int, x2: int, y2: int, steps: int) -> list[tuple[int, int]]:
    """The drag's move points: `steps` evenly-spaced positions from just after (x1,y1) up to and
    INCLUDING (x2,y2). Pure — the runner's --selftest asserts this against the Windows agent's copy.

    Floored at 2 steps deliberately. With 1 step a "drag" is a press and a release at one point, i.e.
    a CLICK — it runs, it reports ok, and it produces a frame identical to the previous one, which is
    indistinguishable from a page that simply does not react. That is the exact failure this verb
    exists to eliminate, so a misconfigured scenario must not be able to degrade into it.

    (Duplicated verbatim in vm_agent_windows.py. Each agent is copied to a guest ALONE, so it cannot
    import a shared module — the same reason _emit and cmd_clean are duplicated between them.)"""
    steps = max(2, int(steps))
    return [(round(x1 + (x2 - x1) * i / steps), round(y1 + (y2 - y1) * i / steps))
            for i in range(1, steps + 1)]


def cmd_drag(a) -> int:
    """Press at (x1,y1), MOVE through interpolated points, release at (x2,y2). `swipe` is this same
    function with a faster default duration — one implementation, two names (see main()).

    THE INTERMEDIATE MOVES ARE THE POINT. A `dd:` immediately followed by a `du:` is a click as far as
    every gesture recognizer is concerned: no pan, no swipe, no SwipeView reveal — and the resulting
    frame is identical to the un-driven one, which reads on the board as "the page does not react".

    WHICH MOVE VERB. cliclick's `m:` posts kCGEventMouseMoved; AppKit/UIKit route a button-down drag
    from kCGEventLeftMouseDragged, which is what cliclick's `dm:` posts. `dm:` is therefore the right
    verb, but it is not present in every cliclick build. So: PROBE with `dm:` on the first move and
    fall back to `m:` for the whole drag ONLY IF that build does not have it, reporting which one was
    used as `move_verb` — a logged fact per frame instead of an untestable assumption about the guest.

    The probe fires on cliclick's UNKNOWN-VERB SIGNATURE, not on "returncode != 0". It used to drop
    the failure for ANY nonzero return, which laundered every real refusal (no Accessibility grant, an
    off-screen coordinate, a wedged WindowServer) into a silent downgrade to `m:` — a verb this
    docstring itself says does not drive a drag — and then reported the whole step ok. Anything that
    is not the signature now STAYS in `fails`, so the step comes back not-ok and the host prints it.

    Each move is its own invocation with a Python sleep between, rather than one cliclick command line:
    a pan recognizer needs TIME-SEPARATED move events, and this needs no assumption about cliclick's
    own `w:` wait. ~10 local fork/execs on the guest, not over SSH — a few hundred ms.

    `duration` is a FLOOR, not a target (the spawns and cliclick's own startup add to it), so the
    measured `elapsed` is emitted for calibration."""
    pts = _drag_points(a.x1, a.y1, a.x2, a.y2, a.steps)
    per_step = max(0.0, float(a.duration)) / len(pts)
    started = time.time()
    fails: list[str] = []
    move_verb = "dm"
    saved = _pointer_pos()  # read BEFORE the press; the release below lands on the far end

    def run(cmd: str) -> subprocess.CompletedProcess:
        rc = subprocess.run([CLICLICK, cmd], capture_output=True, text=True)
        if rc.returncode != 0:
            fails.append(f"{cmd}: {(rc.stderr or rc.stdout).strip()[:120] or f'rc={rc.returncode}'}")
        return rc

    if run(f"dd:{a.x1},{a.y1}").returncode != 0:
        _restore_pointer(saved)
        return _emit(ok=False, gesture=a.gesture, error=f"cliclick dd failed: {fails[-1]}")
    try:
        time.sleep(min(per_step, 0.05))  # let the press register before the first move
        # The FULL list, endpoint included. Iterating pts[:-1] dropped the last interpolated point, so
        # the final drag event landed at (steps-1)/steps of the travel and the remainder arrived only
        # as the mouse-up — a short pan that a distance-thresholded recognizer can refuse outright,
        # producing the un-driven frame this verb exists to prevent. `du:` still releases ON (x2,y2).
        for i, (px, py) in enumerate(pts):
            if i == 0:
                probe = run(f"dm:{px},{py}")
                if probe.returncode != 0:
                    if UNKNOWN_VERB in f"{probe.stdout}\n{probe.stderr}".lower():
                        fails.pop()  # no `dm:` in THIS build — the fallback path, not a failure
                        move_verb = "m"
                        run(f"m:{px},{py}")
                    # …any other refusal stays in `fails`: see the docstring. move_verb stays "dm", so
                    # the remaining moves fail the same way and the step is reported not-ok.
            else:
                run(f"{move_verb}:{px},{py}")
            time.sleep(per_step)
    finally:
        # ALWAYS release. A press left down is not a dropped step, it is a wedged GUEST: every later
        # frame of every later page is captured mid-drag, and that reads on the board as a port defect
        # rather than as tooling — the same shape as the `or bounds` bug run_comparison.py documents.
        run(f"du:{a.x2},{a.y2}")
        # …and only THEN put the pointer back. Restoring before the release would drag the content all
        # the way home and undo the gesture; restoring after leaves nothing parked on the far end.
        restored = _restore_pointer(saved)
    # A whole board driven with `m:` is a TOOLING artifact, not a port finding — say so per frame so
    # nobody reads a wall of unreacting drag pages as the port failing to pan.
    warning = "" if move_verb == "dm" else (
        "this cliclick build has no `dm:`; the drag was driven with plain `m:` moves, which may not "
        "reach a pan/swipe recognizer — an unreacting frame here is TOOLING, not a port defect")
    if warning:
        print(f"[agent] {warning}", file=sys.stderr)
    return _emit(ok=not fails, gesture=a.gesture, move_verb=move_verb, points=len(pts),
                 to=[a.x2, a.y2], elapsed=round(time.time() - started, 3),
                 pointer_restored=restored, warning=warning,
                 stderr="; ".join(fails)[:400])


def _post_scroll(dy: int) -> str:
    """Post a pixel scroll-wheel event at the pointer's CURRENT location. "" on success, else why not.

    Split out of cmd_scroll so the selftest can exercise the verb's pointer handling without posting a
    real wheel event onto the dev machine's desktop."""
    import ctypes
    cg = ctypes.CDLL("/System/Library/Frameworks/ApplicationServices.framework/ApplicationServices")
    cg.CGEventCreateScrollWheelEvent.restype = ctypes.c_void_p
    cg.CGEventCreateScrollWheelEvent.argtypes = [ctypes.c_void_p, ctypes.c_uint32, ctypes.c_uint32, ctypes.c_int32]
    cg.CGEventPost.argtypes = [ctypes.c_uint32, ctypes.c_void_p]
    ev = cg.CGEventCreateScrollWheelEvent(None, 1, 1, int(dy))  # units=1 kCGScrollEventUnitPixel, 1 wheel
    if not ev:
        return "CGEventCreateScrollWheelEvent returned null"
    cg.CGEventPost(0, ev)  # tap = 0 kCGHIDEventTap
    cf = ctypes.CDLL("/System/Library/Frameworks/CoreFoundation.framework/CoreFoundation")
    cf.CFRelease.argtypes = [ctypes.c_void_p]
    cf.CFRelease(ev)
    return ""


def cmd_scroll(a) -> int:
    """Scroll by `dy` pixels over (x, y), then PUT THE POINTER BACK.

    cliclick has no scroll. Move the pointer to the target (cliclick), then post a pixel scroll-wheel
    event via CoreGraphics through ctypes — NO pyobjc needed. Needs Accessibility (same as cliclick).

    The move is what aims the wheel event (it goes to whatever is under the pointer), so unlike
    cmd_click this cannot use `-r`: the pointer has to still be there when the event is posted. Hence
    the explicit save/restore — otherwise the pointer stays parked on the scrolled view and every
    later frame of every later page inherits its hover state (see POINTER CONTAMINATION above).

    The move's own return used to be discarded, so a refused move scrolled whatever the pointer
    happened to be over — a frame that looks legitimate and is not."""
    saved = _pointer_pos()
    try:
        mv = subprocess.run([CLICLICK, f"m:{a.x},{a.y}"], capture_output=True, text=True)
        if mv.returncode != 0:
            return _emit(ok=False, x=a.x, y=a.y, dy=a.dy,
                         error=f"cliclick m: failed, so the scroll would land wherever the pointer "
                               f"already was: {(mv.stderr or mv.stdout).strip()[:200]}")
        err = _post_scroll(a.dy)
        if err:
            return _emit(ok=False, x=a.x, y=a.y, dy=a.dy, error=err)
    finally:
        restored = _restore_pointer(saved)
    return _emit(x=a.x, y=a.y, dy=a.dy, pointer_restored=restored)


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


# ---------------------------------------------------------------- self-check


def cmd_selftest(a) -> int:  # noqa: ARG001 — argparse hands us a namespace we do not need
    """Assert the interaction verbs' EXACT cliclick argv, with the OS seam stubbed.

    `python3 vm_agent_macos.py selftest` — no guest, no cliclick, no desktop touched. Every assertion
    here pins a way this agent can do NOTHING and still report success, which is the entire failure
    class the interaction lane exists to eliminate:
      * a verb that leaves the pointer parked on the UI (every later frame carries a hover state),
      * a drag whose last move never happens (a short pan a recognizer can refuse outright),
      * the `dm:` probe downgrading a REAL refusal into a move verb that does not drive a drag.
    Stubs are installed and removed under try/finally: this is reachable through the argparse tree, so
    a mid-assertion abort must not leak a patched `subprocess` into a live run."""
    import types  # noqa: PLC0415 — selftest-only

    class _FakeCliclick:
        """Stand-in for the module's `subprocess`, recording every cliclick argv it is handed.

        `reject` names verbs this fake BUILD does not have / refuses; `message` is what it prints on
        the `stream` it prints to — the two axes the `dm:` probe has to tell apart."""

        def __init__(self, reject=(), message="", stream="stderr"):
            self.seen: list[list[str]] = []
            self.reject, self.message, self.stream = set(reject), message, stream

        def run(self, argv, **_kw):
            self.seen.append(list(argv))
            verb = argv[-1].split(":", 1)[0]
            if verb in self.reject:
                return types.SimpleNamespace(returncode=1,
                                             stdout=self.message if self.stream == "stdout" else "",
                                             stderr=self.message if self.stream == "stderr" else "")
            return types.SimpleNamespace(returncode=0, stdout="700,400\n" if verb == "p" else "",
                                         stderr="")

    def drive(fn, ns, fake) -> tuple[list[str], dict]:
        """Run one verb against `fake`, returning (cliclick verbs in order, the emitted JSON)."""
        import contextlib  # noqa: PLC0415
        import io  # noqa: PLC0415
        real_sub, real_scroll = globals()["subprocess"], globals()["_post_scroll"]
        globals()["subprocess"] = fake
        globals()["_post_scroll"] = lambda _dy: ""
        try:
            with contextlib.redirect_stdout(io.StringIO()) as out:
                fn(ns)
        finally:
            globals()["subprocess"], globals()["_post_scroll"] = real_sub, real_scroll
        return [c[-1] for c in fake.seen], json.loads(out.getvalue())

    N = argparse.Namespace
    checks = 0

    # (a) POINTER CONTAMINATION — click restores in-process via -r; scroll and drag save+restore
    #     around the gesture; hover is the ONE verb that must NOT restore.
    fake = _FakeCliclick()
    seen, res = drive(cmd_click, N(x=10, y=20), fake)
    assert seen == ["c:10,20"] and res["ok"], (seen, res)
    assert fake.seen[0][1] == "-r", (f"click must pass -r or the pointer stays on the clicked "
                                     f"control for every later page: {fake.seen}")
    checks += 1

    seen, _res = drive(cmd_hover, N(x=10, y=20), _FakeCliclick())
    assert seen == ["m:10,20"], f"hover must PARK the pointer and stop — restoring defeats it: {seen}"
    checks += 1

    seen, res = drive(cmd_scroll, N(x=10, y=20, dy=-400), _FakeCliclick())
    assert seen == ["p:.", "m:10,20", "m:700,400"], f"scroll must save+restore around the move: {seen}"
    assert res["ok"] and res["pointer_restored"], res
    checks += 1

    # A refused move must NOT scroll wherever the pointer already was and call it a success.
    seen, res = drive(cmd_scroll, N(x=10, y=20, dy=-400), _FakeCliclick(reject={"m"}))
    assert not res["ok"], res
    checks += 1

    # (b) the drag reaches its ENDPOINT: pts[:-1] used to drop dm:100,0 entirely.
    dns = N(x1=0, y1=0, x2=100, y2=0, steps=4, duration=0.0, gesture="drag")
    seen, res = drive(cmd_drag, dns, _FakeCliclick())
    assert seen == ["p:.", "dd:0,0", "dm:25,0", "dm:50,0", "dm:75,0", "dm:100,0", "du:100,0",
                    "m:700,400"], seen
    assert res["ok"] and res["move_verb"] == "dm" and res["pointer_restored"], res
    checks += 1

    # (c) the `dm:` probe falls back ONLY on cliclick's unknown-verb signature — on either stream,
    #     because 5.1 was measured printing it to stderr and other builds are reported on stdout.
    for stream in ("stderr", "stdout"):
        seen, res = drive(cmd_drag, dns, _FakeCliclick(
            reject={"dm"}, message='Unrecognized action shortcut “dm” in “dm:25,0”', stream=stream))
        assert seen == ["p:.", "dd:0,0", "dm:25,0", "m:25,0", "m:50,0", "m:75,0", "m:100,0",
                        "du:100,0", "m:700,400"], (stream, seen)
        assert res["ok"] and res["move_verb"] == "m", (stream, res)
        assert res["warning"], "an m:-driven board is a tooling artifact and must say so per frame"
        checks += 1

    # …and any OTHER refusal stays a refusal. This is the regression that mattered: it used to be
    # laundered into `m:`, so a drag that never ran was banked as a driven frame.
    seen, res = drive(cmd_drag, dns, _FakeCliclick(reject={"dm"}, message="not permitted"))
    assert not res["ok"], f"a non-signature dm: failure must NOT downgrade to m:: {res}"
    assert res["move_verb"] == "dm", res
    assert "du:100,0" in seen, f"the press must be released even when the moves fail: {seen}"
    checks += 1

    # A refused press is fatal for the step, and must not leave the pointer on the press point.
    seen, res = drive(cmd_drag, dns, _FakeCliclick(reject={"dd"}, message="not permitted"))
    assert not res["ok"] and seen[-1] == "m:700,400", (seen, res)
    checks += 1

    # …and if the SAVE fails there is nothing to restore TO, so the restore is skipped entirely. That
    # is this fix silently not running, so it must be visible: `pointer_restored: false` (plus the
    # stderr line _restore_pointer prints), never an unqualified ok.
    for fn, ns in ((cmd_scroll, N(x=10, y=20, dy=-400)), (cmd_drag, dns)):
        seen, res = drive(fn, ns, _FakeCliclick(reject={"p"}, message="no accessibility grant"))
        assert res["pointer_restored"] is False, (fn.__name__, res)
        assert not any(s.startswith("m:7") for s in seen), (fn.__name__, seen)
        checks += 1

    return _emit(selftest="ok", checks=checks)


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
    s.add_argument("--pid", type=int, default=0, help="the app's pid; enables the occlusion-proof -l <window> shot")
    s.add_argument("--shot", default="", help="capture the window to this path ATOMICALLY once size is confirmed")
    s.set_defaults(fn=cmd_present)

    s = sub.add_parser("click"); s.add_argument("x", type=int); s.add_argument("y", type=int)
    s.set_defaults(fn=cmd_click)

    s = sub.add_parser("type"); s.add_argument("text"); s.set_defaults(fn=cmd_type)

    s = sub.add_parser("scroll"); s.add_argument("x", type=int); s.add_argument("y", type=int)
    s.add_argument("dy", type=int); s.set_defaults(fn=cmd_scroll)

    s = sub.add_parser("hover"); s.add_argument("x", type=int); s.add_argument("y", type=int)
    s.set_defaults(fn=cmd_hover)

    # `drag` and `swipe` are the SAME implementation (cmd_drag) under two names: a swipe is just a
    # fast drag, so the only thing that differs is the default duration. One code path, so a fix to
    # the press/move/release sequence can never apply to one gesture and not the other.
    for gesture, default_duration, default_steps in (("drag", 0.35, 10), ("swipe", 0.12, 6)):
        s = sub.add_parser(gesture)
        s.add_argument("x1", type=int); s.add_argument("y1", type=int)
        s.add_argument("x2", type=int); s.add_argument("y2", type=int)
        s.add_argument("--steps", type=int, default=default_steps,
                       help="intermediate move events (floored at 2; 1 would be a click)")
        s.add_argument("--duration", type=float, default=default_duration,
                       help="seconds spread over the moves; a FLOOR, see the emitted `elapsed`")
        s.set_defaults(fn=cmd_drag, gesture=gesture)

    s = sub.add_parser("shot"); s.add_argument("out"); s.add_argument("--window", type=int, default=0)
    s.add_argument("--rect", default="", help="x,y,w,h region capture (no-pyobjc fallback)")
    s.set_defaults(fn=cmd_shot)

    s = sub.add_parser("stop"); s.add_argument("pid", type=int); s.set_defaults(fn=cmd_stop)

    # Runs anywhere (the OS seam is stubbed), so it is both a dev-machine pre-flight and the first
    # thing to run ON a guest when a scenario's frames come back looking un-driven.
    s = sub.add_parser("selftest", help="assert the interaction verbs with the OS seam stubbed")
    s.set_defaults(fn=cmd_selftest)

    a = p.parse_args(argv)
    try:
        return a.fn(a)
    except Exception as e:  # never crash the SSH call without a parseable result
        return _emit(ok=False, error=f"{type(e).__name__}: {e}")


if __name__ == "__main__":
    sys.exit(main())
