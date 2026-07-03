#!/usr/bin/env python3
"""Fold the Windows Sonnet-vision judge results into comparison.json.

Adds `platforms.windows` to every page entry: the maui/cpp/xaml screenshot paths (png, or gif for
animated pages) plus the two Sonnet parity verdicts (C++ vs MAUI and C++&XAML vs MAUI). Pages with no
MAUI reference (port-only demos) get null maui shots + an 'n/a' verdict. Idempotent: re-running
overwrites the windows entry from the current judge file.

Usage: python tools/build_windows_comparison.py <judge_result.json>
"""
import json
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
COMP = os.path.normpath(os.path.join(HERE, ".."))
JSON = os.path.join(COMP, "comparison.json")
CAPS = os.path.join(COMP, "captures", "windows")

ANIMATED = {
    "activity_indicator", "animation", "chrome", "empty_view_load_simulate", "gestures",
    "pan_gesture_events", "pointer_gesture", "swipe_gesture", "swipe_item_position", "swipe_refresh",
}


def shot(col: str, name: str, theme: str, ext: str):
    """Relative capture path if the file exists, else None (README shows a placeholder)."""
    rel = f"captures/windows/{col}/{name}_{theme}.{ext}"
    return rel if os.path.exists(os.path.join(COMP, rel)) else None


def main() -> int:
    judge = {d["name"]: d for d in json.load(open(sys.argv[1], encoding="utf-8"))}
    pages = json.load(open(JSON, encoding="utf-8"))
    n_win = 0
    for p in pages:
        name = p["name"]
        ext = "gif" if name in ANIMATED else "png"
        screenshots = {
            col: {"light": shot(col, name, "light", ext), "dark": shot(col, name, "dark", ext)}
            for col in ("maui", "cpp", "xaml")
        }
        # A page renders on Windows if the cpp column captured it.
        if not (screenshots["cpp"]["light"] or screenshots["cpp"]["dark"]):
            continue
        v = judge.get(name)
        if v is None:  # port-only demo (no MAUI page) — captured but not comparable
            sc = {"status": None, "review": "Port-only demo — no MAUI reference page."}
            sx = dict(sc)
        else:
            sc = v["cpp"]
            sx = v["xaml"]
        p.setdefault("platforms", {})["windows"] = {
            "screenshots": screenshots,
            "sonnet_cpp": sc,
            "sonnet_xaml": sx,
        }
        n_win += 1
    json.dump(pages, open(JSON, "w", encoding="utf-8"), indent=1, ensure_ascii=False)
    print(f"wrote {JSON}: added platforms.windows to {n_win}/{len(pages)} pages")
    return 0


if __name__ == "__main__":
    sys.exit(main())
