#!/usr/bin/env python3
"""Windows VM pipeline smoke test — deploy, drive, capture, verify, in one command.

This is the FIRST thing to run against a fresh Windows VM. run_comparison.py can do the whole board,
but when something is wrong on a brand-new guest you want the failure attributed to a single step, not
buried in a 182-tag sweep. So this walks the chain one call at a time and prints PASS/FAIL per step:

    ssh reachable → agent deployed → app deployed → set-resolution → launch → window-id
                  → present(+shot) → pull PNG → verify PNG decodes at the expected size
                  → click / type / scroll → stop

Every step prints the agent's raw JSON on failure, so a broken step is diagnosable without re-running.

Usage:
    tools/parity/windows/vm_smoke.py --host windows-test.local --user testinguser
    tools/parity/windows/vm_smoke.py --host … --user … --exe build/windows-smoke/maui_smoke.exe
    tools/parity/windows/vm_smoke.py --host … --user … --keep     # leave the app running for a look

Prerequisites on the guest (see docs/WINDOWS_TOOLCHAIN.md §3): OpenSSH Server with key auth, the
DefaultShell set to PowerShell, and Python 3 on PATH (or pass --python).
"""
from __future__ import annotations

import argparse
import json
import os
import posixpath
import shlex
import subprocess
import sys
import tempfile
from pathlib import Path

HERE = Path(__file__).resolve().parent
CPP_ROOT = HERE.parents[2]                                   # port/cpp
AGENT = CPP_ROOT / "docs/comparison/tools/vm_agent_windows.py"
DEFAULT_EXE = CPP_ROOT / "build/windows-smoke/maui_smoke.exe"

GREEN, RED, DIM, RESET = "\033[32m", "\033[31m", "\033[2m", "\033[0m"
_failures = 0


def step(name: str, ok: bool, detail: str = "") -> bool:
    global _failures
    tag = f"{GREEN}PASS{RESET}" if ok else f"{RED}FAIL{RESET}"
    print(f"  [{tag}] {name}" + (f"  {DIM}{detail}{RESET}" if detail else ""))
    if not ok:
        _failures += 1
    return ok


class Guest:
    def __init__(self, host: str, user: str, staging: str, python: str):
        self.hostspec = f"{user}@{host}"
        self.staging = staging.rstrip("/")
        self.python = python
        self.agent_remote = posixpath.join(self.staging, "vm_agent_windows.py")
        self.apps = posixpath.join(self.staging, "apps")
        self.scratch = posixpath.join(self.staging, "scratch")

    def _ssh(self) -> list[str]:
        return ["ssh", "-o", "BatchMode=yes", "-o", "ConnectTimeout=10", self.hostspec]

    def sh(self, tokens: list[str], timeout: int = 120) -> subprocess.CompletedProcess:
        # shlex.join gives POSIX single-quoting, which PowerShell also treats as a literal string —
        # hence the DefaultShell=PowerShell requirement (cmd.exe would pass the quotes through).
        return subprocess.run(self._ssh() + [shlex.join(tokens)], capture_output=True, text=True,
                              timeout=timeout)

    def agent(self, subcmd: str, *args, timeout: int = 120) -> dict:
        tokens = [self.python, self.agent_remote, subcmd, *map(str, args)]
        try:
            r = self.sh(tokens, timeout=timeout)
        except subprocess.TimeoutExpired:
            return {"ok": False, "error": f"{subcmd} timed out after {timeout}s"}
        for line in reversed((r.stdout or "").strip().splitlines()):
            line = line.strip()
            if line.startswith("{"):
                try:
                    return json.loads(line)
                except json.JSONDecodeError:
                    continue
        return {"ok": False, "error": "no JSON from agent", "stdout": (r.stdout or "")[-400:],
                "stderr": (r.stderr or "")[-400:]}

    def mkdirs(self, remote: str) -> None:
        self.sh(["New-Item", "-ItemType", "Directory", "-Force", "-Path", remote])

    def push(self, local: Path, remote: str) -> bool:
        self.mkdirs(posixpath.dirname(remote))
        return subprocess.run(["scp", "-o", "BatchMode=yes", str(local),
                               f"{self.hostspec}:{remote}"], capture_output=True).returncode == 0

    def pull(self, remote: str, local: Path) -> bool:
        local.parent.mkdir(parents=True, exist_ok=True)
        rc = subprocess.run(["scp", "-o", "BatchMode=yes", f"{self.hostspec}:{remote}", str(local)],
                            capture_output=True).returncode
        return rc == 0 and local.is_file()


