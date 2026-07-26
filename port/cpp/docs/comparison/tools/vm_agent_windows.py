#!/usr/bin/env python3
"""Windows guest agent for the E2E visual-comparison runner.

The Windows sibling of vm_agent_macos.py, exposing the SAME subcommands so the host orchestrator
(run_comparison.py) never changes — the per-OS seam that file's header describes:

    set-resolution | clean | launch | window-id | present | click | type | scroll | shot | stop

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
import ctypes
import json
import os
import shutil
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
            return _emit(ok=False, error=f"ChangeDisplaySettingsExW returned {rc}",
                         requested=[a.width, a.height], set=list(target))
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


def cmd_present(a) -> int:
    """Foreground the window and force it to an EXPLICIT position+size so every column captures at the
    SAME rect; optionally capture atomically via --shot.

    The macOS twin needs the atomic --shot because any intervening AX call steals key focus and greys
    the traffic lights. On Windows PrintWindow does not care about focus, but --shot is still honoured
    (and preferred) because it removes a whole SSH round-trip between sizing and capture — during
    which a WinUI 3 window can still be settling.

    Like the macOS twin this FAILS LOUDLY if the window never reaches the target size rather than
    returning a short rect as success: a silently short frame is the failure that gets scored."""
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
        ok, err, size = _capture_hwnd(hwnd, a.shot)
        if not ok:
            return _emit(ok=False, proc=a.proc, id=hwnd, bounds=rect, error=f"shot failed: {err}")
        shot_info = {"shot": a.shot, "shot_size": size}
    return _emit(proc=a.proc, id=hwnd, bounds=rect, rect=",".join(map(str, rect)), **shot_info)


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


def _mouse_to(x: int, y: int) -> None:
    # SetCursorPos takes physical pixels (we are DPI-aware), which keeps the scenario's absolute
    # coordinates identical to the ones the macOS agent feeds cliclick.
    user32.SetCursorPos(int(x), int(y))
    time.sleep(0.02)


def cmd_click(a) -> int:
    _mouse_to(a.x, a.y)
    down = INPUT(type=INPUT_MOUSE, u=_INPUTunion(mi=MOUSEINPUT(0, 0, 0, MOUSEEVENTF_LEFTDOWN, 0, None)))
    up = INPUT(type=INPUT_MOUSE, u=_INPUTunion(mi=MOUSEINPUT(0, 0, 0, MOUSEEVENTF_LEFTUP, 0, None)))
    ok = _send(down, up)
    return _emit(ok=ok, x=a.x, y=a.y)


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


def cmd_scroll(a) -> int:
    """Wheel-scroll at (x, y). `dy` is in PIXELS for parity with the macOS agent's pixel scroll event;
    it is converted to wheel notches (120 units each, see SCROLL_PIXELS_PER_NOTCH), keeping the sign
    and always moving at least one notch so a small scenario delta is never a silent no-op."""
    _mouse_to(a.x, a.y)
    notches = int(a.dy) / float(SCROLL_PIXELS_PER_NOTCH)
    amount = int(notches * WHEEL_DELTA)
    if amount == 0 and a.dy != 0:
        amount = WHEEL_DELTA if a.dy > 0 else -WHEEL_DELTA
    ev = INPUT(type=INPUT_MOUSE,
               u=_INPUTunion(mi=MOUSEINPUT(0, 0, ctypes.c_uint32(amount & 0xFFFFFFFF).value,
                                           MOUSEEVENTF_WHEEL, 0, None)))
    ok = _send(ev)
    return _emit(ok=ok, x=a.x, y=a.y, dy=a.dy, wheel=amount)


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
    s.set_defaults(fn=cmd_present)

    s = sub.add_parser("click"); s.add_argument("x", type=int); s.add_argument("y", type=int)
    s.set_defaults(fn=cmd_click)

    s = sub.add_parser("type"); s.add_argument("text"); s.set_defaults(fn=cmd_type)

    s = sub.add_parser("scroll"); s.add_argument("x", type=int); s.add_argument("y", type=int)
    s.add_argument("dy", type=int); s.set_defaults(fn=cmd_scroll)

    s = sub.add_parser("shot"); s.add_argument("out"); s.add_argument("--window", type=int, default=0)
    s.add_argument("--rect", default="", help="x,y,w,h region capture (occlusion-prone fallback)")
    s.set_defaults(fn=cmd_shot)

    s = sub.add_parser("stop"); s.add_argument("pid", type=int); s.set_defaults(fn=cmd_stop)

    a = p.parse_args(argv)
    try:
        return a.fn(a)
    except Exception as e:  # never crash the SSH call without a parseable result
        return _emit(ok=False, error=f"{type(e).__name__}: {e}")


if __name__ == "__main__":
    sys.exit(main())
