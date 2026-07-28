#!/usr/bin/env python3
"""(Re)build docs/comparison/comparison.json — the single source of truth for the parity board.

comparison.json is an array of one object per gallery page:

    {
      "name": "<key>", "title": "<Title>", "description": "<what the screen shows>",
      "platforms": {
        "ios":         {"screenshots": {"maui":{"light","dark"}, "cpp":{...}, "xaml":{...}},
                        "sonnet": {"status","review"}, "gemini": {"status","review"},
                        "sonnet_xaml": {"status","review"}, "gemini_xaml": {"status","review"}},
        "maccatalyst": {"screenshots": {"maui","cpp","xaml","appkit_cpp","appkit_xaml"}, ...},
        "android":     {"screenshots": {"maui","cpp","xaml"}, ...}
      }
    }

`sonnet`/`gemini` are the cpp-vs-maui verdict (the columns tools/gen_readme.py renders). The OPTIONAL
`sonnet_xaml`/`gemini_xaml` slots are the SEPARATE xaml-vs-maui verdict — the user wants cpp and xaml
reviewed independently. They are added only when a framework=xaml sweep has produced them (so pages
without an xaml review are byte-identical to before and the current README is unchanged); a future
template can surface them. tools/parity/comparison_paths.review_slot(model, framework) encodes the
cpp->bare / xaml->`_xaml` slot-naming.

Screenshot paths point at `captures/<platform>/<framework>/<key>_<theme>.{png,gif}` when the file
exists, else null (the README generator renders null as `_placeholder.png`). This script only
REFRESHES the screenshot paths (by scanning captures/); it PRESERVES the description and the
Sonnet/Gemini reviews already in comparison.json, and adds any newly-listed pages with empty
reviews. Reviews are authored/updated directly in comparison.json (or by a parity sweep that writes
into it). Run after any capture change, then regenerate the README with tools/gen_readme.py.

Usage: python3 tools/build_comparison_json.py
"""
import json
import os

HERE = os.path.dirname(os.path.abspath(__file__))
COMP = os.path.normpath(os.path.join(HERE, ".."))
JSON = os.path.join(COMP, "comparison.json")
KEYS_FILE = os.path.normpath(os.path.join(COMP, "..", "..", "tools", "parity", "page_keys.txt"))

THEMES = ("light", "dark")
PLATFORM_FW = {
    "ios": ("maui", "cpp", "xaml"),
    "maccatalyst": ("maui", "cpp", "xaml", "appkit_cpp", "appkit_xaml"),
    "android": ("maui", "cpp", "xaml"),
    # Windows: maui is the real WinUI 3 MauiReference and cpp is the port's own WinUI 3 gallery, both
    # built ON the guest (native arm64). xaml is listed so its ABSENCE renders as a visible placeholder
    # rather than being silently omitted -- that column cannot build at all, because its .xaml.cpp TUs
    # use #embed and MSVC does not implement it (docs/WINDOWS_TOOLCHAIN.md section 6d).
    "windows": ("maui", "cpp", "xaml"),
}
EMPTY_REVIEW = {"status": None, "review": ""}


def keys():
    with open(KEYS_FILE, encoding="utf-8") as f:
        return [l.strip() for l in f if l.strip() and not l.startswith("#")]


def title(k):
    return k.replace("_", " ").title()


def shot_path(platform, fw, k, th):
    for ext in ("gif", "png"):
        rel = f"captures/{platform}/{fw}/{k}_{th}.{ext}"
        if os.path.isfile(os.path.join(COMP, rel)):
            return rel
    return None


def shots(platform, k):
    return {fw: {th: shot_path(platform, fw, k, th) for th in THEMES} for fw in PLATFORM_FW[platform]}


def main():
    prev = {}
    if os.path.isfile(JSON):
        prev = {r["name"]: r for r in json.load(open(JSON, encoding="utf-8"))}

    data = []
    for k in keys():
        old = prev.get(k, {})
        old_plats = old.get("platforms", {})
        page = {
            "name": k,
            "title": old.get("title") or title(k),
            "description": old.get("description") or title(k),
            "platforms": {},
        }
        for plat in PLATFORM_FW:
            op = old_plats.get(plat, {})
            entry = {
                "screenshots": shots(plat, k),
                "sonnet": op.get("sonnet") or dict(EMPTY_REVIEW),
                "gemini": op.get("gemini") or dict(EMPTY_REVIEW),
            }
            # Preserve the OPTIONAL per-framework xaml review slots when present. `sonnet`/`gemini`
            # above are the cpp-vs-maui verdict (what gen_readme.py renders); `sonnet_xaml`/`gemini_xaml`
            # are the separate xaml-vs-maui verdict a framework=xaml sweep writes. They are only added
            # when a sweep has produced them, so pages without an xaml review stay byte-identical and the
            # current README output is unchanged. See tools/parity/comparison_paths.review_slot().
            # …and the pixel-score slots, which are NOT regenerable from here — only
            # tools/parity/pixel_score.py computes them, from the captures. Dropping them made this script
            # silently destructive: the documented publish chain is import -> build_comparison_json ->
            # `pixel_score --only <page>` -> gen_readme, so a SCOPED publish rebuilt the json without the
            # scores and then only put ONE page's back — wiping the other 171 pages' verdicts and shrinking
            # the README by ~20KB with no error. Anything this script cannot recompute, it must carry over.
            for slot in ("sonnet_xaml", "gemini_xaml", "pixel", "pixel_xaml"):
                if op.get(slot):
                    entry[slot] = op[slot]
            page["platforms"][plat] = entry
        data.append(page)

    json.dump(data, open(JSON, "w", encoding="utf-8"), ensure_ascii=False, indent=1)
    real = sum(1 for p in data for plat in PLATFORM_FW for fw in PLATFORM_FW[plat]
               for th in THEMES if p["platforms"][plat]["screenshots"][fw][th])
    print(f"wrote comparison.json: {len(data)} pages, {real} real screenshot cells")


if __name__ == "__main__":
    main()