def verify_png(path: Path, want: tuple[int, int] | None) -> tuple[bool, str]:
    """Decode the pulled capture. A file that merely EXISTS is not a pass: the agent writes PNG bytes
    itself (stdlib zlib), so a malformed chunk would still produce a plausible-looking file that only
    the scorer would later choke on."""
    if not path.is_file() or path.stat().st_size == 0:
        return False, "missing or empty"
    try:
        from PIL import Image
    except ImportError:
        # No Pillow on this host: fall back to parsing the IHDR by hand, which still catches a
        # truncated/garbled header.
        b = path.read_bytes()
        if not b.startswith(b"\x89PNG\r\n\x1a\n"):
            return False, "not a PNG (bad signature)"
        if b[12:16] != b"IHDR":
            return False, "no IHDR chunk"
        w = int.from_bytes(b[16:20], "big")
        h = int.from_bytes(b[20:24], "big")
        if want and (w, h) != want:
            return False, f"{w}x{h}, expected {want[0]}x{want[1]}"
        return True, f"{w}x{h} ({path.stat().st_size} bytes, header-only check)"
    # Pillow raises (UnidentifiedImageError, OSError, …) on a bad file. Convert that into a normal
    # failure tuple: this function's job is to attribute a failure to ONE step, so letting the
    # exception escape would abort the whole smoke run and hide every later step.
    try:
        im = Image.open(path)
        im.load()  # force full decode — catches a corrupt IDAT that the header check would miss
    except Exception as e:
        return False, f"undecodable: {type(e).__name__}: {e}"
    if want and im.size != want:
        return False, f"{im.size[0]}x{im.size[1]}, expected {want[0]}x{want[1]}"
    return True, f"{im.size[0]}x{im.size[1]} {im.mode} ({path.stat().st_size} bytes)"


