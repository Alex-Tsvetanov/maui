#!/usr/bin/env python3
"""Gemini second-model parity sweep for the macOS **Mac Catalyst** board (MAUI vs a C++ framework).

Twin of `gemini_android_sweep.py`, but macOS Catalyst has a light+dark split (like iOS), so each
page compares FOUR images — `captures/maccatalyst/{maui,<framework>}/<key>_{light,dark}.png` (framework
in {cpp, xaml}). .NET MAUI on macOS IS Mac Catalyst (UIKit), so this is the strict pixel-parity board
(the Catalyst C++ backend reuses the iOS handlers). For each canonical page key that has BOTH a MAUI
reference and a framework capture it asks Google Gemini to judge parity and writes the verdict DIRECTLY
into the single `docs/comparison/comparison.json` — the `platforms.maccatalyst` review slot for the
framework: cpp -> `gemini`, xaml -> `gemini_xaml` (see comparison_paths.review_slot). Each slot is
`{"status", "review"}` with status in green | yellow | red | blank.

The user wants cpp and xaml reviewed SEPARATELY against maui — pass --framework cpp (default) or
--framework xaml. NOTE: the strict-parity columns are maui/cpp/xaml (UIKit); the AppKit columns have
no MAUI reference and are not swept here.

Quota-aware + resumable exactly like the Android sweep: on HTTP 429 it rotates through the model
cascade and stops rather than failing; re-running continues where a quota cut it off. Pages missing
either the MAUI or framework capture (in either theme) are skipped (no oracle to compare).

Key:   $GEMINI_API_KEY -> ~/.config/maui-parity/gemini_api_key
Usage: python3 tools/parity/gemini_macos_sweep.py [--framework cpp|xaml] [--limit N]
                                                  [--only key1,key2] [--force] [--dry-run]
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

import comparison_paths as cp

HERE = os.path.dirname(os.path.abspath(__file__))
COMP = cp.COMP
KEY_FILE = os.path.expanduser("~/.config/maui-parity/gemini_api_key")
PLATFORM = "maccatalyst"
THEMES = ("light", "dark")

ENUM = ["match", "minor", "diff", "cpp_blank"]
TO_BOARD = {"match": "green", "minor": "yellow", "diff": "red", "cpp_blank": "blank"}
ENUM_BOARD = {"green", "yellow", "red", "blank"}
FW_LABEL = {"cpp": "C++23 port", "xaml": "C++23 & XAML port"}

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
Assess macOS **Mac Catalyst** layout/render PARITY between real .NET MAUI and the {fw_label} of MAUI for
ONE demo page. .NET MAUI on macOS IS Mac Catalyst (UIKit), so this is a STRICT pixel-parity board — the
Catalyst C++ backend reuses the iOS handlers and should match MAUI closely.
You are given FOUR screenshots of the SAME page, captured on the SAME Mac / Catalyst window, in this order:
  1. MAUI light  (the real Microsoft .NET MAUI Catalyst render — GROUND TRUTH, native-default)
  2. MAUI dark
  3. {fw_label} light  (under test)
  4. {fw_label} dark

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
  - cpp_blank : the port's page is blank/empty/crashed while MAUI shows content.

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


def img_part(path):
    with open(path, "rb") as fh:
        return {"inline_data": {"mime_type": "image/png", "data": base64.b64encode(fh.read()).decode("ascii")}}


def shot(sub, theme, key):
    return cp.find_capture(PLATFORM, sub, key, theme)


def call_gemini(model, api_key, key, framework, timeout=120):
    """Return (board_status, note); None if any of the 4 images is missing; raise QuotaError on 429."""
    fw_label = FW_LABEL[framework]
    files = {(sub, th): shot(sub, th, key) for sub in ("maui", framework) for th in THEMES}
    if not all(files.values()):
        return None
    parts = [
        {"text": "Image: MAUI light"}, img_part(files[("maui", "light")]),
        {"text": "Image: MAUI dark"}, img_part(files[("maui", "dark")]),
        {"text": f"Image: {fw_label} light"}, img_part(files[(framework, "light")]),
        {"text": f"Image: {fw_label} dark"}, img_part(files[(framework, "dark")]),
        {"text": PROMPT.format(fw_label=fw_label)},
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


def slot_status(page, slot):
    """Existing board status in this page's maccatalyst review slot, or '' if unset."""
    return (page.get("platforms", {}).get(PLATFORM, {}).get(slot, {}) or {}).get("status") or ""


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--framework", default="cpp", choices=("cpp", "xaml"),
                    help="which C++ column to review against MAUI (writes gemini or gemini_xaml slot)")
    ap.add_argument("--limit", type=int, default=0, help="only process the first N unreviewed keys")
    ap.add_argument("--only", default="", help="comma-separated keys to (re)review")
    ap.add_argument("--force", action="store_true", help="re-review even keys that already have a status")
    ap.add_argument("--model", default="", help="force ONE model (disables cascade)")
    ap.add_argument("--cascade", default="gemini-3.1-flash-lite,gemini-2.5-flash-lite,gemini-2.0-flash-lite,gemini-3.5-flash,gemini-2.5-flash",
                    help="quota-aware model cascade (best 500-RPD workhorse first); rotates on 429")
    ap.add_argument("--sleep", type=float, default=1.2, help="delay between calls (rate-limit courtesy)")
    ap.add_argument("--dry-run", action="store_true",
                    help="print each key's source captures + target review slot WITHOUT calling Gemini")
    args = ap.parse_args()

    slot = cp.review_slot("gemini", args.framework)
    data = cp.load_comparison()
    by_name = {p["name"]: p for p in data}
    keys = [k for k in cp.load_keys() if k in by_name]

    only = set(k.strip() for k in args.only.split(",") if k.strip())
    if only:
        todo = [k for k in keys if k in only]
    elif args.force:
        todo = list(keys)
    else:
        todo = [k for k in keys if slot_status(by_name[k], slot) not in ENUM_BOARD]
    if args.limit:
        todo = todo[: args.limit]

    if args.dry_run:
        for k in todo:
            files = {(sub, th): shot(sub, th, k) for sub in ("maui", args.framework) for th in THEMES}
            ok = "OK" if all(files.values()) else "MISSING"
            have = ",".join(f"{sub}/{th}" for (sub, th), p in files.items() if p)
            print(f"{k}: {ok} [{have}] -> platforms.{PLATFORM}.{slot}")
        print(f"DRY_RUN_DONE ({len(todo)} keys, framework={args.framework}, slot={slot})")
        return

    api_key = read_key()
    log(f"macos gemini sweep: {len(todo)} to review (of {len(keys)}), framework={args.framework}, "
        f"slot={slot}, model={args.model or 'cascade'}")

    cascade = [args.model] if args.model else [m.strip() for m in args.cascade.split(",") if m.strip()]
    mi, done, exhausted = 0, 0, False

    for i, k in enumerate(todo, 1):
        res = None
        while mi < len(cascade):
            model = cascade[mi]
            try:
                res = call_gemini(model, api_key, k, args.framework)
                break
            except QuotaError as e:
                log(f"[{i}/{len(todo)}] {k}: QUOTA on {model} -> rotate to next model ({e})")
                mi += 1
            except RuntimeError as e:
                log(f"[{i}/{len(todo)}] {k}: ERROR {e} — skipping")
                res = "ERR"
                break
        else:
            log(f"[{i}/{len(todo)}] ALL {len(cascade)} models exhausted at {k} — {len(todo)-i+1} left pending")
            exhausted = True
            break
        if res in ("ERR",):
            continue
        if res is None:
            log(f"[{i}/{len(todo)}] {k}: missing image(s) — skip (no MAUI/{args.framework} oracle)")
            continue
        status, note = res
        cp.write_review(data, k, PLATFORM, "gemini", args.framework, status, note)
        done += 1
        log(f"[{i}/{len(todo)}] {k}: {status} [{cascade[mi]}] — {note[:60]}")
        cp.save_comparison(data)  # persist after each verdict so a quota cut leaves a committable file
        time.sleep(args.sleep)

    cp.save_comparison(data)
    from collections import Counter
    tally = Counter(slot_status(by_name[k], slot) or "pending" for k in keys)
    log(f"DONE: reviewed {done} this run; {PLATFORM}.{slot} board {dict(tally)}"
        + (" [ALL MODELS EXHAUSTED: re-run tomorrow to finish]" if exhausted else ""))


if __name__ == "__main__":
    main()
