#!/usr/bin/env python3
"""Did a recapture silently DELETE content from the MAUI ground truth?

WHY THIS EXISTS. `provenance.py` answers "do the PORT columns come from one binary". Nothing answered the
other half: the MAUI reference is also recaptured, and it can come back MISSING things. On 2026-08-14 the
android clean pass (`eb1c33abd8`) dropped the CollectionView selection highlight from three pages —

    selection_synchronization   orange 11.77% -> 0.42%   (ink 24.49% -> 13.17%)
    preselected_items                  5.08% -> 0.00%
    multiple_bound_selection           4.38% -> 0.00%

— and every downstream score inherited it. Worse, the board then showed "the port paints a highlight MAUI
does not", which reads exactly like a port bug. A fix was drafted to DELETE the port's correct highlight to
match the broken reference. The user caught it by remembering the reference used to look different.

A reference regression and a port regression are indistinguishable on the board. This tells them apart.

THE METRIC is deliberately blunt and content-agnostic: ink = share of pixels that are NOT the frame's
dominant (background) colour. Anything that vanishes — a highlight, an image, a control that failed to
appear — drops it. You do not have to know in advance what to look for, which is the whole point: the three
pages above were found by a metric that had never heard of selection highlights.

Ink RISING is not flagged. Content appearing is not a regression, and the android dark window wash lifts ink
on many pages for reasons unrelated to correctness.

Usage:
    python3 tools/parity/reference_guard.py                    # working tree vs HEAD
    python3 tools/parity/reference_guard.py --base d5c2b93e13  # vs an older state, to audit a past pass
    python3 tools/parity/reference_guard.py --platforms android
Exit 1 if any reference lost ink, so it can gate a recapture.
"""
import argparse
import collections
import io
import os
import subprocess
import sys

COMP = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..", "docs", "comparison"))
REPO = os.path.normpath(os.path.join(COMP, "..", "..", "..", ".."))
LANES = ("android", "ios", "maccatalyst", "windows")
# 0.30 percentage points. The six real regressions ranged -2.18 to -11.32, and the largest INNOCENT movement
# in the same 344-capture sweep was well under this, so it separates them without hand-tuning.
DEFAULT_THRESHOLD = 0.30


def _git(*args, binary=False):
    r = subprocess.run(["git", "-C", REPO, *args], capture_output=True)
    if r.returncode != 0:
        return None
    return r.stdout if binary else r.stdout.decode("utf-8", "replace").strip()


def ink(blob):
    """Share of pixels that are NOT the dominant colour, plus the distinct-colour count.

    Both move when content disappears; the colour count is reported because it is the more legible signal
    for a human (313 -> 174 says "a third of this page's palette is gone").
    """
    from PIL import Image  # imported lazily so --help works without Pillow

    image = Image.open(io.BytesIO(blob)).convert("RGB")
    total = image.width * image.height
    counts = collections.Counter(image.getdata())
    dominant = counts.most_common(1)[0][1]
    return (total - dominant) * 100.0 / total, len(counts)


def _blob(ref, path):
    if ref is None:  # working tree
        full = os.path.join(REPO, path)
        if not os.path.isfile(full):
            return None
        with open(full, "rb") as handle:
            data = handle.read()
    else:
        data = _git("show", f"{ref}:{path}", binary=True)
    return data if data and data[:4] == b"\x89PNG" else None


def scan(base, lanes, threshold):
    losses, compared = [], 0
    for lane in lanes:
        rel = f"port/cpp/docs/comparison/captures/{lane}/maui/"
        changed = _git("diff", "--name-only", base, "--", rel) or ""
        for path in changed.split("\n"):
            if not path.endswith(".png"):
                continue
            before, after = _blob(base, path), _blob(None, path)
            if not before or not after:
                continue
            try:
                i0, n0 = ink(before)
                i1, n1 = ink(after)
            except Exception:  # a truncated/garbage PNG is a different failure; do not mask it as a loss
                print(f"  ! could not read {path}", file=sys.stderr)
                continue
            compared += 1
            if i1 - i0 <= -threshold:
                losses.append((lane, os.path.basename(path), i0, i1, i1 - i0, n0, n1,
                               _port_agrees_with(path, i0, i1)))
    return compared, sorted(losses, key=lambda row: row[4])


