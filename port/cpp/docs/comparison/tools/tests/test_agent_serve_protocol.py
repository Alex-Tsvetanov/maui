#!/usr/bin/env python3
"""Off-guest test of the agent's session-1 `serve` protocol.

The Win32 subcommands need a Windows desktop, but the TRANSPORT does not: framing, token auth, the
dispatch-through-stdout-capture trick, and error handling are all pure Python. Those are exactly the
parts that would otherwise only be exercised against a live VM -- and a transport bug there looks like
"the guest is broken", which is expensive to chase. So this runs the real server on loopback here and
speaks the real protocol to it, using `clean` as the payload because it is the one subcommand with no
Win32 dependency.

Run: python3 tests/test_agent_serve_protocol.py
"""
from __future__ import annotations

import importlib.util
import json
import os
import socket
import subprocess
import sys
import tempfile
import time

HERE = os.path.dirname(os.path.abspath(__file__))
AGENT = os.path.join(os.path.dirname(HERE), "vm_agent_windows.py")

_failures = 0


def check(name: str, cond: bool, detail: str = "") -> None:
    global _failures
    print(f"  {'ok  ' if cond else 'FAIL'} {name}" + (f": {detail}" if detail else ""))
    if not cond:
        _failures += 1


def _free_port() -> int:
    with socket.socket() as s:
        s.bind(("127.0.0.1", 0))
        return int(s.getsockname()[1])


def _call(port: int, payload: dict, timeout: float = 5.0) -> dict:
    with socket.create_connection(("127.0.0.1", port), timeout=timeout) as sock:
        sock.settimeout(timeout)
        sock.sendall((json.dumps(payload) + "\n").encode())
        chunks: list[bytes] = []
        while b"\n" not in b"".join(chunks):
            part = sock.recv(65536)
            if not part:
                break
            chunks.append(part)
    raw = b"".join(chunks).split(b"\n", 1)[0].decode()
    return json.loads(raw) if raw.strip() else {}


def main() -> int:
    spec = importlib.util.spec_from_file_location("vm_agent_windows", AGENT)
    mod = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(mod)
    print(f"agent serve-protocol checks (host={sys.platform})")

    # _dispatch in-process: no socket involved, just the argparse-reuse path.
    with tempfile.TemporaryDirectory() as d:
        target = os.path.join(d, "scratch")
        out = mod._dispatch({"cmd": "clean", "args": [target]})
        check("_dispatch runs a subcommand and returns its JSON",
              out.get("ok") is True and out.get("dir") == target, json.dumps(out)[:120])
        check("_dispatch actually performed the work", os.path.isdir(target))

        bad = mod._dispatch({"cmd": "no-such-command", "args": []})
        check("_dispatch names an unknown subcommand (not a vague 'no JSON')",
              bad.get("ok") is False and "unknown subcommand" in (bad.get("error") or ""),
              json.dumps(bad)[:140])
        check("_dispatch captures argparse's stderr rather than spraying it",
              "invalid choice" in (bad.get("detail") or ""), (bad.get("detail") or "")[:80])

        empty = mod._dispatch({"args": []})
        check("_dispatch rejects a missing cmd", empty.get("ok") is False, json.dumps(empty)[:80])

    # The real server over a real socket, with a token.
    with tempfile.TemporaryDirectory() as d:
        token_file = os.path.join(d, "token.txt")
        with open(token_file, "w", encoding="ascii") as fh:
            fh.write("s3cr3t-token\n")
        port = _free_port()
        proc = subprocess.Popen(
            [sys.executable, AGENT, "serve", "--host", "127.0.0.1", "--port", str(port),
             "--token-file", token_file],
            stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
        try:
            # The server prints one JSON readiness line; the launcher relies on that to know it is up.
            ready = ""
            for _ in range(60):
                if proc.poll() is not None:
                    break
                try:
                    with socket.create_connection(("127.0.0.1", port), timeout=0.3):
                        break
                except OSError:
                    time.sleep(0.1)
            check("server binds and accepts connections", proc.poll() is None)

            pong = _call(port, {"token": "s3cr3t-token", "cmd": "__ping__"})
            check("__ping__ answers with session_id + dpi_mode",
                  pong.get("ok") is True and "session_id" in pong and "dpi_mode" in pong,
                  json.dumps(pong)[:140])

            denied = _call(port, {"token": "wrong", "cmd": "__ping__"})
            check("wrong token is rejected", denied.get("ok") is False
                  and denied.get("error") == "unauthorized", json.dumps(denied)[:100])
            check("rejection does not leak the expected token",
                  "s3cr3t-token" not in json.dumps(denied))

            missing = _call(port, {"cmd": "__ping__"})
            check("absent token is rejected", missing.get("ok") is False, json.dumps(missing)[:100])

            target = os.path.join(d, "remote-scratch")
            done = _call(port, {"token": "s3cr3t-token", "cmd": "clean", "args": [target]})
            check("a real subcommand round-trips over the socket",
                  done.get("ok") is True and os.path.isdir(target), json.dumps(done)[:120])

            junk = None
            with socket.create_connection(("127.0.0.1", port), timeout=5) as sock:
                sock.sendall(b"this is not json\n")
                junk = sock.recv(4096).decode()
            check("malformed JSON gets an error reply, server survives",
                  "bad JSON" in junk and proc.poll() is None, junk.strip()[:100])

            # Two sequential calls prove the server loops rather than serving once and exiting -- the
            # whole point of a persistent agent.
            a1 = _call(port, {"token": "s3cr3t-token", "cmd": "__ping__"})
            a2 = _call(port, {"token": "s3cr3t-token", "cmd": "__ping__"})
            check("server handles repeated connections", a1.get("ok") and a2.get("ok"))

            bye = _call(port, {"token": "s3cr3t-token", "cmd": "__shutdown__"})
            check("__shutdown__ acknowledges", bye.get("ok") is True, json.dumps(bye)[:80])
            for _ in range(40):
                if proc.poll() is not None:
                    break
                time.sleep(0.1)
            check("server exits after __shutdown__", proc.poll() is not None,
                  f"returncode={proc.returncode}")
        finally:
            if proc.poll() is None:
                proc.kill()
            proc.wait(timeout=5)

    # A token file that cannot be read must fail loudly, not serve unauthenticated.
    proc2 = subprocess.run([sys.executable, AGENT, "serve", "--port", str(_free_port()),
                            "--token-file", "/nonexistent/token.txt"],
                           capture_output=True, text=True, timeout=20)
    check("unreadable token file refuses to serve",
          proc2.returncode != 0 and "token" in (proc2.stdout + proc2.stderr).lower(),
          (proc2.stdout or proc2.stderr).strip()[:120])

    print("PASS" if not _failures else f"{_failures} FAILURE(S)")
    return 1 if _failures else 0


if __name__ == "__main__":
    sys.exit(main())
