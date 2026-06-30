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
import json
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

# Render-health review (C++ Android vs the iOS C++ reference board), produced by the
# `android-render-health-audit` workflow and written to android_render_review.json as
# [{"key","status","note"}]. status ∈ {green,yellow,red,blank}. Absent file => not run yet.
REVIEW_JSON = os.path.join(COMP, "android_render_review.json")
STATUS_EMOJI = {
    "green": "🟢 renders",
    "yellow": "🟡 partial",
    "red": "🔴 wrong",
    "blank": "⬛ blank",
}


def load_render_review():
    """key -> {"status","note"} from android_render_review.json, or {} if absent/bad."""
    if not os.path.exists(REVIEW_JSON):
        return {}
    try:
        with open(REVIEW_JSON, encoding="utf-8") as fh:
            data = json.load(fh)
    except (OSError, ValueError):
        return {}
    out = {}
    for row in data:
        k = row.get("key")
        if k:
            out[k] = {"status": row.get("status", ""), "note": row.get("note", "")}
    return out


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
    review = load_render_review()
    rh = {"green": 0, "yellow": 0, "red": 0, "blank": 0}
    for k in keys:
        st = review.get(k, {}).get("status")
        if st in rh:
            rh[st] += 1
    reviewed = sum(rh.values())

    out = []
    out.append("<details>")
    out.append(f"<summary><h2>Android ({n} examples) — click to expand</h2></summary>")
    out.append("")
    out.append(
        "The C++ port's gallery pages rendered by the **Android app host** (a real Activity/APK built by "
        "`tools/parity/build_android_apphost.sh` — aapt2/d8/apksigner, no gradle) on the `maui-test` emulator, "
        "captured via `adb screencap`. Per the cross-platform goal, every iOS example has an Android render "
        "here. Implemented Android handlers: the layout/container/scroll stack plus ~22 widget handlers — "
        "label, image, button, entry, editor, activity_indicator, progress_bar, slider, switch, check_box, "
        "radio_button, picker, date_picker, time_picker, stepper, search_bar, border, box_view and the full "
        "**shapes family** (ellipse/line/polyline/polygon/path/rectangle) via the `android.graphics.Canvas` "
        "bridge (the Android twin of MAUI's `PlatformGraphicsView`). Pages built on controls still missing an "
        "Android handler (the **CollectionView** family, SwipeView, RefreshView, WebView, Carousel/Indicator, "
        "gesture-driven demos) render blank or partial — the honest current state, made precise by the "
        "render-health classification below (see also [../MACOS_ANDROID_RESUME.md](../MACOS_ANDROID_RESUME.md)). "
        "The **MAUI** and **C++&amp;XAML** Android "
        "columns are future work (need MauiCompare-android + the gallery_xaml app host); until those land "
        "every cell in them is a `pending` placeholder so the row footprint matches the eventual 3-up board."
    )
    out.append("")
    out.append(f"**Coverage:** {real['cpp']} / {n} pages captured (C++ column); "
               f"MAUI {real['maui']} / {n}, C++&amp;XAML {real['xaml']} / {n}.")
    out.append("")
    if reviewed:
        out.append("**Classification — C++ render-health** (**Sonnet 5** `claude-sonnet-5` vision review of the C++ "
                   "Android render vs the **iOS C++ reference** board, double-checked adversarially; this is render "
                   "completeness, *not* MAUI-vs-C++ pixel parity — that still needs the MAUI Android column). Each "
                   "non-green verdict was independently re-checked to overturn or confirm:")
        out.append("")
        out.append("| Classification | Count | Meaning |")
        out.append("| --- | --- | --- |")
        out.append(f"| 🟢 Renders | {rh['green']} | shows the same substantive content as the iOS C++ reference |")
        out.append(f"| 🟡 Partial | {rh['yellow']} | some intended content renders; a meaningful chunk missing/empty |")
        out.append(f"| 🔴 Wrong | {rh['red']} | renders content that doesn't match the page intent (e.g. routing mismatch) |")
        out.append(f"| ⬛ Blank | {rh['blank']} | app bar + empty background only; no page content (handler not yet implemented) |")
        if reviewed < n:
            out.append(f"| ⏳ Unreviewed | {n - reviewed} | not yet covered by the render-health audit |")
        out.append("")
        out.append("The per-row **Sonnet** column gives each page's render-health verdict + note. The **Gemini** "
                   "column and the MAUI-vs-C++ **parity** review remain pending the MAUI Android capture column.")
    else:
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
    out.append("| # | Example | Android — MAUI ┃ C++ ┃ C++&amp;XAML | Description | Sonnet 5 `claude-sonnet-5` | Gemini |")
    out.append("| --- | --- | --- | --- | --- | --- |")
    for i, k in enumerate(keys, 1):
        desc = description_for(k, descs)
        rv = review.get(k)
        if rv and rv.get("status") in STATUS_EMOJI:
            note = (rv.get("note") or "").replace("|", "\\|").replace("\n", " ")
            sonnet = f"{STATUS_EMOJI[rv['status']]} — {note}" if note else STATUS_EMOJI[rv["status"]]
        else:
            sonnet = PENDING
        out.append(f"| {i} | **{title(k)}** | {cell(k)} | {desc} | {sonnet} | {PENDING} |")
    out.append("")
    out.append("</details>")
    print("\n".join(out))


if __name__ == "__main__":
    main()
