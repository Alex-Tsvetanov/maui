#!/usr/bin/env python3
"""Objective pixel-parity scoring for the Windows 3-way sweep — the numeric half of "pixel-perfect".

For every page/theme it loads the MAUI reference capture and the port capture (cpp and/or xaml) and
computes, with PIL only (no numpy/skimage):
  * dims_match  — same client pixel dimensions (a size delta is a real layout bug, never AA noise)
  * exact_pct   — % of pixels bit-identical
  * near_pct    — % of pixels within +/-TOL per channel (tolerates sub-pixel AA / font-hinting that
                  two independent WinUI renderers legitimately differ on)
  * mae         — mean absolute per-channel error (0..255)
and buckets each into match / minor / diff. Because MAUI and the port are DIFFERENT rendering stacks
(the port drives WinUI controls through its own handler seam), literal bit-equality is not the bar —
near_pct at TOL=16 is the "visually identical" signal; exact_pct is reported for reference.

GIF (animated) pages compare the FIRST frame here (a still proxy) and flag `animated` so the motion
diff is judged separately; a frame-count/among-frames delta is noted.

Writes docs/comparison/windows_pixel_scores.json and prints a worst-first table. Read the JSON to pick
the next port bug to fix.

Usage:
  python tools/parity/pixel_score.py [--framework cpp,xaml] [--theme light,dark] [key ...]
"""
from __future__ import annotations

import json
import os
import sys

from PIL import Image, ImageChops, ImageSequence

import comparison_paths as cp

HERE = os.path.dirname(os.path.abspath(__file__))
CMP = os.path.normpath(os.path.join(HERE, "..", "..", "docs", "comparison"))
OUT_JSON = os.path.join(CMP, "windows_pixel_scores.json")
PLATFORM = "windows"
TOL = 16  # per-channel tolerance for "near" — swallows AA / subpixel text, not real color/layout diffs

# ios_* pages have no Windows MAUI reference (apple-specific); skip them in the score set.
IOS_PREFIX = "ios_"


def load_rgb(path: str) -> "tuple[Image.Image, int]":
    """Return (first-frame RGB image, frame_count). frame_count>1 marks an animated GIF."""
    img = Image.open(path)
    frames = getattr(img, "n_frames", 1)
    img.seek(0)
    return img.convert("RGB"), frames


def score_pair(maui_path: str, port_path: str) -> dict:
    maui_img, maui_frames = load_rgb(maui_path)
    port_img, port_frames = load_rgb(port_path)
    dims_match = maui_img.size == port_img.size
    if not dims_match:
        port_img = port_img.resize(maui_img.size)  # align so the content diff is still meaningful
    diff = ImageChops.difference(maui_img, port_img).convert("L")
    hist = diff.histogram()  # 256 buckets of the max-channel-abs-diff proxy (L of RGB diff)
    total = sum(hist) or 1
    exact = hist[0] / total * 100.0
    near = sum(hist[0:TOL + 1]) / total * 100.0
    mae = sum(i * c for i, c in enumerate(hist)) / total
    return {
        "dims_match": dims_match,
        "maui_size": list(maui_img.size),
        "port_size": list(port_img.size),
        "exact_pct": round(exact, 2),
        "near_pct": round(near, 2),
        "mae": round(mae, 2),
        "animated": maui_frames > 1 or port_frames > 1,
        "frames": [maui_frames, port_frames],
    }


def verdict(s: dict) -> str:
    if not s["dims_match"]:
        return "diff"            # a dimension mismatch is always a real layout bug
    if s["near_pct"] >= 99.0 and s["mae"] <= 2.0:
        return "match"           # visually identical (AA-level noise only)
    if s["near_pct"] >= 95.0:
        return "minor"
    return "diff"


def main() -> int:
    args = sys.argv[1:]
    frameworks = ["cpp", "xaml"]
    themes = ["light", "dark"]
    keys: list[str] = []
    i = 0
    while i < len(args):
        if args[i] == "--framework":
            frameworks = [f.strip() for f in args[i + 1].split(",") if f.strip()]; i += 2
        elif args[i] == "--theme":
            themes = [t.strip() for t in args[i + 1].split(",") if t.strip()]; i += 2
        else:
            keys.append(args[i]); i += 1
    if not keys:
        keys = [k for k in cp.load_keys() if not k.startswith(IOS_PREFIX)]

    results: dict[str, dict] = {}
    worst: list[tuple[float, str, str, str, str]] = []  # (near_pct, key, fw, theme, verdict)
    for key in keys:
        entry: dict[str, dict] = {}
        for fw in frameworks:
            for theme in themes:
                maui_p = cp.find_capture(PLATFORM, "maui", key, theme)
                port_p = cp.find_capture(PLATFORM, fw, key, theme)
                if not maui_p or not port_p:
                    continue
                try:
                    s = score_pair(maui_p, port_p)
                except Exception as exc:  # a truncated/locked capture shouldn't abort the sweep
                    entry[f"{fw}_{theme}"] = {"error": str(exc)}
                    continue
                s["verdict"] = verdict(s)
                entry[f"{fw}_{theme}"] = s
                worst.append((s["near_pct"], key, fw, theme, s["verdict"]))
        if entry:
            results[key] = entry

    os.makedirs(CMP, exist_ok=True)
    with open(OUT_JSON, "w", encoding="utf-8") as fh:
        json.dump(results, fh, indent=1, ensure_ascii=False)
        fh.write("\n")

    # Summary: counts + the worst 40 pairs (the fix queue).
    buckets = {"match": 0, "minor": 0, "diff": 0}
    for _, _, _, _, v in worst:
        buckets[v] = buckets.get(v, 0) + 1
    print(f"scored {len(worst)} pairs across {len(results)} pages -> {OUT_JSON}")
    print(f"  match={buckets['match']}  minor={buckets['minor']}  diff={buckets['diff']}")
    print("\nworst 40 (fix queue):")
    for near, key, fw, theme, v in sorted(worst)[:40]:
        print(f"  {v:5} near={near:6.2f}%  {fw:4} {theme:5} {key}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
