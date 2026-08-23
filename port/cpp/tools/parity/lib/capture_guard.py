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
# OPEN GAP (2026-08-22): A BLANK WHITE MAUI PAGE PASSED THIS CHECK. An android recapture at 13:34
# returned a page with NO CONTENT, body mean 254.5, and blank_verdict did not flag it — so the frame was
# only caught because a human looked. It was restored rather than committed, which unfortunately means the
# failing artifact no longer exists to test a fix against.
#
# I tried to close it and STOPPED, because the obvious gates are unsafe and the data says so. Measured
# across the 727 committed maui light captures:
#   * COLOUR DOMINANCE (the signal this file's header rightly prefers over raw count) reaches 1.0000 on
#     LEGITIMATE pages — swipe_refresh, menu_bar, ios_safe_area, gap_title_bar, ios_date_picker. A
#     dominance rule fires on pages that really are empty by construction.
#   * QUANTIZING before the count (body//8, to collapse the resize interpolation that can inflate a
#     uniform page past BLANK_COLOURS) changes almost nothing: real content pages sit at 27-155 quantized
#     colours (menu_bar 27, label 84, box_view 155) and the 25 captures that trip <=4 already trip it on
#     the RAW count. So interpolation was not what let 13:34 through.
#   * MEAN BRIGHTNESS is the tempting third option and has only ~7 levels of headroom: the failing frame
#     was 254.5, while legitimate light pages reach 247.4 (ios activity_indicator) and 244.3 (ios
#     absolute_layout). A threshold in that gap would be a guess, and a false positive here costs a
#     re-shoot on every run of those pages.
#
# So the honest state is: the failure is real and reproducible in principle, but NOT diagnosable from the
# committed corpus, because every candidate discriminator either misfires on known-empty pages or has no
# safe margin. THE NEXT PERSON TO SEE ONE MUST KEEP THE FRAME — copy it aside before restoring — and the
# gate should then be designed against it rather than against this reasoning.
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


def capture_until_ready(shoot, out_path: str, attempts: int = 4, backoff: float = 3.0,
                        blank_retries: int = 1, have_frame: bool = False) -> bool:
    """Call shoot() -> writes out_path; re-shoot while the frame is not a capture of THIS page.

    `shoot` takes no arguments and must (re)write out_path. Returns True when the frame may be banked,
    False when the caller SHOULD DROP IT. Banking a known-bad frame is what produced the 38 corrupt
    captures.

    TWO FAILURE SHAPES, TWO POLICIES — and the asymmetry is the whole point:

      SPLASH -> retry, and DROP if it never clears. A .NET splash is unambiguously the wrong thing;
                no page legitimately renders as near-solid #5729DD.

      BLANK  -> retry ONCE, then ACCEPT. A blank verdict is AMBIGUOUS and this module's header has
                always said so: some pages genuinely render empty (the gap_* placeholders, the
                Catalyst menu_bar page whose menu lives outside the captured window). A blank that
                fills in on a re-shoot was a race; one that persists is evidence the page really is
                empty. NEVER drop on it.

    WHY THERE IS NO THIRD, DROPPING PREDICATE HERE — measured 2026-08-23, after three were proposed:
      * "non-trivial ink": under this module's own body crop the known-bad android frame
        (swipe_item_position/cpp/light) is INDISTINGUISHABLE from a legitimate gap_title_bar
        placeholder — both dominant (255,255,255) at fraction 1.0, 1 distinct colour, mean 255.0.
        Any ink floor that catches the first deletes the second; 619 of 5086 committed frames sit
        under 0.01. An earlier margin of 0.00287 vs 0.0306 was an artifact of a crop that left the
        android nav bar inside the body, i.e. it was measuring chrome, not content.
      * "dark frame washed to #2F2F2F": that colour is a LEGITIMATE dark surface here — it is the
        dominant background of 100 of 542 known-good android dark frames, and on 143 pages all three
        columns share it at identical dominance. The red-producing condition is the columns
        DISAGREEING about the background, which is a cross-column fact and cannot be evaluated at the
        shutter, where the other columns do not exist yet.
    Both would have dropped good frames, which is strictly worse than the artifact they targeted. The
    ambiguity is real, so retry-then-accept is the strongest honest policy — this module said that
    before either was proposed.

    MEASURED COST of the blank arm: 165 of 5086 committed frames (3.2%) take ONE extra shot and none
    are dropped — maccatalyst 99, windows 32, ios 22, android 12.
    """
    import time
    blanks = 0
    for attempt in range(attempts):
        # `have_frame` means out_path ALREADY holds a shot, so judge that one before spending another.
        # Without it a caller that has just captured normally pays a second shot on every frame, which
        # would turn "costs nothing on a healthy frame" -- the sentence this whole design rests on --
        # into doubling the pass.
        if not (have_frame and attempt == 0):
            shoot()
        if not os.path.exists(out_path):
            return False
        if is_splash(out_path):
            wait = backoff * (attempt + 1)
            print(f"  ~ splash detected in {os.path.basename(out_path)} — "
                  f"retry {attempt + 1}/{attempts - 1} after {wait:.0f}s")
            time.sleep(wait)
            continue
        if blanks < blank_retries and is_blank(out_path):
            blanks += 1
            print(f"  ~ blank frame in {os.path.basename(out_path)} — one re-shoot "
                  f"(a persistent blank is ACCEPTED: some pages really are empty)")
            time.sleep(backoff)
            continue
        return True
    # Only a frame that is STILL a splash may be dropped here. Falling out of the loop having last
    # seen a blank must ACCEPT it, or the accept-on-persistent-blank rule above would be a lie in the
    # one case that exercises it.
    return os.path.exists(out_path) and not is_splash(out_path)


