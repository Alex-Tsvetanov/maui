#!/usr/bin/env python3
"""E2E visual-comparison test runner — host orchestrator.

Deploys the three framework columns (maui_xaml / cpp / cpp_xaml) to a test environment (a macOS UTM
VM over SSH, to start), sets a fixed display resolution, cleans remote staging, then for each example
tag runs a declarative scenario (clicks/scroll/typing at absolute coordinates), captures the app
window after every step, lays the images out under

    port/cpp/docs/comparison/<YYYY-MM-DD-HH_MM_SS>/<tag>/<platform>/<column>/NNNN.png

(+ an NNNN.json sidecar per shot) and pixel-scores maui_xaml vs cpp / cpp_xaml (reusing
port/cpp/tools/parity/pixel_score.py).

The macOS OS primitives live in the guest agent (vm_agent_macos.py), driven over SSH. Interaction is
behind a small driver interface: CoordinateDriver (cliclick, the default) and DevFlowDriver (MAUI
DevFlow CLI for tap-by-automationId, falling back to coordinates). See tools/README_e2e.md.

Usage:
    python3 run_comparison.py --config config/local.example.toml            # all tags, all envs
    python3 run_comparison.py --config <cfg> --only counter,entry           # a few tags
    python3 run_comparison.py --config <cfg> --plan                         # validate + print, no SSH
"""
from __future__ import annotations

import argparse
import json
import os
import posixpath
import re
import shlex
import subprocess
import sys
import time
import tomllib
from datetime import datetime
from pathlib import Path

HERE = Path(__file__).resolve().parent          # …/docs/comparison/tools
COMP = HERE.parent                              # …/docs/comparison  (== pixel_score.COMP)
REPO = COMP.parents[3]                          # repo root (…/maui)
# The guest agent is chosen by the environment's `os` key — the per-OS seam vm_agent_macos.py's header
# describes. Each agent exposes the SAME subcommands, so nothing below this line is OS-specific except
# the small guest_ops indirection (mkdir/env-prefix/reboot/copy differ between a POSIX and a Windows
# guest, and nothing else does).
GUEST_AGENTS = {
    "macos": "vm_agent_macos.py",
    "windows": "vm_agent_windows.py",
}
MANIFEST = REPO / "port/maui-reference/pages/manifest.json"

sys.path.insert(0, str(REPO / "port/cpp/tools/parity"))
import pixel_score  # noqa: E402  reuse score_theme()/classify()

# Status thresholds (from pixel_score.classify): SSIM>=0.98 & diff<=1% green; >=0.90 & <=8% yellow.


def git_commit() -> str:
    r = subprocess.run(["git", "-C", str(REPO), "rev-parse", "--short", "HEAD"],
                       capture_output=True, text=True)
    return r.stdout.strip() or "unknown"


def png_size(path):
    """(w, h) of a PNG, straight from the IHDR — no image library needed for a 24-byte read."""
    try:
        with open(path, "rb") as f:
            head = f.read(24)
        if len(head) < 24 or head[:8] != b"\x89PNG\r\n\x1a\n":
            return None
        return int.from_bytes(head[16:20], "big"), int.from_bytes(head[20:24], "big")
    except OSError:
        return None


def shoot_presented(env, ccfg, g, remote_shot, pid=None, attempts=3):
    """Present the window at the fixed geometry and capture THAT rect. Returns bounds, or None if the
    window could not be presented.

    Never falls back to a whole-display shot. That fallback used to live inline at the call site and was
    the worst failure mode of this runner: when the VM's WindowServer/AX session desyncs (windows stop
    being enumerable — the same symptom the resolution self-heal exists for), `present` returns no rect,
    every subsequent shot silently captured the DESKTOP, and those full-display screenshots were written
    out as ordinary frames. One sweep banked 154 of them across 27 alphabetically-contiguous pages before
    the session healed itself. A desktop shot is not a degraded capture, it is not a capture — so on
    failure run the self-heal (a resolution TOGGLE re-syncs the session; re-setting the CURRENT mode is a
    no-op) and retry; if it still will not present, return None and let the caller drop the frame.
    """
    for attempt in range(attempts):
        # ATOMIC present-and-capture: present shoots `remote_shot` itself the instant it confirms the size.
        # Splitting present and shot into two SSH round-trips let a heavier app (MauiReference, gallery_xaml)
        # re-lay-out its window to content size in the ~300ms gap — present confirmed 1024x800, the app
        # shrank to 1024x548, and the separate shot captured 548. Capturing in-process closes that gap.
        # REQUIRE the window id + the shot path: the -l <id> capture cannot be occluded, unlike a -R <rect>
        # region shot (which once put gallery_xaml into the MAUI column across 40 pages). No window / no
        # shot => drop and self-heal, never a fallback capture.
        args = ["present", "--proc", ccfg["process"],
                "--x", g["x"], "--y", g["y"], "--w", g["w"], "--h", g["h"], "--shot", remote_shot]
        if pid:
            args += ["--pid", pid]
        pr = env.agent(*args)
        if pr.get("rect") and pr.get("window") and pr.get("shot"):
            return pr.get("bounds")
        if attempt < attempts - 1:
            why = pr.get("error") or ("no window id — Quartz could not see the window" if pr.get("rect") else "?")
            print(f"  ~ present failed ({why}) — resolution-toggle self-heal, retrying")
            env.agent("set-resolution", env.display["width"], env.display["height"])
            time.sleep(2.0)
    return None


