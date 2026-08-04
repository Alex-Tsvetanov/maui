#!/usr/bin/env python3
"""Windows guest agent for the E2E visual-comparison runner.

The Windows sibling of vm_agent_macos.py, exposing the SAME subcommands so the host orchestrator
(run_comparison.py) never changes — the per-OS seam that file's header describes:

    set-resolution | clean | launch | window-id | present | click | type | scroll |
    hover | drag | swipe | shot | stop

Each subcommand prints ONE JSON line to stdout: {"ok": bool, ...}. The host parses the last line.

DEPENDENCIES: NONE beyond CPython 3.11+. Everything is ctypes against user32/gdi32 plus stdlib. This
is a deliberate departure from the macOS agent (which needs `brew install cliclick displayplacer` and
optionally pyobjc): every primitive the runner needs is in the Win32 API, so there is nothing to
provision on the guest but Python itself. In particular screenshots are encoded by the pure-stdlib
PNG writer below (zlib + struct), so Pillow is NOT required.

THREE WINDOWS-SPECIFIC HAZARDS this agent handles up front, because each one silently produces
plausible-but-wrong captures (the failure mode the macOS agent's header warns about at length):

1. DPI VIRTUALISATION. A DPI-unaware process is lied to by Windows: GetWindowRect returns scaled
   logical pixels, captures come back stretched/blurry, and SetWindowPos silently lands somewhere
   else. On a VM whose scaling is 100% you would never notice — until the display scale changes and
   every column shifts. So the agent declares PER_MONITOR_AWARE_V2 at import (see _set_dpi_aware);
   from then on every coordinate in this file is a PHYSICAL pixel, matching the capture.

2. OCCLUSION. BitBlt-from-screen photographs whatever is on top of the rect — the exact bug that put
   the C++ gallery's window into the MAUI column on macOS. PrintWindow(PW_RENDERFULLCONTENT) instead
   asks the window to render ITSELF into our DC, so another window cannot composite in and the target
   does not even need focus. That is the Win32 analogue of `screencapture -l <id>` and it is the
   default path in cmd_shot.

3. WINDOW ANIMATION / LATE LAYOUT. A freshly created window reports its final rect before it has
   actually painted (and WinUI 3 inflates its XAML tree asynchronously). Both cmd_window_id and
   cmd_present therefore POLL for a stable, non-degenerate rect rather than trusting the first read,
   and cmd_present re-asserts position+size until the window actually reaches the target — mirroring
   the macOS agent's patient retry loop, which exists because ~800 short frames were once banked.
"""
from __future__ import annotations

import argparse
import contextlib
import ctypes
import io
import json
import os
import shutil
import socket
import struct
import subprocess
import sys
import time
import zlib
from ctypes import wintypes

# ---------------------------------------------------------------- Win32 plumbing

# Loaded only on Windows, so this module still IMPORTS on the macOS/Linux dev machine — which is what
# lets the pure helpers (_write_png, _bgra_to_rgb_rows) be unit-tested off-guest by
# tests/test_vm_agent_windows.py. Any Win32 call on a non-Windows host raises AttributeError on None
# and is reported as a normal {"ok": false} error line rather than crashing the SSH call.
_IS_WINDOWS = sys.platform == "win32"
user32 = ctypes.WinDLL("user32", use_last_error=True) if _IS_WINDOWS else None
gdi32 = ctypes.WinDLL("gdi32", use_last_error=True) if _IS_WINDOWS else None
kernel32 = ctypes.WinDLL("kernel32", use_last_error=True) if _IS_WINDOWS else None

# Window styles / show commands / SetWindowPos flags we use.
GWL_STYLE = -16
WS_VISIBLE = 0x10000000
WS_CHILD = 0x40000000
SW_RESTORE = 9
SW_SHOW = 5
HWND_TOP = 0
SWP_SHOWWINDOW = 0x0040
SWP_NOZORDER = 0x0004
SWP_NOACTIVATE = 0x0010
SWP_FRAMECHANGED = 0x0020

# PrintWindow: render the FULL window content including DirectComposition/WinUI surfaces. Without this
# flag PrintWindow returns a blank/partial bitmap for composition-rendered (WinUI 3) windows.
PW_RENDERFULLCONTENT = 0x00000002

# SendInput
INPUT_MOUSE, INPUT_KEYBOARD = 0, 1
MOUSEEVENTF_MOVE = 0x0001
MOUSEEVENTF_ABSOLUTE = 0x8000
MOUSEEVENTF_LEFTDOWN = 0x0002
MOUSEEVENTF_LEFTUP = 0x0004
MOUSEEVENTF_WHEEL = 0x0800
KEYEVENTF_UNICODE = 0x0004
KEYEVENTF_KEYUP = 0x0002
WHEEL_DELTA = 120

# ChangeDisplaySettingsEx
ENUM_CURRENT_SETTINGS = -1
CDS_UPDATEREGISTRY = 0x00000001
DISP_CHANGE_SUCCESSFUL = 0
DM_PELSWIDTH = 0x00080000
DM_PELSHEIGHT = 0x00100000
DM_BITSPERPEL = 0x00040000

# One wheel notch is 120 units; scenarios express scroll in PIXELS (the macOS agent posts a pixel
# scroll event), so convert with this ratio and keep the sign. Override if a scenario needs finer
# granularity. Documented in README_e2e.md's Windows section.
SCROLL_PIXELS_PER_NOTCH = int(os.environ.get("MAUI_E2E_SCROLL_PX_PER_NOTCH", "120"))


class MOUSEINPUT(ctypes.Structure):
    _fields_ = [("dx", wintypes.LONG), ("dy", wintypes.LONG), ("mouseData", wintypes.DWORD),
                ("dwFlags", wintypes.DWORD), ("time", wintypes.DWORD),
                ("dwExtraInfo", ctypes.POINTER(ctypes.c_ulong))]


class KEYBDINPUT(ctypes.Structure):
    _fields_ = [("wVk", wintypes.WORD), ("wScan", wintypes.WORD), ("dwFlags", wintypes.DWORD),
                ("time", wintypes.DWORD), ("dwExtraInfo", ctypes.POINTER(ctypes.c_ulong))]


class _INPUTunion(ctypes.Union):
    _fields_ = [("mi", MOUSEINPUT), ("ki", KEYBDINPUT)]


class INPUT(ctypes.Structure):
    _fields_ = [("type", wintypes.DWORD), ("u", _INPUTunion)]


class BITMAPINFOHEADER(ctypes.Structure):
    _fields_ = [("biSize", wintypes.DWORD), ("biWidth", wintypes.LONG), ("biHeight", wintypes.LONG),
                ("biPlanes", wintypes.WORD), ("biBitCount", wintypes.WORD),
                ("biCompression", wintypes.DWORD), ("biSizeImage", wintypes.DWORD),
                ("biXPelsPerMeter", wintypes.LONG), ("biYPelsPerMeter", wintypes.LONG),
                ("biClrUsed", wintypes.DWORD), ("biClrImportant", wintypes.DWORD)]


class BITMAPINFO(ctypes.Structure):
    _fields_ = [("bmiHeader", BITMAPINFOHEADER), ("bmiColors", wintypes.DWORD * 3)]


class DEVMODEW(ctypes.Structure):
    _fields_ = [("dmDeviceName", wintypes.WCHAR * 32), ("dmSpecVersion", wintypes.WORD),
                ("dmDriverVersion", wintypes.WORD), ("dmSize", wintypes.WORD),
                ("dmDriverExtra", wintypes.WORD), ("dmFields", wintypes.DWORD),
                ("dmPositionX", wintypes.LONG), ("dmPositionY", wintypes.LONG),
                ("dmDisplayOrientation", wintypes.DWORD), ("dmDisplayFixedOutput", wintypes.DWORD),
                ("dmColor", wintypes.SHORT), ("dmDuplex", wintypes.SHORT),
                ("dmYResolution", wintypes.SHORT), ("dmTTOption", wintypes.SHORT),
                ("dmCollate", wintypes.SHORT), ("dmFormName", wintypes.WCHAR * 32),
                ("dmLogPixels", wintypes.WORD), ("dmBitsPerPel", wintypes.DWORD),
                ("dmPelsWidth", wintypes.DWORD), ("dmPelsHeight", wintypes.DWORD),
                ("dmDisplayFlags", wintypes.DWORD), ("dmDisplayFrequency", wintypes.DWORD),
                ("dmICMMethod", wintypes.DWORD), ("dmICMIntent", wintypes.DWORD),
                ("dmMediaType", wintypes.DWORD), ("dmDitherType", wintypes.DWORD),
                ("dmReserved1", wintypes.DWORD), ("dmReserved2", wintypes.DWORD),
                ("dmPanningWidth", wintypes.DWORD), ("dmPanningHeight", wintypes.DWORD)]


