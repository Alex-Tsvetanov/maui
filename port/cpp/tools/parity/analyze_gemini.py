#!/usr/bin/env python3
"""Gemini montage analysis: per example, categorize C++-vs-MAUI parity + bullet notes.

Feeds the 2x2 montage (docs/comparison/montages/<key>.png: TL=MAUI light, TR=C++ light, BL=MAUI dark,
BR=C++ dark) + the deterministic diff metrics (diff_results.json) to Gemini, which returns ONE category
+ short notes. Writes/updates docs/comparison/analysis_gemini.json incrementally (resumable: skips keys
already present unless --force). Quota -> cascade to a flash-lite model; if all exhausted, stop (rerun
later picks up where it left off).

Usage: python3 analyze_gemini.py [--only k1,k2] [--force] [--models gemini-2.5-flash,gemini-3.1-flash-lite]
"""
import argparse
import base64
import json
import os
import sys
import time
import urllib.error
import urllib.request

HERE = os.path.dirname(os.path.abspath(__file__))
CMP = os.path.normpath(os.path.join(HERE, "..", "..", "docs", "comparison"))
MONT = os.path.join(CMP, "montages")
OUT = os.path.join(CMP, "analysis_gemini.json")
KEY_FILE = os.path.expanduser("~/.config/maui-parity/gemini_api_key")

CATS = ["pixel_perfect", "match", "cpp_minor", "cpp_major", "cpp_blank",
        "maui_minor", "maui_major", "maui_blank"]
SCHEMA = {"type": "OBJECT", "properties": {
    "category": {"type": "STRING", "enum": CATS},
    "notes": {"type": "ARRAY", "items": {"type": "STRING"}},
}, "required": ["category", "notes"]}

PROMPT = """This is a 2x2 montage comparing a C++ reimplementation of .NET MAUI against real .NET MAUI on
the SAME iPhone-17 simulator (native scale). Quadrants: TOP-LEFT = MAUI light, TOP-RIGHT = C++ light,
BOTTOM-LEFT = MAUI dark, BOTTOM-RIGHT = C++ dark. Compare the C++ port (RIGHT column) to the MAUI
reference (LEFT column) per theme.

A deterministic pixel diff (status-bar clock masked out) reports: {diffhint}

Pick exactly ONE category for the C++ port's parity:
- pixel_perfect: visually identical; only anti-aliasing/sub-pixel/font-hinting differences.
- match: equivalent; only trivial differences a user wouldn't notice.
- cpp_minor: small C++-side differences (slightly off spacing/shade/weight).
- cpp_major: notable C++-side problems (wrong layout/size/color, missing or extra elements).
- cpp_blank: the C++ side is blank/empty while MAUI shows content.
- maui_minor / maui_major / maui_blank: the DEFECT is on the MAUI side (e.g. MAUI failed to load an
  image or render the page) while the C++ side is correct.

Judge CONTENT (sizes, colors, spacing, text, present/missing controls) — IGNORE the status-bar clock and
any harness chrome. Give 1-4 short, concrete bullet notes naming the actual differences."""


def read_key():
    k = os.environ.get("GEMINI_API_KEY", "").strip()
    if k:
        return k
    try:
        return open(KEY_FILE).read().strip()
    except OSError:
        print("ERROR: no GEMINI_API_KEY / key file", file=sys.stderr)
        sys.exit(1)


def diffhint(dr, key):
    e = dr.get(key, {})
    bits = []
    for t in ("light", "dark"):
        v = e.get(t, {})
        bits.append(f"{t} SSIM={v.get('ssim')} diff={v.get('diff_pct')}% (pixel_perfect={v.get('pixel_perfect')})")
    return "; ".join(bits)


def call(model, api_key, img_b64, prompt, timeout=120):
    url = f"https://generativelanguage.googleapis.com/v1beta/models/{model}:generateContent"
    body = json.dumps({
        "contents": [{"parts": [
            {"inline_data": {"mime_type": "image/png", "data": img_b64}},
            {"text": prompt}]}],
        "generationConfig": {"responseMimeType": "application/json", "responseSchema": SCHEMA,
                             "temperature": 0},
    }).encode()
    req = urllib.request.Request(url, data=body, method="POST",
                                headers={"Content-Type": "application/json", "X-goog-api-key": api_key})
    with urllib.request.urlopen(req, timeout=timeout) as resp:
        data = json.loads(resp.read().decode())
    txt = data["candidates"][0]["content"]["parts"][0]["text"]
    return json.loads(txt)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--only", default="")
    ap.add_argument("--force", action="store_true")
    ap.add_argument("--models", default="gemini-2.5-flash:5,gemini-3.1-flash-lite:15,gemini-2.0-flash-lite:30")
    args = ap.parse_args()

    api_key = read_key()
    dr = json.load(open(os.path.join(CMP, "diff_results.json")))
    keys = [ln.strip() for ln in open(os.path.join(HERE, "page_keys.txt")) if ln.strip()]
    if args.only:
        want = set(args.only.split(","))
        keys = [k for k in keys if k in want]
    out = {}
    if os.path.exists(OUT):
        out = json.load(open(OUT))

    cascade = [(m.split(":")[0], float(m.split(":")[1]) if ":" in m else 10.0) for m in args.models.split(",")]
    mi = 0
    last = 0.0
    for i, key in enumerate(keys, 1):
        if key in out and not args.force:
            continue
        mont = os.path.join(MONT, f"{key}.png")
        if not os.path.exists(mont):
            print(f"[{i}] {key}: NO montage, skip", flush=True)
            continue
        img = base64.b64encode(open(mont, "rb").read()).decode()
        prompt = PROMPT.replace("{diffhint}", diffhint(dr, key))
        while mi < len(cascade):
            model, rpm = cascade[mi]
            wait = max(0.0, 60.0 / rpm * 1.05 - (time.monotonic() - last))
            if last and wait > 0:
                time.sleep(wait)
            try:
                v = call(model, api_key, img, prompt)
                last = time.monotonic()
                v["model"] = model
                out[key] = v
                json.dump(out, open(OUT, "w"), indent=1)
                print(f"[{i}/{len(keys)}] {key}: {v['category']} [{model}]", flush=True)
                break
            except urllib.error.HTTPError as e:
                last = time.monotonic()
                if e.code == 429 or e.code == 404:
                    print(f"  {model} quota/unavailable -> next model", flush=True)
                    mi += 1
                    continue
                print(f"  {key}: HTTP {e.code}; skipping", flush=True)
                break
            except Exception as e:  # noqa: BLE001 — keep the sweep alive, record nothing for this key
                print(f"  {key}: {type(e).__name__} {e}; skipping", flush=True)
                break
        if mi >= len(cascade):
            print("ALL Gemini models exhausted; rerun later to resume.", flush=True)
            break
    print(f"ANALYZE_GEMINI_DONE judged={len(out)}", flush=True)


if __name__ == "__main__":
    main()
