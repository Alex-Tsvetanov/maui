#!/usr/bin/env python3
"""Host-side transport for the Windows guest agent running in SESSION 1.

WHY THIS MODULE EXISTS
----------------------
Windows Session 0 isolation. `sshd` runs in session 0 (services, no desktop); the interactive console
the VM actually displays is session 1. So a GUI app launched over SSH has no window on any visible
desktop, and EnumWindows / SendInput / PrintWindow are all PER-SESSION -- an agent living in the SSH
session can neither see nor drive the real UI. (Verified on the UTM guest: an app started over SSH ran
fine with MainWindowHandle=0, while the same agent run inside session 1 found its window immediately.)
macOS has no equivalent problem, which is why vm_agent_macos.py needs nothing like this.

HOW IT WORKS
------------
    host                                    guest
    ----                                    -----
    ssh (session 0) ------- schtasks /it --> agent `serve` in SESSION 1, bound to 127.0.0.1:<gport>
    ssh -N -L <lport>:127.0.0.1:<gport> ---> (tunnel)
    TCP to 127.0.0.1:<lport> --------------> one JSON request/response per connection

Two deliberate choices:

* The agent binds LOOPBACK and is reached through an SSH tunnel, not exposed on the network. It is a
  command-execution endpoint, so this keeps it behind the same SSH key that already administers the
  guest and adds no firewall hole. A shared token is required on every request as defence in depth,
  because any local account on the guest could otherwise reach that loopback port.
* Args and results cross via a generated .cmd wrapper and files rather than being embedded in the
  schtasks command line. Nesting quotes through schtasks -> cmd -> PowerShell is a reliable source of
  silent breakage; a file has no quoting.

Why a persistent server rather than one scheduled task per call: `schtasks /run` costs ~1-2s. That is
fine for a smoke test but ~2200 agent calls across a 182-page board would spend the better part of an
hour purely on task startup. The project already uses a long-lived TCP agent for the same reason (the
macOS config's `devflow_port = 8765`).

Usage as a library:
    from session1 import Session1Agent
    with Session1Agent("WINDOWS-VM.local", "Testings-VM") as ag:
        print(ag.call("window-id", pid))

Usage as a CLI (handy for debugging a guest by hand):
    session1.py --host WINDOWS-VM.local --user Testings-VM status
    session1.py --host … --user … start
    session1.py --host … --user … call window-id 1234
    session1.py --host … --user … stop
"""
from __future__ import annotations

import argparse
import json
import posixpath
import secrets
import shlex
import socket
import subprocess
import sys
import time

DEFAULT_GUEST_PORT = 8770
TASK_NAME = "maui-agent-session1"