def load_manifest() -> list[dict]:
    if not MANIFEST.is_file():
        return []
    return json.loads(MANIFEST.read_text())


class Env:
    """One test environment (a VM/emulator) + its SSH connection and column/tool config."""

    def __init__(self, name: str, cfg: dict):
        self.name = name
        self.cfg = cfg
        # `os` selects the guest agent AND the handful of guest shell primitives that genuinely differ
        # (see _mkdir_tokens / _agent_tokens / reboot_and_wait). Unknown values fail here rather than
        # deploying a macOS agent to a Windows box and failing much later with a confusing error.
        self.os = cfg.get("os", "macos")
        if self.os not in GUEST_AGENTS:
            raise SystemExit(f"[{name}] unsupported os={self.os!r}; known: {sorted(GUEST_AGENTS)}")
        self.is_windows = self.os == "windows"
        self.platform = cfg.get("platform", name)
        c = cfg["connection"]
        self.hostspec = f"{c['user']}@{c['host']}"
        self.connect_timeout = c.get("connect_timeout_seconds", 10)
        self.staging = cfg["staging"]["root"].rstrip("/")
        self.tools = cfg.get("tools", {})
        self.python3 = self.tools.get("python3", "/usr/bin/python3")
        self.display = cfg.get("display", {})
        cap = cfg.get("capture", {})
        # Before each shot, activate the window (key → colored traffic lights) and set it to an explicit
        # rect so every column captures at the SAME size. Default on; [environments.<name>.capture]
        # present = false falls back to a single window-id lookup. The default geometry {128,30} matches the
        # centered-window origin the scenarios are calibrated to; height 800 clamps to the screen max.
        self.present = cap.get("present", True)
        self.geom = {"x": 128, "y": 30, "w": 1024, "h": 800, **cap.get("geometry", {})}
        # Reboot the VM before the run for a guaranteed-clean Aqua/WindowServer session (the VM boots fast).
        # The most reliable cleanup — a confused WindowServer (after external display/UTM changes) opens app
        # windows with bogus geometry that aren't AX-enumerable, which the in-run resolution self-heal can't
        # always fix. Off by default (adds ~1min); needs passwordless `sudo reboot` on the VM.
        self.reboot_before_run = cap.get("reboot_before_run", False)
        self.columns = cfg["columns"]
        # Default python differs: a Windows guest has no /usr/bin/python3. `py -3` (the PEP 397 launcher
        # shipped with python.org installs) is the most reliable invocation over a non-interactive SSH
        # session, where PATH may not include the Python dir.
        if self.is_windows and "python3" not in self.tools:
            self.python3 = "py"
        self.agent_remote = posixpath.join(self.staging, GUEST_AGENTS[self.os])
        # The local file to deploy for THIS env's os (replaces the old module-level AGENT_SRC, which
        # hardcoded the macOS agent).
        self.agent_src = HERE / GUEST_AGENTS[self.os]
        # Set by run_env for a Windows guest: the session-1 transport. See its module docstring -- SSH
        # runs in session 0, which has no desktop, so agent calls must go through session 1.
        self.session1 = None
        self.apps_remote = posixpath.join(self.staging, "apps")
        self.scratch = posixpath.join(self.staging, "scratch")

    # -- ssh plumbing -------------------------------------------------------
    def _ssh(self) -> list[str]:
        return ["ssh", "-o", "BatchMode=yes", "-o", f"ConnectTimeout={self.connect_timeout}", self.hostspec]

    def ssh_run(self, tokens: list[str], timeout: int = 120) -> subprocess.CompletedProcess:
        """Run a command on the VM (tokens are shell-quoted into one remote string)."""
        return subprocess.run(self._ssh() + [shlex.join(tokens)],
                              capture_output=True, text=True, timeout=timeout)

    @property
    def probe_cmd(self) -> str:
        """A no-op remote command that exits 0 on this guest's shell. `true` is a POSIX builtin that
        PowerShell does not have (it errors), which would make every Windows reachability check report
        the VM as down; `exit 0` is valid in both shells."""
        return "exit 0" if self.is_windows else "true"

    def sh(self, tokens: list[str], timeout: int = 60) -> subprocess.CompletedProcess:
        """Run a shell command on the guest (alias of ssh_run; named for readability at call sites that
        are PowerShell-specific)."""
        return self.ssh_run(tokens, timeout=timeout)

    def reachable(self) -> bool:
        return subprocess.run(self._ssh() + [self.probe_cmd], capture_output=True).returncode == 0

    def reboot_and_wait(self) -> None:
        """Reboot the VM (passwordless `sudo reboot`) and block until SSH is back AND the Aqua session has
        settled (load average dropped), so app windows attach to a healthy WindowServer. Best-effort."""
        print(f"[{self.name}] rebooting VM for a clean session …")
        # Windows needs no sudo (the SSH user is an Administrator on a test VM) and has no `uptime`; the
        # post-boot settle below simply polls SSH, which on Windows is late enough that the shell and
        # the interactive session are already up.
        reboot_cmd = "shutdown /r /t 0 /f" if self.is_windows else "/usr/bin/sudo -n /sbin/reboot"
        subprocess.run(self._ssh() + [reboot_cmd], capture_output=True, timeout=30)
        time.sleep(8)  # let it actually go down before we start polling for it to come back
        for _ in range(40):  # ~2min for SSH to return
            if subprocess.run(["ssh", "-o", "BatchMode=yes", "-o", "ConnectTimeout=4", self.hostspec,
                               self.probe_cmd], capture_output=True).returncode == 0:
                break
            time.sleep(3)
        else:
            print(f"  ! {self.name}: SSH did not return after reboot; continuing anyway")
            return
        if self.is_windows:
            # No load-average equivalent worth parsing. Windows accepts SSH only once Session Manager is
            # up, which is late in boot, so a short fixed settle is enough — and a `uptime` poll here
            # would just burn 100s failing on a command the guest does not have.
            time.sleep(10)
        else:
            for _ in range(20):  # wait for post-boot load to settle (services starting)
                out = subprocess.run(self._ssh() + ["/usr/bin/uptime"], capture_output=True, text=True).stdout
                m = re.search(r"load averages?:\s*([0-9.]+)", out)
                if m and float(m.group(1)) < 4.0:
                    break
                time.sleep(5)
        print(f"  {self.name}: back up")

    def mkdirs(self, remote: str) -> None:
        if self.is_windows:
            # PowerShell's New-Item -Force is the idempotent mkdir -p (cmd.exe's mkdir errors when the
            # directory exists). Requires the guest's OpenSSH DefaultShell to be PowerShell — which the
            # provisioning doc mandates anyway, because ssh_run() shell-quotes with shlex (POSIX single
            # quotes) and cmd.exe does not understand single quotes at all.
            self.ssh_run(["New-Item", "-ItemType", "Directory", "-Force", "-Path", remote])
        else:
            self.ssh_run(["/bin/mkdir", "-p", remote])

    def deploy(self, local: Path, remote: str) -> None:
        self.mkdirs(posixpath.dirname(remote))
        if self.is_windows:
            # A Windows guest has no rsync, so: clear the destination then scp -r. The explicit delete
            # preserves rsync --delete's semantics — without it a stale DLL or an old page asset from a
            # previous run lingers in the deployed folder and is silently picked up at launch.
            self.ssh_run(["Remove-Item", "-Recurse", "-Force", "-ErrorAction", "SilentlyContinue", remote])
            if local.is_dir():
                self.mkdirs(remote)
                # scp -r <dir>/. copies the CONTENTS into remote (not remote/<dir>), matching rsync's
                # trailing-slash behaviour used on the macOS path.
                src = str(local) + "/."
            else:
                src = str(local)
            subprocess.run(["scp", "-r", "-o", "BatchMode=yes", src, f"{self.hostspec}:{remote}"],
                           check=True)
            return
        src = str(local) + ("/" if local.is_dir() else "")
        dst = f"{self.hostspec}:{remote}" + ("/" if local.is_dir() else "")
        subprocess.run(["rsync", "-a", "--delete", "-e", "ssh -o BatchMode=yes", src, dst], check=True)

    def pull(self, remote: str, local: Path) -> bool:
        local.parent.mkdir(parents=True, exist_ok=True)
        rc = subprocess.run(["scp", "-o", "BatchMode=yes", f"{self.hostspec}:{remote}", str(local)]).returncode
        return rc == 0 and local.is_file()

    # -- guest agent --------------------------------------------------------
    def agent(self, subcmd: str, *args, timeout: int = 120) -> dict:
        if self.session1 is not None:
            # Session 1: the only place window enumeration, input injection and PrintWindow can see the
            # real desktop. Same subcommand names/args as the SSH form.
            return self.session1.call(subcmd, *args, timeout=float(timeout))
        if self.is_windows:
            # No /usr/bin/env and no per-tool path overrides to inject: the Windows agent talks to
            # user32/gdi32 through ctypes, so it has no external tools to locate (the whole reason it
            # needs nothing provisioned but Python). Invoke the interpreter directly.
            tokens = [self.python3, self.agent_remote, subcmd, *map(str, args)]
        else:
            env_prefix = [f"{k}={v}" for k, v in {
                "MAUI_E2E_CLICLICK": self.tools.get("cliclick"),
                "MAUI_E2E_DISPLAYPLACER": self.tools.get("displayplacer"),
                "MAUI_E2E_SCREENCAPTURE": self.tools.get("screencapture"),
                "MAUI_E2E_OPEN": self.tools.get("open"),
            }.items() if v]
            tokens = ["/usr/bin/env", *env_prefix, self.python3, self.agent_remote, subcmd,
                      *map(str, args)]
        try:
            r = self.ssh_run(tokens, timeout=timeout)
        except subprocess.TimeoutExpired:
            return {"ok": False, "error": f"agent {subcmd} timed out after {timeout}s"}
        return self._parse(r.stdout, r.stderr, r.returncode)

    @staticmethod
    def _parse(stdout: str, stderr: str, rc: int) -> dict:
        for line in reversed(stdout.strip().splitlines()):
            line = line.strip()
            if line.startswith("{"):
                try:
                    return json.loads(line)
                except json.JSONDecodeError:
                    break
        return {"ok": False, "error": f"no JSON from agent (rc={rc})", "stderr": stderr.strip()[:400]}


