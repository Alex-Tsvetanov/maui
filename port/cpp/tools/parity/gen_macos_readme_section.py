#!/usr/bin/env python3
"""Generate the macOS (Mac Catalyst + AppKit) section of docs/comparison/README.md.

The macOS boards are PRE-COMPOSED montages (one PNG per page = the whole side-by-side board):
  maccatalyst/montages/{light,dark}/<key>.png        — Catalyst 3-way: MAUI | C++ | C++&XAML
  maccatalyst/montages_appkit/{light,dark}/<key>.png — AppKit 2-way:   C++ | C++&XAML

This emits one Markdown table row per page (union of all montage keys), Catalyst board (light over
dark) in one cell and AppKit board (light over dark) in the next, mirroring the inline-<table><img>
style the iOS section already uses. Missing files are simply omitted (e.g. AppKit-only or
Catalyst-only pages). Output goes to stdout; the caller appends it to README.md between the
<!-- MACOS:BEGIN --> / <!-- MACOS:END --> markers (idempotent regeneration).

Usage: python3 tools/parity/gen_macos_readme_section.py > /tmp/macos_section.md
"""
import os
import glob

ROOT = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", ".."))
COMP = os.path.join(ROOT, "docs", "comparison")
MC = os.path.join(COMP, "maccatalyst")
CAT = os.path.join(MC, "montages")          # Catalyst MAUI|C++|C++&XAML
APP = os.path.join(MC, "montages_appkit")   # AppKit C++|C++&XAML
H = 240  # montages are wide composites; keep the inline height modest


def keys():
    found = set()
    for base in (CAT, APP):
        for theme in ("light", "dark"):
            for p in glob.glob(os.path.join(base, theme, "*.png")):
                found.add(os.path.splitext(os.path.basename(p))[0])
    return sorted(found)


def title(key):
    return key.replace("_", " ").title()


def board(base_rel, base_abs, key, light_label, dark_label):
    """Return an inline <table> stacking the light board over the dark board, or '' if neither exists."""
    rows = []
    for theme, label in (("light", light_label), ("dark", dark_label)):
        if os.path.exists(os.path.join(base_abs, theme, f"{key}.png")):
            src = f"{base_rel}/{theme}/{key}.png"
            rows.append(f'<tr><td align="center">{label}<br><img src="{src}" height="{H}"></td></tr>')
    if not rows:
        return ""
    return "<table>" + "".join(rows) + "</table>"


def main():
    ks = keys()
    cat_n = len([k for k in ks if os.path.exists(os.path.join(CAT, "light", f"{k}.png"))])
    app_n = len([k for k in ks if os.path.exists(os.path.join(APP, "light", f"{k}.png"))])
    out = []
    out.append("## macOS — Mac Catalyst (strict parity) & AppKit (native look)")
    out.append("")
    out.append(
        ".NET MAUI on macOS *is* **Mac Catalyst** (UIKit), so the Catalyst column is the strict 3-way "
        "parity board — **MAUI ┃ C++ ┃ C++&amp;XAML** — rendering the same gallery the iOS section does, "
        "captured by `tools/parity/capture_maccatalyst.py`. **AppKit** is the native-NSView backend "
        "(**C++ ┃ C++&amp;XAML**), captured by `tools/parity/capture_appkit.py`; being a different UI "
        "framework it targets *completeness* (every coded/XAML element present) and **C++ == C++&amp;XAML**, "
        "not pixel parity. Boards shown light-over-dark. See "
        "[maccatalyst/APPKIT_FINDINGS.md](maccatalyst/APPKIT_FINDINGS.md) and "
        "[../MACOS_ANDROID_RESUME.md](../MACOS_ANDROID_RESUME.md)."
    )
    out.append("")
    out.append(f"**Coverage:** {cat_n} Catalyst boards · {app_n} AppKit boards (light + dark each).")
    out.append("")
    out.append("| # | Example | Mac Catalyst — MAUI ┃ C++ ┃ C++&amp;XAML | AppKit — C++ ┃ C++&amp;XAML |")
    out.append("| --- | --- | --- | --- |")
    for i, k in enumerate(ks, 1):
        cat = board("maccatalyst/montages", CAT, k, "Catalyst light", "Catalyst dark")
        app = board("maccatalyst/montages_appkit", APP, k, "AppKit light", "AppKit dark")
        out.append(f"| {i} | **{title(k)}** | {cat or '—'} | {app or '—'} |")
    out.append("")
    print("\n".join(out))


if __name__ == "__main__":
    main()
