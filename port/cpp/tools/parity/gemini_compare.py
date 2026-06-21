#!/usr/bin/env python3
"""Compare one parity page (real .NET MAUI vs the C++ port) with Google Gemini vision.

This is the *default* image-comparison engine for the iOS parity loop. It judges the
LIGHT pair (MAUI-light vs C++-light) and the DARK pair (MAUI-dark vs C++-dark) for one
page and prints a single JSON verdict that is a drop-in match for the per-page records in
`docs/comparison/parity_status.json` and the `parity_assess_wf.js` workflow schema:

    {"key", "light", "dark", "light_note", "dark_note"}
    light/dark in: match | minor | diff | cpp_blank | cs_blank

Quota handling — the whole reason this is a separate process: when Gemini returns a
quota / rate-limit error (HTTP 429 or status RESOURCE_EXHAUSTED) the script exits with
code 75 (EX_TEMPFAIL) so the caller can FALL BACK to Claude's own vision for that page.
Any other hard failure exits 1; missing input images exit 2.

No third-party deps (urllib + base64 only) so it runs anywhere python3 does.

Key resolution order:  $GEMINI_API_KEY  ->  ~/.config/maui-parity/gemini_api_key
Model:                 $GEMINI_MODEL    (default: gemini-flash-latest)
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

# Exit codes the caller keys off of.
EXIT_OK = 0
EXIT_ERROR = 1        # hard failure (bad response, repeated 5xx, parse error)
EXIT_MISSING = 2      # one or more input images missing
EXIT_QUOTA = 75       # quota / rate limit hit -> caller should fall back to Claude vision

DEFAULT_MODEL = "gemini-flash-latest"
DEFAULT_CMP_ROOT = os.path.normpath(
    os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..", "docs", "comparison")
)
KEY_FILE = os.path.expanduser("~/.config/maui-parity/gemini_api_key")

# The four capture dirs, in (label, subdir) order the model sees them.
PANES = [
    ("MAUI (light)", "csharp_ios_light"),
    ("C++ port (light)", "cpp_ios_light"),
    ("MAUI (dark)", "csharp_ios_dark"),
    ("C++ port (dark)", "cpp_ios_dark"),
]

ENUM = ["match", "minor", "diff", "cpp_blank", "cs_blank"]

# Gemini controlled-generation schema (OpenAPI subset; UPPERCASE types).
RESPONSE_SCHEMA = {
    "type": "OBJECT",
    "properties": {
        "light": {"type": "STRING", "enum": ENUM},
        "dark": {"type": "STRING", "enum": ENUM},
        "light_note": {"type": "STRING"},
        "dark_note": {"type": "STRING"},
        "maui_quirks": {"type": "ARRAY", "items": {"type": "STRING"}},
        "port_diffs": {"type": "ARRAY", "items": {"type": "STRING"}},
    },
    "required": ["light", "dark", "light_note", "dark_note", "maui_quirks", "port_diffs"],
    "propertyOrdering": ["light", "dark", "light_note", "dark_note", "maui_quirks", "port_diffs"],
}

PROMPT = """\
Assess iOS layout/render PARITY between real .NET MAUI and the C++ port of MAUI for one demo page, theme-for-theme.

You are given four iPhone-17-simulator screenshots of the SAME page in this order:
  1. MAUI (light)   2. C++ port (light)   3. MAUI (dark)   4. C++ port (dark)
All four are the SAME capture resolution (1206x2622). Control SIZES, SPACING and how many fit on screen are directly
comparable — a difference in control size/spacing is REAL, never "scale noise."

GROUND TRUTH (ruling): Microsoft's MAUI render IS the source of truth for all page CONTENT — colors, control sizes,
internal spacing, text, corner radius, fonts. The port's job is to match MAUI's content. So any CONTENT difference
(including any color difference) is a PORT BUG to fix, never excused as a "MAUI imperfection."

