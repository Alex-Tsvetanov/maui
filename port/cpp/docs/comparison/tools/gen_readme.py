#!/usr/bin/env python3
"""Generate docs/comparison/README.md from the single comparison.json.

README.md is the source of truth for the MAUI-vs-C++ visual parity comparison. It has one collapsible
section per platform — **iOS**, **macOS**, **Android**, **Windows** — each containing:
  1. a summary table with the discrepancy counts (one column per review model), and
  2. a per-page table with columns  № | Gallery Screen | App Preview | Description | <review columns>.

The **App Preview** cell holds an inner <table> of the screenshots, following the fixed template:
non-macOS shows MAUI / C++ / C++&XAML; macOS additionally shows AppKit/C++ and AppKit/C++&XAML.
Each inner table stacks a Light row over a Dark row; a missing shot uses `_placeholder.png`.

Review models per platform: iOS/macOS/Android are judged by Sonnet 5 + Gemini; **Windows** is judged by
Sonnet 5 twice — once for C++ vs MAUI and once for C++&XAML vs MAUI (no Gemini pass) — so its two review
columns are the two Sonnet verdicts. Everything is driven by comparison.json. Missing screenshots are
stored as null -> placeholder.

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

_SG = [("sonnet", "Sonnet 5"), ("gemini", "Gemini")]
_WIN = [("sonnet_cpp", "Sonnet 5 · C++"), ("sonnet_xaml", "Sonnet 5 · C++ &amp; XAML")]

# (platform-key, display, framework columns in order, is_macos, review models [(key,label)])
PLATFORMS = [
    ("ios", "iOS", ["maui", "cpp", "xaml"], False, _SG),
    ("maccatalyst", "macOS", ["maui", "cpp", "xaml", "appkit_cpp", "appkit_xaml"], True, _SG),
    ("android", "Android", ["maui", "cpp", "xaml"], False, _SG),
    ("windows", "Windows", ["maui", "cpp", "xaml"], False, _WIN),
]
FW_LABEL = {
    "maui": "MAUI", "cpp": "C++", "xaml": "C++ &amp; XAML",
    "appkit_cpp": "AppKit / C++", "appkit_xaml": "AppKit / C++ &amp; XAML",
}
EMOJI = {"green": "🟢", "yellow": "🟡", "red": "🔴", "blank": "⬛"}
CLASS_LABEL = {"green": "🟢 Match", "yellow": "🟡 Minor", "red": "🔴 Major", "blank": "⬛ Blank"}

NOTES = {
    "ios": "Real .NET MAUI (native-default) vs the C++ port vs the compile-time-XAML gallery, "
           "captured on the same iOS simulator in light and dark. MAUI is the content ground truth.",
    "maccatalyst": ".NET MAUI on macOS **is** Mac Catalyst (UIKit) — the MAUI / C++ / C++&amp;XAML "
                   "columns are the strict parity board. The **AppKit** columns are the native-NSView "
                   "backend (no MAUI reference; they track completeness, C++ == C++&amp;XAML).",
    "android": "Real .NET MAUI vs the C++ port vs the compile-time-XAML gallery, captured on the same "
               "Android emulator. Android is captured single-theme, so the Dark row is a placeholder.",
    "windows": "Real .NET MAUI (WinUI 3, native-default) vs the C++ port (`gallery`) vs the "
               "compile-time-XAML gallery (`gallery_xaml`), captured on the same Windows desktop in "
               "light and dark. Animated pages are captured as GIF, the rest as PNG. MAUI is the "
               "content ground truth. The two review columns are Sonnet 5 judging C++ vs MAUI and "
               "C++&amp;XAML vs MAUI independently (no Gemini pass on Windows).",
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


def summary_table(pages, plat, models):
    cols = [(lbl, counts(pages, plat, key)) for key, lbl in models]
    out = ["| Classification | " + " | ".join(lbl for lbl, _ in cols) + " |",
           "| --- | " + " | ".join("---" for _ in cols) + " |"]
    for k in ("green", "yellow", "red", "blank"):
        out.append(f"| {CLASS_LABEL[k]} | " + " | ".join(str(c[k]) for _, c in cols) + " |")
    out.append("| ⏳ Unreviewed | " + " | ".join(str(c["none"]) for _, c in cols) + " |")
    return "\n".join(out)


def section(pages, plat, display, fws, n, models):
    """One collapsible platform section: intro + summary counts + the per-page table."""
    out = ["<details>", f"<summary><h2>{display} ({n} examples) — click to expand</h2></summary>", ""]
    out.append(NOTES[plat])
    out.append("")
    out.append("**Discrepancy counts** (MAUI-vs-C++ parity verdicts; each review model scores every "
               "page independently):")
    out.append("")
    out.append(summary_table(pages, plat, models))
    out.append("")
    review_cols = " | ".join(f"{lbl} Review" for _, lbl in models)
    out.append(f"| № | Gallery Screen | App Preview | Description | {review_cols} |")
    out.append("| --- | --- | --- | --- | " + " | ".join("---" for _ in models) + " |")
    for i, p in enumerate(pages, 1):
        pw = p["platforms"][plat]
        page = {"description": p["description"], **pw}
        reviews = " | ".join(review_text(pw[key]) for key, _ in models)
        out.append(f"| {i} | **{esc(p['title'])}**<br><sub>{esc(p['name'])}</sub> | "
                   f"{preview_table(page, fws)} | {esc(page['description'])} | {reviews} | ")
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
        "(Mac Catalyst + AppKit), **Android** and **Windows** (WinUI 3). Each section is collapsible "
        "and holds a discrepancy-count summary plus one row per page; every row's **App Preview** cell "
        "shows the MAUI / C++ / C++&amp;XAML renders (light over dark) with the per-page description and "
        "the parity reviews. Animated pages are captured as GIF, the rest as PNG. Missing captures show "
        "a placeholder. Generated from `comparison.json` by `tools/gen_readme.py` — do not edit by hand.",
        "",
    ]
    for plat, display, fws, _is_mac, models in PLATFORMS:
        # Only render a section for pages that actually carry that platform (all do today).
        if not any(plat in p.get("platforms", {}) for p in pages):
            continue
        out.append(section(pages, plat, display, fws, n, models))
        out.append("")
    text = "\n".join(out).rstrip("\n") + "\n"
    open(README, "w", encoding="utf-8").write(text)
    print(f"wrote {README} ({len(text)} bytes, {n} pages)")


if __name__ == "__main__":
    main()