# --------------------------------------------------------------------------- drivers
class CoordinateDriver:
    """Absolute-coordinate interaction via the guest agent (cliclick). The default."""

    def __init__(self, env: Env):
        self.env = env

    def run_action(self, step: dict) -> None:
        action = step.get("action")
        if not action:
            return
        if action == "click":
            self.env.agent("click", *step["at"])
        elif action == "type":
            self.env.agent("type", step["text"])
        elif action == "scroll":
            x, y = step["at"]
            self.env.agent("scroll", x, y, step["dy"])
        else:
            raise ValueError(f"unknown scenario action: {action!r}")


class MauiDevFlowDriver(CoordinateDriver):
    """maui_xaml column: tap-by-automationId via Microsoft's MAUI DevFlow CLI; coordinates otherwise.

    EXPERIMENTAL: DevFlow's CLI surface changes between releases. Any DevFlow call that fails falls
    back to the coordinate path, so scenarios that use `at = [x, y]` (no automation_id) work today
    regardless of whether DevFlow is set up on the VM.
    """

    def __init__(self, env: Env, column_cfg: dict):
        super().__init__(env)
        self.cli = column_cfg.get("devflow_cli", "maui")

    def run_action(self, step: dict) -> None:
        if step.get("action") == "click" and step.get("automation_id"):
            try:
                r = self.env.ssh_run([self.cli, "devflow", "agent", "interact", "tap",
                                      "--automationid", step["automation_id"]], timeout=60)
                if r.returncode == 0:
                    return
                print(f"      maui-devflow tap failed (rc={r.returncode}); using coordinates")
            except Exception as e:
                print(f"      maui-devflow tap error ({e}); using coordinates")
        super().run_action(step)


