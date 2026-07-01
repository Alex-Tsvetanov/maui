#!/usr/bin/env python3
"""Gemini second-model parity sweep for the macOS **Mac Catalyst** board (MAUI vs the C++ port).

Twin of `gemini_android_sweep.py`, but macOS Catalyst has a light+dark split (like iOS), so each
page compares FOUR images — `maccatalyst/{maui,cpp}/{light,dark}/<key>.png`. .NET MAUI on macOS IS
Mac Catalyst (UIKit), so this is the strict pixel-parity board (the Catalyst C++ backend reuses the
iOS handlers). For each canonical page key that has BOTH a MAUI reference and a C++ capture it asks
Google Gemini to judge MAUI-vs-C++ Catalyst parity, and writes one verdict per page to
`docs/comparison/macos_gemini_review.json` as:

    [{"key", "status", "note", "model"}]   status in: green | yellow | red | blank

(green=match, yellow=minor, red=major/diff, blank=port page blank/crashed — the same buckets the
Sonnet `macos_render_review.json` uses so `gen_macos_readme_section.py` renders both columns alike.)

Quota-aware + resumable exactly like the Android sweep: on HTTP 429 it rotates through the model
cascade, records remaining pages as pending, and stops rather than failing; re-running continues where
a quota cut it off. Pages missing either the MAUI or C++ capture are skipped (no oracle to compare).

Key:   $GEMINI_API_KEY -> ~/.config/maui-parity/gemini_api_key
Usage: python3 tools/parity/gemini_macos_sweep.py [--limit N] [--only key1,key2]
"""
from __future__ import annotations

import argparse
import base64
import json
import os
import sys
import time
import urllib.error
import urllib.request

HERE = os.path.dirname(os.path.abspath(__file__))
COMP = os.path.normpath(os.path.join(HERE, "..", "..", "docs", "comparison"))
MC = os.path.join(COMP, "maccatalyst")
KEY_FILE = os.path.expanduser("~/.config/maui-parity/gemini_api_key")
OUT = os.path.join(COMP, "macos_gemini_review.json")
PAGE_KEYS = os.path.join(HERE, "page_keys.txt")
THEMES = ("light", "dark")

ENUM = ["match", "minor", "diff", "cpp_blank"]
TO_BOARD = {"match": "green", "minor": "yellow", "diff": "red", "cpp_blank": "blank"}
ENUM_BOARD = {"green", "yellow", "red", "blank"}

RESPONSE_SCHEMA = {
    "type": "OBJECT",
    "properties": {
        "verdict": {"type": "STRING", "enum": ENUM},
        "note": {"type": "STRING"},
    },
    "required": ["verdict", "note"],
    "propertyOrdering": ["verdict", "note"],
}

PROMPT = """\
Assess macOS **Mac Catalyst** layout/render PARITY between real .NET MAUI and the C++23 port of MAUI for
ONE demo page. .NET MAUI on macOS IS Mac Catalyst (UIKit), so this is a STRICT pixel-parity board — the
Catalyst C++ backend reuses the iOS handlers and should match MAUI closely.
You are given FOUR screenshots of the SAME page, captured on the SAME Mac / Catalyst window, in this order:
  1. MAUI light  (the real Microsoft .NET MAUI Catalyst render — GROUND TRUTH, native-default)
  2. MAUI dark
  3. C++ port light  (under test)
  4. C++ port dark

GROUND TRUTH: MAUI's render IS the source of truth for all page CONTENT — colors, control sizes, internal
spacing, text, corner radius, fonts. Any CONTENT difference in the port vs MAUI (including any color
difference), in EITHER appearance, is a PORT BUG. IGNORE ONLY the harness wrapper (NOT port diffs):
  • the macOS traffic-light window chrome / window title bar,
  • the outer window-inset/margin magnitude and the resulting uniform global shift + crop,
  • pure runtime state: a specific date/time value, a toggle's on/off, a spinner caught mid-rotation
    (at most 'minor').
Judge BOTH appearances; if only one theme diverges, say which.

Pick exactly one verdict:
  - match : no port bugs — the port matches MAUI's content (colors/sizes/spacing/text) in BOTH themes,
            pixel-perfect or effectively identical once the window chrome/inset is set aside.
  - minor : small content differences (a slightly-off color/size/spacing, a missing minor detail, one
            theme slightly off) that don't change the page's substance.
  - diff  : a MAJOR difference — a missing/extra/mis-sized control, absent image/shadow/gradient/effect,
            wrong layout, wrong/garbled text, overlap, wrong content, or one theme badly broken.
  - cpp_blank : the C++ port's page is blank/empty/crashed while MAUI shows content.

For "note" give ONE precise, terse line naming the SPECIFIC port-relevant difference (or "matches MAUI"),
mentioning the theme if only one is affected.
"""


def log(m):
    print(m, file=sys.stderr, flush=True)


def read_key():
    k = os.environ.get("GEMINI_API_KEY", "").strip()
    if k:
        return k
    with open(KEY_FILE, encoding="utf-8") as fh:
        return fh.read().strip()


def load_keys():
    with open(PAGE_KEYS, encoding="utf-8") as fh:
        return [ln.strip() for ln in fh if ln.strip() and not ln.startswith("#")]


def img_part(path):
    with open(path, "rb") as fh:
        return {"inline_data": {"mime_type": "image/png", "data": base64.b64encode(fh.read()).decode("ascii")}}


def shot(sub, theme, key):
    return os.path.join(MC, sub, theme, f"{key}.png")


