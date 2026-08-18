#!/usr/bin/env python3
"""Capture each column at its OWN launch geometry, BEFORE the harness resizes the window.

WHY THIS EXISTS
---------------
run_comparison.py calls `present` to force every column to an identical 1024x800 rect before it shoots.
That is correct for parity scoring — two columns must be the same size to be comparable — but it means
the board has NEVER seen the state a real user sees first: the window as the app itself opens it.

resize_probe.py does not cover this either. It presents at a small size and then grows, so its very
first frame is already post-present. A layout that is broken at the app's NATURAL launch size and
correct after any present is invisible to both tools.

That gap is real, not theoretical: on Windows the port's `entry` page opens with its rows OVERLAPPING
each other — "Type here...", "Text", "Placeholder", "FontSize (Large)", "I am read only", "123" all
drawn on top of one another in a squashed band — and then lays out correctly once the window is
resized. Every board frame is post-resize, so every board frame looks fine.

WHAT IT DOES
------------
launch -> settle -> window-id (to learn the app's OWN rect) -> shot by HWND -> pull.
No `present` anywhere: the window is captured exactly as the app opened it, at whatever size that is.
Frames will differ in SIZE between columns, which is expected and is the point — this probe is for
LOOKING at launch layout, not for pixel-scoring two columns against each other.

Usage:
    python3 launch_state_probe.py --config <cfg> --only entry,border_playground \
        --columns maui_xaml,cpp,cpp_xaml --theme light --out <dir>
"""
from __future__ import annotations

import argparse
import json
import posixpath
import sys
import time
import tomllib
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
from run_comparison import REPO, Env, column_artifact, png_size  # noqa: E402  reuse the transport


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--config", required=True)
    ap.add_argument("--only", required=True)
    ap.add_argument("--columns", default="maui_xaml,cpp,cpp_xaml")
    ap.add_argument("--theme", default="light")
    ap.add_argument("--settle", type=float, default=4.0)
    ap.add_argument("--out", required=True)
    a = ap.parse_args()

    cfg = tomllib.loads(Path(a.config).read_text())
    name, ecfg = next(iter(cfg["environments"].items()))
    env = Env(name, ecfg)
    out = Path(a.out); out.mkdir(parents=True, exist_ok=True)

    env.deploy(env.agent_src, env.agent_remote)
    for col, ccfg in env.columns.items():
        if ccfg.get("artifact_remote"):
            ccfg["_remote"] = ccfg["artifact_remote"]; continue
        local = column_artifact(col, ccfg)
        if local is None:
            continue
        remote = posixpath.join(env.apps_remote, col, local.name)
        env.deploy(local, remote); ccfg["_remote"] = remote

    if env.is_windows:
        sys.path.insert(0, str(REPO / "port/cpp/tools/parity/lib/windows"))
        from session1 import Session1Agent  # noqa: PLC0415
        s1 = Session1Agent(env.cfg["connection"]["host"], env.cfg["connection"]["user"],
                           staging=env.cfg["staging"]["root"], python=env.python3)
        s1.deploy(env.agent_src)
        started = s1.start(restart=True)
        if not started.get("ok") or started.get("session_id") in (0, None):
            raise SystemExit(f"session-1 agent did not start: {started.get('error') or started}")
        env.session1 = s1

    results = []
    try:
        for tag in a.only.split(","):
            for col in a.columns.split(","):
                ccfg = env.columns.get(col)
                if not ccfg or ccfg.get("_missing"):
                    print(f"  ~ {tag}/{col}: missing, skipped"); continue
                launch_env = [f"{ccfg['page_env']}={tag}",
                              f"{ccfg['theme_env']}={ccfg[f'theme_{a.theme}']}",
                              "MAUI_CAPTURE_TINT_NORMAL=1", "MAUI_SUPPRESS_FOCUS_VISUAL=1"]
                args = ["launch", "--bundle", ccfg["_remote"], "--proc", ccfg["process"]]
                for kv in launch_env:
                    args += ["--env", kv]
                res = env.agent(*args)
                pid = res.get("pid")
                if pid is None:
                    print(f"  ! {tag}/{col}: launch failed: {res.get('error')}"); continue
                try:
                    time.sleep(a.settle)
                    # Learn the window's OWN rect. Deliberately NOT `present` — presenting is the very
                    # thing that hides the bug this probe exists to find.
                    win = env.agent("window-id", pid, "--proc", ccfg["process"])
                    hwnd, bounds = win.get("id"), win.get("bounds")
                    if not hwnd:
                        print(f"  ! {tag}/{col}: no window id"); continue
                    remote = f"{env.staging}/launch_{tag}_{col}.png"
                    env.agent("shot", remote, "--window", hwnd)
                    local = out / f"{tag}_{col}_{a.theme}.png"
                    if not env.pull(remote, local):
                        print(f"  ! {tag}/{col}: pull failed"); continue
                    sz = png_size(local)
                    results.append({"tag": tag, "column": col, "bounds": bounds, "size": sz})
                    print(f"  {tag}/{col}: launch size {sz}  bounds {bounds} -> {local.name}")
                finally:
                    env.agent("stop", pid); time.sleep(0.3)
    finally:
        if getattr(env, "session1", None) is not None:
            env.session1.stop(quiet=True); env.session1 = None

    (out / "index.json").write_text(json.dumps(results, indent=2))
    print(f"\n{len(results)} launch frames -> {out}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