class HttpDevFlowDriver(CoordinateDriver):
    """cpp / cpp_xaml columns: tap-by-automationId via the C++ port's in-app DevFlow agent.

    The agent (port/cpp/src/devflow) exposes JSON-over-HTTP on 127.0.0.1:<MAUI_DEVFLOW_PORT> — see
    port/cpp/docs/DEVFLOW_PROTOCOL.md. It only implements tap (v1), so type/scroll and coordinate taps
    fall back to cliclick. The app must be built with -DMAUI_DEVFLOW=ON and launched with
    MAUI_DEVFLOW_PORT set (the runner does this when a column's driver is "cpp_devflow"). Requests are
    issued with the VM's curl over SSH, so no persistent port-forward is needed.
    """

    def __init__(self, env: Env, column_cfg: dict):
        super().__init__(env)
        self.port = column_cfg.get("devflow_port")

    def run_action(self, step: dict) -> None:
        aid = step.get("automation_id")
        if step.get("action") == "click" and aid and self.port:
            body = json.dumps({"automation_id": aid})
            try:
                r = self.env.ssh_run(["/usr/bin/curl", "-s", "-X", "POST",
                                      f"http://127.0.0.1:{self.port}/tap", "-d", body], timeout=60)
                resp = json.loads(r.stdout.strip() or "{}")
                if r.returncode == 0 and resp.get("found"):
                    return
                print(f"      cpp-devflow tap miss ({r.stdout.strip()[:80]}); using coordinates")
            except Exception as e:
                print(f"      cpp-devflow tap error ({e}); using coordinates")
        super().run_action(step)


