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


# Pages the Windows MAUI reference (maui-compare, 154 page keys) does NOT implement: capturing them
# yields maui-compare's INDEX page, not a real render, so there is no valid MAUI reference and any
# port-vs-"maui" verdict is meaningless. They are marked port-only (null MAUI shots, no verdict) so the
# parity counts stay honest. The 12 ios_* are iOS-specific demos; the rest are port demos absent from
# maui-compare. (Detected by pixel-matching each maui capture against the known index page.)
PORT_ONLY = {
    "effects", "refresh_view",
    "custom_layout", "hybrid_web_view", "relative_layout", "selection_mode", "staggered_layout",
    "tabbed_flyout", "title_bar",
    "ios_blur_effect", "ios_date_picker", "ios_entry", "ios_first_responder", "ios_pan_gesture",
    "ios_picker", "ios_safe_area", "ios_scroll_view", "ios_search_bar", "ios_slider_update_on_tap",
    "ios_swipe_transition", "ios_time_picker",
}


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
        if name in PORT_ONLY or v is None:  # no valid MAUI reference — not comparable
            screenshots["maui"] = {"light": None, "dark": None}  # drop the bogus index-page ref
            sc = {"status": None, "review": "Port-only demo — maui-compare has no such page (no MAUI reference)."}
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