def _declare_prototypes() -> None:
    """Declare argtypes/restypes for every Win32 call used here.

    NOT optional hygiene -- required for correctness on x64. Without a restype, ctypes assumes the return
    is a C **int**, so a 64-bit handle (HDC, HBITMAP) is TRUNCATED to 32 bits; and without argtypes, a
    handle passed back in as a large Python int is marshalled as c_int and raises
    "OverflowError: int too long to convert". Both are value-dependent, so they appear INTERMITTENTLY --
    fine while the OS hands out small handles, failing once it does not. Observed exactly that way: a
    multi-page run captured most frames and failed a few with `ArgumentError: argument 1: OverflowError`
    from CreateDIBSection's handle flowing into SelectObject."""
    if not _IS_WINDOWS:
        return
    hwnd, hdc, hgdi = wintypes.HWND, wintypes.HDC, ctypes.c_void_p
    bo, ui, dw, ci, lo = wintypes.BOOL, wintypes.UINT, wintypes.DWORD, ctypes.c_int, wintypes.LONG

    for fn, argtypes, restype in (
        (user32.GetWindowDC, [hwnd], hdc),
        (user32.GetDC, [hwnd], hdc),
        (user32.ReleaseDC, [hwnd, hdc], ci),
        (user32.PrintWindow, [hwnd, hdc, ui], bo),
        (user32.GetWindowRect, [hwnd, ctypes.POINTER(wintypes.RECT)], bo),
        (user32.IsWindowVisible, [hwnd], bo),
        (user32.GetWindowLongW, [hwnd, ci], lo),
        (user32.GetWindowThreadProcessId, [hwnd, ctypes.POINTER(dw)], dw),
        (user32.SetWindowPos, [hwnd, hwnd, ci, ci, ci, ci, ui], bo),
        (user32.ShowWindow, [hwnd, ci], bo),
        (user32.SetForegroundWindow, [hwnd], bo),
        (user32.GetForegroundWindow, [], hwnd),
        (user32.GetShellWindow, [], hwnd),
        (user32.SetCursorPos, [ci, ci], bo),
        (user32.GetCursorPos, [ctypes.POINTER(wintypes.POINT)], bo),
        (user32.GetSystemMetrics, [ci], ci),
        (gdi32.CreateCompatibleDC, [hdc], hdc),
        (gdi32.CreateDIBSection, [hdc, ctypes.c_void_p, ui, ctypes.POINTER(ctypes.c_void_p),
                                  ctypes.c_void_p, dw], hgdi),
        (gdi32.SelectObject, [hdc, hgdi], hgdi),
        (gdi32.DeleteObject, [hgdi], bo),
        (gdi32.DeleteDC, [hdc], bo),
        (gdi32.BitBlt, [hdc, ci, ci, ci, ci, hdc, ci, ci, dw], bo),
        (kernel32.ProcessIdToSessionId, [dw, ctypes.POINTER(dw)], bo),
        (kernel32.GetCurrentProcessId, [], dw),
    ):
        fn.argtypes = argtypes
        fn.restype = restype


def _set_dpi_aware() -> str:
    """Opt this process into PER_MONITOR_AWARE_V2 so every rect/coordinate is a PHYSICAL pixel.

    Hazard 1 in the module header. Must run before any window/metric call. Newest API first, then the
    Win8.1 and Vista fallbacks, so the agent still behaves on an older guest."""
    try:  # Win10 1703+
        user32.SetProcessDpiAwarenessContext.argtypes = [ctypes.c_void_p]
        if user32.SetProcessDpiAwarenessContext(ctypes.c_void_p(-4)):  # PER_MONITOR_AWARE_V2
            return "per-monitor-v2"
    except AttributeError:
        pass
    try:  # Win8.1+
        ctypes.WinDLL("shcore").SetProcessDpiAwareness(2)  # PROCESS_PER_MONITOR_DPI_AWARE
        return "per-monitor"
    except Exception:
        pass
    try:
        user32.SetProcessDPIAware()
        return "system"
    except Exception:
        return "none"


_declare_prototypes()
DPI_MODE = _set_dpi_aware()


def _emit(**kw) -> int:
    kw.setdefault("ok", True)
    print(json.dumps(kw))
    return 0 if kw["ok"] else 1


# ---------------------------------------------------------------- stdlib PNG writer


def _write_png(path: str, width: int, height: int, rgb_rows: list[bytes]) -> None:
    """Write 8-bit RGB PNG with zlib + struct only — so the guest needs no Pillow.

    Each scanline is prefixed with filter byte 0 (None), which is what the spec calls for when we do
    no filtering; zlib handles the actual compression."""
    raw = b"".join(b"\x00" + row for row in rgb_rows)

    def chunk(tag: bytes, data: bytes) -> bytes:
        return (struct.pack(">I", len(data)) + tag + data
                + struct.pack(">I", zlib.crc32(tag + data) & 0xFFFFFFFF))

    ihdr = struct.pack(">IIBBBBB", width, height, 8, 2, 0, 0, 0)  # 8bpc, colour type 2 = RGB
    with open(path, "wb") as f:
        f.write(b"\x89PNG\r\n\x1a\n")
        f.write(chunk(b"IHDR", ihdr))
        f.write(chunk(b"IDAT", zlib.compress(raw, 6)))
        f.write(chunk(b"IEND", b""))


def _bgra_to_rgb_rows(buf: bytes, w: int, h: int) -> list[bytes]:
    """Split a top-down 32bpp BGRA DIB buffer into per-row RGB bytes for the PNG writer.

    Done with slice operations rather than a per-pixel loop: a 1024x800 shot is 819k pixels, and the
    naive triple-index comprehension took seconds per capture — multiplied by every step of every
    scenario in every column, that is the difference between a usable runner and an unusable one."""
    stride = w * 4
    rows: list[bytes] = []
    for y in range(h):
        ba = bytearray(buf[y * stride:(y + 1) * stride])
        del ba[3::4]                                  # drop alpha  -> B G R  B G R ...
        ba[0::3], ba[2::3] = ba[2::3], ba[0::3]       # swap B <-> R -> R G B  R G B ...
        rows.append(bytes(ba))
    return rows


def _capture_hwnd(hwnd: int, out: str) -> tuple[bool, str, list[int]]:
    """Capture a window's OWN backing store via PrintWindow(PW_RENDERFULLCONTENT) -> PNG.

    Hazard 2 in the module header: this cannot be occluded and needs no focus. Returns
    (ok, error, [w, h]). The DIB is created top-down (negative biHeight) so its rows are already in
    PNG order, and 32bpp BGRA is sliced to RGB per row."""
    rect = wintypes.RECT()
    if not user32.GetWindowRect(wintypes.HWND(hwnd), ctypes.byref(rect)):
        return False, "GetWindowRect failed", [0, 0]
    w, h = rect.right - rect.left, rect.bottom - rect.top
    if w <= 0 or h <= 0:
        return False, f"degenerate window rect {w}x{h}", [w, h]

    hdc_win = user32.GetWindowDC(wintypes.HWND(hwnd))
    if not hdc_win:
        return False, "GetWindowDC failed", [w, h]
    hdc_mem = gdi32.CreateCompatibleDC(hdc_win)
    bmi = BITMAPINFO()
    bmi.bmiHeader.biSize = ctypes.sizeof(BITMAPINFOHEADER)
    bmi.bmiHeader.biWidth = w
    bmi.bmiHeader.biHeight = -h  # top-down rows
    bmi.bmiHeader.biPlanes = 1
    bmi.bmiHeader.biBitCount = 32
    bmi.bmiHeader.biCompression = 0  # BI_RGB
    bits = ctypes.c_void_p()
    gdi32.CreateDIBSection.restype = wintypes.HBITMAP
    hbm = gdi32.CreateDIBSection(hdc_win, ctypes.byref(bmi), 0, ctypes.byref(bits), None, 0)
    try:
        if not hbm or not bits:
            return False, "CreateDIBSection failed", [w, h]
        gdi32.SelectObject(hdc_mem, hbm)
        user32.PrintWindow.argtypes = [wintypes.HWND, wintypes.HDC, wintypes.UINT]
        ok = bool(user32.PrintWindow(wintypes.HWND(hwnd), hdc_mem, PW_RENDERFULLCONTENT))
        if not ok:
            # Some windows refuse PW_RENDERFULLCONTENT; retry plain PrintWindow before giving up.
            ok = bool(user32.PrintWindow(wintypes.HWND(hwnd), hdc_mem, 0))
        if not ok:
            return False, f"PrintWindow failed (err {ctypes.get_last_error()})", [w, h]

        stride = w * 4
        buf = ctypes.string_at(bits, stride * h)
        rows = _bgra_to_rgb_rows(buf, w, h)
        os.makedirs(os.path.dirname(out) or ".", exist_ok=True)
        _write_png(out, w, h, rows)
        return True, "", [w, h]
    finally:
        if hbm:
            gdi32.DeleteObject(hbm)
        gdi32.DeleteDC(hdc_mem)
        user32.ReleaseDC(wintypes.HWND(hwnd), hdc_win)


