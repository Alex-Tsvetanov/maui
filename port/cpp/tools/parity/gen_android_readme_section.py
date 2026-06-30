#!/usr/bin/env python3
"""Generate the Android section of docs/comparison/README.md.

Emits a **collapsible** Android section mirroring the iOS table shape:
  <details><summary>## Android (172 examples) — click to expand</summary> … </details>
containing
  1. a summary classification table (consensus shape) + a row counting *placeholders
     per framework* (MAUI / C++ / C++&XAML), and
  2. the per-example table
       | # | Example | Android — MAUI ┃ C++ ┃ C++&XAML | Description | Sonnet … | Gemini |
     where the screenshot cell is a <table><tr> of 3 <td> (MAUI / C++ / C++&XAML).
     Each <td> shows the real png if docs/comparison/android/{maui,cpp,xaml}/<key>.png
     exists, else the shared _placeholder.png at the SAME height (so a missing cell
     occupies the same space). Description reuses the iOS row's Description text for
     that key (see readme_common.ios_descriptions), else the page title. Sonnet/Gemini
     are "_pending_" — the C++-vs-MAUI diff needs the MAUI android column first.

The keys are the canonical 172 from page_keys.txt (plus any extra captured under
android/cpp not in that list, so nothing is silently dropped).

Output goes to stdout; the caller splices it between the <!-- ANDROID:BEGIN --> /
<!-- ANDROID:END --> markers (idempotent regeneration).

Usage: python3 tools/parity/gen_android_readme_section.py > /tmp/android_section.md
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

AND = os.path.join(COMP, "android")
H = 360
COLUMNS = (("maui", "MAUI"), ("cpp", "C++"), ("xaml", "C++&amp;XAML"))
PENDING = "_pending — Android vision review not run yet_"


def all_keys():
    """The canonical 172 page keys, so the iOS / macOS / Android tables stay
    row-for-row aligned. Extra captures under android/cpp that aren't canonical
    pages (e.g. border_alignment, box_view_color_list) stay on disk but are not
    tabled here — they have no iOS row / Description to align against."""
    return page_keys()


def has_png(col, key):
    return os.path.exists(os.path.join(AND, col, f"{key}.png"))


def cell(key):
    tds = []
    for col, label in COLUMNS:
        if has_png(col, key):
            src = f"android/{col}/{key}.png"
        else:
            src = PLACEHOLDER  # same height -> same footprint as a real shot
        tds.append(f'<td align="center">{label}<br><img src="{src}" height="{H}"></td>')
    return "<table><tr>" + "".join(tds) + "</tr></table>"


def main():
    keys = all_keys()
    n = len(keys)
    # placeholder counts per framework = keys WITHOUT a real png for that column
    ph = {col: sum(0 if has_png(col, k) else 1 for k in keys) for col, _ in COLUMNS}
    real = {col: n - ph[col] for col, _ in COLUMNS}
    descs = ios_descriptions()

    out = []
    out.append("<details>")
    out.append(f"<summary><h2>Android ({n} examples) — click to expand</h2></summary>")
    out.append("")
    out.append(
        "The C++ port's gallery pages rendered by the **Android app host** (a real Activity/APK built by "
        "`tools/parity/build_android_apphost.sh` — aapt2/d8/apksigner, no gradle) on the `maui-test` emulator, "
        "captured via `adb screencap`. Per the cross-platform goal, every iOS example has an Android render "
        "here. Pages built on controls whose **Android handler is not yet implemented** (CollectionView, "
        "Picker, Date/TimePicker, Border/Shapes, …) render blank or partial — the honest current state of the "
        "Android backend (the layout/container handlers + ~12 widget handlers are done; see "
        "[../MACOS_ANDROID_RESUME.md](../MACOS_ANDROID_RESUME.md)). The **MAUI** and **C++&amp;XAML** Android "
        "columns are future work (need MauiCompare-android + the gallery_xaml app host); until those land "
        "every cell in them is a `pending` placeholder so the row footprint matches the eventual 3-up board."
    )
    out.append("")
    out.append(f"**Coverage:** {real['cpp']} / {n} pages captured (C++ column); "
               f"MAUI {real['maui']} / {n}, C++&amp;XAML {real['xaml']} / {n}.")
    out.append("")
    out.append("**Classification:** the C++-vs-MAUI visual review has **not run yet** for Android (it needs the "
               "MAUI Android column as the oracle), so every example is currently *pending*. The table below "
               "keeps the iOS consensus shape so it can be filled in the same way once the MAUI captures exist.")
    out.append("")
    out.append("| Classification | Count | Notes |")
    out.append("| --- | --- | --- |")
    out.append("| 🟢 Pixel-perfect | 0 | — |")
    out.append("| 🟢 Match | 0 | — |")
    out.append("| 🟡 C++ minor | 0 | — |")
    out.append("| 🔴 C++ major | 0 | — |")
    out.append(f"| ⏳ Pending review | {n} | C++ column captured; MAUI/XAML columns + vision review outstanding |")
    out.append("")
    out.append("**Placeholders per framework** (cells showing `_placeholder.png` because no real capture exists yet):")
    out.append("")
    out.append("| Framework | Real captures | Placeholders |")
    out.append("| --- | --- | --- |")
    out.append(f"| MAUI | {real['maui']} | {ph['maui']} |")
    out.append(f"| C++ | {real['cpp']} | {ph['cpp']} |")
    out.append(f"| C++&amp;XAML | {real['xaml']} | {ph['xaml']} |")
    out.append("")
    out.append("| # | Example | Android — MAUI ┃ C++ ┃ C++&amp;XAML | Description | Sonnet `claude-sonnet-4-6` | Gemini |")
    out.append("| --- | --- | --- | --- | --- | --- |")
    for i, k in enumerate(keys, 1):
        desc = description_for(k, descs)
        out.append(f"| {i} | **{title(k)}** | {cell(k)} | {desc} | {PENDING} | {PENDING} |")
    out.append("")
    out.append("</details>")
    print("\n".join(out))


if __name__ == "__main__":
    main()