The ONLY MAUI imperfections the port need not copy are the HARNESS WRAPPER artifacts (ruling: the port uses modest page
padding for UX but deliberately does NOT replicate MAUI's large inset/crop). Sort EVERY difference into one of two buckets:

A) "maui_quirks" — ONLY the harness-wrapper imperfections below. SUBJECT TO DISCUSSION; they must NOT drive the verdict:
     • Whole-screen padding/margins: MAUI insets the entire page inside a harness card, leaving a large gap to the
       screen edges; the C++ port uses much less (or no) outer padding. The DIFFERENCE IN OUTER-INSET MAGNITUDE and the
       resulting uniform global shift of every element is a quirk — the port is NOT expected to match MAUI's big inset.
     • Top/bottom cropping: MAUI's card is shorter than the screen and crops the page top/bottom; the port shows more of
       the page. That extra-visible top/bottom content is a quirk, not "extra controls."
     • Harness chrome: the gray container card, navigation bar, status-bar clock/battery/notch.
   HARD GUARD — the outer-inset quirk is a UNIFORM OUTER MARGIN ONLY. It shifts all content by the SAME offset and may
   clip content at the far edge, but it NEVER changes a control's SIZE, the SPACING between controls, their colors, or
   their relative arrangement. Before blaming a difference on the inset/crop, CHECK the controls: if a control is a
   DIFFERENT SIZE/COLOR, controls are spaced differently INTERNALLY, or one side fits more/fewer copies of a repeated
   element BECAUSE the elements are sized differently — that is a PORT DIFF (bucket B), NOT a quirk, even if clipping is
   also present. Only call clipping/shift a quirk when the underlying controls are the SAME size, color and spacing.
   One short line per quirk (e.g. "MAUI insets whole page ~60px; port uses ~16px"). Empty list if none.

B) "port_diffs" — every CONTENT difference vs MAUI once the harness wrapper above is set aside: a control missing/extra/
   mis-ordered, wrong INTERNAL size or spacing, wrong corner radius, wrong/garbled text, ANY wrong color (MAUI's color
   is correct by definition), overlap, or a blank side. These ARE fix candidates and DO drive the verdict. One line each.
   Discount only transient animation phase (a spinner caught mid-rotation is at most 'minor'). Empty list if none.

Then judge the LIGHT pair (img 1 vs 2) and the DARK pair (img 3 vs 4) SEPARATELY, based on "port_diffs" ONLY (ignore
maui_quirks for the verdict). Pick exactly one per theme:
  - match     : no port_diffs — only sub-pixel/font-hinting differences once MAUI quirks are set aside.
  - minor     : small port_diffs (a few px of internal spacing/size, font weight, a shade the port got slightly off).
  - diff      : notable port_diffs (missing/extra/mis-sized/mis-coloured control, wrong internal layout, overlap, wrong text).
  - cpp_blank : the C++ side is blank/empty while MAUI shows content.
  - cs_blank  : MAUI blank while C++ shows content.

