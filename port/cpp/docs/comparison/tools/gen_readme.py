#!/usr/bin/env python3
"""Generate docs/comparison/README.md from the single comparison.json.

README.md is the source of truth for the MAUI-vs-C++ visual parity comparison. It has three
collapsible sections — **iOS**, **macOS**, **Android** — each containing:
  1. a summary table with the discrepancy counts (Sonnet 5 + Gemini), and
  2. a per-page table with columns  № | Gallery Screen | App Preview  (one row per gallery page).

The **App Preview** cell holds an inner <table> of the screenshots, following the fixed template:
non-macOS shows MAUI / C++ / C++&XAML; macOS additionally shows AppKit/C++ and AppKit/C++&XAML.
Each inner table stacks a Light row over a Dark row; a missing shot uses `_placeholder.png`.

Everything is driven by comparison.json (page name/title/description + per-platform screenshot
paths and Sonnet/Gemini parity reviews). Missing screenshots are stored as null → placeholder.

Usage: python3 tools/gen_readme.py      (writes ../README.md relative to this script)
"""
import html
import json
import os

HERE = os.path.dirname(os.path.abspath(__file__))
COMP = os.path.normpath(os.path.join(HERE, ".."))
JSON = os.path.join(COMP, "comparison.json")
README = os.path.join(COMP, "README.md")
PLACEHOLDER = "_placeholder.png"
IMG_W = "300px"

# (platform-key, display, framework columns in order, is_macos)
PLATFORMS = [
    ("ios", "iOS", ["maui", "cpp", "xaml"], False),
    ("maccatalyst", "macOS", ["maui", "cpp", "xaml", "appkit_cpp", "appkit_xaml"], True),
    ("android", "Android", ["maui", "cpp", "xaml"], False),
]
FW_LABEL = {
    "maui": "MAUI", "cpp": "C++", "xaml": "C++ &amp; XAML",
    "appkit_cpp": "AppKit / C++", "appkit_xaml": "AppKit / C++ &amp; XAML",
}
EMOJI = {"green": "🟢", "yellow": "🟡", "red": "🔴", "blank": "⬛"}
CLASS_LABEL = {"green": "🟢 Match", "yellow": "🟡 Minor", "red": "🔴 Major", "blank": "⬛ Blank"}

# Android has no dark theme; its dark cells are always placeholders.
NOTES = {
    "ios": "Real .NET MAUI (native-default) vs the C++ port vs the compile-time-XAML gallery, "
           "captured on the same iOS simulator in light and dark. MAUI is the content ground truth.",
    "maccatalyst": ".NET MAUI on macOS **is** Mac Catalyst (UIKit) — the MAUI / C++ / C++&amp;XAML "
                   "columns are the strict parity board. The **AppKit** columns are the native-NSView "
                   "backend (no MAUI reference; they track completeness, C++ == C++&amp;XAML).",
    "android": "Real .NET MAUI vs the C++ port vs the compile-time-XAML gallery, captured on the same "
               "Android emulator. Android is captured single-theme, so the Dark row is a placeholder.",
}


def esc(s):
    return html.escape((s or "").strip(), quote=False)


def img_td(path):
    src = path if path else PLACEHOLDER
    return f'<td><img width="{IMG_W}" src="{src}" /></td>'


def review_text(rev):
    st = (rev or {}).get("status")
    txt = esc((rev or {}).get("review", ""))
    emoji = EMOJI.get(st, "")
    if emoji and txt:
        return f"{emoji} {txt}"
    if emoji:
        return emoji
    return txt or "—"


def preview_table(page, fws):
    """The inner screenshot <table> for one page, per the fixed template."""
    sc = page["screenshots"]
    head = ["<th></th>"] + [f"<th>{FW_LABEL[fw]}</th>" for fw in fws]
    light = ["<th>Light</th>"] + [img_td(sc[fw]["light"]) for fw in fws]
    dark = ["<th>Dark</th>"] + [img_td(sc[fw]["dark"]) for fw in fws]
    return ("<table>"
            f"<tr>{''.join(head)}</tr>"
            f"<tr>{''.join(light)}</tr>"
            f"<tr>{''.join(dark)}</tr>"
            "</table>").replace("|", "\\|")


def counts(pages, plat, model):
    c = {"green": 0, "yellow": 0, "red": 0, "blank": 0, "none": 0}
    for p in pages:
        st = p["platforms"][plat][model].get("status")
        c[st if st in c else "none"] += 1
    return c


def summary_table(pages, plat, n):
    s = counts(pages, plat, "sonnet")
    g = counts(pages, plat, "gemini")
    out = ["| Classification | Sonnet 5 | Gemini |", "| --- | --- | --- |"]
    for k in ("green", "yellow", "red", "blank"):
        out.append(f"| {CLASS_LABEL[k]} | {s[k]} | {g[k]} |")
    out.append(f"| ⏳ Unreviewed | {s['none']} | {g['none']} |")
    return "\n".join(out)


def section(pages, plat, display, fws, n):
    """One collapsible platform section: intro + summary counts + the per-page table."""
    out = ["<details>", f"<summary><h2>{display} ({n} examples) — click to expand</h2></summary>", ""]
    out.append(NOTES[plat])
    out.append("")
    out.append("**Discrepancy counts** (MAUI-vs-C++ parity verdicts; Sonnet 5 `claude-sonnet-5` and "
               "Gemini review each page independently):")
    out.append("")
    out.append(summary_table(pages, plat, n))
    out.append("")
    out.append("| № | Gallery Screen | App Preview | Description | Sonnet 5 Review | Gemini Review |")
    out.append("| --- | --- | --- | --- | --- | --- |")
    for i, p in enumerate(pages, 1):
        page = {"description": p["description"], **p["platforms"][plat]}
        out.append(f"| {i} | **{esc(p['title'])}**<br><sub>{esc(p['name'])}</sub> | {preview_table(page, fws)} | {esc(page["description"])} | {review_text(page["sonnet"])} | {review_text(page["gemini"])} | ")
    out.append("")
    out.append("</details>")
    return "\n".join(out)


def main():
    pages = json.load(open(JSON, encoding="utf-8"))
    n = len(pages)
    out = [
        "# .NET MAUI C++ port — visual parity comparison",
        "",
        f"Per-page MAUI-vs-C++ visual parity for the **{n} gallery pages**, on **iOS**, **macOS** "
        "(Mac Catalyst + AppKit) and **Android**. Each section is collapsible and holds a discrepancy-"
        "count summary plus one row per page; every row's **App Preview** cell shows the MAUI / C++ / "
        "C++&amp;XAML renders (light over dark) with the per-page description and the Sonnet 5 + Gemini "
        "parity reviews. Missing captures show a placeholder. Generated from `comparison.json` by "
        "`tools/gen_readme.py` — do not edit by hand.",
        "",
    ]
    for plat, display, fws, _is_mac in PLATFORMS:
        out.append(section(pages, plat, display, fws, n))
        out.append("")
    text = "\n".join(out).rstrip("\n") + "\n"
    open(README, "w", encoding="utf-8").write(text)
    print(f"wrote {README} ({len(text)} bytes, {n} pages x {len(PLATFORMS)} sections)")


if __name__ == "__main__":
    main()
