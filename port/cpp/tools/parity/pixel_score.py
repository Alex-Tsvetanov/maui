#!/usr/bin/env python3
"""Compute a real pixel-diff / SSIM score between each page's MAUI and C++ screenshots, and write
the result into comparison.json as a review-like model ("pixel" / "pixel_xaml") alongside
"sonnet"/"sonnet_xaml" and "gemini"/"gemini_xaml" — same {status, review} shape, so gen_readme.py's
existing status_emoji()/review_body() renders it for free.

**The four required comparisons** (port/CLAUDE.md "Parity comparison policy" §5, 2026-07-05 ruling —
applies to EVERY review model, not just this one): MAUI is ground truth, judged independently against
BOTH framework columns, in both themes:
  1. MAUI light vs C++ light            -> written to the "pixel" slot
  2. MAUI light vs C++ & XAML light      -> written to the "pixel_xaml" slot
  3. MAUI dark  vs C++ dark              -> written to the "pixel" slot
  4. MAUI dark  vs C++ & XAML dark       -> written to the "pixel_xaml" slot
This mirrors the existing cpp->bare / xaml->`_xaml` slot convention `comparison_paths.review_slot()`
already encodes for sonnet/gemini. Android has no dark theme, so only comparisons 1 and 2 apply there.

Dependency-light: PIL + numpy only (no scipy/scikit-image on this machine). SSIM uses a proper 11x11
uniform-window average (via an integral image, not a naive global mean) per Wang et al. 2004, computed
on grayscale luma. Also reports diff_pct: the percentage of pixels whose per-channel max absolute
difference exceeds a visibility threshold (25/255) — a blunter, more intuitive companion number.

Mismatched image dimensions are resized (LANCZOS) to the smaller common size before comparing — a
known limitation: this is a global comparison, not a registered/aligned diff, so a uniform outer-inset
shift (exempt per parity policy) can still show up as a nonzero diff_pct/SSIM<1. Missing screenshots
(either side null, or file absent) produce no score for that theme; a page with NO computable theme for
a given framework gets status "blank" on that framework's slot (mirrors the sonnet/gemini convention).

Usage: python3 tools/parity/pixel_score.py [--only key1,key2] [--platform ios,maccatalyst,android]
Writes results directly into docs/comparison/comparison.json (preserves everything else). Run
docs/comparison/tools/gen_readme.py afterward to render the scores.
"""
import argparse
import json
import os

import numpy as np
from PIL import Image

HERE = os.path.dirname(os.path.abspath(__file__))
CPP_ROOT = os.path.normpath(os.path.join(HERE, "..", ".."))
COMP = os.path.join(CPP_ROOT, "docs", "comparison")
JSON = os.path.join(COMP, "comparison.json")

PLATFORMS = ("ios", "maccatalyst", "android")
THEMES = ("light", "dark")
DIFF_THRESHOLD = 25  # per-channel 0-255 abs-diff above this counts as a "visibly different" pixel
WINDOW = 11  # the standard SSIM window size (Wang et al. 2004)
C1 = (0.01 * 255) ** 2
C2 = (0.03 * 255) ** 2


def to_luma(img):
    """RGB (or RGBA, alpha dropped) PIL Image -> float64 grayscale array via ITU-R BT.601."""
    arr = np.asarray(img.convert("RGB"), dtype=np.float64)
    return arr[..., 0] * 0.299 + arr[..., 1] * 0.587 + arr[..., 2] * 0.114


def box_filter(a, k):
    """Uniform k x k box filter over a 2D array via an integral image (reflect-padded at the
    border so the output stays the same size as the input, like scipy.ndimage.uniform_filter)."""
    pad = k // 2
    padded = np.pad(a, pad, mode="reflect")
    ii = np.pad(padded, ((1, 0), (1, 0))).cumsum(0).cumsum(1)
    H, W = a.shape
    # sum over each k x k window ending at (i+k, j+k) in the padded+integral coordinate frame
    total = ii[k:, k:] - ii[:H, k:] - ii[k:, :W] + ii[:H, :W]
    return total / (k * k)


def ssim(a, b):
    """Mean windowed SSIM between two same-size float64 grayscale arrays (Wang et al. 2004)."""
    mu_a, mu_b = box_filter(a, WINDOW), box_filter(b, WINDOW)
    mu_a2, mu_b2, mu_ab = mu_a * mu_a, mu_b * mu_b, mu_a * mu_b
    var_a = box_filter(a * a, WINDOW) - mu_a2
    var_b = box_filter(b * b, WINDOW) - mu_b2
    cov_ab = box_filter(a * b, WINDOW) - mu_ab
    num = (2 * mu_ab + C1) * (2 * cov_ab + C2)
    den = (mu_a2 + mu_b2 + C1) * (var_a + var_b + C2)
    return float(np.mean(num / den))


