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

MC = os.path.join(COMP, "maccatalyst")
H = 240  # per-shot inline height (light over dark in each cell)
THEMES = ("light", "dark")
PENDING = "_pending — macOS vision review not run yet_"

# MAUI-vs-C++ **Mac Catalyst** parity reviews (the strict parity board; AppKit has no MAUI
# reference so it is not classified here). Same buckets/schema as the Android board so the
# generators stay parallel. status ∈ {green,yellow,red,blank}. Absent file => not run yet.
#   Sonnet: macos_render_review.json  [{"key","status","note"}]  (macos_parity_wf.js)
#   Gemini: macos_gemini_review.json  [{"key","status","note","model"}]  (gemini_macos_sweep.py)
REVIEW_JSON = os.path.join(COMP, "macos_render_review.json")
GEMINI_REVIEW_JSON = os.path.join(COMP, "macos_gemini_review.json")
STATUS_EMOJI = {
    "green": "🟢 match",
    "yellow": "🟡 minor",
    "red": "🔴 major",
    "blank": "⬛ blank",
}
GEMINI_PENDING = "_pending — Gemini macOS review not run yet_"


def _load_review(path):
    """key -> {"status","note"} from a review JSON, or {} if absent/bad. Only rows with a real
    status (green/yellow/red/blank) are kept; pending/"" are dropped."""
    if not os.path.exists(path):
        return {}
    try:
        with open(path, encoding="utf-8") as fh:
            data = json.load(fh)
    except (OSError, ValueError):
        return {}
    out = {}
    for row in data:
        k = row.get("key")
        if k and row.get("status") in STATUS_EMOJI:
            out[k] = {"status": row["status"], "note": row.get("note", "")}
    return out

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

    # MAUI-vs-C++ Catalyst parity reviews (Sonnet + Gemini). The review only covers keys that
    # have a real MAUI Catalyst reference AND a C++ Catalyst capture (the strict parity board).
    review = _load_review(REVIEW_JSON)
    gemini = _load_review(GEMINI_REVIEW_JSON)
    rh = {"green": 0, "yellow": 0, "red": 0, "blank": 0}
    gh = {"green": 0, "yellow": 0, "red": 0, "blank": 0}
    for k in keys:
        st = review.get(k, {}).get("status")
        if st in rh:
            rh[st] += 1
        gst = gemini.get(k, {}).get("status")
        if gst in gh:
            gh[gst] += 1
    reviewed = sum(rh.values())
    gemini_reviewed = sum(gh.values())
    # How many keys are even comparable (have a MAUI reference) — the parity denominator.
    comparable = sum(1 for k in keys if has_shot("maui", "light", k) or has_shot("maui", "dark", k))

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
    if reviewed:
        out.append("**Classification — MAUI-vs-C++ Catalyst parity** (**Sonnet 5** `claude-sonnet-5` vision "
                   "review comparing the **Catalyst C++ port** against the **real .NET MAUI** Catalyst render of "
                   "each page — light + dark, both captured on the same Mac, native-default. .NET MAUI on macOS "
                   "IS Mac Catalyst, so this is the strict parity board; MAUI is the content ground truth and the "
                   "window chrome / outer inset are ignored per the parity policy. **AppKit** is a different UI "
                   "framework with no MAUI reference, so it is *not* classified here — its AppKit shots track "
                   "completeness and C++ == C++&amp;XAML):")
        out.append("")
        out.append("| Classification | Count | Meaning |")
        out.append("| --- | --- | --- |")
        out.append(f"| 🟢 Match | {rh['green']} | the port matches MAUI's Catalyst content (colors/sizes/spacing/text) in both themes — pixel-perfect or effectively identical |")
        out.append(f"| 🟡 Minor diff | {rh['yellow']} | small content differences that don't change the page's substance |")
        out.append(f"| 🔴 Major diff | {rh['red']} | the port differs substantially from MAUI (missing controls/images/effects, wrong layout) |")
        out.append(f"| ⬛ Port blank | {rh['blank']} | the port's page is blank/crashed while MAUI shows content |")
        out.append(f"| ⏳ Unreviewed | {n - reviewed} | no MAUI Catalyst reference yet (handler/capture outstanding) — {comparable} of {n} pages have a MAUI oracle |")
        out.append("")
        out.append("The per-row **Sonnet 5** column gives each page's Catalyst MAUI-vs-C++ parity verdict + the "
                   "specific diff (or \"matches MAUI\").")
        if gemini_reviewed:
            out.append("")
            out.append("**Classification — MAUI-vs-C++ Catalyst parity (Gemini** second-model pass, Google Gemini "
                       "vision via a quota-aware cascade — the SAME Catalyst MAUI-vs-C++ comparison and parity "
                       "policy as the Sonnet column, run independently. Carried alongside Sonnet 5 because the two "
                       "models diverge: a page green in both is high-confidence parity, while a split marks a page "
                       "worth a closer look. The free-tier daily quota caps a single run, so the column fills "
                       "incrementally across runs):")
            out.append("")
            out.append("| Classification | Count | Meaning |")
            out.append("| --- | --- | --- |")
            out.append(f"| 🟢 Match | {gh['green']} | Gemini judges the port matches MAUI's Catalyst content |")
            out.append(f"| 🟡 Minor diff | {gh['yellow']} | small content differences |")
            out.append(f"| 🔴 Major diff | {gh['red']} | the port differs substantially from MAUI |")
            out.append(f"| ⬛ Port blank | {gh['blank']} | the port's page is blank/crashed |")
            out.append(f"| ⏳ Unreviewed | {n - gemini_reviewed} | not yet covered by the Gemini pass |")
            out.append("")
            agree = sum(1 for k in keys if review.get(k, {}).get("status")
                        and review[k]["status"] == gemini.get(k, {}).get("status"))
            both = sum(1 for k in keys if review.get(k, {}).get("status") in STATUS_EMOJI
                       and gemini.get(k, {}).get("status") in STATUS_EMOJI)
            if both:
                out.append(f"**Model agreement:** Sonnet 5 and Gemini give the SAME verdict on **{agree} / {both}** "
                           f"Catalyst pages reviewed by both. Divergences (one model stricter) are the value of "
                           f"carrying two independent reviewers — compare the two columns per row below.")
    else:
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
               "Description | Sonnet 5 `claude-sonnet-5` | Gemini |")
    out.append("| --- | --- | --- | --- | --- | --- |")
    for i, k in enumerate(keys, 1):
        desc = description_for(k, descs)
        rv = review.get(k)
        if rv and rv.get("status") in STATUS_EMOJI:
            note = (rv.get("note") or "").replace("|", "\\|").replace("\n", " ")
            sonnet = f"{STATUS_EMOJI[rv['status']]} — {note}" if note else STATUS_EMOJI[rv["status"]]
        else:
            sonnet = PENDING
        gv = gemini.get(k)
        if gv and gv.get("status") in STATUS_EMOJI:
            gnote = (gv.get("note") or "").replace("|", "\\|").replace("\n", " ")
            gemini_cell = f"{STATUS_EMOJI[gv['status']]} — {gnote}" if gnote else STATUS_EMOJI[gv["status"]]
        else:
            gemini_cell = GEMINI_PENDING
        out.append(f"| {i} | **{title(k)}** | {cell(k)} | {desc} | {sonnet} | {gemini_cell} |")
    out.append("")
    out.append("</details>")
    print("\n".join(out))


if __name__ == "__main__":
    main()