def _port_agrees_with(maui_path, before, after):
    """Which reference does the PORT's own render match — the old one or the new one?

    THE FLAG ALONE IS NOT A VERDICT. Ink loss against a baseline is only a regression if the BASELINE was
    right, and on 2026-08-14 three of eleven flags were the baseline being wrong:
      windows/drag_drop_dark      Aug4 59.25% (black 20.39%) vs now 41.41% -- and its own LIGHT twin, and the
                                  port, all sit at 41.41%. The Aug4 DARK frame carried a ~20%-of-frame black
                                  region nothing else had. A bad frame in the baseline.
      maccatalyst/header_footer_view (light+dark)  port matches NOW to 0.01pp; Aug4 was the outlier.
    Whereas the six android captures and maccatalyst/swipe_item_size had the port matching the OLD value
    exactly -- those were real.

    The port column is an independent witness: it is rendered by different code from a different source
    tree, so when it agrees with one side, that side is almost certainly the correct render.
    Returns 'OLD' (real regression), 'NEW' (baseline was the outlier), or '?' when the port frame is absent.
    """
    port_path = maui_path.replace("/maui/", "/cpp/")
    blob = _blob(None, port_path)
    if not blob:
        return "?"
    try:
        value, _ = ink(blob)
    except Exception:
        return "?"
    return "OLD" if abs(value - before) < abs(value - after) else "NEW"


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0],
                                 formatter_class=argparse.RawDescriptionHelpFormatter,
                                 epilog="\n".join(__doc__.splitlines()[2:]))
    ap.add_argument("--base", default="HEAD", help="git ref to compare the working tree against (default HEAD)")
    ap.add_argument("--platforms", default=",".join(LANES))
    ap.add_argument("--threshold", type=float, default=DEFAULT_THRESHOLD,
                    help=f"percentage points of ink loss that count as a regression (default {DEFAULT_THRESHOLD})")
    a = ap.parse_args()
    lanes = [p for p in a.platforms.split(",") if p in LANES]

    compared, losses = scan(a.base, lanes, a.threshold)
    if compared == 0:
        print(f"no changed MAUI reference captures vs {a.base} — nothing to check")
        return 0

    print(f"checked {compared} changed MAUI reference capture(s) vs {a.base}\n")
    if not losses:
        print(f"OK — none lost >= {a.threshold}pp of ink. The ground truth did not lose content.")
        return 0

    print(f"{'lane':12s} {'capture':38s} {'before':>8s} {'after':>8s} {'delta':>8s}  port agrees")
    print("-" * 92)
    for lane, name, i0, i1, d, n0, n1, port in losses:
        verdict = {"OLD": "OLD -> REAL", "NEW": "NEW -> baseline was the outlier", "?": "? (no port frame)"}[port]
        print(f"{lane:12s} {name:38s} {i0:7.2f}% {i1:7.2f}% {d:+8.2f}  {verdict}")
    real = [row for row in losses if row[7] != "NEW"]
    print(f"\n{len(losses)} capture(s) lost ink; {len(real)} look like REAL reference regressions.")
    if len(real) != len(losses):
        print("The rest are the BASELINE being wrong, not the reference losing content — the port render")
        print("matches the NEW value, and the port is independent code from an independent tree.")
    if not real:
        return 0
    print(f"\nREFERENCE REGRESSION: {len(real)} capture(s) LOST content.")
    print("The MAUI ground truth got worse, not the port. Do NOT change the port to match these frames —")
    print("recapture the reference for these pages (a longer --settle is the usual cause; a selection or")
    print("other async state applied after the shot) and re-run this before judging them.")
    return 1


if __name__ == "__main__":
    raise SystemExit(main())
