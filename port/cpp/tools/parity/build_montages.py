#!/usr/bin/env python3
"""Build the per-example 2x2 demo montage from the NATIVE captures (the new pixel-perfect pipeline).

Row 1: MAUI light | C++ light    Row 2: MAUI dark | C++ dark   (same convention as montage.py).
Sources docs/comparison/captures/{maui,cpp}_{light,dark}/<key>.png -> docs/comparison/montages/<key>.png.
Keys from tools/parity/page_keys.txt (or --only). Animated pages (a still frame exists too) get a still
montage here; the README embeds their GIFs separately.

Usage: python3 build_montages.py [--only k1,k2]
"""
import argparse
import os

from PIL import Image, ImageDraw

HERE = os.path.dirname(os.path.abspath(__file__))
CMP = os.path.normpath(os.path.join(HERE, "..", "..", "docs", "comparison"))
CAP = os.path.join(CMP, "captures")
OUT = os.path.join(CMP, "montages")
GRID = [[("maui_light", "MAUI light"), ("cpp_light", "C++ light")],
        [("maui_dark", "MAUI dark"), ("cpp_dark", "C++ dark")]]
H, BAND, PAD = 740, 24, 6


def load(key, d, lab):
    p = os.path.join(CAP, d, f"{key}.png")
    if os.path.exists(p):
        im = Image.open(p).convert("RGB")
        return im.resize((max(1, int(im.width * H / im.height)), H)), lab
    return Image.new("RGB", (int(H * 0.46), H), (40, 40, 40)), lab + " (missing)"


def build(key):
    rows = [[load(key, d, lab) for d, lab in row] for row in GRID]
    colw = max(im.width for row in rows for im, _ in row)
    tw = colw * 2 + PAD * 3
    rh = H + BAND
    th = rh * 2 + PAD * 3
    canvas = Image.new("RGB", (tw, th), (255, 255, 255))
    dr = ImageDraw.Draw(canvas)
    for r, row in enumerate(rows):
        y = PAD + r * (rh + PAD)
        for c, (im, lab) in enumerate(row):
            x = PAD + c * (colw + PAD)
            dr.rectangle([x, y, x + colw, y + BAND], fill=(20, 20, 20))
            dr.text((x + 5, y + 6), lab, fill=(255, 255, 255))
            canvas.paste(im, (x + (colw - im.width) // 2, y + BAND))
    out = os.path.join(OUT, f"{key}.png")
    canvas.save(out)
    return out


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--only", default="")
    args = ap.parse_args()
    os.makedirs(OUT, exist_ok=True)
    with open(os.path.join(HERE, "page_keys.txt")) as fh:
        keys = [ln.strip() for ln in fh if ln.strip()]
    if args.only:
        want = set(args.only.split(","))
        keys = [k for k in keys if k in want]
    for i, k in enumerate(keys, 1):
        build(k)
        print(f"[{i}/{len(keys)}] montage {k}", flush=True)
    print("MONTAGES_DONE", flush=True)


if __name__ == "__main__":
    main()
