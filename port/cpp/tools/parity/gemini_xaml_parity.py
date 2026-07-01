#!/usr/bin/env python3
"""Gemini second opinion for the C++&XAML parity column (the Sonnet xaml-twin-parity sibling).

For one gallery page, compares the THREE iOS LIGHT captures (canonical layout):
  A = MAUI reference   docs/comparison/captures/ios/maui/<key>_light.png
  B = C++ builder      docs/comparison/captures/ios/cpp/<key>_light.png
  C = C++ & XAML       docs/comparison/captures/ios/xaml/<key>_light.png
and judges the C++&XAML render (C) on two axes — vs the builder (B, a markup/loader-faithfulness signal,
same renderer) and vs MAUI (A, the 1-to-1 goal). Prints a JSON verdict {page, xaml_vs_builder,
xaml_vs_maui, notes}. With --write-comparison it also folds the xaml_vs_maui verdict into
comparison.json's platforms.ios `gemini_xaml` slot (comparison_paths.review_slot). --dry-run prints the
3 source paths with no API call. Quota/availability -> exit 3. Key: $GEMINI_API_KEY or
~/.config/maui-parity/gemini_api_key.

Usage:  python3 gemini_xaml_parity.py <key> [--framework unused] [--write-comparison] [--dry-run]
"""
import argparse
import base64
import json
import os
import sys
import time
import urllib.error
import urllib.request

import comparison_paths as cp

KEY_FILE = os.path.expanduser("~/.config/maui-parity/gemini_api_key")
# Root is docs/comparison (paths resolve via the captures/ios/<fw>/<key>_light.png layout).
DEFAULT_CMP_ROOT = cp.COMP
DEFAULT_MODEL = "gemini-2.5-flash"
PLATFORM, THEME = "ios", "light"
EXIT_OK, EXIT_ERROR, EXIT_MISSING, EXIT_QUOTA = 0, 1, 2, 3

# (label, framework) in the order the model sees them. LIGHT theme only (the Sonnet workflow's axis).
PANES = [
    ("A = MAUI reference", "maui"),
    ("B = C++ port, hand-written builder", "cpp"),
    ("C = C++ port, rendered from XAML markup (the new column)", "xaml"),
]

ENUM = ["match", "minor", "major"]
RESPONSE_SCHEMA = {
    "type": "OBJECT",
    "properties": {
        "xaml_vs_builder": {"type": "STRING", "enum": ENUM},
        "xaml_vs_maui": {"type": "STRING", "enum": ENUM},
        "notes": {"type": "STRING"},
    },
    "required": ["xaml_vs_builder", "xaml_vs_maui", "notes"],
    "propertyOrdering": ["xaml_vs_builder", "xaml_vs_maui", "notes"],
}

PROMPT = """\
You are a meticulous visual-parity judge. Compare three iOS screenshots of the SAME gallery page (LIGHT theme),
given above in order A, B, C:
  A = MAUI reference        B = C++ port, hand-written builder        C = C++ port, rendered from XAML markup
Judge the C++&XAML render (C) on two axes:
  - xaml_vs_builder: does C match B? Same renderer, so any difference means the XAML markup/loader did not reproduce
    the builder page (missing/approximated control, wrong layout, dropped text). match = visually equivalent;
    minor = small spacing/size/color nits; major = missing content or clearly different structure.
  - xaml_vs_maui: does C match the MAUI reference A (the 1-to-1 goal)? IGNORE the known harness outer-inset / uniform
    global shift / top-bottom crop and sub-pixel system-font metric differences; judge CONTENT — controls present,
    layout, colors, text. Same match/minor/major scale.
Be concrete and terse in notes (name the actual difference, e.g. "C truncates cell labels to one line; A/B wrap to two").
Return ONLY the structured verdict.
"""


def log(msg):
    print(msg, file=sys.stderr, flush=True)


def read_key():
    key = os.environ.get("GEMINI_API_KEY", "").strip()
    if not key:
        try:
            with open(KEY_FILE, "r", encoding="utf-8") as fh:
                key = fh.read().strip()
        except OSError:
            key = ""
    if not key:
        log(f"ERROR: no API key. Set $GEMINI_API_KEY or write it to {KEY_FILE}")
        sys.exit(EXIT_ERROR)
    return key


def pane_path(cmp_root, framework, key):
    """Absolute LIGHT capture path under `cmp_root` using the canonical relative layout."""
    return os.path.join(cmp_root, cp.rel_capture(PLATFORM, framework, key, THEME, "png"))


def load_image_parts(key, cmp_root):
    parts, missing = [], []
    for label, framework in PANES:
        path = pane_path(cmp_root, framework, key)
        if not os.path.isfile(path):
            missing.append(path)
            continue
        with open(path, "rb") as fh:
            data = base64.b64encode(fh.read()).decode("ascii")
        parts.append({"text": f"Image: {label}"})
        parts.append({"inline_data": {"mime_type": "image/png", "data": data}})
    if missing:
        log("ERROR: missing image(s):\n  " + "\n  ".join(missing))
        sys.exit(EXIT_MISSING)
    return parts