def selftest() -> None:
    """The retry POLICY, which is the part that fails silently: a wrong branch here either drops good
    frames or banks bad ones, and both look like a normal run in the log."""
    import tempfile
    import numpy as np
    from PIL import Image

    d = tempfile.mkdtemp()
    out = os.path.join(d, "f.png")

    def write(kind):
        if kind == "splash":
            a = np.full((300, 160, 3), NET_PURPLE, dtype="uint8")
        elif kind == "blank":
            a = np.full((300, 160, 3), 255, dtype="uint8")
        else:                                   # real content: many colours, no dominance
            rng = np.random.default_rng(0)
            a = rng.integers(0, 255, (300, 160, 3), dtype="uint8")
        Image.fromarray(a).save(out)

    def run(seq, **kw):
        """seq is consumed one entry per shoot(); the last entry repeats forever."""
        calls = []

        def shoot():
            kind = seq[min(len(calls), len(seq) - 1)]
            calls.append(kind)
            write(kind)
        return capture_until_ready(shoot, out, backoff=0.0, **kw), len(calls)

    assert write("good") is None and is_splash(out) is False        # fixtures behave
    write("splash"); assert is_splash(out), "splash fixture must read as a splash"
    write("blank"); assert is_blank(out), "blank fixture must read as blank"

    # GOOD: banked on the first shot, no retry cost on a healthy frame.
    assert run(["good"]) == (True, 1)
    # SPLASH that clears: retried, then banked.
    assert run(["splash", "good"]) == (True, 2)
    # SPLASH forever: DROPPED. A splash is unambiguously the wrong thing.
    ok, n = run(["splash"], attempts=3)
    assert ok is False and n == 3, (ok, n)
    # BLANK that fills in: exactly ONE re-shoot, then banked.
    assert run(["blank", "good"]) == (True, 2)
    # BLANK forever: ACCEPTED after one re-shoot, NEVER dropped — some pages really are empty.
    # This is the case the whole asymmetry exists for; if it ever returns False, the gap_*
    # placeholders and the Catalyst menu_bar page start disappearing from the board.
    ok, n = run(["blank"], attempts=4)
    assert ok is True and n == 2, (ok, n)
    # …and with retries disabled a blank is still banked, not dropped.
    assert run(["blank"], blank_retries=0) == (True, 1)
    # THE FALL-OUT PATH, which the cases above never reach: when the blank retries outlast `attempts`
    # the loop ENDS mid-retry and the final return decides. That line is the one that must not say
    # False. Added after a mutation that changed it to `return False` passed every other case here —
    # the whole no-drop guarantee lived on a line no test executed.
    ok, n = run(["blank"], attempts=2, blank_retries=5)
    assert ok is True and n == 2, (ok, n)
    # A frame that is still a SPLASH on the same fall-out path must still drop.
    ok, n = run(["splash"], attempts=2, blank_retries=5)
    assert ok is False and n == 2, (ok, n)

    # have_frame: a caller that already shot must pay NOTHING for a healthy frame, and exactly one
    # re-shoot for a blank one. This is what keeps the guard free on the ~97% of frames that are fine.
    write("good")
    assert run(["good"], have_frame=True) == (True, 0), "a healthy existing frame must not be re-shot"
    write("blank")
    assert run(["good"], have_frame=True) == (True, 1), "a blank existing frame gets one re-shoot"
    write("splash")
    ok, n = run(["splash"], attempts=2, have_frame=True)
    assert ok is False and n == 1, (ok, n)
    print("capture_guard selftest: splash drops, blank accepts, good frames cost no retry — OK")


def main() -> int:
    import argparse
    import glob
    ap = argparse.ArgumentParser(description="report splash / soft-keyboard frames under one or more paths")
    ap.add_argument("paths", nargs="*")
    ap.add_argument("--keyboard", action="store_true",
                    help="check for a stray Android soft keyboard instead of a .NET splash. Exit 1 on a "
                         "hit, so a shell capture loop can `if ! python3 capture_guard.py --keyboard f`.")
    ap.add_argument("--quiet", action="store_true", help="exit code only (for per-frame shell checks)")
    ap.add_argument("--selftest", action="store_true", help="check the retry policy; no files needed")
    a = ap.parse_args()
    if a.selftest:
        selftest()
        return 0
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