For light_note / dark_note give ONE precise line summarising the PORT-relevant difference for that theme (or
"identical" / "only MAUI quirks"). Be specific and terse.
"""


def log(msg: str) -> None:
    print(msg, file=sys.stderr, flush=True)


def read_key() -> str:
    key = os.environ.get("GEMINI_API_KEY", "").strip()
    if key:
        return key
    try:
        with open(KEY_FILE, "r", encoding="utf-8") as fh:
            key = fh.read().strip()
    except OSError:
        key = ""
    if not key:
        log(f"ERROR: no API key. Set $GEMINI_API_KEY or write it to {KEY_FILE}")
        sys.exit(EXIT_ERROR)
    return key


def load_image_parts(key: str, cmp_root: str):
    """Return interleaved [text-label, inline-image] parts, or exit(EXIT_MISSING)."""
    parts = []
    missing = []
    for label, subdir in PANES:
        path = os.path.join(cmp_root, subdir, f"{key}.png")
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


def call_gemini(model: str, api_key: str, parts: list, timeout: int, retries: int) -> dict:
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
            headers={"Content-Type": "application/json", "X-goog-api-key": api_key},
        )
        try:
            with urllib.request.urlopen(req, timeout=timeout) as resp:
                return json.loads(resp.read().decode("utf-8"))
        except urllib.error.HTTPError as e:
            detail = e.read().decode("utf-8", "replace")
            if e.code == 429 or "RESOURCE_EXHAUSTED" in detail:
                log(f"QUOTA: Gemini returned {e.code} RESOURCE_EXHAUSTED -> caller should fall back.\n{detail[:400]}")
                sys.exit(EXIT_QUOTA)
            last_err = f"HTTP {e.code}: {detail[:400]}"
            # 5xx: transient — retry with backoff. 4xx (other than 429): fatal.
            if 500 <= e.code < 600 and attempt < retries:
                wait = 2 ** attempt
                log(f"{last_err}\n  transient; retrying in {wait}s ({attempt + 1}/{retries})")
                time.sleep(wait)
                continue
            break
        except (urllib.error.URLError, TimeoutError) as e:
            last_err = f"network error: {e}"
            if attempt < retries:
                wait = 2 ** attempt
                log(f"{last_err}\n  retrying in {wait}s ({attempt + 1}/{retries})")
                time.sleep(wait)
                continue
            break
    log(f"ERROR: Gemini call failed: {last_err}")
    sys.exit(EXIT_ERROR)


def extract_verdict(resp: dict) -> dict:
    try:
        text = resp["candidates"][0]["content"]["parts"][0]["text"]
    except (KeyError, IndexError, TypeError):
        # Surface block reasons (safety filters etc.) clearly.
        fb = resp.get("promptFeedback") or resp.get("candidates", [{}])[0].get("finishReason")
        log(f"ERROR: no text in Gemini response (finish/feedback: {fb}); raw: {json.dumps(resp)[:400]}")
        sys.exit(EXIT_ERROR)
    try:
        verdict = json.loads(text)
    except json.JSONDecodeError:
        log(f"ERROR: Gemini did not return valid JSON: {text[:400]}")
        sys.exit(EXIT_ERROR)
    for field in ("light", "dark"):
        if verdict.get(field) not in ENUM:
            log(f"ERROR: '{field}'={verdict.get(field)!r} not in {ENUM}")
            sys.exit(EXIT_ERROR)
    return verdict


def main() -> None:
    ap = argparse.ArgumentParser(description="Compare one parity page with Gemini vision.")
    ap.add_argument("key", help="page key, e.g. 'button' (matches <root>/<dir>/<key>.png)")
    ap.add_argument("--root", default=DEFAULT_CMP_ROOT, help="comparison root (default: docs/comparison)")
    ap.add_argument("--model", default=os.environ.get("GEMINI_MODEL", DEFAULT_MODEL))
    ap.add_argument("--timeout", type=int, default=120)
    ap.add_argument("--retries", type=int, default=2, help="retries for 5xx/network (NOT quota)")
    args = ap.parse_args()

    api_key = read_key()
    parts = load_image_parts(args.key, args.root)
    resp = call_gemini(args.model, api_key, parts, args.timeout, args.retries)
    verdict = extract_verdict(resp)
    # Stable field order. light/dark/notes stay parity_status.json-compatible; the two buckets
    # (maui_quirks / port_diffs) are the review-time separation of MAUI imperfections from port bugs.
    out = {
        "key": args.key,
        "light": verdict["light"],
        "dark": verdict["dark"],
        "light_note": verdict.get("light_note", ""),
        "dark_note": verdict.get("dark_note", ""),
        "maui_quirks": verdict.get("maui_quirks", []),
        "port_diffs": verdict.get("port_diffs", []),
    }
    print(json.dumps(out))


if __name__ == "__main__":
    main()
