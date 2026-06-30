#!/usr/bin/env python3
"""Generate the macOS (Mac Catalyst + AppKit) section of docs/comparison/README.md.

Emits a **collapsible** macOS section mirroring the iOS table shape:
  <details><summary>## macOS (172 examples) — click to expand</summary> … </details>
containing
  1. a summary classification table (consensus shape) + a row counting *placeholders
     per framework* (MAUI / C++ / C++&XAML, across BOTH Catalyst and AppKit), and
  2. the per-example table
       | # | Example | macOS — Catalyst / AppKit | Description | Sonnet … | Gemini |
     where the screenshot cell is a **2-row** <table>:
       row 1 = Catalyst MAUI │ Catalyst C++ │ Catalyst C++&XAML
       row 2 = (empty)       │ AppKit C++   │ AppKit C++&XAML
     so the two AppKit C++ shots line up under the two Catalyst C++ shots and the two
     C++&XAML shots line up too. Each <td> stacks the light shot over the dark shot;
     a missing shot uses the shared _placeholder.png at the same height.

Per-cell captures live at:
  maccatalyst/{maui,cpp,xaml}/{light,dark}/<key>.png        — Catalyst MAUI/C++/C++&XAML
  maccatalyst/{appkit_cpp,appkit_xaml}/{light,dark}/<key>.png — AppKit C++/C++&XAML

Description reuses the iOS row's Description text for that key (readme_common), else
the page title. Sonnet/Gemini are "_pending_" — the macOS vision review hasn't run.

Output goes to stdout; the caller splices it between the <!-- MACOS:BEGIN --> /
<!-- MACOS:END --> markers (idempotent regeneration).

Usage: python3 tools/parity/gen_macos_readme_section.py > /tmp/macos_section.md
"""
import os

from readme_common import (
    COMP,
    PLACEHOLDER,
    page_keys,
    title,
    ios_descriptions,
    description_for,
)

MC = os.path.join(COMP, "maccatalyst")
H = 240  # per-shot inline height (light over dark in each cell)
THEMES = ("light", "dark")
PENDING = "_pending — macOS vision review not run yet_"

# (subdir, framework-bucket) — the framework bucket is what the placeholder count groups by.
# Catalyst row buckets:
CAT_COLS = (("maui", "Catalyst MAUI", "Catalyst MAUI"),
            ("cpp", "Catalyst C++", "Catalyst C++"),
            ("xaml", "Catalyst C++&amp;XAML", "Catalyst C++&amp;XAML"))
# AppKit row buckets (col 1 stays empty so AppKit C++/XAML align under Catalyst C++/XAML):
APP_COLS = ((None, None, None),
            ("appkit_cpp", "AppKit C++", "AppKit C++"),
            ("appkit_xaml", "AppKit C++&amp;XAML", "AppKit C++&amp;XAML"))

# Framework buckets for the placeholder summary (each spans both Catalyst + AppKit):
#   MAUI         = Catalyst maui
#   C++          = Catalyst cpp + AppKit appkit_cpp
#   C++&XAML     = Catalyst xaml + AppKit appkit_xaml
FRAMEWORKS = {
    "MAUI": [("maui",)],
    "C++": [("cpp",), ("appkit_cpp",)],
    "C++&amp;XAML": [("xaml",), ("appkit_xaml",)],
}


def has_shot(subdir, theme, key):
    return os.path.exists(os.path.join(MC, subdir, theme, f"{key}.png"))


def stacked_td(subdir, label, key):
    """A <td> stacking the light shot over the dark shot for one framework, using
    the placeholder for any theme whose shot is missing. If subdir is None the cell
    is an empty spacer (keeps the AppKit row aligned to 3 columns)."""
    if subdir is None:
        return '<td align="center"></td>'
    parts = []
    for theme in THEMES:
        if has_shot(subdir, theme, key):
            src = f"maccatalyst/{subdir}/{theme}/{key}.png"
        else:
            src = PLACEHOLDER
        cap = f"{label} {theme}"
        parts.append(f'{cap}<br><img src="{src}" height="{H}">')
    return '<td align="center">' + "<br>".join(parts) + "</td>"


def cell(key):
    """2-row <table>: Catalyst MAUI|C++|C++&XAML over AppKit (empty)|C++|C++&XAML."""
    r1 = "".join(stacked_td(sub, lbl, key) for sub, lbl, _ in CAT_COLS)
    r2 = "".join(stacked_td(sub, lbl, key) for sub, lbl, _ in APP_COLS)
    return f"<table><tr>{r1}</tr><tr>{r2}</tr></table>"


