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
MEASUREMENTS = os.path.join(COMP, "measurements.json")   # written by measure_size.py / measure_runtime.py
README = os.path.join(COMP, "README.md")
PLACEHOLDER = "_placeholder.png"
IMG_W = "300px"

# (platform-key, display, framework columns in order, is_macos)
PLATFORMS = [
    ("ios", "iOS", ["maui", "cpp", "xaml"], False),
    ("maccatalyst", "macOS", ["maui", "cpp", "xaml", "appkit_cpp", "appkit_xaml"], True),
    ("android", "Android", ["maui", "cpp", "xaml"], False),
    ("windows", "Windows", ["maui", "cpp", "xaml"], False),
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
    "windows": "Real .NET MAUI as **WinUI 3** (`Microsoft.UI.Xaml`) — MAUI's actual Windows backend — "
               "vs the C++ port's own WinUI 3 backend, both built and captured NATIVE arm64 on a "
               "Windows 11 ARM64 VM. **Partial coverage by design:** only `window`, `content_page`, "
               "`layout`, `label` and `button` have real WinUI handlers so far; every other control "
               "still uses its headless mirror and renders nothing, so a page built from one of those "
               "is expected to be blank or partial. That is the fan-out being incomplete, not a "
               "regression — see `docs/WINDOWS_TOOLCHAIN.md` §6. The **C++ &amp; XAML** column is not "
               "captured yet: its committed translation units use `#embed`, which MSVC does not "
               "implement, so it builds through the bytes-mode codegen the android lane already uses.",
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


# The motion verdict's own glyphs. A SEPARATE alphabet from the board's 🟢/🟡/🔴 on purpose: motion is
# reported ALONGSIDE the colour, never folded into it (pixel_score.classify explains why — a layer
# covering 24.6% of cells must not drive the colour of the other 75%), and reusing the board's circles
# would invite exactly the conflation the four-verdict lattice was built to end.
VERDICT_GLYPH = {"PASS": "✅ PASS", "FAIL": "❌ FAIL",
                 "INVALID": "🚫 INVALID", "INCONCLUSIVE": "❔ INCONCLUSIVE"}


# The four states a reader asks about, and the distinct work each implies:
#   BOTH STILL   nothing animated on either side -- usually the harness never drove the page, so this
#                is a capture-coverage question, not a port question.
#   MAUI ONLY    MAUI animates and the port does not: a MISSING animation in the port.
#   PORT ONLY    the port animates and MAUI does not: a SPURIOUS animation.
#   BOTH MOVE    both animate; then the verdict says whether they agree.
# A bare PASS/FAIL cannot separate these, which is why the roll-up table this replaces was not
# actionable: it counted how many cells were validly compared, never what a cell was telling you.
#
# 0.05% of visibly-changed pixels is the floor motion_score itself treats as "moved" for a step-paired
# sequence; below it a column is indistinguishable from screenshot noise.
MOTION_FLOOR = 0.05
MOTION_STATE = {
    "both_still": "⏸ neither moves",
    "maui_only": "⚠ MAUI moves, C++ does not",
    "port_only": "⚠ C++ moves, MAUI does not",
    "both_move": "▶ both move",
}


def motion_state(rev):
    """→ (key, label) for the 4-way split, or None when the cell has no motion evidence."""
    m = (rev or {}).get("motion")
    if not m:
        return None
    self_ = m.get("self")
    if not self_:
        # Scored before `self` was recorded. Say so rather than guessing a state from the verdict --
        # a PASS is equally consistent with "both moved and agreed" and "neither moved at all".
        return ("unknown", "▫ motion scored (rerun for the moved-by-which-column split)")
    mm, pp = self_.get("maui", 0.0), self_.get("port", 0.0)
    moved_m, moved_p = mm >= MOTION_FLOOR, pp >= MOTION_FLOOR
    key = ("both_move" if moved_m and moved_p else
           "maui_only" if moved_m else
           "port_only" if moved_p else "both_still")
    label = MOTION_STATE[key]
    if key == "both_move":
        # Only here does the verdict add anything: both sides animated, so the open question is
        # whether the animations MATCH.
        v = m.get("verdict")
        label += " — " + ("comparison OK" if v == "PASS" else f"comparison {v}")
    return (key, label)


def motion_line(rev):
    """The motion badge under a review, or "" on a page with no motion evidence at all.

    Reads like a claim a human can check: the governing verdict, why, each theme's own verdict, and the
    run the frames came from — the last so a surprising verdict can be traced to a capture without
    grepping the prose."""
    m = (rev or {}).get("motion")
    if not m:
        return ""
    v = m.get("verdict", "")
    bits = [f"**Motion:** {VERDICT_GLYPH.get(v, v)}"]
    if m.get("why"):
        bits.append(f"`{m['why']}`")
    themes = m.get("themes") or {}
    if len(set(themes.values())) > 1:      # only worth printing when the themes DISAGREE
        bits.append(" / ".join(f"{t} {x}" for t, x in sorted(themes.items())))
    if m.get("run"):
        bits.append(f"<sub>run {esc(m['run'])} · {esc(str(m.get('captured_at') or ''))}</sub>")
    return " · ".join(bits)


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


# Board-wide roll-up, rendered ABOVE the collapsed per-platform sections. Deliberately built from the
# SAME counts() the per-section tables use, so a cell here cannot drift from the section it summarises —
# the two are the same call with the same arguments, not two implementations of the same idea.
#
# HTML rather than a markdown table: the spec calls for one merged title spanning each model's five
# status columns, and GFM tables cannot span cells. This file already emits <table> for the screenshot
# grids, so the style is consistent.
#
# SCOPE, stated in the table itself rather than left implicit:
#   - the macOS row is MAC CATALYST. The two AppKit columns are captured and shown, but are NOT
#     pixel-scored — capture_appkit.py's own header says AppKit "can't pixel-match MAUI/Catalyst
#     (different UI framework)", its requirement being element completeness plus cpp-vs-xaml agreement.
#     comparison.json carries no pixel_appkit* key and pixel_score.py has no appkit path. Emitting
#     counts for it would assert a parity claim the capture path explicitly disclaims.
#   - Android DARK is currently not comparable: the MAUI reference renders light for both
#     MAUI_THEME=Light and =Dark (measured 137.7 vs 139.3 body mean) while the port renders genuinely
#     dark, so android's reds are the port being right against a broken ground truth. See
#     PARITY_REVIEW.md. The counts are shown unaltered — annotating is honest, silently excluding or
#     re-weighting them would not be.
def board_summary_table(pages):
    rows = []
    totals = {key: {k: 0 for k in ("green", "yellow", "red", "blank", "none")} for key, _ in MODELS}
    for plat, display, _fws, _is_mac in PLATFORMS:
        cells = []
        for key, _label in MODELS:
            c = counts(pages, plat, key)
            for k in totals[key]:
                totals[key][k] += c[k]
            cells += [c["green"], c["yellow"], c["red"], c["blank"], c["none"]]
        rows.append((display, cells))

    head = ("<table>\n"
            "<tr><th rowspan=\"2\">Platform</th>"
            + "".join(f'<th colspan="5">{label}</th>' for _key, label in MODELS)
            + "</tr>\n<tr>"
            + ("<th>🟢</th><th>🟡</th><th>🔴</th><th>⬛</th><th>⏳</th>" * len(MODELS))
            + "</tr>")
    body = "".join(
        "\n<tr><td>" + d + "</td>" + "".join(f"<td>{v}</td>" for v in cells) + "</tr>"
        for d, cells in rows)
    tcells = []
    for key, _ in MODELS:
        t = totals[key]
        tcells += [t["green"], t["yellow"], t["red"], t["blank"], t["none"]]
    total = ("\n<tr><td><strong>Total</strong></td>"
             + "".join(f"<td><strong>{v}</strong></td>" for v in tcells) + "</tr>")
    return head + body + total + "\n</table>"


VERDICTS = ("PASS", "FAIL", "INVALID", "INCONCLUSIVE")


def motion_counts(pages, plat):
    """Motion verdicts on one platform, over BOTH model slots -> {verdict: n, "cells": n}."""
    c = {v: 0 for v in VERDICTS}
    c["cells"] = 0
    for p in pages:
        for key, _label in MODELS:
            m = ((p["platforms"].get(plat) or {}).get(key) or {}).get("motion")
            if not m:
                continue
            c["cells"] += 1
            if m.get("verdict") in c:
                c[m["verdict"]] += 1
    return c


def motion_summary_table(pages):
    """The board-wide motion roll-up — reported BESIDE the colour table, never merged into it.

    Two separate tables is the whole point. The colour table answers "do the pixels match?" over every
    cell; this one answers "was the motion validly compared?" over the subset that has motion evidence
    at all. Merging them would let a 24.6%-coverage layer silently re-colour the other 75%, and would
    hide the number that matters most here — how many cells have NO usable motion evidence."""
    rows = []
    tot = {v: 0 for v in VERDICTS}
    tot["cells"] = 0
    for plat, display, _fws, _is_mac in PLATFORMS:
        c = motion_counts(pages, plat)
        if not c["cells"]:
            continue
        for k in tot:
            tot[k] += c[k]
        rows.append((display, c))
    if not rows:
        return ""
    head = ("| Platform | Cells with motion evidence | " + " | ".join(VERDICT_GLYPH[v] for v in VERDICTS) + " |\n"
            "| --- | --- | " + " | ".join("---" for _ in VERDICTS) + " |")
    body = "\n".join(f"| {d} | {c['cells']} | " + " | ".join(str(c[v]) for v in VERDICTS) + " |"
                     for d, c in rows)
    total = (f"| **Total** | **{tot['cells']}** | "
             + " | ".join(f"**{tot[v]}**" for v in VERDICTS) + " |")
    return "\n".join([head, body, total])


def summary_table(pages, plat, n):
    cols = [(label, counts(pages, plat, key)) for key, label in MODELS]
    out = ["| Classification | " + " | ".join(l for l, _ in cols) + " |",
           "| --- | " + " | ".join("---" for _ in cols) + " |"]
    for k in ("green", "yellow", "red", "blank"):
        out.append(f"| {CLASS_LABEL[k]} | " + " | ".join(str(c[k]) for _, c in cols) + " |")
    out.append("| ⏳ Unreviewed | " + " | ".join(str(c["none"]) for _, c in cols) + " |")
    return "\n".join(out)


# --------------------------------------------------------------------------- cost measurements
# Size / memory / startup, from measurements.json. Separate from comparison.json on purpose: parity
# is the CONTROL CONDITION for these numbers (same pixels, same widgets → a cost difference is
# attributable to the implementation strategy), so the two are recorded independently and only
# joined here. See PREDICTIONS.md for the pre-registered hypotheses these tables answer.

# Which board section a measurement lane belongs under. A lane with no mapping is still listed in
# the global table; it just gets no per-page glyphs.
LANE_PLATFORM = {"macos-arm64": "maccatalyst", "macos-appkit": "maccatalyst", "windows-arm64": "windows"}


def load_measurements():
    try:
        with open(MEASUREMENTS, encoding="utf-8") as fh:
            return json.load(fh)
    except (OSError, ValueError):
        return {}


def mb(n):
    return f"{n / 2**20:.1f} MB"


def size_table(meas):
    """Artifact size per lane per column, with the build configuration ALWAYS shown.

    The build-config column is not decoration. A Debug managed build measured against an optimized
    native one (or the reverse — which is the current state) is a strawman, so no headline ratio is
    printed unless both sides of a lane are release-grade."""
    sizes = meas.get("size") or {}
    if not sizes:
        return ""
    rows, provisional = [], False
    for lane, cols in sizes.items():
        for col, r in cols.items():
            # remote_only says WHERE it was measured, not WHETHER. A guest artifact measured over SSH
            # renders like any other row; only one that could not be reached or does not exist yet
            # falls back to em-dashes, and it says WHICH so the two are not confused.
            if r.get("remote_only") and not r.get("exists"):
                rows.append(f"| {lane} | `{col}` | — | — | — | _{r.get('note', 'not measured')}_ |")
                continue
            if not r.get("exists"):
                rows.append(f"| {lane} | `{col}` | — | — | — | _artifact missing_ |")
                continue
            if not r.get("release_grade"):
                provisional = True
            stripped = r.get("total_bytes_stripped")
            main = r.get("main_binary") or {}
            sym = main.get("linkedit_bytes") or 0
            # The build DATE belongs beside the number. A size is only as current as the binary it
            # came from, and one of these artifacts stamps itself 1981 (reproducible Android
            # packaging writes the ZIP epoch), so "it looked recent" is not a check anyone can do.
            # An APK has no stripped figure to show, so without this its row is a bare total -- and on
            # this board that total is the most misleading number in the table (android `cpp` reads
            # 261.6 MB against MAUI's 28.6). Naming the native_libs bucket inline says where it went.
            nat = (r.get("buckets") or {}).get("native_libs") or 0
            extra = (f"native libs {mb(nat)}" if (nat and not stripped) else
                     (("symbols " + mb(sym)) if sym else ""))
            note = " · ".join(x for x in (extra,
                                          f"built {r['built_at']}" if r.get("built_at") else "") if x)
            rows.append(
                f"| {lane} | `{col}` | {mb(r['total_bytes'])} | "
                f"{mb(stripped) if stripped else '—'} | "
                f"{r['build_config']}{'' if r.get('release_grade') else ' ⚠'} | "
                f"{note} |")
    if not rows:
        return ""
    out = ["<details>", "<summary><h2>Artifact size — click to expand</h2></summary>", "",
           "Per-lane artifact size, decomposed. Answers **H1** in `PREDICTIONS.md`. `Stripped` is "
           "always measured, never estimated, but it is obtained differently per platform because "
           "debug information lives in different places: on Apple lanes it is an actual "
           "`strip -S -x` of a copy of the main binary (symbols sit *inside* the Mach-O); on "
           "Windows it is total minus the sidecar `.pdb` files, which is exact since they are never "
           "deployed. Android APKs report no stripped figure — read their `native_libs` bucket "
           "instead, which is where an unstripped `lib/*.so` hides. The Windows rows are walked on "
           "the guest over SSH; every other lane is measured locally.", "",
           "**Two rows must not be read as a bare ratio.** *Windows* compares different DEPLOYMENT "
           "MODES, not two builds of the same thing: MAUI ships a self-contained WindowsAppSDK "
           "(135.18 MiB, 90.5% of its payload) while the port ships only the bootstrapper and takes "
           "the runtime from the machine. The like-for-like comparison there is MAUI's own managed "
           "framework+app, 4.99 MiB, against `gallery.exe`'s 4.97 MiB. *Android* compares a FAT APK "
           "(arm64-v8a **and** x86_64, 99 `.so` each) against a single-ABI APK; a device installs one "
           "ABI, so `native_libs_per_abi` is the figure a size claim may use — normalized that way the "
           "sign of the comparison reverses. Full derivation: "
           "`docs/thesis/EVIDENCE_ARTIFACT_SIZE.md`.", ""]
    if provisional:
        out += ["> **⚠ PROVISIONAL — no ratio may be quoted from this table yet.** Rows marked ⚠ are "
                "not release-grade: the managed reference is a `Debug` build and the native builds "
                "have an empty `CMAKE_BUILD_TYPE` (no `-O`, no `NDEBUG`). Comparing them is a "
                "strawman in *both* directions. H1 stays open until both sides are rebuilt "
                "release-grade (managed: `-c Release` + trimming + AOT; native: "
                "`CMAKE_BUILD_TYPE=Release` + strip).", ""]
    out += ["| Lane | Column | On disk | Stripped | Build config | Notes |",
            "| --- | --- | --- | --- | --- | --- |", *rows, "", "</details>"]
    return "\n".join(out)


def page_metrics(meas, plat, name):
    """The `{measurements}` suffix on a page's `###` header — RSS and startup, native vs managed.

    Returns "" when nothing has been measured for this page, so the header degrades to exactly the
    parity glyphs it had before. Written by measure_runtime.py as
    measurements["runtime"][lane][page][column] = {"rss_bytes": …, "startup_ms": …}."""
    runtime = (meas.get("runtime") or {})
    lane = next((l for l, p in LANE_PLATFORM.items() if p == plat and l in runtime), None)
    if not lane:
        return ""
    page = (runtime[lane].get(name) or {})
    cpp, maui = page.get("cpp") or {}, page.get("maui_xaml") or {}
    if not cpp and not maui:
        return ""
    bits = []
    if cpp.get("rss_bytes") or maui.get("rss_bytes"):
        bits.append(f"RAM {mb(cpp.get('rss_bytes', 0))}/{mb(maui.get('rss_bytes', 0))}")
    if cpp.get("startup_ms") or maui.get("startup_ms"):
        bits.append(f"start {cpp.get('startup_ms', 0):.0f}/{maui.get('startup_ms', 0):.0f} ms")
    return ("  ·  " + "  ·  ".join(bits)) if bits else ""


def page_section(i, p, plat, fws, meas=None):
    """One gallery page: a `###` subheader (title + sonnet/gemini emoji combo), the screenshot
    table, then a `####` subsubheader per review model (Sonnet, Gemini, Pixel-Perfect Score)."""
    page = p["platforms"][plat]
    sc = page["screenshots"]
    # The compact header glyph is now the deterministic pixel score (C1/C3 over C2/C4); AI review removed.
    combo = f"{EMOJI.get((page.get('pixel') or {}).get('status'), '⏳')}/{EMOJI.get((page.get('pixel_xaml') or {}).get('status'), '⏳')}"

    # The motion STATE rides in the header, where the colour glyph already is, because it answers a
    # question the colour cannot: which column animated. Taken from the `pixel` slot (MAUI-vs-C++);
    # the xaml twin shares the same drive, so a second badge would only ever repeat it.
    st = motion_state(page.get("pixel"))
    motion_badge = f" · {st[1]}" if st else ""
    out = [f"### {i}. {esc(p['title'])} — {combo}{motion_badge}{page_metrics(meas or {}, plat, p['name'])}",
           f"<sub>{esc(p['name'])}</sub>", ""]
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
        motion = motion_line(model)
        if motion:
            out.append(motion)
            out.append("")
        out.append(review_body(model))
        out.append("")
    return "\n".join(out).rstrip()


def section(pages, plat, display, fws, n, meas=None):
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
        out.append(page_section(i, p, plat, fws, meas))
        out.append("")
    out.append("</details>")
    return "\n".join(out)



def main():
    pages = json.load(open(JSON, encoding="utf-8"))
    meas = load_measurements()
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
        "`tools/gen_readme.py` — do not edit by hand.",
        "",
        "**Cost of the implementation strategy.** Parity is the *control condition* for the size, "
        "memory and startup numbers below: both columns drive the same native widgets and produce the "
        "same pixels, so a cost difference is attributable to the implementation strategy rather than "
        "to one side doing less work. The hypotheses these tables test were registered **before** any "
        "number existed — see `PREDICTIONS.md`. Sizes come from `tools/measure_size.py`; per-page "
        "memory/startup glyphs in a page header read `RAM cpp/maui` and `start cpp/maui`.",
        "",
    ]
    out.append(board_summary_table(pages))
    out.append("")
    out.append("_macOS row = **Mac Catalyst**. The AppKit columns (`appkit_cpp`, `appkit_xaml`) are "
               "captured and shown per page but are not pixel-scored — AppKit is a different UI "
               "framework (NSViews vs UIKit) and cannot pixel-match, so its requirement is element "
               "completeness plus cpp-vs-xaml agreement, not a parity score._")
    out.append("")
    out.append("_Android **dark** is not currently comparable: the MAUI reference renders light under "
               "both `MAUI_THEME=Light` and `=Dark` (measured body mean 137.7 vs 139.3) while the port "
               "renders genuinely dark, so Android's reds are the port being correct against a broken "
               "ground truth rather than a port defect. See `PARITY_REVIEW.md`._")
    out.append("")
    # The board-wide motion roll-up table used to sit here and has been REMOVED. It counted how many
    # cells were validly compared, per platform, per verdict -- which never told a reader what to do
    # about any particular page. The per-example header now carries the state that does: which column
    # moved. See motion_state().
    tbl = size_table(meas)
    if tbl:
        out.append(tbl)
        out.append("")
    for plat, display, fws, _is_mac in PLATFORMS:
        out.append(section(pages, plat, display, fws, n, meas))
        out.append("")
    text = "\n".join(out).rstrip("\n") + "\n"
    open(README, "w", encoding="utf-8").write(text)
    print(f"wrote {README} ({len(text)} bytes, {n} pages x {len(PLATFORMS)} sections)")


if __name__ == "__main__":
    main()
