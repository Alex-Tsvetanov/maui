#!/usr/bin/env python3
"""Canonical path helpers for the restructured `docs/comparison/` parity layout.

Single source of truth for WHERE a capture lives and HOW a review is stored, so every capture
script and Gemini sweep agrees on the layout that `docs/comparison/tools/build_comparison_json.py`
and `gen_readme.py` expect. The restructure (commit 46d0dc2ea7) collapsed the old per-framework /
per-theme capture dirs and the six per-platform review JSONs into:

  * screenshots -> captures/<platform>/<framework>/<key>_<theme>.<ext>
        platform  in {ios, maccatalyst, android}
        framework in {maui, cpp, xaml} (+ appkit_cpp, appkit_xaml for maccatalyst only)
        theme     in {light, dark}      (android is single-theme: only _light)
        ext       in {png, gif}         (gif for the ANIMATED pages)
  * reviews     -> ONE comparison.json, per page, under platforms.<platform>.<review-slot>.

Review slots (schema, backward-compatible with the user-owned gen_readme.py template):
  * `sonnet` / `gemini`            = the cpp-vs-maui verdict (the columns gen_readme.py renders today).
  * `sonnet_xaml` / `gemini_xaml`  = the OPTIONAL xaml-vs-maui verdict (new; the user wants cpp and
                                     xaml reviewed SEPARATELY). gen_readme.py ignores these extra keys,
                                     so adding them changes nothing in the current README; a future
                                     template can surface them. build_comparison_json.py preserves them.

`review_slot(model, framework)` maps (model in {sonnet, gemini}, framework in {cpp, xaml}) to the slot
name — the ONLY place the cpp->bare / xaml->_xaml convention is encoded.

This module has NO device or network dependency; importing it and running it as a script executes a
dry-run self-check (see `selfcheck()` / `main()`), so the whole layout is verifiable with no simulator.
"""
from __future__ import annotations

import json
import os

HERE = os.path.dirname(os.path.abspath(__file__))
# port/cpp/docs/comparison
COMP = os.path.normpath(os.path.join(HERE, "..", "..", "docs", "comparison"))
CAPTURES = os.path.join(COMP, "captures")
COMPARISON_JSON = os.path.join(COMP, "comparison.json")
PAGE_KEYS = os.path.join(HERE, "page_keys.txt")

PLATFORMS = ("ios", "maccatalyst", "android", "windows")
THEMES = ("light", "dark")
# The frameworks each platform has a capture column for (mirrors build_comparison_json.PLATFORM_FW).
PLATFORM_FW = {
    "ios": ("maui", "cpp", "xaml"),
    "maccatalyst": ("maui", "cpp", "xaml", "appkit_cpp", "appkit_xaml"),
    "android": ("maui", "cpp", "xaml"),
    "windows": ("maui", "cpp", "xaml"),
}
# Android is captured single-theme; its dark cells are always placeholders.
PLATFORM_THEMES = {
    "ios": ("light", "dark"),
    "maccatalyst": ("light", "dark"),
    "android": ("light",),
    "windows": ("light", "dark"),
}
MODELS = ("sonnet", "gemini")


def rel_capture(platform: str, framework: str, key: str, theme: str, ext: str = "png") -> str:
    """Return the capture path RELATIVE to docs/comparison (the form stored in comparison.json)."""
    if platform not in PLATFORM_FW:
        raise ValueError(f"unknown platform {platform!r} (expected one of {PLATFORMS})")
    if framework not in PLATFORM_FW[platform]:
        raise ValueError(f"framework {framework!r} not valid for platform {platform!r} "
                         f"(expected one of {PLATFORM_FW[platform]})")
    if theme not in THEMES:
        raise ValueError(f"unknown theme {theme!r} (expected one of {THEMES})")
    if ext not in ("png", "gif"):
        raise ValueError(f"unknown ext {ext!r} (expected png or gif)")
    return f"captures/{platform}/{framework}/{key}_{theme}.{ext}"


def capture_path(platform: str, framework: str, key: str, theme: str, ext: str = "png") -> str:
    """Absolute on-disk path for a capture (docs/comparison + rel_capture)."""
    return os.path.join(COMP, rel_capture(platform, framework, key, theme, ext))


def find_capture(platform: str, framework: str, key: str, theme: str) -> str | None:
    """First existing capture for (platform, framework, key, theme), gif preferred over png; else None."""
    for ext in ("gif", "png"):
        p = capture_path(platform, framework, key, theme, ext)
        if os.path.isfile(p):
            return p
    return None