def call_gemini(model, api_key, key, timeout=120):
    """Return (board_status, note); None if any of the 4 images is missing; raise QuotaError on 429."""
    paths = [(sub, th) for sub in ("maui", "cpp") for th in THEMES]
    files = [shot(sub, th, key) for sub, th in paths]
    if not all(os.path.isfile(f) for f in files):
        return None
    parts = [
        {"text": "Image: MAUI light"}, img_part(shot("maui", "light", key)),
        {"text": "Image: MAUI dark"}, img_part(shot("maui", "dark", key)),
        {"text": "Image: C++ port light"}, img_part(shot("cpp", "light", key)),
        {"text": "Image: C++ port dark"}, img_part(shot("cpp", "dark", key)),
        {"text": PROMPT},
    ]
    body = json.dumps({
        "contents": [{"parts": parts}],
        "generationConfig": {"responseMimeType": "application/json", "responseSchema": RESPONSE_SCHEMA, "temperature": 0},
    }).encode("utf-8")
    url = f"https://generativelanguage.googleapis.com/v1beta/models/{model}:generateContent"
    for attempt in range(4):
        req = urllib.request.Request(url, data=body, method="POST",
                                     headers={"Content-Type": "application/json", "X-goog-api-key": api_key})
        try:
            with urllib.request.urlopen(req, timeout=timeout) as resp:
                data = json.loads(resp.read().decode("utf-8"))
            text = data["candidates"][0]["content"]["parts"][0]["text"]
            v = json.loads(text)
            return TO_BOARD.get(v["verdict"], "yellow"), v.get("note", "")
        except urllib.error.HTTPError as e:
            detail = e.read().decode("utf-8", "replace")
            if e.code == 429 or "RESOURCE_EXHAUSTED" in detail:
                raise QuotaError(detail[:200])
            if 500 <= e.code < 600 and attempt < 3:
                time.sleep(2 ** attempt); continue
            raise RuntimeError(f"HTTP {e.code}: {detail[:200]}")
        except (urllib.error.URLError, TimeoutError) as e:
            if attempt < 3:
                time.sleep(2 ** attempt); continue
            raise RuntimeError(f"network: {e}")
    raise RuntimeError("exhausted retries")


class QuotaError(Exception):
    pass


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--limit", type=int, default=0, help="only process the first N unreviewed keys")
    ap.add_argument("--only", default="", help="comma-separated keys to (re)review")
    ap.add_argument("--model", default="", help="force ONE model (disables cascade)")
    ap.add_argument("--cascade", default="gemini-3.1-flash-lite,gemini-2.5-flash-lite,gemini-2.0-flash-lite,gemini-3.5-flash,gemini-2.5-flash",
                    help="quota-aware model cascade (best 500-RPD workhorse first); rotates on 429")
    ap.add_argument("--sleep", type=float, default=1.2, help="delay between calls (rate-limit courtesy)")
    args = ap.parse_args()

    api_key = read_key()
    keys = load_keys()
    existing = {}
    if os.path.exists(OUT):
        try:
            existing = {r["key"]: r for r in json.load(open(OUT, encoding="utf-8"))}
        except (OSError, ValueError):
            existing = {}

    only = set(k.strip() for k in args.only.split(",") if k.strip())
    todo = [k for k in keys if (k in only if only else existing.get(k, {}).get("status", "") not in ENUM_BOARD)]
    if args.limit:
        todo = todo[: args.limit]
    log(f"macos gemini sweep: {len(todo)} to review (of {len(keys)}), model={args.model}")

    cascade = [args.model] if args.model else [m.strip() for m in args.cascade.split(",") if m.strip()]
    mi, done, exhausted = 0, 0, False

    def persist():
        rows = [existing.get(kk, {"key": kk, "status": "", "note": "", "model": ""}) for kk in keys]
        json.dump(rows, open(OUT, "w", encoding="utf-8"), ensure_ascii=False, indent=0)

    for i, k in enumerate(todo, 1):
        res = None
        while mi < len(cascade):
            model = cascade[mi]
            try:
                res = call_gemini(model, api_key, k)
                break
            except QuotaError as e:
                log(f"[{i}/{len(todo)}] {k}: QUOTA on {model} -> rotate to next model ({e})")
                mi += 1
            except RuntimeError as e:
                log(f"[{i}/{len(todo)}] {k}: ERROR {e} — marking pending")
                existing[k] = {"key": k, "status": "", "note": f"(gemini error: {e})", "model": model}
                res = "ERR"
                break
        else:
            log(f"[{i}/{len(todo)}] ALL {len(cascade)} models exhausted at {k} — {len(todo)-i+1} left pending")
            exhausted = True
            break
        if res in ("ERR",):
            continue
        if res is None:
            log(f"[{i}/{len(todo)}] {k}: missing image(s) — skip (no MAUI/C++ oracle)")
            continue
        status, note = res
        existing[k] = {"key": k, "status": status, "note": note, "model": cascade[mi]}
        done += 1
        log(f"[{i}/{len(todo)}] {k}: {status} [{cascade[mi]}] — {note[:60]}")
        persist()
        time.sleep(args.sleep)

    persist()
    from collections import Counter
    rows = json.load(open(OUT, encoding="utf-8"))
    log(f"DONE: reviewed {done} this run; board {dict(Counter(r['status'] or 'pending' for r in rows))}"
        + (" [ALL MODELS EXHAUSTED: re-run tomorrow to finish]" if exhausted else ""))


if __name__ == "__main__":
    main()
