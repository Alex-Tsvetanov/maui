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

# ---- the Android soft keyboard (a SECOND capture-of-the-wrong-thing) -------------------------------
# Same class of defect as the splash, different mechanism. On Android the IME is a separate PROCESS, so
# force-stopping the gallery between pages does not take its window down: a page whose scenario focused
# a field can leave the keyboard composited over the NEXT page's still. Verified by eye on the committed
# board — `focus` and `shadow_playground` carry a full Gboard in BOTH port columns while MAUI's column is
# clean, which is most of what scored those two pages RED. It is not app behaviour: cold-launching either
# page with no input shows no keyboard and `dumpsys input_method` reports mInputShown=false.
#
# The IME is composited OVER the page, unchanged, so the band is byte-identical between two frames of
# entirely different pages (measured: y1478..2340 of 2340, 36.8% of the frame, zero differing pixels).
# That is what makes a colour-fraction test safe here rather than a guess.
#
# WHY NOT A COLOUR TEST: two of them failed here. "Is the bottom band busy" called `label` (plain text
# over a grey button) a keyboard while MISSING the real one in `cpp/focus_light.png` — both errors
# confirmed by looking at the frames. "What fraction of the band is IME chrome grey" fails the other
# way: a real keyboard scores 0.367, but a page whose own background is that grey (shapes,
# staggered_layout, path_transform_string) scores 0.99-1.00, so no one-sided threshold separates them.
#
# What DOES separate them is that the IME is not page content at all — it is the same widget composited
# over whatever is beneath, so its key grid is reproduced EXACTLY. Matching the qwerty rows against a
# stored crop of them splits the board cleanly:
#     contaminated                                        1.000 (exact, every one)
#     cleanest-possible false positive (a white page)     0.606
# The gap is enormous because an exact match is the actual physical claim being tested. The qwerty rows
# are the reference rather than the whole keyboard because the suggestion strip carries typed text and
# the bottom-right action key changes shape per field (✓ / search glyph) — those rows differ between
# otherwise identical captures, the letter rows never do.
#
# The reference is stored as a FRACTION of frame height, not pixel rows, so a different device
# resolution rescales instead of silently missing.
IME_REFERENCE = os.path.join(os.path.dirname(os.path.abspath(__file__)), "ime_reference_android.png")
IME_BAND = (1650 / 2340, 2000 / 2340)  # the qwerty key grid, as a fraction of frame height
IME_REF_SIZE = (216, 70)               # the reference is stored downscaled; frames are matched at it
IME_PIXEL_TOLERANCE = 18               # per-channel, to survive the PNG round-trip and the downscale
IME_MATCH = 0.90                       # contaminated frames land on 1.000; the best clean frame is 0.61


def keyboard_verdict(path: str) -> tuple[bool, float]:
    """(shows_soft_keyboard, key_grid_match). Never raises on an unreadable file or a missing ref."""
    try:
        import numpy as np
        from PIL import Image
    except ImportError:  # keep the capture scripts usable without numpy/PIL installed
        return False, 0.0
    try:
        import numpy as np
        from PIL import Image

        def grid(p: str):
            im = Image.open(p).convert("RGB")
            w, h = im.size
            return np.asarray(im.crop((0, int(h * IME_BAND[0]), w, int(h * IME_BAND[1])))
                              .resize(IME_REF_SIZE)).astype(int)

        ref = np.asarray(Image.open(IME_REFERENCE).convert("RGB").resize(IME_REF_SIZE)).astype(int)
        frac = float((np.abs(grid(path) - ref).max(axis=2) <= IME_PIXEL_TOLERANCE).mean())
    except Exception:
        return False, 0.0
    return frac >= IME_MATCH, frac


def has_soft_keyboard(path: str) -> bool:
    return keyboard_verdict(path)[0]


# ---- an EMPTY frame (a third capture-of-the-wrong-thing) -------------------------------------------
# A page photographed before its content drew: chrome only, uniform body. Found on the committed board
# in ios/xaml LIGHT for clip_corner_radius and clip_gallery — 1 distinct body colour each, while their
# own dark twins and both other columns rendered fully. Both scored RED and neither was a port defect:
# re-shooting the two pages produced frames BYTE-IDENTICAL to MAUI (0.00%). A blank frame also reads as
# FROZEN to the motion scorer, which is how clip_gallery earned "MAUI ANIMATES and C++ & XAML IS
# FROZEN (0.0000%)" — nothing was frozen, nothing had been drawn.
#
# UNLIKE the splash and the keyboard, a blank verdict is AMBIGUOUS: some pages genuinely render empty
# (the gap_* placeholders, the Catalyst menu_bar page, whose menu lives outside the captured window).
# So this one drives a RETRY and then ACCEPTS — a persistent blank is evidence the page really is
# empty, while a blank that fills in on a re-shoot was a race. Never drop on it.
BLANK_COLOURS = 4          # distinct body colours at the sample size below
BLANK_SAMPLE = (160, 340)


def blank_verdict(path: str) -> tuple[bool, int]:
    """(looks_empty, distinct_body_colours). Never raises on an unreadable file."""
    try:
        import numpy as np
        from PIL import Image
    except ImportError:
        return False, 0
    try:
        import numpy as np
        from PIL import Image
        im = Image.open(path)
        if getattr(im, "n_frames", 1) > 1:
            im.seek(0)
        a = np.asarray(im.convert("RGB").resize(BLANK_SAMPLE)).astype("uint8")
    except Exception:
        return False, 0
    body = a[35:-15]  # drop status bar / home indicator: present even on a page that never drew
    n = int(len(np.unique(body.reshape(-1, 3), axis=0)))
    return n <= BLANK_COLOURS, n


def is_blank(path: str) -> bool:
    return blank_verdict(path)[0]


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
    ap = argparse.ArgumentParser(description="report splash / soft-keyboard frames under one or more paths")
    ap.add_argument("paths", nargs="+")
    ap.add_argument("--keyboard", action="store_true",
                    help="check for a stray Android soft keyboard instead of a .NET splash. Exit 1 on a "
                         "hit, so a shell capture loop can `if ! python3 capture_guard.py --keyboard f`.")
    ap.add_argument("--quiet", action="store_true", help="exit code only (for per-frame shell checks)")
    a = ap.parse_args()
    bad = 0
    for p in a.paths:
        files = sorted(glob.glob(os.path.join(p, "*.png"))) if os.path.isdir(p) else [p]
        for f in files:
            if a.keyboard:
                ok, frac = keyboard_verdict(f)
                if ok:
                    bad += 1
                    if not a.quiet:
                        print(f"KEYBOARD {f}  key grid matches the IME reference at {frac * 100:.1f}%")
                continue
            ok, frac, dom = splash_verdict(f)
            if ok:
                bad += 1
                print(f"SPLASH {f}  dominant {dom} at {frac * 100:.1f}%")
    if not a.quiet:
        print(f"{bad} {'keyboard' if a.keyboard else 'splash'} frame(s)")
    return 1 if bad else 0


if __name__ == "__main__":
    raise SystemExit(main())