def review_slot(model: str, framework: str) -> str:
    """Map (model, framework) to the comparison.json review key.

    cpp is the canonical column the user-owned gen_readme.py already renders, so it keeps the bare slot
    name (`sonnet` / `gemini`); xaml gets a `_xaml`-suffixed sibling slot so cpp and xaml are reviewed
    and stored SEPARATELY without touching the template.
    """
    if model not in MODELS:
        raise ValueError(f"unknown model {model!r} (expected one of {MODELS})")
    if framework == "cpp":
        return model
    if framework == "xaml":
        return f"{model}_xaml"
    raise ValueError(f"framework {framework!r} is not reviewable (expected cpp or xaml)")


def load_keys() -> list[str]:
    with open(PAGE_KEYS, encoding="utf-8") as fh:
        return [ln.strip() for ln in fh if ln.strip() and not ln.startswith("#")]


# ---------------------------------------------------------------------------
# Review normalization + writing verdicts INTO comparison.json.
# ---------------------------------------------------------------------------
BOARD = ("green", "yellow", "red", "blank")

# iOS verdict categories -> the board's 4 statuses. The iOS comparator judges a PAIR (maui vs the
# framework), so match/pixel_perfect -> green; the *_minor/*_major categories collapse by severity
# regardless of which side "wins" (MAUI is ground truth, so a maui_* asymmetry is still a real diff);
# any *_blank -> blank. android/macos sweeps already emit board statuses directly.
IOS_TO_BOARD = {
    "match": "green", "pixel_perfect": "green",
    "minor": "yellow", "cpp_minor": "yellow", "maui_minor": "yellow",
    "diff": "red", "major": "red", "cpp_major": "red", "maui_major": "red",
    "cpp_blank": "blank", "maui_blank": "blank", "cs_blank": "blank",
}


def normalize_ios_status(cat: str) -> str:
    """Map an iOS comparator category to a board status (green/yellow/red/blank)."""
    return IOS_TO_BOARD.get((cat or "").strip().lower(), "yellow")


def normalize_status(status: str) -> str:
    """Idempotently coerce ANY known category/status to a board status.

    Accepts either an already-board status (green/yellow/red/blank) or an iOS category
    (match/cpp_minor/maui_major/...), so a sweep can pass whatever its model emitted.
    """
    s = (status or "").strip().lower()
    if s in BOARD:
        return s
    return normalize_ios_status(s)


def load_comparison() -> list[dict]:
    with open(COMPARISON_JSON, encoding="utf-8") as fh:
        return json.load(fh)


def save_comparison(data: list[dict]) -> None:
    # Match build_comparison_json.py's format (indent=1, unicode kept) so diffs stay minimal.
    with open(COMPARISON_JSON, "w", encoding="utf-8") as fh:
        json.dump(data, fh, ensure_ascii=False, indent=1)


def write_review(data: list[dict], key: str, platform: str, model: str, framework: str,
                 status: str, review: str) -> bool:
    """Set the (platform, model, framework) review slot for `key` in an in-memory comparison list.

    `status` may be a board status or an iOS category — it is normalized. Returns True if the page
    existed and was updated, False if `key` is absent (caller decides whether that's an error).
    Does NOT persist; call save_comparison(data) once after a batch.
    """
    slot = review_slot(model, framework)
    board = normalize_status(status)
    for page in data:
        if page.get("name") != key:
            continue
        plat = page.setdefault("platforms", {}).setdefault(platform, {})
        plat[slot] = {"status": board, "review": review or ""}
        return True
    return False


