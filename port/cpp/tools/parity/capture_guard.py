#!/usr/bin/env python3
"""Reject a capture that shows a startup SPLASH instead of the page, and retry it.

WHY
---
A .NET MAUI app shows the purple ".NET" splash while its runtime starts. The C++ galleries have no
such phase, so this only ever hits the MAUI REFERENCE column — which is precisely the column every
score is measured against. A splash frame is not a degraded capture; it is a capture of the wrong
thing, and it scores as a huge port "defect" on a page where the port may be perfect.

This is NOT hypothetical. A sweep of all 5841 committed captures found 38 splash frames, all in the
MAUI column: 18 on iOS and 2 on Android. The 18 were alphabetically CONTIGUOUS (clipping, editor,
effects, ellipse_gallery, empty_view*) — one stretch of a single run, which is what load-induced slow
startup looks like: that run happened while three other platforms were building on the same host.

WHY NOT JUST RAISE --settle
---------------------------
Because the failure is load-dependent, so any fixed settle is a bet on how busy the machine happens to
be. A settle long enough for the worst case wastes that time on all ~1000 frames of every run. Detect
and retry costs nothing on a healthy frame and is correct under any load.

WHY THE OBVIOUS CHEAP CHECKS DO NOT WORK
----------------------------------------
Counting distinct colours does NOT find these. The iOS splash has 217 distinct colours in a 240x110
downscale (solid fill + antialiased ".NET" + the status bar), so a "fewer than N colours" screen passes
it straight through — an earlier check did exactly that and reported "0 of 364 splash-like" on a set
that contained 18. The signal is not colour COUNT, it is colour DOMINANCE: one colour, matching .NET
purple, over almost the whole body.

The status bar and home indicator are excluded from the measurement: they are never the splash and
their content (clock, battery) varies, so including them only adds noise to the fraction.
"""
from __future__ import annotations

import os

# Measured off port/maui-reference/captures/ios/clipping_dark.png: the dominant body colour is
# (87,41,221) at 99.7% of the body. The tolerance is generous because the same splash renders slightly
# differently per platform/scale, and the fraction threshold is what actually makes this specific.
NET_PURPLE = (87, 41, 221)
COLOUR_TOLERANCE = 60      # sum of per-channel absolute differences
DOMINANCE = 0.90           # fraction of the body that must be that one colour


def splash_verdict(path: str) -> tuple[bool, float, tuple[int, int, int]]:
    """(is_splash, dominant_fraction, dominant_rgb). Never raises on an unreadable file."""
    try:
        from PIL import Image
        import numpy as np
    except ImportError:  # keep the capture scripts usable without numpy/PIL installed
        return False, 0.0, (0, 0, 0)
    try:
        import numpy as np
        from PIL import Image
        a = np.asarray(Image.open(path).convert("RGB").resize((160, 300))).astype(int)
    except Exception:
        return False, 0.0, (0, 0, 0)
    body = a[30:-20]  # drop status bar / home indicator: never the splash, and their content varies
    cols, cts = np.unique(body.reshape(-1, 3), axis=0, return_counts=True)
    i = int(cts.argmax())
    frac = float(cts[i]) / (body.shape[0] * body.shape[1])
    dom = tuple(int(v) for v in cols[i])
    near = sum(abs(dom[k] - NET_PURPLE[k]) for k in range(3)) < COLOUR_TOLERANCE
    return (bool(near and frac > DOMINANCE), frac, dom)


def is_splash(path: str) -> bool:
    return splash_verdict(path)[0]


def capture_until_ready(shoot, out_path: str, attempts: int = 4, backoff: float = 3.0) -> bool:
    """Call shoot() -> writes out_path; retry while it lands on a splash.

    `shoot` takes no arguments and must (re)write out_path. Returns True when a NON-splash frame was
    obtained, False when every attempt was a splash — in which case the CALLER SHOULD DROP THE FRAME
    rather than bank it. Banking a known-bad frame is what produced the 38 corrupt captures.
    """
    import time
    for attempt in range(attempts):
        shoot()
        if not os.path.exists(out_path):
            return False
        if not is_splash(out_path):
            return True
        wait = backoff * (attempt + 1)
        print(f"  ~ splash detected in {os.path.basename(out_path)} — "
              f"retry {attempt + 1}/{attempts - 1} after {wait:.0f}s")
        time.sleep(wait)
    return False


def main() -> int:
    import argparse
    import glob
    ap = argparse.ArgumentParser(description="report splash frames under one or more directories")
    ap.add_argument("paths", nargs="+")
    a = ap.parse_args()
    bad = 0
    for p in a.paths:
        files = sorted(glob.glob(os.path.join(p, "*.png"))) if os.path.isdir(p) else [p]
        for f in files:
            ok, frac, dom = splash_verdict(f)
            if ok:
                bad += 1
                print(f"SPLASH {f}  dominant {dom} at {frac * 100:.1f}%")
    print(f"{bad} splash frame(s)")
    return 1 if bad else 0


if __name__ == "__main__":
    raise SystemExit(main())