# ---------------------------------------------------------------- window discovery


def _windows_of_pid(pid: int) -> list[tuple[int, list[int]]]:
    """Top-level visible windows owned by `pid`, as (hwnd, [x, y, w, h]), largest first.

    Mirrors the macOS agent's "largest normal-layer window" heuristic: a real app can own invisible
    helper/message-only windows, and WinUI 3 in particular creates extra composition host windows, so
    picking the biggest visible non-child one is what reliably lands on the app's main window."""
    found: list[tuple[int, list[int]]] = []

    @ctypes.WINFUNCTYPE(wintypes.BOOL, wintypes.HWND, wintypes.LPARAM)
    def _cb(hwnd, _lparam):
        owner = wintypes.DWORD()
        user32.GetWindowThreadProcessId(hwnd, ctypes.byref(owner))
        if owner.value != pid:
            return True
        if not user32.IsWindowVisible(hwnd):
            return True
        style = user32.GetWindowLongW(hwnd, GWL_STYLE)
        if style & WS_CHILD:
            return True  # child windows are not the app window
        r = wintypes.RECT()
        if not user32.GetWindowRect(hwnd, ctypes.byref(r)):
            return True
        w, h = r.right - r.left, r.bottom - r.top
        if w > 0 and h > 0:
            found.append((int(hwnd), [r.left, r.top, w, h]))
        return True

    user32.EnumWindows(_cb, 0)
    found.sort(key=lambda t: t[1][2] * t[1][3], reverse=True)
    return found


# ---------------------------------------------------------------- subcommands


def cmd_set_resolution(a) -> int:
    """Set the primary display mode to WxH via ChangeDisplaySettingsExW.

    Mirrors the macOS same-width fallback: if the exact mode is unavailable, take any mode with the
    requested WIDTH (the scenario x-calibration only needs the width) whose height is closest, and
    report the ACTUAL mode so the caller can trust the geometry."""
    modes: list[tuple[int, int]] = []
    dm = DEVMODEW()
    dm.dmSize = ctypes.sizeof(DEVMODEW)
    i = 0
    while user32.EnumDisplaySettingsW(None, i, ctypes.byref(dm)):
        modes.append((int(dm.dmPelsWidth), int(dm.dmPelsHeight)))
        i += 1
    if not modes:
        return _emit(ok=False, error="EnumDisplaySettingsW returned no modes")

    want = (a.width, a.height)
    if want in modes:
        target = want
    else:
        same_w = sorted((h for (w, h) in modes if w == a.width), key=lambda h: abs(h - a.height))
        if not same_w:
            return _emit(ok=False, error=f"no {a.width}-wide mode available",
                         available=sorted(set(modes))[:40])
        target = (a.width, same_w[0])

    cur = DEVMODEW()
    cur.dmSize = ctypes.sizeof(DEVMODEW)
    user32.EnumDisplaySettingsW(None, ENUM_CURRENT_SETTINGS, ctypes.byref(cur))
    if (int(cur.dmPelsWidth), int(cur.dmPelsHeight)) != target:
        new = DEVMODEW()
        new.dmSize = ctypes.sizeof(DEVMODEW)
        new.dmPelsWidth, new.dmPelsHeight = target
        new.dmBitsPerPel = 32
        new.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT | DM_BITSPERPEL
        rc = user32.ChangeDisplaySettingsExW(None, ctypes.byref(new), None, CDS_UPDATEREGISTRY, None)
        if rc != DISP_CHANGE_SUCCESSFUL:
            # Most often this means the caller is NOT in the interactive session. A display mode belongs
            # to a session: from session 0 (where sshd lives) there is no console display to change, and
            # ChangeDisplaySettingsExW returns DISP_CHANGE_FAILED (-1) even for a mode the driver
            # enumerates. Measured on Windows 11 ARM64/UTM: identical calls failed from session 0 and
            # SUCCEEDED from session 1 (1280x800 applied and read back), with the SPICE agent running in
            # both cases. So a failure here is FIRST a hint to check the session (see session1.py), and
            # only then a genuine driver limitation — some display-only drivers really do refuse.
            #
            # This is reported as ok=True with driver_refused=True, deliberately. Failing here would abort
            # every run on such a guest at the very first step, and it does NOT need to: this agent
            # captures each window with PrintWindow (the window's OWN backing store), so the screen mode
            # only has to be LARGE ENOUGH to contain the presented window — it does not have to equal the
            # requested size. The caller gets `actual` to size its geometry against, plus `fits_request`
            # so a genuinely-too-small screen is still visible. A real mode-list miss (the branch above)
            # remains a hard error.
            back_fail = DEVMODEW()
            back_fail.dmSize = ctypes.sizeof(DEVMODEW)
            user32.EnumDisplaySettingsW(None, ENUM_CURRENT_SETTINGS, ctypes.byref(back_fail))
            actual_fail = [int(back_fail.dmPelsWidth), int(back_fail.dmPelsHeight)]
            return _emit(ok=True, driver_refused=True, change_result=rc,
                         error=f"ChangeDisplaySettingsExW returned {rc}; the usual cause is calling "
                               f"from a non-interactive session (a display mode belongs to a session) "
                               f"- run the agent in session 1, see session1.py",
                         requested=[a.width, a.height], set=list(target), actual=actual_fail,
                         fits_request=actual_fail[0] >= a.width and actual_fail[1] >= a.height,
                         dpi_mode=DPI_MODE)
        time.sleep(1.0)  # let the mode change settle before anything reads geometry

    back = DEVMODEW()
    back.dmSize = ctypes.sizeof(DEVMODEW)
    user32.EnumDisplaySettingsW(None, ENUM_CURRENT_SETTINGS, ctypes.byref(back))
    actual = [int(back.dmPelsWidth), int(back.dmPelsHeight)]
    return _emit(ok=actual == list(target), requested=[a.width, a.height], set=list(target),
                 actual=actual, dpi_mode=DPI_MODE)


def cmd_clean(a) -> int:
    shutil.rmtree(a.dir, ignore_errors=True)
    os.makedirs(a.dir, exist_ok=True)
    return _emit(dir=a.dir)


def cmd_launch(a) -> int:
    """Start the app and return its pid.

    Simpler than the macOS twin: CreateProcess hands us the pid directly, so there is no pgrep
    before/after diffing and no `open -n` new-instance dance. `--bundle` is the .exe path (a directory
    is accepted and searched for <proc>). Env vars are passed as K=V, same as macOS."""
    exe = a.bundle
    if os.path.isdir(exe):
        cand = os.path.join(exe, a.proc)
        exe = cand if os.path.isfile(cand) else os.path.join(exe, a.proc + ".exe")
    if not os.path.isfile(exe):
        return _emit(ok=False, error=f"executable not found: {exe}")

    env = os.environ.copy()
    for kv in a.env or []:
        k, _, v = kv.partition("=")
        env[k] = v
    try:
        # cwd = the exe's dir so side-by-side runtime DLLs (Windows App SDK, vcruntime) resolve, which
        # is how an unpackaged WinUI 3 app finds its bootstrapper without being installed.
        proc = subprocess.Popen([exe], env=env, cwd=os.path.dirname(exe) or ".",
                                creationflags=0x00000008)  # DETACHED_PROCESS: survive this SSH call
    except OSError as e:
        return _emit(ok=False, error=f"launch failed: {e}")

    # Wait for the process to actually own a visible top-level window; a pid alone does not mean the
    # UI exists yet (hazard 3), and every later step assumes a window.
    for _ in range(60):
        if _windows_of_pid(proc.pid):
            return _emit(pid=proc.pid, exe=exe)
        if proc.poll() is not None:
            return _emit(ok=False, error=f"process exited early with code {proc.returncode}", exe=exe)
        time.sleep(0.25)
    return _emit(pid=proc.pid, exe=exe, warning="no visible window yet after 15s")