def main():
    keys = page_keys()
    n = len(keys)
    descs = ios_descriptions()

    # placeholder counts per framework = missing per-theme shots across its subdirs.
    # Each subdir contributes 2 cells per key (light + dark).
    ph = {}
    real = {}
    for fw, buckets in FRAMEWORKS.items():
        missing = 0
        total = 0
        for (subdir,) in buckets:
            for k in keys:
                for theme in THEMES:
                    total += 1
                    if not has_shot(subdir, theme, k):
                        missing += 1
        ph[fw] = missing
        real[fw] = total - missing

    # Coverage: count distinct keys with at least one Catalyst-cpp / AppKit-cpp shot.
    cat_keys = sum(1 for k in keys if has_shot("cpp", "light", k) or has_shot("cpp", "dark", k))
    app_keys = sum(1 for k in keys if has_shot("appkit_cpp", "light", k) or has_shot("appkit_cpp", "dark", k))

    out = []
    out.append("<details>")
    out.append(f"<summary><h2>macOS ({n} examples) — click to expand</h2></summary>")
    out.append("")
    out.append(
        ".NET MAUI on macOS *is* **Mac Catalyst** (UIKit), so the Catalyst row of each cell is the strict "
        "3-way parity board — **MAUI ┃ C++ ┃ C++&amp;XAML** — rendering the same gallery the iOS section does, "
        "captured by `tools/parity/capture_maccatalyst.py`. **AppKit** is the native-NSView backend "
        "(**C++ ┃ C++&amp;XAML**), captured by `tools/parity/capture_appkit.py`; being a different UI framework "
        "it targets *completeness* (every coded/XAML element present) and **C++ == C++&amp;XAML**, not pixel "
        "parity. In each screenshot cell the **AppKit C++ / C++&amp;XAML** shots sit directly under the "
        "matching **Catalyst C++ / C++&amp;XAML** shots (the first AppKit column is intentionally empty — "
        "AppKit has no MAUI reference). Every shot is shown light over dark. See "
        "[maccatalyst/APPKIT_FINDINGS.md](maccatalyst/APPKIT_FINDINGS.md) and "
        "[../MACOS_ANDROID_RESUME.md](../MACOS_ANDROID_RESUME.md)."
    )
    out.append("")
    out.append(f"**Coverage:** {cat_keys} / {n} Catalyst C++ boards · {app_keys} / {n} AppKit C++ boards "
               "(light + dark each).")
    out.append("")
    out.append("**Classification:** the C++-vs-MAUI visual review has **not run yet** for macOS, so every "
               "example is currently *pending*. The table below keeps the iOS consensus shape so it can be "
               "filled in the same way once the review runs.")
    out.append("")
    out.append("| Classification | Count | Notes |")
    out.append("| --- | --- | --- |")
    out.append("| 🟢 Pixel-perfect | 0 | — |")
    out.append("| 🟢 Match | 0 | — |")
    out.append("| 🟡 C++ minor | 0 | — |")
    out.append("| 🔴 C++ major | 0 | — |")
    out.append(f"| ⏳ Pending review | {n} | Catalyst + AppKit boards captured where handlers exist; vision review outstanding |")
    out.append("")
    out.append("**Placeholders per framework** (per-theme cells showing `_placeholder.png` because no real "
               "capture exists yet; counted across both Catalyst and AppKit, light + dark):")
    out.append("")
    out.append("| Framework | Real captures | Placeholders |")
    out.append("| --- | --- | --- |")
    for fw in ("MAUI", "C++", "C++&amp;XAML"):
        out.append(f"| {fw} | {real[fw]} | {ph[fw]} |")
    out.append("")
    out.append("| # | Example | macOS — Catalyst (MAUI ┃ C++ ┃ C++&amp;XAML) over AppKit ( ┃ C++ ┃ C++&amp;XAML) | "
               "Description | Sonnet `claude-sonnet-4-6` | Gemini |")
    out.append("| --- | --- | --- | --- | --- | --- |")
    for i, k in enumerate(keys, 1):
        desc = description_for(k, descs)
        out.append(f"| {i} | **{title(k)}** | {cell(k)} | {desc} | {PENDING} | {PENDING} |")
    out.append("")
    out.append("</details>")
    print("\n".join(out))


if __name__ == "__main__":
    main()