# ---------------------------------------------------------------------------
# Dry-run self-check — verifiable with NO device (no simulator/emulator boot).
# ---------------------------------------------------------------------------
def selfcheck() -> list[str]:
    """Assert the path + slot logic matches the canonical layout for sample inputs.

    Returns the list of "input -> resolved" lines it verified (for --dry-run printing). Raises
    AssertionError on any mismatch.
    """
    lines: list[str] = []

    def check_path(platform, framework, key, theme, ext, expected):
        got = rel_capture(platform, framework, key, theme, ext)
        assert got == expected, f"rel_capture({platform},{framework},{key},{theme},{ext}) = {got!r} != {expected!r}"
        lines.append(f"capture  {platform:12} {framework:11} {key:16} {theme:5} .{ext} -> {got}")

    # iOS: maui/cpp/xaml, both themes, png + one gif (animated page).
    check_path("ios", "maui", "absolute_layout", "light", "png", "captures/ios/maui/absolute_layout_light.png")
    check_path("ios", "cpp", "absolute_layout", "dark", "png", "captures/ios/cpp/absolute_layout_dark.png")
    check_path("ios", "xaml", "button", "light", "png", "captures/ios/xaml/button_light.png")
    check_path("ios", "cpp", "animation", "light", "gif", "captures/ios/cpp/animation_light.gif")
    # maccatalyst: the two appkit-only columns + a strict-parity column.
    check_path("maccatalyst", "maui", "border", "dark", "png", "captures/maccatalyst/maui/border_dark.png")
    check_path("maccatalyst", "appkit_cpp", "border", "light", "png", "captures/maccatalyst/appkit_cpp/border_light.png")
    check_path("maccatalyst", "appkit_xaml", "border", "dark", "png", "captures/maccatalyst/appkit_xaml/border_dark.png")
    # android: single-theme -> only _light.
    check_path("android", "cpp", "grid", "light", "png", "captures/android/cpp/grid_light.png")
    check_path("android", "xaml", "grid", "light", "png", "captures/android/xaml/grid_light.png")

    def check_slot(model, framework, expected):
        got = review_slot(model, framework)
        assert got == expected, f"review_slot({model},{framework}) = {got!r} != {expected!r}"
        lines.append(f"review   {model:8} {framework:5} -> platforms.<platform>.{got}")

    check_slot("sonnet", "cpp", "sonnet")
    check_slot("gemini", "cpp", "gemini")
    check_slot("sonnet", "xaml", "sonnet_xaml")
    check_slot("gemini", "xaml", "gemini_xaml")

    def check_norm(inp, expected):
        got = normalize_status(inp)
        assert got == expected, f"normalize_status({inp!r}) = {got!r} != {expected!r}"
        lines.append(f"normalize {inp:14} -> {got}")

    check_norm("match", "green")
    check_norm("pixel_perfect", "green")
    check_norm("cpp_minor", "yellow")
    check_norm("maui_minor", "yellow")
    check_norm("cpp_major", "red")
    check_norm("maui_major", "red")
    check_norm("cpp_blank", "blank")
    check_norm("green", "green")   # already-board is idempotent
    check_norm("blank", "blank")

    # In-memory write_review round-trip: cpp -> sonnet slot, xaml -> sonnet_xaml slot, both preserved.
    sample = [{"name": "button", "platforms": {"ios": {"screenshots": {}}}}]
    assert write_review(sample, "button", "ios", "sonnet", "cpp", "match", "looks good")
    assert write_review(sample, "button", "ios", "sonnet", "xaml", "cpp_minor", "spacing nit")
    plat = sample[0]["platforms"]["ios"]
    assert plat["sonnet"] == {"status": "green", "review": "looks good"}, plat.get("sonnet")
    assert plat["sonnet_xaml"] == {"status": "yellow", "review": "spacing nit"}, plat.get("sonnet_xaml")
    assert not write_review(sample, "no_such_key", "ios", "gemini", "cpp", "match", ""), "missing key must return False"
    lines.append("write_review round-trip: cpp->sonnet green, xaml->sonnet_xaml yellow, both kept; missing-key -> False")

    # Negative cases: invalid framework/platform/theme must raise.
    for bad in (
        lambda: rel_capture("ios", "appkit_cpp", "x", "light"),   # appkit only on maccatalyst
        lambda: rel_capture("nope", "cpp", "x", "light"),          # bad platform
        lambda: rel_capture("ios", "cpp", "x", "sideways"),        # bad theme
        lambda: review_slot("sonnet", "maui"),                      # maui isn't reviewable
        lambda: review_slot("claude", "cpp"),                      # bad model
    ):
        try:
            bad()
        except ValueError:
            pass
        else:  # pragma: no cover
            raise AssertionError("expected ValueError for an invalid input but none was raised")
    lines.append("negative: invalid platform/framework/theme/model all rejected (ValueError)")
    return lines


def main() -> None:
    lines = selfcheck()
    print("comparison_paths dry-run self-check — all assertions PASSED\n")
    print("\n".join(lines))


if __name__ == "__main__":
    main()
