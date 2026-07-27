#!/usr/bin/env python3
"""Generate docs/comparison/README.md from the single comparison.json.

README.md is the source of truth for the MAUI-vs-C++ visual parity comparison. It has three
collapsible sections — **iOS**, **macOS**, **Android** — each containing:
  1. a summary table with the discrepancy counts (Sonnet 5 + Gemini), and
  2. one `###` subheader per gallery page, titled with the page name and a "{sonnet}/{gemini}"
     emoji combo (e.g. "🟢/⏳"), followed by:
       - the nested screenshot <table> (the fixed MAUI/C++/C++&XAML[+AppKit] x Light/Dark template),
       - a `####` subsubheader per review model (Sonnet, Gemini, Pixel-Perfect Score), each titled
         with that model's own status emoji (🟢/🟡/🔴/⬛ <framework>/⏳) and containing the review
         prose underneath.

The ⬛ (blank) emoji is followed by which capture is missing (MAUI / C++ / C++ & XAML), inferred by
checking which framework has no `light` screenshot on that platform.

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
# Optional: the Windows PIPELINE-validation record (tools/parity/windows/summarize_board_run.py).
# Absent until a Windows board run has been summarized, in which case the section is omitted.
WINDOWS_JSON = os.path.join(COMP, "windows_pipeline.json")
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
# Plain-text (non-HTML-escaped) twin of FW_LABEL, for use in markdown headers rather than <table> cells.
FW_LABEL_PLAIN = {
    "maui": "MAUI", "cpp": "C++", "xaml": "C++ & XAML",
    "appkit_cpp": "AppKit / C++", "appkit_xaml": "AppKit / C++ & XAML",
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
               "Android emulator in light and dark. MAUI is the content ground truth.",
}


def esc(s):
    return html.escape((s or "").strip(), quote=False)


def img_td(path):
    src = path if path else PLACEHOLDER
    return f'<td><img width="{IMG_W}" src="{src}" /></td>'


def blank_framework(sc, fws):
    """Which framework column has no `light` screenshot on this platform (the ⬛ blank cause),
    or None if every framework has at least a light shot (a 'blank' verdict from something the
    schema can't see, e.g. a corrupted/wrong-window capture — the emoji then stands alone)."""
    for fw in fws:
        if not sc.get(fw, {}).get("light"):
            return FW_LABEL_PLAIN.get(fw, fw)
    return None


def status_emoji(rev, sc, fws):
    """The compact status glyph: 🟢/🟡/🔴, ⬛ (+ which capture is blank), or ⏳ unreviewed."""
    st = (rev or {}).get("status")
    if st == "blank":
        label = blank_framework(sc, fws)
        return f"⬛ {label}" if label else "⬛"
    return EMOJI.get(st, "⏳")


def review_body(rev):
    """The review prose under a subsubheader (no leading emoji — that's in the heading)."""
    txt = esc((rev or {}).get("review", ""))
    return txt if txt else "_Not yet reviewed._"


def preview_table(sc, fws):
    """The screenshot <table> for one page, per the fixed template."""
    head = ["<th></th>"] + [f"<th>{FW_LABEL[fw]}</th>" for fw in fws]
    light = ["<th>Light</th>"] + [img_td(sc[fw]["light"]) for fw in fws]
    dark = ["<th>Dark</th>"] + [img_td(sc[fw]["dark"]) for fw in fws]
    return ("<table>"
            f"<tr>{''.join(head)}</tr>"
            f"<tr>{''.join(light)}</tr>"
            f"<tr>{''.join(dark)}</tr>"
            "</table>")


def counts(pages, plat, model):
    c = {"green": 0, "yellow": 0, "red": 0, "blank": 0, "none": 0}
    for p in pages:
        st = (p["platforms"][plat].get(model) or {}).get("status")
        c[st if st in c else "none"] += 1
    return c


# The review models rendered per page, in order: (comparison.json key, display name). AI-based review
# (Sonnet 5 + Gemini) was INVALIDATED (removed) — those models diverged and over-flagged, so the board
# now reports ONLY the deterministic pixel-perfect score. Per CLAUDE.md parity ruling 5 the pixel score
# judges MAUI-vs-C++ AND MAUI-vs-C++&XAML independently (C1/C3 and C2/C4), carried as separate rows.
MODELS = [
    ("pixel", "Pixel-Perfect Score — C++ (C1/C3)"),
    ("pixel_xaml", "Pixel-Perfect Score — C++ &amp; XAML (C2/C4)"),
]


def summary_table(pages, plat, n):
    cols = [(label, counts(pages, plat, key)) for key, label in MODELS]
    out = ["| Classification | " + " | ".join(l for l, _ in cols) + " |",
           "| --- | " + " | ".join("---" for _ in cols) + " |"]
    for k in ("green", "yellow", "red", "blank"):
        out.append(f"| {CLASS_LABEL[k]} | " + " | ".join(str(c[k]) for _, c in cols) + " |")
    out.append("| ⏳ Unreviewed | " + " | ".join(str(c["none"]) for _, c in cols) + " |")
    return "\n".join(out)


def page_section(i, p, plat, fws):
    """One gallery page: a `###` subheader (title + sonnet/gemini emoji combo), the screenshot
    table, then a `####` subsubheader per review model (Sonnet, Gemini, Pixel-Perfect Score)."""
    page = p["platforms"][plat]
    sc = page["screenshots"]
    # The compact header glyph is now the deterministic pixel score (C1/C3 over C2/C4); AI review removed.
    combo = f"{EMOJI.get((page.get('pixel') or {}).get('status'), '⏳')}/{EMOJI.get((page.get('pixel_xaml') or {}).get('status'), '⏳')}"

    out = [f"### {i}. {esc(p['title'])} — {combo}", f"<sub>{esc(p['name'])}</sub>", ""]
    out.append(preview_table(sc, fws))
    out.append("")
    if p["description"]:
        out.append(esc(p["description"]))
        out.append("")

    # Every model that has a section in MODELS, INCLUDING the pixel scores. The pixel rows used to be
    # hardcoded to "⏳ Not yet computed" here, which meant pixel_score.py's verdicts never reached the
    # README at all — it showed a stale hand-written tally while the automated score sat unread in
    # comparison.json. The README is the project's status reference; it renders what was computed.
    for key, label in MODELS:
        model = page.get(key)
        out.append(f"#### {status_emoji(model, sc, fws)} {label}")
        out.append("")
        out.append(review_body(model))
        out.append("")
    return "\n".join(out).rstrip()


def section(pages, plat, display, fws, n):
    """One collapsible platform section: intro + summary counts + one subheader per page."""
    out = ["<details>", f"<summary><h2>{display} ({n} examples) — click to expand</h2></summary>", ""]
    out.append(NOTES[plat])
    out.append("")
    out.append("**Discrepancy counts** (MAUI-vs-C++ parity verdicts from the deterministic pixel-perfect "
               "score — SSIM + per-pixel diff; AI-based review has been invalidated/removed):")
    out.append("")
    out.append(summary_table(pages, plat, n))
    out.append("")
    for i, p in enumerate(pages, 1):
        out.append(page_section(i, p, plat, fws))
        out.append("")
    out.append("</details>")
    return "\n".join(out)



def windows_pipeline_section():
    """The Windows section.

    Rendered from windows_pipeline.json, NOT from comparison.json, because it is deliberately not a
    parity section: the Windows column is currently the mingw Win32 smoke app and there is no MAUI
    Windows reference to score against. It is titled and worded so it can never be mistaken for a
    fidelity verdict -- 182 rows of green-looking "results" that said nothing about MAUI would be worse
    than no section at all. Returns "" when the record is absent."""
    if not os.path.isfile(WINDOWS_JSON):
        return ""
    d = json.load(open(WINDOWS_JSON, encoding="utf-8"))
    dropped = d.get("dropped_frames") or []
    bad = d.get("frames_undecodable") or []
    cross = d.get("duplicate_frames_cross_page") or []
    sizes = d.get("sizes") or {}
    size_line = ", ".join(f"`{k}` x{v}" for k, v in sorted(sizes.items(), key=lambda kv: -kv[1]))
    bounds = d.get("window_bounds") or {}
    bounds_line = ", ".join(f"`{k}` x{v}" for k, v in sorted(bounds.items(), key=lambda kv: -kv[1]))

    rows = [
        ("Pages captured", str(d.get("pages_captured", "?"))),
        ("Frames captured", str(d.get("frames_total", "?"))),
        ("Frames dropped (window never presented)", f"**{len(dropped)}**" if dropped else "0"),
        ("Frames undecodable", f"**{len(bad)}**" if bad else "0"),
        ("Distinct images", f"{d.get('unique_images', '?')} of {d.get('frames_total', '?')}"),
        ("Identical frames within a page", str(d.get("duplicate_frames_same_page", "?"))),
        ("Identical frames ACROSS pages", f"**{len(cross)}**" if cross else "0"),
        ("Capture sizes", size_line or "-"),
        ("Presented window bounds", bounds_line or "-"),
        ("Themes", ", ".join(f"{k} x{v}" for k, v in sorted((d.get("themes") or {}).items())) or "-"),
    ]

    out = ["<details>", "<summary><h2>Windows — pipeline validation (NOT a parity result) — click to "
           "expand</h2></summary>", ""]
    out.append("> **This section is not a visual-parity result and must not be read as one.** The "
               "Windows column is the mingw-cross **Win32 smoke app**, which paints the same layout for "
               "every page (only the title and a page-hash-derived swatch row change) — it is not the "
               "gallery. There is also no MAUI Windows reference column, so **no scores are computed**. "
               "MAUI's Windows backend is WinUI 3 (`Microsoft.UI.Xaml`), so only a WinUI 3 render can "
               "ever be compared against MAUI — see `docs/WINDOWS_TOOLCHAIN.md`.")
    out.append("")
    out.append("What it *does* establish: the end-to-end Windows harness works at board scale — every "
               "page launched in the guest's interactive session, was pinned to an exact window rect, and "
               "produced a decodable, correctly-sized, page-distinct PNG that the host pulled back.")
    out.append("")
    out.append(f"Run `{esc(str(d.get('run_dir')))}` at commit `{esc(str(d.get('commit'))[:10])}` "
               f"(column `{esc(str(d.get('column')))}`). Captures live in a gitignored dated run "
               f"directory; this table is the committed record.")
    out.append("")
    out.append("| Metric | Value |")
    out.append("| --- | --- |")
    for k, v in rows:
        out.append(f"| {k} | {v} |")
    out.append("")
    if dropped:
        out.append(f"**{len(dropped)} dropped frame(s)** — these pages were NOT captured:")
        out.append("")
        for f in dropped[:40]:
            out.append(f"- `{esc(f)}`")
        out.append("")
    if cross:
        out.append("**Identical images across DIFFERENT pages** — this would mean a page's environment "
                   "never took effect or a stale window was photographed:")
        out.append("")
        for grp in cross[:20]:
            out.append(f"- {', '.join('`' + esc(x) + '`' for x in grp)}")
        out.append("")
    out.append("Caveats recorded with the run:")
    out.append("")
    for c in d.get("caveats") or []:
        out.append(f"- {esc(c)}")
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
        "count summary, then one subheader per page titled with a `{Sonnet}/{Gemini}` status-emoji "
        "combo (🟢 match / 🟡 minor / 🔴 major / ⬛ blank / ⏳ unreviewed). Under each page: the MAUI / "
        "C++ / C++&amp;XAML renders (light over dark; missing captures show a placeholder), then a "
        "subsubheader per review model (Sonnet, Gemini, Pixel-Perfect Score) titled with that model's "
        "own status emoji and holding its review prose. Generated from `comparison.json` by "
        "`tools/gen_readme.py` — do not edit by hand. A final **Windows** section appears when "
        "`windows_pipeline.json` is present; it records PIPELINE validation, not parity — see "
        "its own preamble.",
        "",
    ]
    for plat, display, fws, _is_mac in PLATFORMS:
        out.append(section(pages, plat, display, fws, n))
        out.append("")
    win = windows_pipeline_section()
    if win:
        out.append(win)
        out.append("")
    text = "\n".join(out).rstrip("\n") + "\n"
    open(README, "w", encoding="utf-8").write(text)
    print(f"wrote {README} ({len(text)} bytes, {n} pages x {len(PLATFORMS)} sections)")


if __name__ == "__main__":
    main()
