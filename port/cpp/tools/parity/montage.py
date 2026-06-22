#!/usr/bin/env python3
"""Composite a page's 4 parity captures into a 2x2 labeled grid for review.
Row 1: MAUI light | C++ light   Row 2: MAUI dark | C++ dark.
Usage: montage.py <key> [<key> ...]  -> build/cmp/<key>.png
"""
import os, sys
from PIL import Image, ImageDraw

HERE = os.path.dirname(os.path.abspath(__file__))
CMP = os.path.normpath(os.path.join(HERE, "..", "..", "docs", "comparison"))
OUT = os.path.normpath(os.path.join(HERE, "..", "..", "build", "cmp"))
os.makedirs(OUT, exist_ok=True)
GRID = [[("csharp_ios_light", "MAUI light"), ("cpp_ios_light", "C++ light")],
        [("csharp_ios_dark", "MAUI dark"), ("cpp_ios_dark", "C++ dark")]]
H = 740        # panel height
BAND = 24
PAD = 6

def load(key, d, lab):
    p = os.path.join(CMP, d, f"{key}.png")
    if os.path.exists(p):
        im = Image.open(p).convert("RGB")
        return im.resize((max(1, int(im.width * H / im.height)), H)), lab
    return Image.new("RGB", (int(H*0.46), H), (40, 40, 40)), lab + " (missing)"

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
            canvas.paste(im, (x + (colw - im.width)//2, y + BAND))
    out = os.path.join(OUT, f"{key}.png")
    canvas.save(out)
    return out

for k in sys.argv[1:]:
    print(build(k))
