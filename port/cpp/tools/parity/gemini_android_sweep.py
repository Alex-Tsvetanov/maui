#!/usr/bin/env python3
"""Gemini second-model parity sweep for the ANDROID column (MAUI vs a C++ framework).

Twin of `gemini_compare.py` but for Android's SINGLE pair per page (there is no
light/dark split on the Android board): for each of the 172 canonical page keys it
loads `docs/comparison/captures/android/{maui,<framework>}/<key>_light.png` (framework in
{cpp, xaml}), asks Google Gemini to judge MAUI-vs-<framework> parity, and writes the verdict
DIRECTLY into the single `docs/comparison/comparison.json` — the `platforms.android` review
slot for the framework: cpp -> `gemini`, xaml -> `gemini_xaml` (see comparison_paths.review_slot).
Each slot is `{"status", "review"}` with status in green | yellow | red | blank
(green=match, yellow=minor, red=major/diff, blank=port page blank/crashed).

The user wants cpp and xaml reviewed SEPARATELY against maui — pass --framework cpp (default)
or --framework xaml to target the matching column + review slot.

Quota-aware: on HTTP 429 / RESOURCE_EXHAUSTED it backs off and retries a few times, then
stops rather than failing the run, persisting what it judged so a partial sweep is committable.
Resumes: pages that already have a real status in the target slot are skipped unless --only /
--force, so re-running continues where a quota cut it off.

Key:   $GEMINI_API_KEY -> ~/.config/maui-parity/gemini_api_key
Model: $GEMINI_MODEL (default gemini-2.5-flash)
Usage: python3 tools/parity/gemini_android_sweep.py [--framework cpp|xaml] [--limit N]
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
PLATFORM = "android"
THEME = "light"  # Android is captured single-theme on the board.
DEFAULT_MODEL = os.environ.get("GEMINI_MODEL", "gemini-2.5-flash")

# Gemini enum -> board status.
ENUM = ["match", "minor", "diff", "cpp_blank"]
TO_BOARD = {"match": "green", "minor": "yellow", "diff": "red", "cpp_blank": "blank"}

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
Assess ANDROID layout/render PARITY between real .NET MAUI and the {fw_label} of MAUI for ONE demo page.
You are given TWO screenshots of the SAME page, captured on the SAME Android emulator, in this order:
  1. MAUI (the real Microsoft .NET MAUI render — GROUND TRUTH, native-default)
  2. {fw_label} (under test)

GROUND TRUTH: MAUI's render IS the source of truth for all page CONTENT — colors, control sizes, internal
spacing, text, corner radius, fonts. Any CONTENT difference in the port vs MAUI (including any color
difference) is a PORT BUG. IGNORE ONLY the harness wrapper (these are NOT port diffs and must NOT drive the
verdict):
  • The status-bar (clock/battery/wifi icons) and the app's TOP ACTION/TITLE BAR. The port's title bar shows
    "MAUI C++ Gallery"; MAUI's harness may show a different title or none. A different/extra/missing app
    title bar is HARNESS CHROME — ignore it entirely, never call it an "extra header" or missing content.
  • The outer page-inset/margin magnitude and the resulting uniform global shift + top/bottom crop (the port
    uses modest padding vs MAUI's larger inset).
  • SCROLL POSITION / VIEWPORT CROP: the two sides may be scrolled to different offsets, so one may show MORE
    or FEWER items of a scrollable list/button-column in the viewport. That is NOT "missing" or "extra"
    controls — only report a control as missing if the page is clearly short/non-scrollable and the control
    is genuinely absent, not merely scrolled out of view.
  • Pure runtime state: a specific date/time value, a toggle's on/off, a spinner caught mid-rotation
    (at most 'minor').

Pick exactly one verdict:
  - match : no port bugs — the port matches MAUI's content (colors/sizes/spacing/text), pixel-perfect or
            effectively identical once the harness wrapper is set aside.
  - minor : small content differences (a slightly-off color/size/spacing, a missing minor detail, a glyph
            style nit) that don't change the page's substance.
  - diff  : a MAJOR difference — a missing/extra/mis-sized control, absent image/shadow/gradient/effect,
            wrong layout, wrong/garbled text, overlap, or the wrong content entirely.
  - cpp_blank : the port's page is blank/empty/crashed while MAUI shows content.

For "note" give ONE precise, terse line naming the SPECIFIC port-relevant difference (or "matches MAUI").
"""

FW_LABEL = {"cpp": "C++23 port", "xaml": "C++23 & XAML port"}


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


def call_gemini(model, api_key, key, framework, timeout=120):
    """Return (board_status, note) or raise QuotaError / returns None on missing images."""
    fw_label = FW_LABEL[framework]
    maui = cp.find_capture(PLATFORM, "maui", key, THEME)
    port = cp.find_capture(PLATFORM, framework, key, THEME)
    if not (maui and port):
        return None
    parts = [
        {"text": "Image: MAUI"}, img_part(maui),
        {"text": f"Image: {fw_label}"}, img_part(port),
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


ENUM_BOARD = {"green", "yellow", "red", "blank"}


def slot_status(page, slot):
    """Existing board status in this page's android review slot, or '' if unset."""
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
            maui = cp.find_capture(PLATFORM, "maui", k, THEME)
            port = cp.find_capture(PLATFORM, args.framework, k, THEME)
            ok = "OK" if (maui and port) else "MISSING"
            print(f"{k}: {ok} maui={maui and os.path.relpath(maui, COMP)} "
                  f"{args.framework}={port and os.path.relpath(port, COMP)} -> "
                  f"platforms.{PLATFORM}.{slot}")
        print(f"DRY_RUN_DONE ({len(todo)} keys, framework={args.framework}, slot={slot})")
        return

    api_key = read_key()
    log(f"android gemini sweep: {len(todo)} to review (of {len(keys)}), framework={args.framework}, "
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
            log(f"[{i}/{len(todo)}] {k}: missing image(s) — skip")
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