class Session1Agent:
    """Starts (or reuses) the guest agent in session 1 and talks to it over an SSH tunnel."""

    def __init__(self, host: str, user: str,
                 staging: str = "C:/Users/%(user)s/maui-comparison",
                 python: str = "py", guest_port: int = DEFAULT_GUEST_PORT,
                 local_port: int = 0, verbose: bool = False):
        self.hostspec = f"{user}@{host}"
        self.user = user
        self.staging = (staging % {"user": user}).rstrip("/")
        self.python = python
        self.guest_port = guest_port
        # 0 -> pick a free local port, so concurrent runs against different guests cannot collide.
        self.local_port = local_port or _free_local_port()
        self.verbose = verbose
        self.token = secrets.token_hex(16)
        self._tunnel: subprocess.Popen | None = None
        self.agent_remote = posixpath.join(self.staging, "vm_agent_windows.py")
        self.token_remote = posixpath.join(self.staging, "agent_token.txt")
        self.cmd_remote = posixpath.join(self.staging, "serve_session1.cmd")
        self.log_remote = posixpath.join(self.staging, "agent_serve.log")
        self.session_id: int | None = None

    # -- ssh plumbing ------------------------------------------------------
    def _ssh(self, extra: list[str] | None = None) -> list[str]:
        return ["ssh", "-o", "BatchMode=yes", "-o", "ConnectTimeout=10", *(extra or []), self.hostspec]

    def sh(self, tokens: list[str], timeout: int = 60) -> subprocess.CompletedProcess:
        # shlex.join yields POSIX single quotes, which PowerShell also treats as literal strings -- hence
        # the DefaultShell=PowerShell requirement. Keep remote probes free of `$`: a $-expression arrives
        # single-quoted and PowerShell echoes it verbatim instead of evaluating it.
        return subprocess.run(self._ssh() + [shlex.join(tokens)], capture_output=True, text=True,
                              timeout=timeout)

    def _log(self, msg: str) -> None:
        if self.verbose:
            print(f"[session1] {msg}", file=sys.stderr)

    # -- lifecycle ---------------------------------------------------------
    def deploy(self, agent_src) -> None:
        """Copy the agent + a freshly generated token to the guest."""
        self.sh(["New-Item", "-ItemType", "Directory", "-Force", "-Path", self.staging])
        subprocess.run(["scp", "-q", "-o", "BatchMode=yes", str(agent_src),
                        f"{self.hostspec}:{self.agent_remote}"], check=True)
        # Written via a here-string over SSH rather than scp'ing a temp file: the token never touches the
        # host filesystem, and Set-Content with -Encoding ascii matches the agent's read.
        self.sh(["Set-Content", "-Path", self.token_remote, "-Value", self.token,
                 "-Encoding", "ascii"])
        self._log(f"deployed agent + token to {self.staging}")

    def _write_cmd_wrapper(self) -> None:
        """Write the .cmd the scheduled task runs. A file, not an argument, so nothing needs quoting."""
        line = (f'@echo off\r\n'
                f'"{_win(self.python)}" "{_win(self.agent_remote)}" serve '
                f'--host 127.0.0.1 --port {self.guest_port} '
                f'--token-file "{_win(self.token_remote)}" > "{_win(self.log_remote)}" 2>&1\r\n')
        # Out-File with -Encoding ascii: the guest runs this under cmd.exe, and a UTF-16 or BOM-prefixed
        # .cmd fails to parse with a cryptic error. (Same class of bug as the ASCII-only rule for .ps1.)
        self.sh(["Set-Content", "-Path", self.cmd_remote, "-Value", line, "-Encoding", "ascii",
                 "-NoNewline"])

    def start(self, restart: bool = False) -> dict:
        """Register + run the session-1 task, open the tunnel, and confirm the agent answers."""
        if not restart:
            probe = self._try_connect_existing()
            if probe is not None:
                self._log("reusing a live agent")
                self.session_id = probe.get("session_id")
                return probe

        self.stop(quiet=True)
        self._write_cmd_wrapper()
        self.sh(["Remove-Item", self.log_remote, "-ErrorAction", "SilentlyContinue"])
        # /it = run in the INTERACTIVE session (requires the user to be logged on at the console). This
        # single flag is the whole point: without it the task runs in session 0 and we are back to
        # invisible windows.
        create = self.sh(["schtasks", "/create", "/tn", TASK_NAME, "/tr", _win(self.cmd_remote),
                          "/sc", "ONCE", "/st", "00:00", "/ru", self.user, "/it", "/f"])
        if create.returncode != 0:
            return {"ok": False, "error": "schtasks /create failed",
                    "stderr": (create.stderr or create.stdout or "")[-300:]}
        run = self.sh(["schtasks", "/run", "/tn", TASK_NAME])
        if run.returncode != 0:
            return {"ok": False, "error": "schtasks /run failed",
                    "stderr": (run.stderr or run.stdout or "")[-300:]}

        self._open_tunnel()
        for _ in range(40):  # ~10s for python to start and bind
            probe = self._try_connect_existing()
            if probe is not None:
                self.session_id = probe.get("session_id")
                if self.session_id == 0:
                    return {"ok": False, "error": "agent started in SESSION 0 -- the task did not run "
                                                  "interactively. Is the guest user logged on at the "
                                                  "console?", **probe}
                self._log(f"agent up in session {self.session_id}")
                return probe
            time.sleep(0.25)
        return {"ok": False, "error": "agent did not answer after start", "log": self.tail_log()}

    def _open_tunnel(self) -> None:
        if self._tunnel is not None and self._tunnel.poll() is None:
            return
        self._tunnel = subprocess.Popen(
            self._ssh(["-N", "-L", f"{self.local_port}:127.0.0.1:{self.guest_port}"]),
            stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        self._log(f"tunnel 127.0.0.1:{self.local_port} -> guest 127.0.0.1:{self.guest_port}")
        time.sleep(0.6)  # let ssh establish the forward before the first connect

    def _try_connect_existing(self) -> dict | None:
        self._open_tunnel()
        try:
            reply = self._raw_call({"token": self.token, "cmd": "__ping__"}, timeout=3)
        except OSError:
            return None
        return reply if reply.get("ok") else None

    def stop(self, quiet: bool = False) -> None:
        """Ask the agent to exit, then remove the task and close the tunnel."""
        try:
            self._raw_call({"token": self.token, "cmd": "__shutdown__"}, timeout=3)
        except OSError:
            pass
        self.sh(["schtasks", "/end", "/tn", TASK_NAME])
        self.sh(["schtasks", "/delete", "/tn", TASK_NAME, "/f"])
        if self._tunnel is not None:
            self._tunnel.terminate()
            try:
                self._tunnel.wait(timeout=5)
            except subprocess.TimeoutExpired:
                self._tunnel.kill()
            self._tunnel = None
        if not quiet:
            self._log("stopped")

    def tail_log(self, lines: int = 20) -> str:
        r = self.sh(["Get-Content", self.log_remote, "-Tail", str(lines)])
        return (r.stdout or r.stderr or "").strip()[-800:]

    # -- calls -------------------------------------------------------------
    def _raw_call(self, payload: dict, timeout: float = 120.0) -> dict:
        with socket.create_connection(("127.0.0.1", self.local_port), timeout=timeout) as sock:
            sock.settimeout(timeout)
            sock.sendall((json.dumps(payload) + "\n").encode())
            chunks: list[bytes] = []
            while b"\n" not in b"".join(chunks):
                part = sock.recv(65536)
                if not part:
                    break
                chunks.append(part)
        raw = b"".join(chunks).split(b"\n", 1)[0].decode("utf-8", "replace").strip()
        if not raw:
            return {"ok": False, "error": "empty reply from agent"}
        try:
            return json.loads(raw)
        except json.JSONDecodeError as e:
            return {"ok": False, "error": f"bad JSON from agent: {e}", "raw": raw[:300]}

    def call(self, cmd: str, *args, timeout: float = 120.0) -> dict:
        """Run one agent subcommand in session 1. Same names/args as the CLI form."""
        try:
            return self._raw_call({"token": self.token, "cmd": cmd,
                                   "args": [str(a) for a in args]}, timeout=timeout)
        except OSError as e:
            return {"ok": False, "error": f"transport: {type(e).__name__}: {e}", "cmd": cmd}

    # -- context manager ---------------------------------------------------
    def __enter__(self) -> Session1Agent:
        return self

    def __exit__(self, *_exc) -> None:
        self.stop(quiet=True)


def _win(p: str) -> str:
    """Backslashed form, for the places that are parsed by cmd.exe / schtasks rather than by Python."""
    return p.replace("/", "\\")


def _free_local_port() -> int:
    with socket.socket() as s:
        s.bind(("127.0.0.1", 0))
        return int(s.getsockname()[1])


def main(argv=None) -> int:
    from pathlib import Path
    here = Path(__file__).resolve().parent
    default_agent = here.parents[2] / "docs/comparison/tools/vm_agent_windows.py"

    ap = argparse.ArgumentParser(description="Drive the Windows guest agent in session 1")
    ap.add_argument("--host", required=True)
    ap.add_argument("--user", required=True)
    ap.add_argument("--python", default="py")
    ap.add_argument("--guest-port", type=int, default=DEFAULT_GUEST_PORT)
    ap.add_argument("--agent", default=str(default_agent))
    ap.add_argument("-v", "--verbose", action="store_true")
    ap.add_argument("action", choices=("start", "stop", "status", "call", "log"))
    ap.add_argument("rest", nargs="*", help="for `call`: <subcommand> [args…]")
    a = ap.parse_args(argv)

    ag = Session1Agent(a.host, a.user, python=a.python, guest_port=a.guest_port, verbose=True)
    if a.action == "stop":
        ag.stop()
        print(json.dumps({"ok": True, "stopped": True}))
        return 0
    if a.action == "log":
        print(ag.tail_log(40))
        return 0
    if a.action == "status":
        probe = ag._try_connect_existing()
        print(json.dumps(probe or {"ok": False, "error": "no live agent (token differs or not running)"}))
        return 0 if probe else 1

    ag.deploy(a.agent)
    started = ag.start(restart=True)
    if not started.get("ok"):
        print(json.dumps(started))
        return 1
    if a.action == "start":
        print(json.dumps(started))
        print("NOTE: this CLI generates a fresh token per invocation, so the agent it started cannot be "
              "reused by a later CLI call. Use the library form for multi-call sessions.", file=sys.stderr)
        return 0
    if not a.rest:
        print(json.dumps({"ok": False, "error": "call needs a subcommand"}))
        return 2
    out = ag.call(a.rest[0], *a.rest[1:])
    print(json.dumps(out))
    ag.stop(quiet=True)
    return 0 if out.get("ok") else 1


if __name__ == "__main__":
    sys.exit(main())