def cmd_window_id(a) -> int:
    """The pid's main window HWND + bounds. `id` is an HWND (the host treats it opaquely, exactly as
    it treats a macOS CGWindowID) so `shot --window <id>` is the occlusion-proof path."""
    for _ in range(a.retries):
        wins = _windows_of_pid(a.pid)
        if wins:
            hwnd, bounds = wins[0]
            return _emit(id=hwnd, bounds=bounds)
        time.sleep(a.delay)
    return _emit(ok=False, error=f"no visible top-level window for pid {a.pid}")


def _defocus_before_shot(hwnd: int) -> tuple[bool, bool]:
    """Hand OS foreground to the desktop shell so `hwnd` is the INACTIVE window at the instant we
    photograph it — an attempt to suppress WinUI's keyboard-focus visual (a thin near-black outline WinUI
    paints around whichever control is Tab-order-first at launch) without sending the app a single click,
    keystroke, or scroll. Returns (requested_ok, verified) — see the two levels of "did it work" below.

    WHY THIS EXISTS. PARITY_REVIEW.md documents ~24 pages whose ONLY diff vs the MAUI reference is this
    focus outline, present on some captures and absent on others with the SAME code+binary — a run-to-run
    reference-side coin flip that straddles the SSIM gate (up to ~12 pages of green-count noise per pass).
    Its toggling was observed in LOCKSTEP across unrelated pages at consistent run boundaries — the
    signature of a session-wide condition, not a per-page one. `cmd_present` above already calls
    `SetForegroundWindow(hwnd)` in its geometry-settle loop but never checks its return value; Windows can
    silently refuse a foreground-steal request (the foreground-lock timeout), which would leave the window
    INACTIVE on some runs and ACTIVE on others — exactly this toggle. Rather than depend on that call
    succeeding, this makes the end state explicit: deactivate on purpose, every time, right before the
    shot, so the outcome is asserted instead of inherited.

    EFFICACY IS NOT PROVEN, ONLY MEASURED PER-CALL. The brief's mechanism is FocusState::Keyboard vs
    ::Pointer, a XAML-level concept this Win32-level call does not directly touch; this function's actual
    lever is window ACTIVATION, a plausible but distinct trigger for the same visual (WinUI commonly ties
    focus-visual and other "active" chrome to WM_ACTIVATE) that cannot be confirmed without a Windows box.
    Two things ARE knowable from ctypes alone and are both returned so the runner can log them per frame
    instead of trusting silently: `requested_ok` is SetForegroundWindow's own BOOL (it can lie -- return
    TRUE without the switch landing); `verified` is GetForegroundWindow() != hwnd read back afterwards,
    which is the real check. If `SetForegroundWindow(hwnd)` above is itself being silently refused by the
    same OS-level foreground-lock this theory blames, `SetForegroundWindow(shell)` here is the SAME API
    under a plausibly-but-not-provably different exemption (deactivating TO the shell is not "stealing"
    the user's attention the way activating an arbitrary app is) — cannot be ruled out from macOS, which
    is exactly why this call's own success is checked and reported rather than assumed.

    WHY THIS IS SAFE REGARDLESS OF EFFICACY. Deactivation is a window-manager Z/activation change
    (WM_ACTIVATE), not an input event — nothing is posted into the app's message queue, so no control can
    be clicked, no page can scroll, no dialog can dismiss, whether or not the call actually lands.
    `_capture_hwnd` uses PrintWindow(PW_RENDERFULLCONTENT), which (module docstring hazard 2) reads the
    window's own composed backing store and needs neither focus nor Z-order — an inactive, non-topmost
    window still captures its full content. This is why activation, not a click, was chosen: it is the one
    lever in the brief's candidate list that cannot mutate content even when the underlying theory is wrong.

    Applied unconditionally from `cmd_present`'s single --shot call site, so every column (maui_xaml, cpp,
    cpp_xaml) is deactivated identically before every frame — this must never be reference-only, or the
    comparison itself becomes asymmetric. `--no-defocus` exists only to debug this behavior on the guest.
    """
    shell = user32.GetShellWindow()
    if not shell:
        return False, False  # no desktop shell window found (unusual); nothing to hand foreground to
    requested_ok = bool(user32.SetForegroundWindow(wintypes.HWND(shell)))
    # 0.15s is an UNCALIBRATED guess at the WM_ACTIVATE-driven repaint (chrome + focus-visual removal)
    # landing before PrintWindow reads the backing store -- there is no guest to time this against from
    # here. If the first real run's captures still show the focus band despite `verified=True`, widen
    # this before suspecting the mechanism itself.
    time.sleep(0.15)
    verified = user32.GetForegroundWindow() != hwnd
    return requested_ok, verified


def cmd_present(a) -> int:
    """Foreground the window and force it to an EXPLICIT position+size so every column captures at the
    SAME rect; optionally capture atomically via --shot.

    The macOS twin needs the atomic --shot because any intervening AX call steals key focus and greys
    the traffic lights. On Windows PrintWindow does not care about focus, but --shot is still honoured
    (and preferred) because it removes a whole SSH round-trip between sizing and capture — during
    which a WinUI 3 window can still be settling.

    Like the macOS twin this FAILS LOUDLY if the window never reaches the target size rather than
    returning a short rect as success: a silently short frame is the failure that gets scored.

    Unless --no-defocus is passed, the window is deliberately made INACTIVE right before the --shot
    capture (see _defocus_before_shot) in an attempt to make WinUI's keyboard-focus visual
    deterministically absent from every column's frame, instead of a run-to-run coin flip on the MAUI
    reference column alone. The result is reported as `defocused`/`defocus_verified` rather than assumed
    -- see _defocus_before_shot's docstring for why efficacy is a measurement here, not a guarantee."""
    wins = _windows_of_pid(a.pid) if a.pid else []
    if not wins:
        # Fall back to matching by process image name, so --proc alone still works.
        pids = _pids_for_image(a.proc)
        for p in pids:
            wins = _windows_of_pid(p)
            if wins:
                break
    if not wins:
        return _emit(ok=False, proc=a.proc, error="no window to present")
    hwnd = wins[0][0]

    x, y, w, h = a.x, a.y, a.w, a.h
    rect = [0, 0, 0, 0]
    for _ in range(20):  # patient loop, same rationale as the macOS agent's 20x0.4s
        user32.ShowWindow(wintypes.HWND(hwnd), SW_RESTORE)
        user32.SetForegroundWindow(wintypes.HWND(hwnd))
        user32.SetWindowPos(wintypes.HWND(hwnd), wintypes.HWND(HWND_TOP), x, y, w, h, SWP_SHOWWINDOW)
        time.sleep(0.4)
        r = wintypes.RECT()
        user32.GetWindowRect(wintypes.HWND(hwnd), ctypes.byref(r))
        rect = [r.left, r.top, r.right - r.left, r.bottom - r.top]
        if rect[2] >= w - 5 and rect[3] >= h - 5:
            break
    if rect[2] < w - 5 or rect[3] < h - 5:
        return _emit(ok=False, proc=a.proc, id=hwnd, bounds=rect,
                     error=f"window did not reach target: got {rect[2]}x{rect[3]}, want {w}x{h}")

    shot_info: dict = {}
    if a.shot:
        if a.defocus:
            # Reported, not assumed -- see _defocus_before_shot's docstring on why this call's own
            # success is measured per-frame rather than trusted. The host (run_comparison.py) currently
            # only logs these via the printed JSON; nothing downstream depends on their values yet.
            requested_ok, verified = _defocus_before_shot(hwnd)
            shot_info["defocused"] = requested_ok
            shot_info["defocus_verified"] = verified
        ok, err, size = _capture_hwnd(hwnd, a.shot)
        if not ok:
            return _emit(ok=False, proc=a.proc, id=hwnd, bounds=rect, error=f"shot failed: {err}")
        shot_info["shot"] = a.shot
        shot_info["shot_size"] = size
    # `window` is REQUIRED by the shared host helper (run_comparison.shoot_presented checks
    # rect+window+shot); the macOS agent emits it as the CGWindowID. Emitting only `id` made every
    # present look like a failure: the host retried 3x per frame and recorded window_bounds=null, while
    # the captures themselves were fine. Emit BOTH -- `window` for the shared contract, `id` to match
    # this agent's own window-id subcommand.
    return _emit(proc=a.proc, window=hwnd, id=hwnd, bounds=rect, rect=",".join(map(str, rect)),
                 **shot_info)