def make_driver(env: Env, column: str, column_cfg: dict):
    driver = column_cfg.get("driver", "coordinate")
    if driver == "maui_devflow":
        return MauiDevFlowDriver(env, column_cfg)
    if driver == "cpp_devflow":
        return HttpDevFlowDriver(env, column_cfg)
    return CoordinateDriver(env)


# --------------------------------------------------------------------------- scenarios
def load_scenario(scenarios_dir: Path, tag: str) -> dict:
    """The tag's scenario, or a one-step default (single idle screenshot) when absent."""
    f = scenarios_dir / f"{tag}.toml"
    if f.is_file():
        s = tomllib.loads(f.read_text())
        s.setdefault("steps", [{"name": "initial"}])
        s.setdefault("themes", ["light"])
        return s
    return {"tag": tag, "themes": ["light"], "steps": [{"name": "initial"}]}


# --------------------------------------------------------------------------- run
def columns_for(env: Env, tag: str, twin_keys: set[str] | None) -> list[str]:
    cols = []
    for col in env.columns:
        if col == "cpp" and twin_keys is not None and tag not in twin_keys:
            continue  # builder_twin:false — no code-first page (would capture a wrong-page fallback)
        cols.append(col)
    return cols


def run_env(env: Env, tags: list[str], scenarios_dir: Path, run_root: Path,
            settle: float, twin_keys: set[str] | None, commit: str,
            themes_override: list[str] | None = None) -> dict:
    if env.reboot_before_run:
        env.reboot_and_wait()
    print(f"[{env.name}] reachability check …")
    if not env.reachable():
        raise SystemExit(f"[{env.name}] SSH not reachable: ssh -o BatchMode=yes {env.hostspec} true failed")

    print(f"[{env.name}] deploy agent + clean scratch + set resolution")
    env.deploy(env.agent_src, env.agent_remote)

    if env.is_windows:
        # Windows Session 0 isolation: sshd has no desktop, and window enumeration / input / PrintWindow
        # are per-session. Every agent call from here on goes through an agent running in session 1.
        # Started ONCE per run rather than per call -- `schtasks /run` costs ~1-2s, which across a
        # multi-page board would dominate the wall clock.
        sys.path.insert(0, str(REPO / "port/cpp/tools/parity/windows"))
        from session1 import Session1Agent  # noqa: PLC0415  optional, Windows-only dependency
        s1 = Session1Agent(env.cfg["connection"]["host"], env.cfg["connection"]["user"],
                           staging=env.cfg["staging"]["root"], python=env.python3)
        s1.deploy(env.agent_src)
        started = s1.start(restart=True)
        sid = started.get("session_id")
        if not started.get("ok") or sid in (0, None):
            raise SystemExit(
                f"[{env.name}] could not start the guest agent in session 1: "
                f"{started.get('error') or started}\n"
                f"  hint: {started.get('hint') or 'is the guest user logged on at the CONSOLE? '
                                                  '/it needs an interactive session (an RDP login moves it)'}\n"
                f"  guest log: {s1.tail_log(10)}")
        print(f"  agent serving in session {sid} via {started.get('serving')}")
        env.session1 = s1

    env.agent("clean", env.scratch)
    if env.display:
        r = env.agent("set-resolution", env.display["width"], env.display["height"])
        if not r.get("ok"):
            print(f"  ! set-resolution: {r.get('error') or r.get('stderr')}")

    # Deploy each column's artifact once. A column may instead declare `artifact_remote`: a path that
    # ALREADY exists on the guest, which is then used as-is with no deploy. That is the case for a column
    # built ON the guest -- MAUI's Windows target is WinUI 3 and cannot be cross-built from macOS, so the
    # reference app is compiled there; copying its output down to the host only to push it back would move
    # ~100MB twice for no reason.
    for col, ccfg in env.columns.items():
        if ccfg.get("artifact_remote"):
            remote = ccfg["artifact_remote"]
            check = env.sh(["Test-Path", remote]) if env.is_windows else env.ssh_run(["test", "-e", remote])
            present = ("True" in (check.stdout or "")) if env.is_windows else check.returncode == 0
            if not present:
                print(f"  ! {col}: artifact_remote missing on the guest: {remote} (skipping this column)")
                ccfg["_missing"] = True
                continue
            print(f"  use {col}: {remote} (built on the guest; not deployed)")
            ccfg["_remote"] = remote
            continue
        local = REPO / ccfg["artifact"]
        if not local.exists():
            print(f"  ! {col}: artifact missing on host: {local} (skipping this column)")
            ccfg["_missing"] = True
            continue
        remote = posixpath.join(env.apps_remote, col, local.name)
        print(f"  deploy {col}: {local.name}")
        env.deploy(local, remote)
        ccfg["_remote"] = remote

    remote_shot = posixpath.join(env.scratch, "shot.png")
    frames: dict[tuple, dict] = {}  # (tag, column, n) -> {theme, step, local}
    failed_frames: list[str] = []   # frames dropped as un-presentable (see shoot_presented)
    for tag in tags:
        scenario = load_scenario(scenarios_dir, tag)
        themes = themes_override or scenario["themes"]
        for col in columns_for(env, tag, twin_keys):
            ccfg = env.columns[col]
            if ccfg.get("_missing"):
                continue
            driver = make_driver(env, col, ccfg)
            n = 0
            for theme in themes:
                theme_val = ccfg[f"theme_{theme}"]
                launch_env = [f"{ccfg['page_env']}={tag}", f"{ccfg['theme_env']}={theme_val}",
                              "MAUI_CAPTURE_TINT_NORMAL=1",
                              # In-process WinUI keyboard-focus-visual suppression (PARITY_REVIEW.md
                              # "Focus-visual suppression" section) -- consumed only by App.xaml.cs's
                              # `#if WINDOWS` block, harmlessly unused on every other column/platform, same
                              # shape as MAUI_CAPTURE_TINT_NORMAL above. Passed unconditionally so all three
                              # columns get it identically rather than only the reference column.
                              "MAUI_SUPPRESS_FOCUS_VISUAL=1",
                              # Where BOTH columns write diagnostics: the C++ backend's boot_log()
                              # (host_run.cpp) and App.xaml.cs's focus-suppression line. Without this
                              # they go nowhere -- cmd_launch uses Popen(DETACHED_PROCESS) with no stdout
                              # redirect (vm_agent_windows.py) and an unpackaged WinUI 3 app has no
                              # console attached, so Console.Out is discarded outright. Per-column and
                              # per-theme so two columns never interleave into one file.
                              f"MAUI_WINUI_LOG={posixpath.dirname(ccfg['_remote'])}\\diag_{col}_{theme}.log"]
                # Raw-glyph BGRA dump (image_source_services.cpp's render_font_glyph). Forwarded ONLY when
                # the HOST sets it, so an ordinary board run does not write a .bgra per font source per
                # page. Kept because looking at the bitmap is what finally settled the `image` page after
                # five wrong hypotheses; the guest path must be writable by the app process.
                if os.environ.get("MAUI_FONT_GLYPH_DUMP"):
                    launch_env.append(f"MAUI_FONT_GLYPH_DUMP={os.environ['MAUI_FONT_GLYPH_DUMP']}")
                if ccfg.get("driver") == "cpp_devflow" and ccfg.get("devflow_port"):
                    launch_env.append(f"MAUI_DEVFLOW_PORT={ccfg['devflow_port']}")  # starts the in-app agent
                launch_args = ["launch", "--bundle", ccfg["_remote"], "--proc", ccfg["process"]]
                for kv in launch_env:
                    launch_args += ["--env", kv]
                res = env.agent(*launch_args)
                pid = res.get("pid")
                if pid is None:
                    print(f"  ! {tag}/{col}/{theme}: launch failed: {res.get('error')}")
                    continue
                time.sleep(settle)
                # With `present`, we activate + set an explicit rect right before EACH shot and capture that
                # exact rect — no window-id call in between (a System Events query there steals key focus back
                # and greys the traffic lights). Otherwise, resolve the window rect once up front.
                win_id = win_rect = bounds = None
                g = env.geom
                if not env.present:
                    win = env.agent("window-id", pid, "--proc", ccfg["process"])
                    win_id, win_rect, bounds = win.get("id"), win.get("rect"), win.get("bounds")
                try:
                    for step in scenario["steps"]:
                        driver.run_action(step)
                        time.sleep(settle)
                        n += 1
                        if env.present:
                            # `or bounds` HERE WAS THE WORST BUG IN THIS RUNNER. shoot_presented returns
                            # None when it cannot present after the self-heal, and its docstring says to
                            # "let the caller drop the frame" -- but `or bounds` swallowed the None, kept
                            # the previous bounds and fell through to the pull. A failed present never
                            # overwrites remote_shot, so the pull then fetched the PREVIOUS COLUMN'S file:
                            # a perfectly valid 1024x800 capture of a different app. The size guard below
                            # cannot catch that (the dimensions are right) and `launch failures: 0` does
                            # not either (the launch succeeded; the PRESENT failed).
                            #
                            # Measured cost: header_footer_grid banked MAUI's dark frame in both cpp
                            # columns and scored ~80% for a full day as a phantom port defect, across four
                            # separate full board passes. check_capture_integrity.py found it only by
                            # hashing bytes across columns (3b95065b77, 3a15e1613c).
                            presented = shoot_presented(env, ccfg, g, remote_shot, pid=pid)
                            if presented is None:
                                print(f"  ! {tag}/{col}/{theme}#{n}: DROPPED — present failed after "
                                      f"self-heal (no window to capture)")
                                failed_frames.append(f"{tag}/{col}/{theme}#{n}")
                                continue
                            bounds = presented
                        elif win_id:
                            env.agent("shot", remote_shot, "--window", win_id)
                        elif win_rect:
                            env.agent("shot", remote_shot, "--rect", win_rect)
                        else:
                            env.agent("shot", remote_shot)  # whole-display last resort
                        local = run_root / tag / env.platform / col / f"{n:04d}.png"
                        if not env.pull(remote_shot, local):
                            print(f"  ! {tag}/{col}/{theme}#{n}: capture pull failed")
                            continue
                        # The window is presented at an EXPLICIT size, so a correct shot is exactly g[w]xg[h].
                        # ANY other size is a failed present, and neither failure looks broken on the board:
                        #   - LARGER  => the desktop (present returned no rect, or an occluding window);
                        #   - SMALLER => the window had not finished resizing to the target. This is what a
                        #     freshly-restarted / loaded VM produces: `present`'s retry loop times out before
                        #     Catalyst settles the window and returns the actual (short) rect, e.g. 1024x548.
                        #     The old guard only caught LARGER, so ~400 short frames banked silently before a
                        #     size sweep caught them. pixel_score then LANCZOS-resizes the mismatch into noise.
                        # Reject either way — the frame is dropped, reported, and the page re-run when healthy.
                        size = png_size(local)
                        if size is not None and (abs(size[0] - g["w"]) > 4 or abs(size[1] - g["h"]) > 4):
                            why = "desktop captured" if size[1] > g["h"] + 4 else "window not settled to target"
                            print(f"  ! {tag}/{col}/{theme}#{n}: DROPPED — {size[0]}x{size[1]} is not the "
                                  f"{g['w']}x{g['h']} window ({why})")
                            local.unlink(missing_ok=True)
                            failed_frames.append(f"{tag}/{col}/{theme}#{n}")
                            continue
                        sidecar = {
                            "tag": tag, "platform": env.platform, "column": col, "theme": theme,
                            "step": step.get("name", f"step{n}"), "frame": n,
                            "window_bounds": bounds, "commit": commit,
                            "captured_at": datetime.now().astimezone().isoformat(),
                        }
                        local.with_suffix(".json").write_text(json.dumps(sidecar, indent=2))
                        frames[(tag, col, n)] = {"theme": theme, "step": sidecar["step"], "local": local}
                        print(f"  {tag}/{col}/{theme} {sidecar['step']:16} -> {local.relative_to(run_root)}")
                finally:
                    env.agent("stop", pid)
                    time.sleep(0.3)

    if failed_frames:
        # Loud + non-optional: a sweep that silently omits frames looks identical to a clean one, and
        # the affected pages would keep whatever STALE capture the board already had.
        print(f"\n  !! {len(failed_frames)} frame(s) DROPPED (window never presented) — these pages are\n     NOT refreshed; re-run --only <pages> once the VM session is healthy:")
        for f in failed_frames:
            print(f"       {f}")

    if env.session1 is not None:
        # Always tear down: a leaked agent keeps the guest port AND its scheduled task, so the next run
        # meets a stale agent holding a token it cannot authenticate against.
        env.session1.stop(quiet=True)
        env.session1 = None
        print(f"[{env.name}] session-1 agent stopped")

    summary = score(env, tags, run_root, frames)
    summary["dropped_frames"] = failed_frames
    return summary


