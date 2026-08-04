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

**Animated pages are scored FRAME BY FRAME**, at full resolution, by lib/motion_score.py — worst-frame
and mean SSIM across the sequence, so a port that reaches the right END state through the wrong
intermediate frames is caught. The worst frame supplies the {ssim, diff_pct} the thresholds below judge.
When the frames are unavailable (they live in the per-run, gitignored run directory — see that module's
header for why that source was chosen, and why iOS/Android cannot be motion-scored yet) the cell keeps
its single-still number and the review SAYS "NOT motion-scored"; it never passes a still off as motion.

Mismatched image dimensions are resized (LANCZOS) to the smaller common size before comparing — a
known limitation: this is a global comparison, not a registered/aligned diff, so a uniform outer-inset
shift (exempt per parity policy) can still show up as a nonzero diff_pct/SSIM<1. Missing screenshots
(either side null, or file absent) produce no score for that theme; a page with NO computable theme for
a given framework gets status "blank" on that framework's slot (mirrors the sonnet/gemini convention).

Usage: python3 tools/parity/pixel_score.py [--only key1,key2] [--platform ios,maccatalyst,android,windows]
Writes results directly into docs/comparison/comparison.json (preserves everything else). Run
docs/comparison/tools/gen_readme.py afterward to render the scores.
"""
import argparse
import json
import os

import numpy as np
from PIL import Image

import motion_score

HERE = os.path.dirname(os.path.abspath(__file__))
CPP_ROOT = os.path.normpath(os.path.join(HERE, "..", "..", ".."))
COMP = os.path.join(CPP_ROOT, "docs", "comparison")
JSON = os.path.join(COMP, "comparison.json")

PLATFORMS = ("ios", "maccatalyst", "android", "windows")
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


def score_images(ia, ib, crop_top=0):
    """Two same-size PIL Images -> {"ssim", "diff_pct"}. Split out of score_theme so motion_score can
    score a pair of RUN-DIRECTORY frames through exactly this code rather than a second copy of it.

    crop_top: rows to drop from the TOP of both images before comparing — used on Android to exclude the
    system STATUS BAR (clock/battery/wifi), which differs between captures purely because they were shot at
    different times, not because of any port rendering (the same capture-chrome exemption the iOS harness
    inset gets under ruling 2). Measured: the status bar occupies rows 0..~135 and differs on 100% of pages;
    the page content below it aligns. Both hosts run NoActionBar, so there is no app title bar to keep."""
    if crop_top > 0:
        w, h = ia.size
        if h > crop_top:
            ia, ib = ia.crop((0, crop_top, w, h)), ib.crop((0, crop_top, w, h))
    a_rgb = np.asarray(ia.convert("RGB"), dtype=np.int16)
    b_rgb = np.asarray(ib.convert("RGB"), dtype=np.int16)
    diff_pct = float(np.mean(np.max(np.abs(a_rgb - b_rgb), axis=-1) > DIFF_THRESHOLD) * 100)
    s = ssim(to_luma(ia), to_luma(ib))
    return {"ssim": round(s, 4), "diff_pct": round(diff_pct, 2)}


def score_theme(maui_path, other_path, crop_top=0):
    """One (page, platform, theme) MAUI-vs-<framework> score, or None if either file is missing."""
    if not maui_path or not other_path:
        return None
    abs_maui = os.path.join(COMP, maui_path)
    abs_other = os.path.join(COMP, other_path)
    if not (os.path.isfile(abs_maui) and os.path.isfile(abs_other)):
        return None
    return score_images(*load_pair(abs_maui, abs_other), crop_top)


def full_res(rel_path):
    """Score from the PNG even when the board DISPLAYS a GIF.

    An animated page's board cell points at `<key>_<theme>.gif`, which ffmpeg wrote at 400px wide —
    2.7x smaller in each dimension than the 1080px still beside it. Scoring that downscale throws away
    6/7 of the pixels and inflates SSIM: activity_indicator scored 0.9942 from its GIF while the
    equivalent still-vs-still comparison sees the real detail. The GIF stays the thing humans look at;
    the numbers come from the full-resolution still whenever one exists.
    """
    if rel_path and rel_path.endswith(".gif"):
        png = rel_path[:-4] + ".png"
        if os.path.isfile(os.path.join(COMP, png)):
            return png
    return rel_path


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
    # One column ANIMATES and the other is FROZEN is a parity failure by definition, and the per-frame
    # SSIM cannot be trusted to expose it — a spinner is a few hundred pixels, so a frozen port can
    # still score 0.99 on every frame. So the verdict is forced rather than inferred; motion_score puts
    # both self-motion percentages in the review text, so a forced red is arguable from that one line.
    if any(v.get("mismatch") for v in have.values()):
        status = "red"
    # NEITHER column moved, on a page the board calls animated. Two frozen columns are byte-identical,
    # so this arrives as a perfect score — the single most misleading green the board can produce, and
    # exactly the state the whole interaction pass exists to eliminate. It is capped at yellow rather
    # than forced red because it is not evidence of a PORT defect: it says the page was never driven
    # (no scenario, or an interaction this lane cannot reach). Yellow puts it in front of a human
    # without accusing the port of something the capture never tested.
    if any(v.get("both_frozen") for v in have.values()) and status == "green":
        status = "yellow"
    parts = []
    for t in THEMES:
        v = theme_scores.get(t)
        if v is not None:
            # `detail` is motion_score's sentence (a frame-by-frame score, or the still number plus the
            # reason it could NOT be motion-scored). Absent on the ~158 non-animated pages.
            parts.append(f"{t.capitalize()}: " + (v.get("detail") or
                         f"SSIM {v['ssim']:.4f}, {v['diff_pct']:.2f}% pixels differ"))
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
    FW_LABEL = {"cpp": "C++", "xaml": "C++ & XAML"}   # how the motion review names the port column

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
            themes = THEMES  # Android is now captured in both light + dark (like iOS/macOS)
            for fw, slot in SLOTS:
                other = sc.get(fw, {})
                crop_top = 140 if plat == "android" else 0  # exclude the Android status bar (see score_images)
                theme_scores = {t: score_theme(full_res(maui.get(t)), full_res(other.get(t)), crop_top)
                                for t in themes}
                if page["name"] in motion_score.ANIMATED:
                    # The trigger is the ANIMATED set, not "captures/ holds a .gif": a page whose GIF
                    # assembly FAILED is still an animated page, and the run frames can still score its
                    # motion. Every score here either becomes a frame-by-frame number or keeps the
                    # still one carrying the reason it could not — never a silent single-frame verdict.
                    theme_scores = {t: motion_score.score_cell(page["name"], plat, fw, t, crop_top, v,
                                                               fw_label=FW_LABEL[fw])
                                    for t, v in theme_scores.items()}
                status, review = classify(theme_scores)
                platform[slot] = {"status": status, "review": review}
                scored += 1

    json.dump(pages, open(JSON, "w", encoding="utf-8"), indent=2)
    print(f"scored {scored} page x platform x framework comparisons -> {JSON}")


if __name__ == "__main__":
    main()