def _pids_for_image(image: str) -> list[int]:
    """Pids whose image name matches (with or without .exe), via tasklist — no extra deps."""
    if not image:
        return []
    names = {image.lower(), (image + ".exe").lower()}
    out = subprocess.run(["tasklist", "/fo", "csv", "/nh"], capture_output=True, text=True).stdout
    pids: list[int] = []
    for line in out.splitlines():
        parts = [p.strip('" ') for p in line.split('","')]
        if len(parts) >= 2 and parts[0].lower() in names:
            try:
                pids.append(int(parts[1]))
            except ValueError:
                pass
    return pids


def _send(*inputs: INPUT) -> bool:
    n = len(inputs)
    arr = (INPUT * n)(*inputs)
    user32.SendInput.argtypes = [wintypes.UINT, ctypes.POINTER(INPUT), ctypes.c_int]
    return user32.SendInput(n, arr, ctypes.sizeof(INPUT)) == n


# ---------------------------------------------------------------- pointer hygiene
#
# POINTER CONTAMINATION. The cursor is MACHINE-global: it outlives this agent process and the app
# relaunch between columns. A cursor left sitting on a control keeps that control in its PointerOver
# visual state for every later frame of every later PAGE, and that reads on the board as a port defect
# rather than as tooling — the same "plausible but wrong capture" class hazards 1-3 above are about.
# So every verb that moves the cursor as a MEANS TO AN END puts it back (cmd_hover is the deliberate
# exception: parking IS the gesture there).


def _cursor_pos() -> list[int] | None:
    """Where the cursor is RIGHT NOW as [x, y] physical pixels, or None.

    Guarded on _IS_WINDOWS rather than a blanket try/except so the module still imports on the dev
    machine WITHOUT making a genuine on-guest GetCursorPos failure look like "we are off-guest" — a
    real failure still surfaces, as the None that _restore_pointer reports."""
    if not _IS_WINDOWS:
        return None
    pt = wintypes.POINT()
    if not user32.GetCursorPos(ctypes.byref(pt)):
        return None
    return [int(pt.x), int(pt.y)]


def _mouse_to(x: int, y: int) -> tuple[bool, list[int] | None, str]:
    """Move the cursor to (x, y). Returns (ok, where it ACTUALLY landed, why not).

    SetCursorPos takes physical pixels (we are DPI-aware), which keeps the scenario's absolute
    coordinates identical to the ones the macOS agent feeds cliclick.

    Its BOOL used to be DISCARDED and every caller defaulted to ok=True, which hid two whole classes
    of wrong frame: a move that failed outright, and — worse because it "succeeds" — a coordinate
    outside the virtual desktop, which SetCursorPos CLAMPS to the nearest edge. The click then landed
    on some other control (or the desktop) and the run banked a perfectly plausible frame. So the
    landing point is read back and a clamp is a FAILED STEP, which the host prints; `at` carries the
    real position either way so the log shows how far off the scenario's calibration is."""
    ok = bool(user32.SetCursorPos(int(x), int(y)))
    time.sleep(0.02)
    at = _cursor_pos()
    if not ok:
        return False, at, f"SetCursorPos({int(x)}, {int(y)}) failed (err {ctypes.get_last_error()})"
    if at is not None and at != [int(x), int(y)]:
        return False, at, (f"cursor CLAMPED: asked for {[int(x), int(y)]}, landed on {at} — the "
                           f"coordinate is outside the virtual desktop, so this step hit the wrong "
                           f"place and the frame would look legitimate")
    return True, at, ""


def _restore_pointer(saved: list[int] | None) -> bool:
    """Put the cursor back where `saved` says it was. False if it could not be done — which callers
    EMIT rather than swallow: an un-restored cursor contaminates every later frame silently.

    Both failure branches say so on stderr. A missing `saved` is the sneakier one: the restore is then
    skipped ENTIRELY while the gesture still reports ok, i.e. this fix quietly not running — the very
    shape it exists to eliminate, aimed at itself."""
    if not saved:
        print("[agent] cursor NOT restored: GetCursorPos never gave a position to restore TO, so the "
              "cursor is left wherever this gesture put it", file=sys.stderr)
        return False
    ok, _at, err = _mouse_to(saved[0], saved[1])
    if not ok:
        print(f"[agent] cursor NOT restored to {saved}: {err}", file=sys.stderr)
    return ok


def cmd_click(a) -> int:
    """Click at (x, y) and PUT THE CURSOR BACK (see POINTER CONTAMINATION above).

    Windows has no cliclick `-r`, so the save/restore is explicit. The move is checked: a click whose
    cursor never reached the target presses SOMETHING ELSE, and used to report ok=True."""
    saved = _cursor_pos()
    ok, at, err = _mouse_to(a.x, a.y)
    if not ok:
        _restore_pointer(saved)
        return _emit(ok=False, x=a.x, y=a.y, at=at, error=err)
    down = INPUT(type=INPUT_MOUSE, u=_INPUTunion(mi=MOUSEINPUT(0, 0, 0, MOUSEEVENTF_LEFTDOWN, 0, None)))
    up = INPUT(type=INPUT_MOUSE, u=_INPUTunion(mi=MOUSEINPUT(0, 0, 0, MOUSEEVENTF_LEFTUP, 0, None)))
    ok = _send(down, up)
    restored = _restore_pointer(saved)
    return _emit(ok=ok, x=a.x, y=a.y, at=at, pointer_restored=restored,
                 error="" if ok else "SendInput click failed")


def cmd_type(a) -> int:
    """Type text as UNICODE scan codes (KEYEVENTF_UNICODE), so arbitrary characters work without
    worrying about the guest keyboard layout — the layout-independent equivalent of `cliclick t:`."""
    evs: list[INPUT] = []
    for ch in a.text:
        code = ord(ch)
        evs.append(INPUT(type=INPUT_KEYBOARD,
                         u=_INPUTunion(ki=KEYBDINPUT(0, code, KEYEVENTF_UNICODE, 0, None))))
        evs.append(INPUT(type=INPUT_KEYBOARD,
                         u=_INPUTunion(ki=KEYBDINPUT(0, code, KEYEVENTF_UNICODE | KEYEVENTF_KEYUP, 0, None))))
    if not evs:
        return _emit(text="")
    ok = _send(*evs)
    return _emit(ok=ok, text=a.text)


def cmd_hover(a) -> int:
    """Park the pointer at (x, y) and LEAVE it there.

    That is the whole gesture: a hover-reactive page (PointerGestureRecognizer entered/moved, a
    VisualState PointerOver, a tooltip) only changes while the pointer is inside it, so the pointer
    must still be there when the shot is taken — which is why this is the ONE verb that deliberately
    does not restore the cursor (see POINTER CONTAMINATION above; click/scroll/drag all do).

    The flip side, for scenario authors: the pointer is MACHINE-global and survives the app relaunch
    between columns, so a parked pointer bleeds into every later frame of the run. End a
    pointer-moving scenario with a hover to a neutral point (see the syntax block in
    run_comparison.py's load_scenario).

    A hover that did not land is the one thing this verb CAN get wrong, and it used to be unreportable
    (ok defaulted True): the page then shows no hover state and reads as a port defect."""
    ok, at, err = _mouse_to(a.x, a.y)
    return _emit(ok=ok, x=a.x, y=a.y, at=at, error=err)


def _drag_points(x1: int, y1: int, x2: int, y2: int, steps: int) -> list[tuple[int, int]]:
    """The drag's move points: `steps` evenly-spaced positions from just after (x1,y1) up to and
    INCLUDING (x2,y2). Pure — the runner's --selftest asserts this against the macOS agent's copy.

    Floored at 2 steps deliberately. With 1 step a "drag" is a press and a release at one point, i.e.
    a CLICK — it runs, it reports ok, and it produces a frame identical to the previous one, which is
    indistinguishable from a page that simply does not react. That is the exact failure this verb
    exists to eliminate, so a misconfigured scenario must not be able to degrade into it.

    (Duplicated verbatim in vm_agent_macos.py. Each agent is copied to a guest ALONE, so it cannot
    import a shared module — the same reason _emit and cmd_clean are duplicated between them.)"""
    steps = max(2, int(steps))
    return [(round(x1 + (x2 - x1) * i / steps), round(y1 + (y2 - y1) * i / steps))
            for i in range(1, steps + 1)]