def score(env: Env, tags: list[str], run_root: Path, frames: dict) -> dict:
    """Pixel-score maui_xaml vs cpp / cpp_xaml per frame; return the per-column summary."""
    summary = {"platform": env.platform, "pages": {}}
    cpp_cols = [c for c in env.columns if c != "maui_xaml"]
    for tag in tags:
        maxframe = max((n for (t, c, n) in frames if t == tag), default=0)
        page = {}
        # per-column theme-grouped scores for a final classify()
        grouped: dict[str, dict] = {c: {} for c in cpp_cols}
        compare_dir = run_root / tag / env.platform / "compare"
        for n in range(1, maxframe + 1):
            maui = frames.get((tag, "maui_xaml", n))
            if not maui:
                continue
            report = {"frame": n, "step": maui["step"], "theme": maui["theme"], "scores": {}}
            for col in cpp_cols:
                other = frames.get((tag, col, n))
                if not other:
                    continue
                s = pixel_score.score_theme(str(maui["local"].relative_to(COMP)),
                                            str(other["local"].relative_to(COMP)))
                report["scores"][col] = s
                if s is not None:
                    grouped[col].setdefault(maui["theme"], []).append(s)
            compare_dir.mkdir(parents=True, exist_ok=True)
            (compare_dir / f"{n:04d}-report.json").write_text(json.dumps(report, indent=2))
        # worst-case classify per column across all its frames
        for col in cpp_cols:
            # reduce each theme's frames to its worst (min ssim / max diff), then classify the themes.
            theme_scores = {}
            for theme, lst in grouped[col].items():
                if lst:
                    theme_scores[theme] = {"ssim": min(x["ssim"] for x in lst),
                                           "diff_pct": max(x["diff_pct"] for x in lst)}
            if theme_scores:
                status, review = pixel_score.classify(theme_scores)
                page[col] = {"status": status, "review": review}
        if page:
            summary["pages"][tag] = page
    return summary