def call_gemini(model, api_key, parts, timeout, retries):
    url = f"https://generativelanguage.googleapis.com/v1beta/models/{model}:generateContent"
    body = json.dumps({
        "contents": [{"parts": parts + [{"text": PROMPT}]}],
        "generationConfig": {
            "responseMimeType": "application/json",
            "responseSchema": RESPONSE_SCHEMA,
            "temperature": 0,
        },
    }).encode("utf-8")
    last_err = None
    for attempt in range(retries + 1):
        req = urllib.request.Request(
            url, data=body, method="POST",
            headers={"Content-Type": "application/json", "X-goog-api-key": api_key})
        try:
            with urllib.request.urlopen(req, timeout=timeout) as resp:
                return json.loads(resp.read().decode("utf-8"))
        except urllib.error.HTTPError as e:
            detail = e.read().decode("utf-8", "replace")
            if e.code == 429 or "RESOURCE_EXHAUSTED" in detail:
                log(f"QUOTA: Gemini {e.code} RESOURCE_EXHAUSTED.\n{detail[:300]}")
                sys.exit(EXIT_QUOTA)
            if e.code == 404:
                log(f"MODEL UNAVAILABLE: Gemini 404.\n{detail[:200]}")
                sys.exit(EXIT_QUOTA)
            last_err = f"HTTP {e.code}: {detail[:300]}"
            if 500 <= e.code < 600 and attempt < retries:
                time.sleep(2 ** attempt)
                continue
            break
        except (urllib.error.URLError, TimeoutError) as e:
            last_err = f"network error: {e}"
            if attempt < retries:
                time.sleep(2 ** attempt)
                continue
            break
    log(f"ERROR: Gemini call failed: {last_err}")
    sys.exit(EXIT_ERROR)


def extract_verdict(resp):
    try:
        text = resp["candidates"][0]["content"]["parts"][0]["text"]
    except (KeyError, IndexError, TypeError):
        fb = resp.get("promptFeedback") or resp.get("candidates", [{}])[0].get("finishReason")
        log(f"ERROR: no text in Gemini response (finish/feedback: {fb})")
        sys.exit(EXIT_ERROR)
    try:
        verdict = json.loads(text)
    except json.JSONDecodeError:
        log(f"ERROR: Gemini did not return valid JSON: {text[:300]}")
        sys.exit(EXIT_ERROR)
    for field in ("xaml_vs_builder", "xaml_vs_maui"):
        if verdict.get(field) not in ENUM:
            log(f"ERROR: '{field}'={verdict.get(field)!r} not in {ENUM}")
            sys.exit(EXIT_ERROR)
    return verdict


def main():
    ap = argparse.ArgumentParser(description="Gemini second opinion for one C++&XAML iOS parity page.")
    ap.add_argument("key", help="page key, e.g. 'items' (matches captures/ios/<fw>/<key>_light.png)")
    ap.add_argument("--root", default=DEFAULT_CMP_ROOT)
    ap.add_argument("--model", default=os.environ.get("GEMINI_MODEL", DEFAULT_MODEL))
    ap.add_argument("--timeout", type=int, default=120)
    ap.add_argument("--retries", type=int, default=2)
    ap.add_argument("--write-comparison", action="store_true",
                    help="fold the xaml_vs_maui verdict into comparison.json's platforms.ios gemini_xaml slot")
    ap.add_argument("--dry-run", action="store_true",
                    help="print the 3 source capture paths (+ target slot) WITHOUT calling Gemini")
    args = ap.parse_args()

    if args.dry_run:
        for label, framework in PANES:
            p = pane_path(args.root, framework, args.key)
            print(f"{'OK' if os.path.isfile(p) else 'MISSING':7} {framework:5} {os.path.relpath(p, args.root)}")
        if args.write_comparison:
            print(f"-> platforms.{PLATFORM}.{cp.review_slot('gemini', 'xaml')}")
        print("DRY_RUN_DONE")
        sys.exit(EXIT_OK)

    api_key = read_key()
    parts = load_image_parts(args.key, args.root)
    resp = call_gemini(args.model, api_key, parts, args.timeout, args.retries)
    verdict = extract_verdict(resp)
    verdict["page"] = args.key
    print(json.dumps(verdict, indent=2))

    if args.write_comparison:
        # This tool's 1-to-1 goal axis is xaml_vs_maui -> the ios gemini_xaml slot. Keep the builder
        # cross-check (xaml_vs_builder) in the review text so it isn't lost.
        status = cp.normalize_status(verdict["xaml_vs_maui"])
        review = (verdict.get("notes", "").strip()
                  + f" (xaml_vs_builder={verdict.get('xaml_vs_builder', '?')})").strip()
        data = cp.load_comparison()
        if cp.write_review(data, args.key, PLATFORM, "gemini", "xaml", status, review):
            cp.save_comparison(data)
            log(f"wrote platforms.{PLATFORM}.{cp.review_slot('gemini', 'xaml')} = {status} for {args.key}")
        else:
            log(f"WARNING: key {args.key!r} not in comparison.json — verdict printed but not written")
    sys.exit(EXIT_OK)


if __name__ == "__main__":
    main()