def cmd_drag(a) -> int:
    """Press at (x1,y1), MOVE through interpolated points, release at (x2,y2). `swipe` is this same
    function with a faster default duration — one implementation, two names (see main()).

    THE INTERMEDIATE MOVES ARE THE POINT. A LEFTDOWN immediately followed by a LEFTUP is a click as
    far as XAML's pointer/manipulation pipeline is concerned: no pan, no swipe, no SwipeView reveal —
    and the resulting frame is identical to the un-driven one, which reads on the board as "the page
    does not react".

    The moves go through _mouse_to (SetCursorPos), the same primitive cmd_click already relies on to
    land its clicks: the cursor move synthesizes WM_MOUSEMOVE to the window under it, in the physical
    pixels this process is DPI-aware in (hazard 1). Known ceiling: SetCursorPos is a cursor-position
    API, not an injected input event, so a window that reads the raw input stream directly rather than
    the message queue could miss it. Upgrade path if a WinUI drag turns out not to track: send each
    move as SendInput MOUSEEVENTF_MOVE|MOUSEEVENTF_ABSOLUTE, with the coordinates normalised to
    0..65535 over the virtual screen (GetSystemMetrics SM_XVIRTUALSCREEN..SM_CYVIRTUALSCREEN).

    `duration` is a FLOOR, not a target (_mouse_to's own settle adds to it), so the measured `elapsed`
    is emitted for calibration."""
    pts = _drag_points(a.x1, a.y1, a.x2, a.y2, a.steps)
    per_step = max(0.0, float(a.duration)) / len(pts)
    started = time.time()
    down = INPUT(type=INPUT_MOUSE, u=_INPUTunion(mi=MOUSEINPUT(0, 0, 0, MOUSEEVENTF_LEFTDOWN, 0, None)))
    up = INPUT(type=INPUT_MOUSE, u=_INPUTunion(mi=MOUSEINPUT(0, 0, 0, MOUSEEVENTF_LEFTUP, 0, None)))
    saved = _cursor_pos()  # read BEFORE the press; the release below lands on the far end
    fails: list[str] = []

    # The PRESS POINT is checked, unlike before. SetCursorPos succeeds while CLAMPING (see _mouse_to),
    # so an off-surface x1,y1 pressed the desktop or the wrong control and the drag then "worked" on
    # nothing — a banked frame identical to the un-driven one, which is what this verb exists to stop.
    ok, at, err = _mouse_to(a.x1, a.y1)
    if not ok:
        _restore_pointer(saved)
        return _emit(ok=False, gesture=a.gesture, at=at, error=f"press point not reached: {err}")
    if not _send(down):
        _restore_pointer(saved)
        return _emit(ok=False, gesture=a.gesture, error="SendInput LEFTDOWN failed")
    try:
        time.sleep(min(per_step, 0.05))  # let the press register before the first move
        # The FULL list — pts[-1] IS (x2, y2), so this both interpolates and lands on the endpoint.
        for px, py in pts:
            moved_ok, _at, moved_err = _mouse_to(px, py)
            if not moved_ok:
                fails.append(moved_err)
            time.sleep(per_step)
    finally:
        # ALWAYS release. A button left down is not a dropped step, it is a wedged GUEST: the mouse
        # stays captured, every later frame of every later page is captured mid-drag, and that reads
        # on the board as a port defect rather than as tooling — the same shape as the `or bounds`
        # bug run_comparison.py documents.
        if not _send(up):
            fails.append("SendInput LEFTUP failed")
        # …and only THEN put the cursor back. Restoring before the release would drag the content all
        # the way home and undo the gesture; restoring after leaves nothing parked on the far end.
        restored = _restore_pointer(saved)
    return _emit(ok=not fails, gesture=a.gesture, points=len(pts), to=[a.x2, a.y2],
                 elapsed=round(time.time() - started, 3), pointer_restored=restored,
                 error="; ".join(fails)[:400])


def cmd_scroll(a) -> int:
    """Wheel-scroll at (x, y). `dy` is in PIXELS for parity with the macOS agent's pixel scroll event;
    it is converted to wheel notches (120 units each, see SCROLL_PIXELS_PER_NOTCH), keeping the sign
    and always moving at least one notch so a small scenario delta is never a silent no-op.

    The move AIMS the wheel event (WM_MOUSEWHEEL goes to the window under the cursor), so it has to
    be checked — a refused or clamped move scrolls whatever the cursor already happened to be over —
    and the cursor is put back afterwards so the scrolled view is not left in a hover state for every
    later frame (see POINTER CONTAMINATION above)."""
    saved = _cursor_pos()
    ok, at, err = _mouse_to(a.x, a.y)
    if not ok:
        _restore_pointer(saved)
        return _emit(ok=False, x=a.x, y=a.y, dy=a.dy, at=at,
                     error=f"the scroll would have landed wherever the cursor already was: {err}")
    notches = int(a.dy) / float(SCROLL_PIXELS_PER_NOTCH)
    amount = int(notches * WHEEL_DELTA)
    if amount == 0 and a.dy != 0:
        amount = WHEEL_DELTA if a.dy > 0 else -WHEEL_DELTA
    ev = INPUT(type=INPUT_MOUSE,
               u=_INPUTunion(mi=MOUSEINPUT(0, 0, ctypes.c_uint32(amount & 0xFFFFFFFF).value,
                                           MOUSEEVENTF_WHEEL, 0, None)))
    ok = _send(ev)
    restored = _restore_pointer(saved)
    return _emit(ok=ok, x=a.x, y=a.y, dy=a.dy, wheel=amount, at=at, pointer_restored=restored,
                 error="" if ok else "SendInput WHEEL failed")


def cmd_shot(a) -> int:
    """Capture to `out`. Prefers --window (PrintWindow, occlusion-proof); --rect is the screen-region
    fallback, which CAN photograph an overlapping window — same caveat as the macOS `-R` path."""
    os.makedirs(os.path.dirname(a.out) or ".", exist_ok=True)
    if a.window:
        ok, err, size = _capture_hwnd(int(a.window), a.out)
        return _emit(ok=ok, out=a.out, window=a.window, size=size, error=err)
    if a.rect:
        try:
            x, y, w, h = (int(v) for v in a.rect.split(","))
        except ValueError:
            return _emit(ok=False, error=f"bad --rect {a.rect!r}, want x,y,w,h")
        ok, err, size = _capture_screen_rect(x, y, w, h, a.out)
        return _emit(ok=ok, out=a.out, rect=a.rect, size=size, error=err)
    # Last resort: the whole primary display.
    w = user32.GetSystemMetrics(0)
    h = user32.GetSystemMetrics(1)
    ok, err, size = _capture_screen_rect(0, 0, w, h, a.out)
    return _emit(ok=ok, out=a.out, size=size, error=err)


def _capture_screen_rect(x: int, y: int, w: int, h: int, out: str) -> tuple[bool, str, list[int]]:
    """BitBlt a screen region -> PNG. Occlusion-prone by nature; only for the --rect fallback."""
    if w <= 0 or h <= 0:
        return False, f"degenerate rect {w}x{h}", [w, h]
    hdc_screen = user32.GetDC(None)
    hdc_mem = gdi32.CreateCompatibleDC(hdc_screen)
    bmi = BITMAPINFO()
    bmi.bmiHeader.biSize = ctypes.sizeof(BITMAPINFOHEADER)
    bmi.bmiHeader.biWidth = w
    bmi.bmiHeader.biHeight = -h
    bmi.bmiHeader.biPlanes = 1
    bmi.bmiHeader.biBitCount = 32
    bmi.bmiHeader.biCompression = 0
    bits = ctypes.c_void_p()
    gdi32.CreateDIBSection.restype = wintypes.HBITMAP
    hbm = gdi32.CreateDIBSection(hdc_screen, ctypes.byref(bmi), 0, ctypes.byref(bits), None, 0)
    try:
        if not hbm or not bits:
            return False, "CreateDIBSection failed", [w, h]
        gdi32.SelectObject(hdc_mem, hbm)
        if not gdi32.BitBlt(hdc_mem, 0, 0, w, h, hdc_screen, x, y, 0x00CC0020):  # SRCCOPY
            return False, f"BitBlt failed (err {ctypes.get_last_error()})", [w, h]
        buf = ctypes.string_at(bits, w * 4 * h)
        rows = _bgra_to_rgb_rows(buf, w, h)
        os.makedirs(os.path.dirname(out) or ".", exist_ok=True)
        _write_png(out, w, h, rows)
        return True, "", [w, h]
    finally:
        if hbm:
            gdi32.DeleteObject(hbm)
        gdi32.DeleteDC(hdc_mem)
        user32.ReleaseDC(None, hdc_screen)


