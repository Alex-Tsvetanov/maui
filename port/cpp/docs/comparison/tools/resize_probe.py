#!/usr/bin/env python3
"""Resize-behaviour probe — does the layout survive a SMALL window, and recover when grown?

The parity board only ever measures ONE window size: it presents every column at a fixed 1024x800 and
scores the settled frame. So a page can be pixel-perfect on the board and still be broken at any other
size — clipped controls, overlapping text, unreadable fields — and nothing would notice. This probe
covers that blind spot.

For each (page, column) it drives the SAME window through a size sequence in one process lifetime:

    present @ small -> shot   ... then WITHOUT relaunching ...
    present @ large -> shot

Capturing both from one launch is the whole point. Relaunching per size would only prove the app can
lay out at each size from scratch; it would say nothing about whether a live RESIZE re-flows correctly,
which is the case the user actually observed going wrong. A layout that is correct at 1024x800 from a
cold start but stays squashed after growing from 700x520 is a real bug that a per-size relaunch hides.

Everything OS-level is reused from run_comparison.py (Env, the session-1 agent, present/shot/pull), so
this probe inherits the same transport and the same fail-loudly-on-short-frame guarantees rather than
re-implementing them.

Usage:
    python3 resize_probe.py --config <cfg> --only entry,border_playground \
        --columns maui_xaml,cpp --sizes 700x520,1024x800 --out <dir>
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
from run_comparison import REPO, Env, load_manifest, png_size  # noqa: E402  reuse, don't re-implement


def parse_size(s: str) -> tuple[int, int]:
    w, h = s.lower().split("x")
    return int(w), int(h)


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--config", required=True)
    ap.add_argument("--only", required=True, help="comma-separated page tags")
    ap.add_argument("--columns", default="maui_xaml,cpp")
    ap.add_argument("--sizes", default="700x520,1024x800",
                    help="comma-separated WxH, applied IN ORDER to the same live window")
    ap.add_argument("--theme", default="light")
    ap.add_argument("--settle", type=float, default=2.5)
    ap.add_argument("--out", required=True)
    a = ap.parse_args()

    cfg = tomllib.loads(Path(a.config).read_text())
    envs = cfg["environments"]
    name, ecfg = next(iter(envs.items()))
    env = Env(name, ecfg)
    out = Path(a.out)
    out.mkdir(parents=True, exist_ok=True)
    sizes = [parse_size(s) for s in a.sizes.split(",")]
    tags = a.only.split(",")
    cols = a.columns.split(",")

    # Honour builder_twin, exactly as run_comparison.columns_for does. A tag with no code-first twin has
    # no `cpp` page, so the gallery falls back to some OTHER page and the comparison is meaningless — it
    # reads as a ~70% diff that looks like a catastrophic layout bug and is nothing of the sort. Learned
    # the hard way: an early run of this probe "found" grid_layout at 68.75%, which is simply not a board
    # page. Skip the cpp columns for those tags and say so, rather than emitting a fake number.
    twin_keys = {p["key"] for p in load_manifest() if p.get("builder_twin", True)} or None

    # Mirror run_comparison.py's own prologue exactly: deploy the agent, resolve each column's remote
    # artifact, then bring up the session-1 transport. Reusing the shapes rather than paraphrasing them
    # keeps this probe honest about what it is measuring.
    env.deploy(env.agent_src, env.agent_remote)
    for col, ccfg in env.columns.items():
        if ccfg.get("artifact_remote"):
            ccfg["_remote"] = ccfg["artifact_remote"]
            continue
        local = REPO / ccfg["artifact"]
        if not local.exists():
            ccfg["_missing"] = True
            continue
        remote = posixpath.join(env.apps_remote, col, local.name)
        env.deploy(local, remote)
        ccfg["_remote"] = remote

    if env.is_windows:
        sys.path.insert(0, str(REPO / "port/cpp/tools/parity/lib/windows"))
        from session1 import Session1Agent  # noqa: PLC0415  Windows-only
        s1 = Session1Agent(env.cfg["connection"]["host"], env.cfg["connection"]["user"],
                           staging=env.cfg["staging"]["root"], python=env.python3)
        s1.deploy(env.agent_src)
        started = s1.start(restart=True)
        if not started.get("ok") or started.get("session_id") in (0, None):
            raise SystemExit(f"session-1 agent did not start: {started.get('error') or started}\n"
                             f"  hint: {started.get('hint') or 'is the guest logged on at the CONSOLE?'}")
        print(f"  agent serving in session {started.get('session_id')}")
        env.session1 = s1

    results = []
    try:
        for tag in tags:
            for col in cols:
                ccfg = env.columns[col]
                if ccfg.get("_missing"):
                    print(f"  ~ {tag}/{col}: column missing, skipped")
                    continue
                if col.startswith("cpp") and twin_keys is not None and tag not in twin_keys:
                    print(f"  ~ {tag}/{col}: no code-first twin (builder_twin:false) — skipped, "
                          f"comparing it would score the gallery's FALLBACK page")
                    continue
                launch_env = [f"{ccfg['page_env']}={tag}",
                              f"{ccfg['theme_env']}={ccfg[f'theme_{a.theme}']}",
                              "MAUI_CAPTURE_TINT_NORMAL=1", "MAUI_SUPPRESS_FOCUS_VISUAL=1"]
                args = ["launch", "--bundle", ccfg["_remote"], "--proc", ccfg["process"]]
                for kv in launch_env:
                    args += ["--env", kv]
                res = env.agent(*args)
                pid = res.get("pid")
                if pid is None:
                    print(f"  ! {tag}/{col}: launch failed: {res.get('error')}")
                    continue
                time.sleep(a.settle)
                try:
                    # ONE launch, N sizes, applied in order to the SAME window — see the module docstring.
                    for w, h in sizes:
                        remote = f"{env.staging}/resize_{tag}_{col}_{w}x{h}.png"
                        pr = env.agent("present", "--proc", ccfg["process"], "--pid", pid,
                                       "--x", 128, "--y", 30, "--w", w, "--h", h, "--shot", remote)
                        if not (pr.get("rect") and pr.get("shot")):
                            print(f"  ! {tag}/{col} @{w}x{h}: present failed ({pr.get('error')})")
                            continue
                        time.sleep(a.settle)
                        # Re-shoot AFTER the settle: the atomic --shot above fires the instant the size is
                        # confirmed, which is exactly when a mid-reflow frame would be caught. The value
                        # here is the SETTLED layout at each size, not the transient.
                        env.agent("shot", remote, "--window", pr.get("window"))
                        local = out / f"{tag}_{col}_{w}x{h}.png"
                        if not env.pull(remote, local):
                            print(f"  ! {tag}/{col} @{w}x{h}: pull failed")
                            continue
                        got = png_size(local)
                        # Same fail-loudly rule the board uses: a short frame means the window never
                        # reached the target, and scoring it would measure the harness, not the layout.
                        if got is None or abs(got[0] - w) > 4 or abs(got[1] - h) > 4:
                            print(f"  ! {tag}/{col} @{w}x{h}: DROPPED — got {got}, not {w}x{h}")
                            local.unlink(missing_ok=True)
                            continue
                        results.append({"tag": tag, "column": col, "w": w, "h": h,
                                        "file": str(local), "size": got})
                        print(f"  {tag}/{col} @{w}x{h} -> {local.name}")
                finally:
                    env.agent("stop", pid)
                    time.sleep(0.3)
    finally:
        if getattr(env, "session1", None) is not None:
            env.session1.stop(quiet=True)
            env.session1 = None

    (out / "index.json").write_text(json.dumps(results, indent=2))
    print(f"\n{len(results)} frames -> {out}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