def main() -> int:
    ap = argparse.ArgumentParser(description="Windows VM pipeline smoke test")
    ap.add_argument("--host", required=True)
    ap.add_argument("--user", required=True)
    ap.add_argument("--staging", default="C:/Users/%(user)s/maui-comparison",
                    help="remote staging dir; %%(user)s is substituted")
    ap.add_argument("--python", default="py", help="guest python (default: the `py` launcher)")
    ap.add_argument("--exe", default=str(DEFAULT_EXE), help="app to deploy (default: the smoke exe)")
    ap.add_argument("--page", default="vm_smoke", help="MAUI_SAMPLE_PAGE value")
    ap.add_argument("--theme", default="light", choices=("light", "dark"))
    ap.add_argument("--width", type=int, default=1280)
    ap.add_argument("--height", type=int, default=800)
    ap.add_argument("--win-w", type=int, default=1024)
    ap.add_argument("--win-h", type=int, default=800)
    # Window ORIGIN. Default 0,0 rather than the macOS lane's 128,30: a UTM/QEMU guest's screen is only
    # as large as the VM window (its display-only driver refuses guest-side mode changes), so an inset
    # origin easily pushes a 1024-wide window off a 1024-wide screen. At 0,0 the window is fully on
    # screen at any size that can hold it, and window coordinates equal screen coordinates, which makes
    # scenario taps trivial to reason about.
    ap.add_argument("--x", type=int, default=0)
    ap.add_argument("--y", type=int, default=0)
    ap.add_argument("--keep", action="store_true", help="do not stop the app at the end")
    a = ap.parse_args()

    staging = a.staging % {"user": a.user}
    g = Guest(a.host, a.user, staging, a.python)
    exe = Path(a.exe)

    print(f"Windows VM pipeline smoke  ->  {g.hostspec}   staging={staging}")

    # -- connectivity + prerequisites ---------------------------------------------------------------
    r = subprocess.run(g._ssh() + ["exit 0"], capture_output=True, text=True)
    if not step("ssh reachable (key auth, BatchMode)", r.returncode == 0,
                (r.stderr or "").strip()[:160]):
        print("\nCannot continue without SSH. See docs/WINDOWS_TOOLCHAIN.md §3.")
        return 1

    # Read the registry value rather than evaluating $PSVersionTable: ssh_run quotes every token with
    # shlex (POSIX single quotes), and PowerShell treats a single-quoted string as LITERAL, so any probe
    # containing `$` comes back as its own source text and the check silently "fails". Keep remote
    # probes $-free.
    shell = g.sh(["Get-ItemPropertyValue", "-Path", "HKLM:\\SOFTWARE\\OpenSSH",
                  "-Name", "DefaultShell"])
    shell_path = (shell.stdout or "").strip().splitlines()[-1:] or [""]
    step("guest DefaultShell is PowerShell", "powershell" in shell_path[0].lower(),
         shell_path[0] or "DefaultShell unset - POSIX-quoted commands will break (see docs section 3)")

    ver = g.sh([a.python, "--version"])
    step("guest python present", ver.returncode == 0,
         (ver.stdout or ver.stderr or "").strip()[:80])

    # -- deploy -------------------------------------------------------------------------------------
    if not step("agent source exists locally", AGENT.is_file(), str(AGENT)):
        return 1
    step("deploy vm_agent_windows.py", g.push(AGENT, g.agent_remote), g.agent_remote)

    if not step("app exists locally", exe.is_file(),
                f"{exe} — build it with tools/parity/windows/build_smoke.sh"):
        return 1
    remote_exe = posixpath.join(g.apps, exe.name)
    step("deploy app", g.push(exe, remote_exe), remote_exe)

    res = g.agent("clean", g.scratch)
    step("agent clean (scratch)", res.get("ok"), json.dumps(res)[:160])

    # -- display ------------------------------------------------------------------------------------
    res = g.agent("set-resolution", a.width, a.height)
    if res.get("driver_refused"):
        # Display-only guest driver: the mode is host-controlled. Not a failure (see the agent), but the
        # screen must still be big enough for the window we are about to present, so check that here.
        fits = bool(res.get("fits_request")) or (
            (res.get("actual") or [0, 0])[0] >= a.win_w and (res.get("actual") or [0, 0])[1] >= a.win_h)
        step(f"set-resolution {a.width}x{a.height} (driver-controlled)", fits,
             f"actual={res.get('actual')} - host-controlled mode; "
             + ("large enough for the window" if fits
                else f"TOO SMALL for a {a.win_w}x{a.win_h} window - enlarge the VM window"))
    else:
        step(f"set-resolution {a.width}x{a.height}", res.get("ok"),
             f"actual={res.get('actual')} dpi_mode={res.get('dpi_mode')}"
             if res.get("ok") else json.dumps(res)[:200])

    # -- launch -------------------------------------------------------------------------------------
    res = g.agent("launch", "--bundle", remote_exe, "--proc", exe.name,
                  "--env", f"MAUI_SAMPLE_PAGE={a.page}", "--env", f"MAUI_APPEARANCE={a.theme}",
                  timeout=90)
    pid = res.get("pid", 0)
    if not step("launch app", bool(res.get("ok") and pid), json.dumps(res)[:200]):
        return 1

    res = g.agent("window-id", pid)
    hwnd = res.get("id", 0)
    step("window-id (find the app window)", bool(res.get("ok") and hwnd),
         f"hwnd={hwnd} bounds={res.get('bounds')}" if res.get("ok") else json.dumps(res)[:200])

    # -- present + capture atomically ---------------------------------------------------------------
    remote_shot = posixpath.join(g.scratch, "smoke.png")
    res = g.agent("present", "--proc", exe.name, "--pid", pid,
                  "--x", a.x, "--y", a.y, "--w", a.win_w, "--h", a.win_h, "--shot", remote_shot,
                  timeout=90)
    presented = step(f"present to {a.win_w}x{a.win_h} + shot", res.get("ok"),
                     f"bounds={res.get('bounds')} shot_size={res.get('shot_size')}"
                     if res.get("ok") else json.dumps(res)[:240])

    with tempfile.TemporaryDirectory() as td:
        local_shot = Path(td) / "smoke.png"
        if presented:
            step("pull capture", g.pull(remote_shot, local_shot), str(local_shot))
            ok, detail = verify_png(local_shot, (a.win_w, a.win_h))
            step("capture decodes at the presented size", ok, detail)
            keep_to = CPP_ROOT / "build/windows-smoke/vm_smoke_capture.png"
            if local_shot.is_file():
                keep_to.parent.mkdir(parents=True, exist_ok=True)
                keep_to.write_bytes(local_shot.read_bytes())
                print(f"        {DIM}saved a copy: {keep_to}{RESET}")

        # A standalone `shot --window <hwnd>` too: present's atomic shot is the path the runner uses,
        # but the runner ALSO shoots by window id between scenario steps, so both must work.
        if hwnd:
            remote_shot2 = posixpath.join(g.scratch, "by_hwnd.png")
            res = g.agent("shot", remote_shot2, "--window", hwnd)
            if step("shot --window (PrintWindow, occlusion-proof)", res.get("ok"),
                    f"size={res.get('size')}" if res.get("ok") else json.dumps(res)[:200]):
                local2 = Path(td) / "by_hwnd.png"
                if g.pull(remote_shot2, local2):
                    ok, detail = verify_png(local2, None)
                    step("window capture decodes", ok, detail)

    # -- interaction --------------------------------------------------------------------------------
    res = g.agent("click", 640, 400)
    step("click", res.get("ok"), json.dumps(res)[:120])
    res = g.agent("type", "maui")
    step("type", res.get("ok"), json.dumps(res)[:120])
    res = g.agent("scroll", 640, 400, -240)
    step("scroll", res.get("ok"),
         f"wheel={res.get('wheel')}" if res.get("ok") else json.dumps(res)[:160])

    # -- the core probe: does the port's cross-platform core actually RUN on Windows? ----------------
    # Separate from the GUI app on purpose. The smoke window only proves the pipeline can drive a
    # window; the probe proves the port's own core (property system, control construction, text
    # measurement, generic mount, measure+arrange) executes correctly on this OS. It self-checks and
    # exits non-zero, so we assert on its exit status rather than eyeballing output.
    probe_local = CPP_ROOT / "build/windows-core/maui_core_probe.exe"
    if probe_local.is_file():
        remote_probe = posixpath.join(g.apps, probe_local.name)
        if step("deploy core probe", g.push(probe_local, remote_probe), remote_probe):
            r = g.sh([remote_probe], timeout=180)
            out = (r.stdout or "") + (r.stderr or "")
            passed = r.returncode == 0 and "PASS" in out
            step("core probe runs on Windows", passed,
                 out.strip().replace("\n", " | ")[:400] or f"exit={r.returncode}")
            for line in (r.stdout or "").splitlines():
                if line.strip().startswith(("ok", "FAIL")):
                    print(f"        {DIM}{line.strip()}{RESET}")
    else:
        print(f"  {DIM}skip core probe: {probe_local} not built "
              f"(tools/parity/windows/build_core_check.sh){RESET}")

    # -- teardown -----------------------------------------------------------------------------------
    if a.keep:
        print(f"  {DIM}--keep: leaving pid {pid} running{RESET}")
    else:
        res = g.agent("stop", pid)
        step("stop app", res.get("ok"), json.dumps(res)[:120])

    print()
    if _failures:
        print(f"{RED}{_failures} step(s) FAILED{RESET} — the pipeline is not ready; fix the first "
              f"failure above.")
        return 1
    print(f"{GREEN}all steps passed{RESET} — the Windows VM pipeline works end to end. Next: "
          f"python3 docs/comparison/tools/run_comparison.py --config docs/comparison/config/windows.toml")
    return 0


if __name__ == "__main__":
    sys.exit(main())