def cmd_stop(a) -> int:
    subprocess.run(["taskkill", "/PID", str(a.pid), "/T", "/F"], capture_output=True)
    return _emit(pid=a.pid)


# ---------------------------------------------------------------- self-check


def cmd_selftest(a) -> int:  # noqa: ARG001 — argparse hands us a namespace we do not need
    """Assert the interaction verbs' cursor handling with the Win32 seam stubbed.

    `python vm_agent_windows.py selftest` — runs on the guest AND on the macOS/Linux dev machine (the
    module already imports off-Windows for exactly this reason; see the _IS_WINDOWS note). Each
    assertion pins a way this agent can do NOTHING and still report success:
      * a verb that leaves the cursor parked on the UI (every later frame carries a PointerOver),
      * SetCursorPos succeeding while CLAMPING an off-surface coordinate to the screen edge,
      * a drag whose press point was never reached, or whose last move never happened.
    The stubs are installed and removed under try/finally: `selftest` is reachable through the
    session-1 `serve` dispatch, so a mid-assertion abort must not leak a patched _mouse_to into a run.
    """
    N = argparse.Namespace
    real_mouse, real_cursor, real_send = _mouse_to, _cursor_pos, _send
    moves: list[tuple[int, int]] = []
    flags: list[int] = []
    checks = 0

    def fake_env(clamp_at=None, send_ok=True, start=(700, 400)):
        """Patch the OS seam. `clamp_at` is a point SetCursorPos pretends to clamp (lands 1px short),
        which is the failure that looks like success."""
        moves.clear()
        flags.clear()

        def _fake_mouse_to(x, y):
            moves.append((int(x), int(y)))
            if clamp_at is not None and (int(x), int(y)) == tuple(clamp_at):
                landed = [int(x) - 1, int(y)]
                return False, landed, f"cursor CLAMPED: asked for {[int(x), int(y)]}, landed {landed}"
            return True, [int(x), int(y)], ""

        def _fake_send(*inputs):
            flags.append(inputs[0].u.mi.dwFlags)
            return send_ok

        globals()["_mouse_to"] = _fake_mouse_to
        globals()["_cursor_pos"] = lambda: list(start) if start else None
        globals()["_send"] = _fake_send

    def drive(fn, ns, **kw) -> dict:
        import contextlib  # noqa: PLC0415 — selftest-only
        import io  # noqa: PLC0415
        fake_env(**kw)
        try:
            with contextlib.redirect_stdout(io.StringIO()) as out:
                fn(ns)
        finally:
            globals()["_mouse_to"] = real_mouse
            globals()["_cursor_pos"] = real_cursor
            globals()["_send"] = real_send
        return json.loads(out.getvalue())

    # (a) POINTER CONTAMINATION — click, scroll and drag all end back at the saved position; hover
    #     deliberately does not move back, because parking IS the gesture.
    res = drive(cmd_click, N(x=10, y=20))
    assert moves == [(10, 20), (700, 400)], f"click must restore the cursor: {moves}"
    assert res["ok"] and res["pointer_restored"], res
    checks += 1

    res = drive(cmd_hover, N(x=10, y=20))
    assert moves == [(10, 20)], f"hover must PARK the cursor and stop — restoring defeats it: {moves}"
    assert res["ok"] and res["at"] == [10, 20], res
    checks += 1

    res = drive(cmd_scroll, N(x=10, y=20, dy=-400))
    assert moves == [(10, 20), (700, 400)], f"scroll must restore the cursor: {moves}"
    assert res["ok"] and flags == [MOUSEEVENTF_WHEEL] and res["wheel"] < 0, (res, flags)
    checks += 1

    dns = N(x1=0, y1=0, x2=100, y2=0, steps=4, duration=0.0, gesture="drag")
    res = drive(cmd_drag, dns)
    assert moves == [(0, 0), (25, 0), (50, 0), (75, 0), (100, 0), (700, 400)], moves
    assert flags == [MOUSEEVENTF_LEFTDOWN, MOUSEEVENTF_LEFTUP], flags
    assert res["ok"] and res["pointer_restored"] and res["points"] == 4, res
    checks += 1
    # The restore comes AFTER the release: restoring first would drag the content back to the start.
    assert moves.index((700, 400)) == len(moves) - 1, moves
    checks += 1

    # (d) a CLAMPED move is a failed step, not a plausible frame. SetCursorPos returns TRUE while
    #     clamping, which is why the landing point is read back rather than assumed.
    res = drive(cmd_click, N(x=9999, y=20), clamp_at=(9999, 20))
    assert not res["ok"] and res["at"] == [9998, 20], res
    assert flags == [], "a click whose cursor never arrived must not press anything"
    assert moves[-1] == (700, 400), f"even a refused click must not leave the cursor adrift: {moves}"
    checks += 1

    res = drive(cmd_hover, N(x=9999, y=20), clamp_at=(9999, 20))
    assert not res["ok"] and res["error"], f"hover could never report failure before: {res}"
    checks += 1

    res = drive(cmd_drag, dns, clamp_at=(0, 0))
    assert not res["ok"] and "press point" in res["error"], res
    assert flags == [], "a drag whose press point was never reached must not press"
    checks += 1

    res = drive(cmd_drag, dns, clamp_at=(50, 0))     # a mid-gesture clamp
    assert not res["ok"] and "CLAMPED" in res["error"], res
    assert flags == [MOUSEEVENTF_LEFTDOWN, MOUSEEVENTF_LEFTUP], f"the press must still be released: {flags}"
    checks += 1

    res = drive(cmd_scroll, N(x=10, y=20, dy=-400), clamp_at=(10, 20))
    assert not res["ok"] and flags == [], f"a mis-aimed scroll must not fire the wheel: {res}"
    checks += 1

    # If the SAVE fails (GetCursorPos refused) there is nothing to restore TO, so the restore is
    # skipped entirely. That is this fix silently not running, so it must be visible:
    # `pointer_restored: false` (plus the stderr line _restore_pointer prints), never a bare ok.
    for fn, ns in ((cmd_click, N(x=10, y=20)), (cmd_scroll, N(x=10, y=20, dy=-400)), (cmd_drag, dns)):
        res = drive(fn, ns, start=None)
        assert res["pointer_restored"] is False, (fn.__name__, res)
        assert moves[-1] != (700, 400), (fn.__name__, moves)
        checks += 1

    return _emit(selftest="ok", checks=checks)


# ---------------------------------------------------------------- session-1 server


def _dispatch(payload: dict) -> dict:
    """Run one subcommand in-process and return its JSON object.

    Deliberately reuses main()'s argparse dispatch by capturing stdout instead of re-implementing the
    command table: every subcommand keeps exactly one definition, so `serve` can never drift from the
    CLI form (which is still what vm_smoke uses for one-off calls and what a human debugs with)."""
    cmd = str(payload.get("cmd") or "")
    if not cmd:
        return {"ok": False, "error": "no cmd"}
    argv = [cmd] + [str(x) for x in (payload.get("args") or [])]
    buf = io.StringIO()
    err = io.StringIO()
    bad_args = False
    try:
        # Capture stderr too: argparse writes its usage/error there and would otherwise spray the
        # server's log for every mistyped call, while the caller got only a vague "no JSON".
        with contextlib.redirect_stdout(buf), contextlib.redirect_stderr(err):
            main(argv)
    except SystemExit as e:
        # argparse calls sys.exit(2) for an unknown subcommand or bad arguments. Distinguish that from a
        # subcommand that ran and simply printed nothing, so a typo is obvious in the reply.
        bad_args = (e.code or 0) != 0
    except Exception as e:  # a subcommand must never kill the server
        return {"ok": False, "error": f"{type(e).__name__}: {e}", "cmd": cmd}
    for line in reversed(buf.getvalue().strip().splitlines()):
        line = line.strip()
        if line.startswith("{"):
            try:
                return json.loads(line)
            except json.JSONDecodeError:
                continue
    if bad_args:
        return {"ok": False, "error": "unknown subcommand or bad arguments", "cmd": cmd,
                "argv": argv[1:], "detail": err.getvalue().strip()[-300:]}
    return {"ok": False, "error": "subcommand produced no JSON", "cmd": cmd,
            "raw": buf.getvalue()[-400:], "stderr": err.getvalue().strip()[-300:]}