def main(argv=None) -> int:
    ap = argparse.ArgumentParser(description="E2E visual-comparison runner")
    ap.add_argument("--config", required=True)
    ap.add_argument("--env", help="comma-separated environment names (default: all in config)")
    ap.add_argument("--only", help="comma-separated tags (default: all manifest pages minus gap_*)")
    ap.add_argument("--scenarios", default=str(COMP / "scenarios"), help="scenarios dir")
    ap.add_argument("--settle", type=float, default=1.0, help="seconds to settle after each action")
    ap.add_argument("--themes", help="comma-separated theme override for ALL tags, e.g. 'light,dark'")
    ap.add_argument("--plan", action="store_true", help="validate config/scenarios and print the plan; no SSH")
    args = ap.parse_args(argv)
    themes_override = [t.strip() for t in args.themes.split(",")] if args.themes else None

    cfg = tomllib.loads(Path(args.config).read_text())
    out_root = REPO / cfg.get("output", {}).get("root", "port/cpp/docs/comparison")
    ts_fmt = cfg.get("output", {}).get("timestamp_format", "%Y-%m-%d-%H_%M_%S")

    manifest = load_manifest()
    all_tags = [r["key"] for r in manifest if not r["key"].startswith("gap_")]
    twin_keys = {r["key"] for r in manifest if r.get("builder_twin", True)} or None
    tags = [t.strip() for t in args.only.split(",")] if args.only else all_tags
    if not tags:
        print("no tags to run (empty manifest and no --only)")
        return 2

    env_names = args.env.split(",") if args.env else list(cfg["environments"])
    scenarios_dir = Path(args.scenarios)

    if args.plan:
        print(f"plan: {len(tags)} tag(s) x envs {env_names}")
        for name in env_names:
            env = Env(name, cfg["environments"][name])
            print(f"  env {name}: platform={env.platform} columns={list(env.columns)} host={env.hostspec}")
        custom = [t for t in tags if (scenarios_dir / f"{t}.toml").is_file()]
        print(f"  {len(custom)} tag(s) have a scenario file; the rest get one idle screenshot: {custom[:20]}")
        return 0

    stamp = datetime.now().strftime(ts_fmt)
    run_root = out_root / stamp
    disp = run_root.relative_to(REPO) if run_root.is_relative_to(REPO) else run_root
    commit = git_commit()
    print(f"run {stamp}  ->  {disp}  ({len(tags)} tag(s), commit {commit})")

    all_summaries = {}
    for name in env_names:
        env = Env(name, cfg["environments"][name])
        try:
            summary = run_env(env, tags, scenarios_dir, run_root,
                              args.settle, twin_keys, commit, themes_override)
        finally:
            # run_env stops the session-1 agent on its normal path; this covers the abort paths (a
            # SystemExit from an unpresentable window, Ctrl-C, a scoring error). A leaked agent holds the
            # guest port and its scheduled task, which breaks the NEXT run with a stale-token failure.
            if getattr(env, "session1", None) is not None:
                env.session1.stop(quiet=True)
                env.session1 = None
        all_summaries[name] = summary

    (run_root / "summary.json").write_text(json.dumps(all_summaries, indent=2))
    (run_root / "run-manifest.json").write_text(json.dumps({
        "timestamp": stamp, "commit": commit, "tags": tags, "environments": env_names,
    }, indent=2))
    _write_report_md(run_root, all_summaries)
    print(f"\ndone -> {disp} (summary.json, report.md)")
    return 0


def _write_report_md(run_root: Path, summaries: dict) -> None:
    lines = [f"# Comparison run {run_root.name}\n"]
    for env_name, summary in summaries.items():
        lines.append(f"## {env_name} ({summary['platform']})\n")
        lines.append("| Tag | Column | Status | Review |")
        lines.append("|---|---|---|---|")
        for tag, cols in sorted(summary["pages"].items()):
            for col, v in cols.items():
                lines.append(f"| {tag} | {col} | {v['status']} | {v['review']} |")
        lines.append("")
    (run_root / "report.md").write_text("\n".join(lines))


if __name__ == "__main__":
    sys.exit(main())