def load_pair(path_a, path_b):
    """Load two images, resize to a shared (smaller) size if dimensions differ, return RGB arrays."""
    ia, ib = Image.open(path_a), Image.open(path_b)
    if ia.size != ib.size:
        w = min(ia.size[0], ib.size[0])
        h = min(ia.size[1], ib.size[1])
        ia, ib = ia.resize((w, h), Image.LANCZOS), ib.resize((w, h), Image.LANCZOS)
    return ia, ib


def score_theme(maui_path, other_path):
    """One (page, platform, theme) MAUI-vs-<framework> score, or None if either file is missing."""
    if not maui_path or not other_path:
        return None
    abs_maui = os.path.join(COMP, maui_path)
    abs_other = os.path.join(COMP, other_path)
    if not (os.path.isfile(abs_maui) and os.path.isfile(abs_other)):
        return None
    ia, ib = load_pair(abs_maui, abs_other)
    a_rgb = np.asarray(ia.convert("RGB"), dtype=np.int16)
    b_rgb = np.asarray(ib.convert("RGB"), dtype=np.int16)
    diff_pct = float(np.mean(np.max(np.abs(a_rgb - b_rgb), axis=-1) > DIFF_THRESHOLD) * 100)
    s = ssim(to_luma(ia), to_luma(ib))
    return {"ssim": round(s, 4), "diff_pct": round(diff_pct, 2)}


def classify(theme_scores):
    """theme_scores: {"light": {...}|None, "dark": {...}|None}. -> ({status}, review text)."""
    have = {t: v for t, v in theme_scores.items() if v is not None}
    if not have:
        return "blank", "No comparable MAUI/C++ screenshot pair exists for this page on this platform."
    worst_ssim = min(v["ssim"] for v in have.values())
    worst_diff = max(v["diff_pct"] for v in have.values())
    if worst_ssim >= 0.98 and worst_diff <= 1.0:
        status = "green"
    elif worst_ssim >= 0.90 and worst_diff <= 8.0:
        status = "yellow"
    else:
        status = "red"
    parts = []
    for t in THEMES:
        v = theme_scores.get(t)
        if v is not None:
            parts.append(f"{t.capitalize()}: SSIM {v['ssim']:.4f}, {v['diff_pct']:.2f}% pixels differ")
        elif t in theme_scores:
            parts.append(f"{t.capitalize()}: no comparable pair")
    review = " · ".join(parts)
    return status, review


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--only", default="", help="comma-separated page keys (default: all)")
    ap.add_argument("--platform", default=",".join(PLATFORMS), help="comma-separated platforms")
    args = ap.parse_args()

    plats = [p for p in args.platform.split(",") if p in PLATFORMS]
    pages = json.load(open(JSON, encoding="utf-8"))
    want = set(k.strip() for k in args.only.split(",") if k.strip()) if args.only else None

    # (screenshot framework key, comparison.json slot) — the two required MAUI-vs-<framework> pairs
    # per port/CLAUDE.md "Parity comparison policy" §5: cpp -> "pixel" (comparisons 1/3), xaml ->
    # "pixel_xaml" (comparisons 2/4). Mirrors comparison_paths.review_slot()'s cpp->bare/xaml->_xaml.
    SLOTS = [("cpp", "pixel"), ("xaml", "pixel_xaml")]

    scored = 0
    for page in pages:
        if want is not None and page["name"] not in want:
            continue
        for plat in plats:
            platform = page["platforms"].get(plat)
            if platform is None:
                continue
            sc = platform["screenshots"]
            maui = sc.get("maui", {})
            themes = THEMES if plat != "android" else ("light",)
            for fw, slot in SLOTS:
                other = sc.get(fw, {})
                theme_scores = {t: score_theme(maui.get(t), other.get(t)) for t in themes}
                status, review = classify(theme_scores)
                platform[slot] = {"status": status, "review": review}
                scored += 1

    json.dump(pages, open(JSON, "w", encoding="utf-8"), indent=2)
    print(f"scored {scored} page x platform x framework comparisons -> {JSON}")


if __name__ == "__main__":
    main()