def cmd_serve(a) -> int:
    """Serve subcommands over TCP so the caller can drive a session the SSH session cannot reach.

    WHY THIS EXISTS -- Windows Session 0 isolation. sshd runs in session 0 (services, no desktop) while
    the interactive console the VM displays is session 1. A GUI app launched over SSH therefore has no
    window on any visible desktop, and EnumWindows / SendInput / PrintWindow are all PER-SESSION, so an
    agent living in session 0 can neither see nor drive the real UI. macOS has no equivalent problem, so
    this is the one place the Windows lane must differ from vm_agent_macos.py.

    The fix is to run THIS process in session 1 (a scheduled task registered with /it -- see
    tools/parity/windows/session1.py) and let the host talk to it. Everything the agent does then
    happens where the desktop is, including `launch`, so apps it starts are session-1 children and are
    visible and driveable.

    SECURITY. This endpoint executes commands, so it binds 127.0.0.1 by DEFAULT and is reached through an
    SSH tunnel (`ssh -L`) rather than being exposed on the network: no firewall hole, and access is
    gated by the same SSH key that already administers the guest. A shared token is required on every
    request as defence in depth, since any local account on the guest could otherwise reach the loopback
    port. Do not bind this to a routable address on a machine you care about.

    Protocol: one JSON object per connection, newline-terminated, one JSON object back.
        -> {"token": "...", "cmd": "shot", "args": ["C:/x.png", "--window", "12345"]}
        <- {"ok": true, ...}
    The control command "__ping__" reports liveness; "__shutdown__" stops the server."""
    token = ""
    if a.token_file:
        try:
            with open(a.token_file, encoding="ascii") as fh:
                token = fh.read().strip()
        except OSError as e:
            return _emit(ok=False, error=f"cannot read token file {a.token_file}: {e}")
        if not token:
            return _emit(ok=False, error=f"token file {a.token_file} is empty")

    srv = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    srv.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    try:
        srv.bind((a.host, a.port))
    except OSError as e:
        return _emit(ok=False, error=f"bind {a.host}:{a.port} failed: {e}")
    srv.listen(8)

    # The launcher greps this line out of the task's log to know the server is up, and reads back the
    # session id so a mis-registered task (one that landed in session 0 again) is caught immediately
    # rather than producing invisible windows later.
    session_id = -1
    if kernel32 is not None:
        sid = wintypes.DWORD()
        if kernel32.ProcessIdToSessionId(kernel32.GetCurrentProcessId(), ctypes.byref(sid)):
            session_id = int(sid.value)
    print(json.dumps({"ok": True, "serving": f"{a.host}:{a.port}", "session_id": session_id,
                      "dpi_mode": DPI_MODE, "auth": bool(token)}), flush=True)

    while True:
        try:
            conn, _peer = srv.accept()
        except OSError:
            break
        try:
            conn.settimeout(a.timeout)
            chunks: list[bytes] = []
            while b"\n" not in b"".join(chunks):
                part = conn.recv(65536)
                if not part:
                    break
                chunks.append(part)
            raw = b"".join(chunks).split(b"\n", 1)[0].decode("utf-8", "replace").strip()
            if not raw:
                continue
            try:
                payload = json.loads(raw)
            except json.JSONDecodeError as e:
                conn.sendall((json.dumps({"ok": False, "error": f"bad JSON: {e}"}) + "\n").encode())
                continue
            if token and str(payload.get("token") or "") != token:
                # Do not echo the expected value.
                conn.sendall((json.dumps({"ok": False, "error": "unauthorized"}) + "\n").encode())
                continue
            cmd = str(payload.get("cmd") or "")
            if cmd == "__ping__":
                reply = {"ok": True, "pong": True, "session_id": session_id, "dpi_mode": DPI_MODE}
            elif cmd == "__shutdown__":
                conn.sendall((json.dumps({"ok": True, "shutdown": True}) + "\n").encode())
                conn.close()
                break
            else:
                reply = _dispatch(payload)
            conn.sendall((json.dumps(reply) + "\n").encode())
        except (OSError, socket.timeout):
            pass
        finally:
            try:
                conn.close()
            except OSError:
                pass
    srv.close()
    return 0


def main(argv=None) -> int:
    p = argparse.ArgumentParser(description="Windows guest agent for the E2E comparison runner")
    sub = p.add_subparsers(dest="cmd", required=True)

    s = sub.add_parser("set-resolution"); s.add_argument("width", type=int); s.add_argument("height", type=int)
    s.set_defaults(fn=cmd_set_resolution)

    s = sub.add_parser("clean"); s.add_argument("dir"); s.set_defaults(fn=cmd_clean)

    s = sub.add_parser("launch")
    s.add_argument("--bundle", required=True); s.add_argument("--proc", required=True)
    s.add_argument("--env", action="append", help="K=V, repeatable"); s.set_defaults(fn=cmd_launch)

    s = sub.add_parser("window-id"); s.add_argument("pid", type=int)
    s.add_argument("--proc", default="", help="accepted for macOS-agent parity; unused on Windows")
    s.add_argument("--retries", type=int, default=15); s.add_argument("--delay", type=float, default=0.3)
    s.set_defaults(fn=cmd_window_id)

    s = sub.add_parser("present"); s.add_argument("--proc", required=True)
    s.add_argument("--x", type=int, default=128); s.add_argument("--y", type=int, default=30)
    s.add_argument("--w", type=int, default=1024); s.add_argument("--h", type=int, default=800)
    s.add_argument("--zoom", action="store_true", help="(ignored; explicit --x/--y/--w/--h supersede)")
    s.add_argument("--pid", type=int, default=0, help="the app's pid (preferred over --proc)")
    s.add_argument("--shot", default="", help="capture the window to this path once size is confirmed")
    s.add_argument("--defocus", dest="defocus", action="store_true", default=False,
                   help="OPT-IN: hand OS foreground to the shell before --shot, to suppress WinUI's "
                        "keyboard-focus visual. DEFAULT OFF -- enabling it killed the session-1 agent "
                        "transport mid-run on 2026-07-31 (see PARITY_REVIEW.md). Do not enable for a "
                        "scoring run until that is resolved.")
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
    # the press/move/release sequence can never apply to one gesture and not the other. Defaults are
    # identical to the macOS agent's, so a scenario drives both platforms the same way.
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
    s.add_argument("--rect", default="", help="x,y,w,h region capture (occlusion-prone fallback)")
    s.set_defaults(fn=cmd_shot)

    s = sub.add_parser("stop"); s.add_argument("pid", type=int); s.set_defaults(fn=cmd_stop)

    # Runs anywhere (the Win32 seam is stubbed), so it is both a dev-machine pre-flight and the first
    # thing to run ON a guest when a scenario's frames come back looking un-driven. Mirrors the macOS
    # agent's `selftest`.
    s = sub.add_parser("selftest", help="assert the interaction verbs with the Win32 seam stubbed")
    s.set_defaults(fn=cmd_selftest)

    # The session-1 server (see cmd_serve). Not part of the macOS agent's surface: it exists purely to
    # escape Windows Session 0 isolation, so the shared subcommands above stay identical across agents.
    s = sub.add_parser("serve")
    s.add_argument("--host", default="127.0.0.1",
                   help="bind address; keep loopback and reach it via an SSH tunnel")
    s.add_argument("--port", type=int, default=8770)
    s.add_argument("--token-file", default="", help="file holding the shared token required per request")
    s.add_argument("--timeout", type=float, default=120.0, help="per-connection recv timeout")
    s.set_defaults(fn=cmd_serve)

    a = p.parse_args(argv)
    try:
        return a.fn(a)
    except Exception as e:  # never crash the SSH call without a parseable result
        return _emit(ok=False, error=f"{type(e).__name__}: {e}")


if __name__ == "__main__":
    sys.exit(main())
