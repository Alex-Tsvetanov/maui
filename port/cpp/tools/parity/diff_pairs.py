#!/usr/bin/env python3
"""Deterministic pixel diff of the C++ vs MAUI native captures (per key x theme).

For each key/theme it compares docs/comparison/captures/cpp_{theme}/<key>.png against
captures/maui_{theme}/<key>.png:
  - Masks the top status-bar band (clock/battery/dynamic-island) in BOTH first so the emulator clock
    never counts toward the diff (the user's one allowed exception to pixel-perfect).
  - Pillow exact per-pixel delta -> diff_pct (any nonzero channel), thresh_pct (max-channel delta >
    THRESH, ignores sub-pixel AA), max_delta, and a diff heatmap PNG in docs/comparison/diffs/.
  - ImageMagick `compare` -> SSIM (perceptual, 1.0=identical) and AE (count of differing px @ fuzz).
  - pixel_perfect := diff_pct == 0 after masking.

Writes docs/comparison/diff_results.json: { "<key>": { "<theme>": {diff_pct, thresh_pct, max_delta,
ssim, ae, pixel_perfect, heatmap, note} } }.

Usage: python3 diff_pairs.py [--only k1,k2] [--themes light,dark] [--mask-top 165] [--thresh 16]
"""
import argparse
import json
import os
import subprocess
import tempfile

from PIL import Image, ImageChops

HERE = os.path.dirname(os.path.abspath(__file__))
CMP = os.path.normpath(os.path.join(HERE, "..", "..", "docs", "comparison"))
CAP = os.path.join(CMP, "captures")
DIFFS = os.path.join(CMP, "diffs")


def load_keys():
    with open(os.path.join(HERE, "page_keys.txt")) as fh:
        return [ln.strip() for ln in fh if ln.strip()]


def mask_top(img, n):
    """Return a copy with the top n px filled black (drops the status bar / clock from the compare)."""
    out = img.copy()
    if n > 0:
        Image.new(out.mode, (out.width, min(n, out.height)), 0).paste  # noqa: no-op clarity
        out.paste((0, 0, 0), (0, 0, out.width, min(n, out.height)))
    return out


def imagemagick_metrics(cpp_png, maui_png):
    """Return (ssim, ae): ssim in [0,1] (1=identical) derived from IM DSSIM, ae = differing-pixel count.

    IM7 prints "<raw> (<normalized>)"; the normalized parenthesized value is the [0,1] form. SSIM/DSSIM
    report dissimilarity in this build, so we read DSSIM's normalized value and use ssim = 1 - dssim.
    AE's leading token is the differing-pixel count (@2% fuzz)."""
    import re

    def run_metric(name, extra=()):
        p = subprocess.run(["magick", "compare", "-metric", name, *extra, cpp_png, maui_png, "null:"],
                           capture_output=True, text=True)
        return (p.stderr or p.stdout).strip()

    ssim = None
    dssim_out = run_metric("DSSIM")
    m = re.search(r"\(([\d.eE+-]+)\)", dssim_out)
    if m:
        try:
            ssim = round(1.0 - float(m.group(1)), 4)
        except ValueError:
            ssim = None
    ae = None
    ae_out = run_metric("AE", ("-fuzz", "2%")).split()
    if ae_out:
        try:
            ae = int(float(ae_out[0]))
        except ValueError:
            ae = None
    return ssim, ae


def diff_one(key, theme, mask, thresh):
    cpp = os.path.join(CAP, f"cpp_{theme}", f"{key}.png")
    maui = os.path.join(CAP, f"maui_{theme}", f"{key}.png")
    if not (os.path.exists(cpp) and os.path.exists(maui)):
        return {"note": "missing capture", "pixel_perfect": False,
                "missing": [p for p in (cpp, maui) if not os.path.exists(p)]}
    a = Image.open(cpp).convert("RGB")
    b = Image.open(maui).convert("RGB")
    note = ""
    if a.size != b.size:
        note = f"size mismatch {a.size} vs {b.size}; cropped to common"
        w, h = min(a.width, b.width), min(a.height, b.height)
        a, b = a.crop((0, 0, w, h)), b.crop((0, 0, w, h))
    a, b = mask_top(a, mask), mask_top(b, mask)

    diff = ImageChops.difference(a, b)
    bbox = diff.getbbox()
    total = a.width * a.height
    # Per-pixel max channel delta via the grayscale of the max-projected difference.
    gray = diff.convert("L")
    hist = gray.histogram()
    nonzero = sum(hist[1:])
    over = sum(c for v, c in enumerate(hist) if v > thresh)
    max_delta = max((v for v, c in enumerate(hist) if c), default=0)

    os.makedirs(DIFFS, exist_ok=True)
    heat = os.path.join(DIFFS, f"{key}_{theme}.png")
    # Amplify the difference for human-visible heatmap (x6, clamped).
    diff.point(lambda p: min(255, p * 6)).save(heat)

    ssim, ae = imagemagick_metrics(cpp, maui)
    diff_pct = round(100.0 * nonzero / total, 4)
    return {
        "diff_pct": diff_pct,
        "thresh_pct": round(100.0 * over / total, 4),
        "max_delta": max_delta,
        "ssim": ssim,
        "ae": ae,
        "pixel_perfect": diff_pct == 0.0,
        "bbox": bbox,
        "heatmap": os.path.relpath(heat, CMP),
        "note": note,
    }


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--only", default="")
    ap.add_argument("--themes", default="light,dark")
    ap.add_argument("--mask-top", type=int, default=165)  # ~status-bar height at 3x (1206x2622)
    ap.add_argument("--thresh", type=int, default=16)     # ignore <=16/255 channel deltas (AA/hinting)
    ap.add_argument("--out", default=os.path.join(CMP, "diff_results.json"))
    args = ap.parse_args()

    keys = load_keys()
    if args.only:
        want = set(args.only.split(","))
        keys = [k for k in keys if k in want]
    themes = [t for t in args.themes.split(",") if t in ("light", "dark")]

    results = {}
    for i, key in enumerate(keys, 1):
        results[key] = {}
        for theme in themes:
            results[key][theme] = diff_one(key, theme, args.mask_top, args.thresh)
        pp = all(results[key][t].get("pixel_perfect") for t in themes)
        print(f"[{i}/{len(keys)}] {key}: " +
              " ".join(f"{t}={results[key][t].get('diff_pct','?')}%/SSIM={results[key][t].get('ssim','?')}"
                       for t in themes) + (" PIXEL-PERFECT" if pp else ""), flush=True)
    with open(args.out, "w") as fh:
        json.dump(results, fh, indent=1)
    print(f"DIFF_DONE -> {args.out}", flush=True)


if __name__ == "__main__":
    main()
