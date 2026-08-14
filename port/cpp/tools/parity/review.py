#!/usr/bin/env python3
"""THE parity check: read every capture + the measurements, and report what is INCONSISTENT.

    tools/parity/review.py --plan                       # what would be checked/judged; no API calls
    tools/parity/review.py --no-judge                   # integrity + measurements + pixel only (free)
    tools/parity/review.py --platforms ios --limit 40   # …plus Gemini on the first 40 undecided pairs
    tools/parity/review.py --commit-board               # also write the verdicts into comparison.json

THREE COMPARISONS per page, per platform, per theme (port/CLAUDE.md ruling 5 — MAUI is judged against
BOTH port columns independently, and the two port columns against each other):

    1. MAUI  vs  C++              -> comparison.json slot `gemini`
    2. MAUI  vs  C++ & XAML       -> slot `gemini_xaml`
    3. C++   vs  C++ & XAML       -> slot `gemini_twin`   (same renderer: a difference here is a
                                     markup/loader faithfulness bug, not a rendering one)

APPKIT IS CHECKED ONLY STRUCTURALLY -> slot `gemini_appkit`. NSViews can never pixel-match UIKit, so
there is no MAUI comparison for it at all; the requirement is that every element specified in the
code/XAML is PRESENT, and that the builder and XAML columns do not differ from each other. Its verdict
is deliberately kept out of the board's pixel-parity tallies.

THREE PHASES, cheapest first — the first two need no API key and always run:

  1. INTEGRITY (free).  Hashes the capture tree and reports one thing: a frame filed under TWO
     DIFFERENT PAGE KEYS, which is how a wrong-page capture gets scored as a port defect. Identical
     bytes across COLUMNS for the same page is the opposite of a defect — it is the port matching its
     reference exactly (measured: ~300 such cells per device platform, none on maccatalyst) — so those
     are counted, not flagged. Missing captures are listed (builder columns are exempt on non-twins).
  2. MEASUREMENTS (free).  Coverage only, never thresholds: which captured platform/column has no
     artifact-size or TTFF entry in measurements.json, and which measured entry has no captures.
  3. JUDGE.  SSIM/diff% first (pixel_score), and only pairs that are neither identical nor obviously
     fine go to Gemini vision. That prefilter is what makes a full board affordable: 4 platforms x 172
     pages x 3 comparisons is ~2000 calls, and the cpp-vs-xaml pair is usually byte-identical.

Quota is normal, not a crash: the client rotates through its model cascade on 429/404 and the run
stops cleanly when they are exhausted, keeping everything scored so far. `--resume` skips pages whose
slot already holds a verdict, so tomorrow's run continues where today's stopped.

Output: a markdown report (default docs/comparison/PARITY_INCONSISTENCIES.md). The board is NOT
touched unless you pass --commit-board — per port/CLAUDE.md the workflow is sweep -> user rules on the
findings -> adopt.

EXIT CODE = the number of wrong-page findings (0 = no frame is filed under more than one page).
"""
from __future__ import annotations

import argparse
import hashlib
import json
import os
import sys
import time
from collections import Counter
from datetime import datetime
from pathlib import Path

HERE = Path(__file__).resolve().parent
LIB = HERE / "lib"
CPP = HERE.parents[1]
PORT = CPP.parent
COMP = CPP / "docs" / "comparison"
sys.path.insert(0, str(LIB))

import comparison_paths as cp        # noqa: E402
import gemini                        # noqa: E402
import pixel_score                   # noqa: E402

PLATFORMS = ("android", "ios", "macos", "windows")
PLATFORM_DIR = {"android": "android", "ios": "ios", "macos": "maccatalyst", "windows": "windows"}
COLUMNS = {"ios": ("maui", "cpp", "xaml"),
           "android": ("maui", "cpp", "xaml"),
           "maccatalyst": ("maui", "cpp", "xaml", "appkit_cpp", "appkit_xaml"),
           "windows": ("maui", "cpp", "xaml")}
THEMES = ("light", "dark")
# The Android status bar (clock/battery/wifi) differs between captures purely because they were shot at
# different times — same exemption pixel_score.py applies.
CROP_TOP = {"android": 140}

# name -> (column A, column B, review-slot framework, structural?)
COMPARISONS = {
    "maui_cpp": ("maui", "cpp", "cpp", False),
    "maui_xaml": ("maui", "xaml", "xaml", False),
    "cpp_xaml": ("cpp", "xaml", "twin", False),
    "appkit": ("appkit_cpp", "appkit_xaml", "appkit", True),
}
LABEL = {"maui": "real .NET MAUI (the ground truth)", "cpp": "the C++ port (code-first builder)",
         "xaml": "the C++ port rendered from XAML markup",
         "appkit_cpp": "the C++ port on AppKit (code-first builder)",
         "appkit_xaml": "the C++ port on AppKit, rendered from XAML markup"}

# Pairs this good are not worth an API call: SSIM 0.995+ with under 0.5% of pixels differing is below
# what the vision model can see, and the board's own green gate is looser than this (0.98 / 1.0%).
AUTO_GREEN_SSIM, AUTO_GREEN_DIFF = 0.995, 0.5

ENUM = ["match", "minor", "diff", "b_blank", "a_blank"]
PAIR_SCHEMA = {
    "type": "OBJECT",
    "properties": {"light": {"type": "STRING", "enum": ENUM}, "dark": {"type": "STRING", "enum": ENUM},
                   "light_note": {"type": "STRING"}, "dark_note": {"type": "STRING"},
                   "differences": {"type": "ARRAY", "items": {"type": "STRING"}}},
    "required": ["light", "dark", "light_note", "dark_note", "differences"],
}
STRUCT_SCHEMA = {
    "type": "OBJECT",
    "properties": {"verdict": {"type": "STRING", "enum": ["complete", "columns_differ", "missing_elements"]},
                   "note": {"type": "STRING"},
                   "missing": {"type": "ARRAY", "items": {"type": "STRING"}}},
    "required": ["verdict", "note", "missing"],
}

GROUND_TRUTH_RULES = """\
GROUND TRUTH (project ruling): Microsoft's MAUI render IS the source of truth for page CONTENT — colors,
control sizes, internal spacing, text, corner radius, fonts. Any CONTENT difference, INCLUDING any color
difference, is a PORT BUG, never excused as a MAUI imperfection.

The ONLY MAUI imperfection the port need not copy is the HARNESS WRAPPER: MAUI insets the whole page
inside a card (large outer padding) and crops the page top/bottom; the port uses much less outer padding
and may show more of the page. IGNORE the outer-inset magnitude, the resulting UNIFORM global shift, and
the top/bottom crop. HARD GUARD: that inset is a uniform OUTER margin only. It never changes a control's
SIZE, COLOR, or the SPACING BETWEEN controls. If a control is a different size or color, or controls are
spaced differently INTERNALLY, that is a real difference even when clipping is also present."""

TWIN_RULES = """\
Both images come from the SAME renderer (the C++ port). A is built by hand in C++; B is hydrated from
XAML markup at build time. So there is no "framework difference" excuse available: any visible
difference is a markup/loader faithfulness bug — a property the XAML loader did not apply, a default it
got wrong, a child it did not attach. Sub-pixel/font-hinting noise is 'match'."""

PAIR_PROMPT = """\
Compare two screenshots of the SAME demo page, theme-for-theme, and report where they DISAGREE.

  A = {label_a}
  B = {label_b}

You are given four images in this order: 1. A (light)  2. B (light)  3. A (dark)  4. B (dark).
Both sides are the same capture resolution, so control SIZES and SPACING are directly comparable — a
size or spacing difference is REAL, never "scale noise".

{rules}

List every difference you can see in "differences", one short line each (empty list if none). Then
judge the LIGHT pair (img 1 vs 2) and the DARK pair (img 3 vs 4) SEPARATELY:
  match   : no differences beyond sub-pixel/font-hinting noise.
  minor   : small differences (a few px of internal spacing or size, font weight, a slightly-off shade).
  diff    : notable differences (missing/extra/mis-sized/mis-coloured control, wrong internal layout,
            overlap, wrong or garbled text).
  b_blank : B is blank/empty while A shows content.
  a_blank : A is blank/empty while B shows content.
Discount transient animation phase only (a spinner caught mid-rotation is at most 'minor').
light_note / dark_note: ONE precise, terse line each ("identical" if there is nothing to say)."""

STRUCT_PROMPT = """\
This is a STRUCTURAL check of a native macOS AppKit render — NOT a pixel-parity check.

  A = {label_a}
  B = {label_b}
{maui_line}
AppKit draws NSViews where the reference draws UIKit, so they can NEVER look alike. IGNORE ENTIRELY:
geometry, position, size, spacing, color, font, corner radius, control chrome, scrollbars, window
decoration. Judge ONLY:
  1. PRESENCE — is every element of the page present in both A and B (each label, button, field,
     image, list row, switch, etc.)? An element that exists in one and not the other is a finding.
  2. AGREEMENT — A and B are the same app built two ways (hand-written C++ vs the same page hydrated
     from XAML). They must contain the SAME elements. Any element in one but not the other is a bug.

verdict: "complete" (both columns show the same complete set of elements), "columns_differ" (A and B
disagree), or "missing_elements" (both are missing something the page clearly specifies).
"missing": one short line per missing/extra element. note: one terse summarising line."""

_SEV = {"green": 0, "yellow": 1, "red": 2, "blank": 3}
CATEGORY_TO_BOARD = {"match": "green", "minor": "yellow", "diff": "red",
                     "a_blank": "blank", "b_blank": "blank",
                     "complete": "green", "columns_differ": "red", "missing_elements": "red"}


# --------------------------------------------------------------------------- capture inventory
def capture_file(plat_dir: str, column: str, key: str, theme: str, ext: str | None = None) -> Path | None:
    """The capture for this cell. GIF first when no ext is forced — that is what the board renders."""
    for e in ([ext] if ext else ("gif", "png")):
        p = COMP / "captures" / plat_dir / column / f"{key}_{theme}.{e}"
        if p.is_file():
            return p
    return None


def twin_keys() -> set[str]:
    """Pages with a hand-written builder page. The `cpp`/`appkit_cpp` columns are legitimately absent
    for the others (the builder gallery falls back to another page), so they must not read as missing."""
    manifest = PORT / "maui-reference" / "pages" / "manifest.json"
    try:
        return {r["key"] for r in json.loads(manifest.read_text()) if r.get("builder_twin", True)}
    except (OSError, json.JSONDecodeError):
        return set()


def phase_integrity(platforms, keys, themes) -> tuple[list[str], set, list[str], Counter]:
    """Hash every capture; return (findings, the cells they hit, missing cells, tally).

    WHAT COUNTS AS CORRUPTION, and what does not — this took a full board to get right.

    A frame that is byte-identical ACROSS COLUMNS for the SAME page is *not* a defect. It is the port
    rendering the same native controls, on the same device, to the same pixels. Measured on this tree:
    306 such cells on iOS, 290 on Android, 292 on Windows — and ZERO on maccatalyst, the one platform
    whose captures come through a different path. A mis-filed frame would be scattered and rare; this
    is systematic and one-sided, which is what parity looks like.

    A frame that is byte-identical under two DIFFERENT PAGE KEYS is the real signature: it means one
    page's capture was banked under another page's name (the runner's historic `or bounds` bug pulled
    the previous column's file after a failed present, and `header_footer_grid` scored ~80% as a
    phantom port defect for a day because of it). Those are what this reports — with the caveat that a
    few demo pages are genuine near-duplicates (`vertical_stack` vs `vertical_stack_layout`), so each
    one is a "verify this", not an automatic bug.
    """
    twins = twin_keys()
    findings: list[str] = []
    suspect_cells: set[tuple[str, str, str]] = set()
    missing: list[str] = []
    tally: Counter = Counter()
    by_hash: dict[str, dict[str, list]] = {}
    for platform in platforms:
        plat = PLATFORM_DIR[platform]
        for key in keys:
            for theme in themes:
                digests: dict[str, str] = {}
                for col in COLUMNS[plat]:
                    f = capture_file(plat, col, key, theme)
                    if f is None:
                        if col in ("cpp", "appkit_cpp") and twins and key not in twins:
                            continue        # no builder page for this key — expected
                        missing.append(f"{plat}/{col}/{key}_{theme}")
                        continue
                    digests[col] = hashlib.sha256(f.read_bytes()).hexdigest()
                    tally[f"{plat}/{col}"] += 1
                    by_hash.setdefault(digests[col], {}).setdefault(plat, []).append((col, key, theme))
                # Same page, different columns, same bytes = the port matched MAUI exactly. Counted,
                # not flagged — see this function's docstring for why that is parity, not corruption.
                for port_col in ("cpp", "xaml", "appkit_xaml"):
                    if digests.get("maui") and digests.get("maui") == digests.get(port_col):
                        tally[f"pixel-exact maui=={port_col}"] += 1
                if digests.get("cpp") and digests.get("cpp") == digests.get("xaml"):
                    tally["pixel-exact cpp==xaml"] += 1

    # One frame under two different PAGE keys: somebody's capture is filed under the wrong name.
    for cells in by_hash.values():
        for plat, occurrences in cells.items():
            pages = {k for _, k, _ in occurrences}
            if len(pages) < 2:
                continue
            where = ", ".join(f"{c}/{k}_{t}" for c, k, t in sorted(occurrences))
            findings.append(f"{plat}: ONE frame is filed under {len(pages)} different pages "
                            f"({where}) — verify these are not near-duplicate demo pages")
            for _, k, t in occurrences:
                suspect_cells.add((plat, k, t))
    return findings, suspect_cells, missing, tally


# --------------------------------------------------------------------------- measurements
SIZE_ENV = {"macos-arm64": ("maccatalyst", ("maui_xaml", "cpp", "cpp_xaml")),
            "macos-appkit": ("maccatalyst", ("appkit_cpp", "appkit_xaml")),
            "windows-x64": ("windows", ("maui_xaml", "cpp", "cpp_xaml")),
            # iOS and Android measure through config/mobile.toml. Until they did, this table did not
            # list them, so the coverage check below correctly reported both as captured-but-never-
            # measured -- the "gap since day 1" it exists to catch. Adding them here is what lets
            # that finding CLEAR; leaving it out would keep asserting a gap that is now closed.
            "ios": ("ios", ("maui_xaml", "cpp", "cpp_xaml")),
            "android": ("android", ("maui_xaml", "cpp", "cpp_xaml"))}


def phase_measurements(platforms) -> list[str]:
    """Coverage only — what was captured but never measured, and what was measured but never captured.
    Deliberately no thresholds on the numbers themselves: this reports gaps, it does not grade."""
    findings: list[str] = []
    path = COMP / "measurements.json"
    if not path.is_file():
        return [f"{path.name} does not exist — nothing has been measured"]
    data = json.loads(path.read_text())
    size, ttff = data.get("size", {}), data.get("ttff", {})

    measured_plats = {SIZE_ENV[e][0] for e in size if e in SIZE_ENV}
    for platform in platforms:
        plat = PLATFORM_DIR[platform]
        captured = any((COMP / "captures" / plat / c).is_dir() for c in COLUMNS[plat])
        if captured and plat not in measured_plats:
            findings.append(f"size: {plat} has captures but NO artifact-size entry "
                            f"(measure_size.py covers {sorted(size)})")
        want_ttff = ["maccatalyst", "appkit"] if plat == "maccatalyst" else [plat]
        for w in want_ttff:
            entry = ttff.get(w)
            if entry is None:
                findings.append(f"ttff: {w} has captures but no time-to-first-frame entry")
                continue
            for col, rec in entry.items():
                if isinstance(rec, dict) and not rec.get("measured", True):
                    findings.append(f"ttff: {w}/{col} recorded as UNMEASURED "
                                    f"({rec.get('note') or rec.get('error') or 'no reason given'})")
    for env, (plat, cols) in SIZE_ENV.items():
        if env not in size:
            continue
        for col in cols:
            if col not in size[env]:
                findings.append(f"size: {env} measured, but column {col} is missing from it")
    return findings


# --------------------------------------------------------------------------- judging
def rel(p: Path) -> str:
    return str(p.relative_to(COMP))


def pixel_pair(plat: str, col_a: str, col_b: str, key: str, themes) -> dict:
    """SSIM/diff% per theme for one comparison (None for a theme with no comparable pair)."""
    crop = CROP_TOP.get(plat, 0)
    out = {}
    for theme in themes:
        a = capture_file(plat, col_a, key, theme, ext="png")
        b = capture_file(plat, col_b, key, theme, ext="png")
        out[theme] = pixel_score.score_theme(rel(a) if a else None, rel(b) if b else None, crop)
    return out


def auto_verdict(scores: dict) -> tuple[str, str] | None:
    """Decide a pair WITHOUT an API call when the pixels already settle it."""
    have = {t: v for t, v in scores.items() if v}
    if not have:
        return "blank", "No comparable pair exists for this page."
    if all(v["ssim"] >= AUTO_GREEN_SSIM and v["diff_pct"] <= AUTO_GREEN_DIFF for v in have.values()):
        return "green", "pixel-identical within tolerance · " + pixel_note(scores)
    return None


def pixel_note(scores: dict) -> str:
    return " · ".join(f"{t.capitalize()}: SSIM {v['ssim']:.4f}, {v['diff_pct']:.2f}% differ"
                      for t, v in scores.items() if v)


def judge_pair(client_state, plat, col_a, col_b, key, themes, structural: bool):
    """One Gemini call for a comparison (both themes). Returns (status, review) or raises gemini.Quota."""
    parts = []
    for theme in themes:
        for label, col in ((f"A = {LABEL[col_a]}", col_a), (f"B = {LABEL[col_b]}", col_b)):
            f = capture_file(plat, col, key, theme, ext="png")
            if f is None:
                return "blank", f"missing capture: {plat}/{col}/{key}_{theme}.png"
            parts += gemini.image_part(str(f), f"{label} ({theme})")
    if structural:
        prompt = STRUCT_PROMPT.format(label_a=LABEL[col_a], label_b=LABEL[col_b],
                                      maui_line="\nThere is deliberately NO MAUI reference here.\n")
        schema = STRUCT_SCHEMA
    else:
        rules = TWIN_RULES if col_a != "maui" else GROUND_TRUTH_RULES
        prompt = PAIR_PROMPT.format(label_a=LABEL[col_a], label_b=LABEL[col_b], rules=rules)
        schema = PAIR_SCHEMA

    verdict = client_state.call(parts, prompt, schema)
    if structural:
        status = CATEGORY_TO_BOARD.get(verdict.get("verdict", ""), "yellow")
        note = verdict.get("note", "").strip()
        if verdict.get("missing"):
            note += " | " + "; ".join(verdict["missing"][:6])
        return status, note
    lb = CATEGORY_TO_BOARD.get(verdict.get("light", ""), "yellow")
    db = CATEGORY_TO_BOARD.get(verdict.get("dark", ""), "yellow")
    status = lb if _SEV[lb] >= _SEV[db] else db
    ln, dn = verdict.get("light_note", "").strip(), verdict.get("dark_note", "").strip()
    review = f"Light: {ln} · Dark: {dn}" if ln and dn and ln != dn else (ln or dn)
    if verdict.get("differences"):
        review += " | " + "; ".join(verdict["differences"][:6])
    return status, review


class Client:
    """Gemini with the model cascade + a spend counter, so one exhausted model does not end the run."""

    def __init__(self, models, timeout):
        self.key = gemini.read_key()
        self.models = list(models)
        self.timeout = timeout
        self.calls = 0

    def call(self, parts, prompt, schema) -> dict:
        while self.models:
            try:
                # Count the ATTEMPT, not the success: a call that fails still spends quota, and
                # --limit is the user's budget for calls made, not for calls that worked.
                self.calls += 1
                return gemini.generate(self.models[0], self.key, parts, prompt, schema, self.timeout)
            except gemini.Quota as q:
                print(f"    quota: {q} — rotating", flush=True)
                self.models.pop(0)
                time.sleep(1)
        raise gemini.Quota("all models exhausted")


# --------------------------------------------------------------------------- report
def parse_comparisons(path: Path) -> dict:
    """Section 3's rows from an EXISTING report, keyed by (platform, page, comparison).

    Section 3 is the only part of this report that costs API calls, and a run almost never covers all
    of it: --resume skips scored pages, --limit stops spending, --examples/--platforms scope it
    deliberately, and a quota failure truncates it outright. Every one of those used to overwrite the
    whole section with just what that run produced.

    Measured on 2026-08-15: a run whose key hit its rate limit after 21 comparisons rewrote this file
    from 2305 lines to 85, discarding ~2250 recorded findings. It exited 0 and the result was
    well-formed, so nothing looked wrong -- it surfaced only as '2244 deletions' in a commit diff.

    Reading the old rows back is what makes a partial run additive instead of destructive. The detail
    column has its pipes replaced with '/' when written, so splitting on '|' is safe.
    """
    if not path.is_file():
        return {}
    rows, in_section = {}, False
    for line in path.read_text(errors="replace").splitlines():
        if line.startswith("## 3."):
            in_section = True
            continue
        # "### Tally" is derived, not data: it is recomputed from whatever the merge produces.
        if in_section and (line.startswith("## ") or line.startswith("### ")):
            break
        if not in_section or not line.startswith("|"):
            continue
        cells = [c.strip() for c in line.strip().strip("|").split("|")]
        if len(cells) < 6 or cells[0] in ("platform", "---"):
            continue
        plat, key, comp, status, source, detail = cells[:6]
        rows[(plat, key, comp)] = {"platform": plat, "key": key, "comparison": comp,
                                   "status": status, "source": source, "review": detail,
                                   # Reports written before the `scored` column existed have no date;
                                   # empty is honest, and reads as "older than this column".
                                   "scored": cells[6] if len(cells) > 6 else ""}
    return rows


def write_report(path: Path, args, findings, missing, tally, meas, verdicts,
                 partial_reason: str = "") -> None:
    examples = getattr(args, "examples", "all")
    lines = [f"# Parity inconsistencies — {datetime.now():%Y-%m-%d %H:%M}", "",
             f"`review.py --platforms {args.platforms} --comparisons {args.comparisons}"
             f"{'' if examples == 'all' else ' --examples ' + examples}"
             f"{' --no-judge' if args.no_judge else ''}`", ""]
    # Sections 1 and 2 are regenerated from THIS run's scope, so a narrowed run narrows them. Unlike
    # section 3 that costs nothing to put right -- both phases are free -- but it is invisible unless
    # said out loud, and a reader would otherwise take a short list as "few problems" rather than
    # "few pages looked at".
    scoped = [f"`--{n} {v}`" for n, v in (("platforms", args.platforms), ("examples", examples),
                                          ("comparisons", args.comparisons))
              if v != "all" and "," not in v]
    if scoped:
        lines += [f"> **Scoped run** ({', '.join(scoped)}). Sections 1 and 2 below describe only that "
                  "scope, not the whole board — re-run without the narrowing flags to refresh them "
                  "(both phases are free). Section 3 is merged across runs and is NOT narrowed.", ""]
    lines += ["## 1. Capture integrity", ""]
    if findings:
        lines += [f"**{len(findings)} page(s) share a frame with a DIFFERENT page.** One capture filed "
                  "under two names is how a wrong-page frame gets scored as a port defect. A few demo "
                  "pages are genuine near-duplicates (`vertical_stack` / `vertical_stack_layout`), so "
                  "check each before treating it as a capture bug:", ""]
        lines += [f"- {f}" for f in findings] + [""]
    else:
        lines += ["No frame is filed under more than one page. ✅", ""]
    exact = {k: v for k, v in tally.items() if k.startswith("pixel-exact")}
    lines += [f"- captures hashed: {sum(v for k, v in tally.items() if '/' in k)}",
              f"- missing cells: {len(missing)}", "",
              "Byte-identical cells across columns for the SAME page — this is the port matching its "
              "reference exactly, NOT a defect:", ""]
    lines += [f"  - {k.removeprefix('pixel-exact ')}: {v}" for k, v in sorted(exact.items())] or \
        ["  - none"]
    lines += [""]
    if missing:
        lines += ["<details><summary>missing cells</summary>", ""]
        lines += [f"- {m}" for m in missing[:200]]
        if len(missing) > 200:
            lines.append(f"- …and {len(missing) - 200} more")
        lines += ["", "</details>", ""]

    lines += ["## 2. Measurements coverage", ""]
    lines += ([f"- {m}" for m in meas] if meas else ["Every captured platform/column is measured. ✅"])
    # MERGE, never replace. This run is authoritative only for the pairs it actually scored; every
    # other row stays exactly as the run that produced it left it. See parse_comparisons().
    today = f"{datetime.now():%Y-%m-%d}"
    merged = parse_comparisons(path)
    carried_before = len(merged)
    for v in verdicts:
        merged[(v["platform"], v["key"], v["comparison"])] = {**v, "scored": today}
    rows = sorted(merged.values(), key=lambda r: (r["platform"], r["key"], r["comparison"]))
    carried = len(rows) - len(verdicts)

    lines += ["", "## 3. Comparisons", ""]
    if partial_reason:
        lines += [f"> **⚠ PARTIAL RUN — {partial_reason}.** {len(verdicts)} comparison(s) were scored "
                  f"here; the other {carried} row(s) below are CARRIED OVER from earlier runs and were "
                  "not re-checked now. Their `scored` date says when each was last judged. Re-run "
                  "without the interruption to refresh them.", ""]
    elif carried:
        lines += [f"> {len(verdicts)} comparison(s) scored in this run; {carried} row(s) carried over "
                  "from earlier runs (this run's scope did not include them). The `scored` column "
                  "dates each row.", ""]
    lines += ["| platform | page | comparison | status | source | detail | scored |",
              "| --- | --- | --- | --- | --- | --- | --- |"]
    for v in rows:
        detail = v["review"].replace("|", "/")[:160]
        lines.append(f"| {v['platform']} | {v['key']} | {v['comparison']} | {v['status']} | "
                     f"{v['source']} | {detail} | {v.get('scored', '')} |")
    # Tally over the MERGED set: a tally of only this run's rows would disagree with the table above it.
    counts = Counter((v["comparison"], v["status"]) for v in rows)
    lines += ["", "### Tally", "", "| comparison | green | yellow | red | blank |", "| --- | --- | --- | --- | --- |"]
    for name in COMPARISONS:
        if any(c[0] == name for c in counts):
            lines.append(f"| {name} | " + " | ".join(str(counts.get((name, s), 0))
                                                     for s in ("green", "yellow", "red", "blank")) + " |")
    path.write_text("\n".join(lines) + "\n")


# --------------------------------------------------------------------------- main
def selftest() -> int:
    """Device- and API-free check of the decision logic (the file I/O is exercised by a real run)."""
    # Slot mapping: the three comparisons plus the structural one must each land in their OWN slot.
    assert [cp.review_slot("gemini", COMPARISONS[c][2]) for c in
            ("maui_cpp", "maui_xaml", "cpp_xaml", "appkit")] == \
        ["gemini", "gemini_xaml", "gemini_twin", "gemini_appkit"]
    # appkit is macOS-only: asking for it on ios/android/windows must yield no unit at all.
    units = [(p, c) for p in PLATFORMS for c in COMPARISONS
             if not (c == "appkit" and PLATFORM_DIR[p] != "maccatalyst")]
    assert [p for p, c in units if c == "appkit"] == ["macos"], units

    # auto_verdict: only spend an API call when the pixels do NOT settle it.
    good = {"ssim": 0.999, "diff_pct": 0.1}
    meh = {"ssim": 0.97, "diff_pct": 2.0}
    assert auto_verdict({"light": good, "dark": good})[0] == "green"
    assert auto_verdict({"light": good, "dark": meh}) is None      # one bad theme => judge it
    assert auto_verdict({"light": None, "dark": None})[0] == "blank"

    # Verdict folding: the WORST theme wins, so a page that is fine in light and broken in dark is red.
    assert max(("green", "red"), key=lambda s: _SEV[s]) == "red"
    assert CATEGORY_TO_BOARD["columns_differ"] == "red" and CATEGORY_TO_BOARD["complete"] == "green"

    # Section 3 MERGES; a partial run must never drop rows it did not score. On 2026-08-15 a run whose
    # API key hit its rate limit after 21 comparisons rewrote the report from 2305 lines to 85,
    # silently discarding ~2250 findings, and exited 0. This is that scenario in miniature.
    import tempfile  # noqa: PLC0415  selftest-only
    args = argparse.Namespace(platforms="ios", comparisons="maui_cpp", no_judge=False)
    def v(key, status, review, source="pixel"):
        return {"platform": "ios", "key": key, "comparison": "maui_cpp", "status": status,
                "review": review, "source": source, "slot": "gemini"}
    with tempfile.TemporaryDirectory() as td:
        rep = Path(td) / "r.md"
        write_report(rep, args, [], [], Counter(), [], [v("button", "green", "first run"),
                                                        v("label", "red", "first run")])
        assert len(parse_comparisons(rep)) == 2
        # A later run that scored ONLY `button`, then died on quota.
        write_report(rep, args, [], [], Counter(), [], [v("button", "red", "rescored", "gemini")],
                     "the run stopped early: all models exhausted")
        rows = parse_comparisons(rep)
        assert len(rows) == 2, f"carried row lost: {sorted(rows)}"
        assert rows[("ios", "button", "maui_cpp")]["status"] == "red"        # rescore wins
        assert rows[("ios", "label", "maui_cpp")]["review"] == "first run"   # untouched
        assert "PARTIAL RUN" in rep.read_text()
        # The tally must count the MERGED rows, not just this run's, or it contradicts its own table.
        assert "| maui_cpp | 0 | 0 | 2 | 0 |" in rep.read_text(), rep.read_text()

    print("review selftest: slots + appkit scoping + pixel prefilter + section-3 merge OK")
    return 0


def main(argv=None) -> int:
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0],
                                 formatter_class=argparse.RawDescriptionHelpFormatter,
                                 epilog="\n".join(__doc__.splitlines()[2:]))
    ap.add_argument("--platforms", default=",".join(PLATFORMS))
    ap.add_argument("--comparisons", default=",".join(COMPARISONS),
                    help=f"subset of {','.join(COMPARISONS)} (appkit is macos-only and structural)")
    ap.add_argument("--examples", default="all", help="comma-separated page keys (default: all)")
    ap.add_argument("--themes", default=",".join(THEMES))
    ap.add_argument("--no-judge", action="store_true", help="skip Gemini; integrity + pixel only")
    ap.add_argument("--limit", type=int, default=0, help="max Gemini calls this run (0 = no limit)")
    ap.add_argument("--resume", action="store_true",
                    help="skip pages whose comparison.json slot already holds a verdict")
    ap.add_argument("--commit-board", action="store_true",
                    help="write the verdicts into comparison.json (default: report only)")
    ap.add_argument("--report", default=str(COMP / "PARITY_INCONSISTENCIES.md"))
    ap.add_argument("--model", default="", help="force one Gemini model instead of the cascade")
    ap.add_argument("--timeout", type=int, default=120)
    ap.add_argument("--plan", action="store_true", help="print what would be checked; no API calls")
    ap.add_argument("--selftest", action="store_true",
                    help="check the slot mapping, appkit scoping and pixel prefilter; exits")
    a = ap.parse_args(argv)
    if a.selftest:
        return selftest()

    platforms = [p for p in PLATFORMS if p in a.platforms.split(",")]
    comparisons = [c for c in COMPARISONS if c in a.comparisons.split(",")]
    themes = [t for t in THEMES if t in a.themes.split(",")]
    keys = cp.load_keys() if a.examples == "all" else [k.strip() for k in a.examples.split(",") if k.strip()]
    if not (platforms and comparisons and themes and keys):
        raise SystemExit("nothing selected")

    units = [(p, k, c) for p in platforms for k in keys for c in comparisons
             if not (c == "appkit" and PLATFORM_DIR[p] != "maccatalyst")]
    if a.plan:
        for platform, key, comp in units:
            col_a, col_b, slot_fw, structural = COMPARISONS[comp]
            plat = PLATFORM_DIR[platform]
            have = [t for t in themes
                    if capture_file(plat, col_a, key, t, "png") and capture_file(plat, col_b, key, t, "png")]
            print(f"{plat:12} {key:28} {comp:10} {'structural' if structural else 'visual':10} "
                  f"themes={','.join(have) or 'NONE'} -> platforms.{plat}.{cp.review_slot('gemini', slot_fw)}")
        print(f"\n{len(units)} comparison(s) over {len(keys)} page(s), {len(platforms)} platform(s)")
        return 0

    print(f"phase 1: integrity ({len(platforms)} platform(s) x {len(keys)} page(s))", flush=True)
    findings, suspect_cells, missing, tally = phase_integrity(platforms, keys, themes)
    for f in findings:
        print(f"  WRONG-PAGE? {f}", flush=True)
    print(f"phase 2: measurements coverage", flush=True)
    meas = phase_measurements(platforms)
    for m in meas:
        print(f"  gap   {m}", flush=True)

    board = cp.load_comparison()
    by_name = {p.get("name"): p for p in board}
    client = None
    limit_hit = False
    # Why section 3 is incomplete, if it is. Drives the PARTIAL banner and, more importantly, the
    # decision to MERGE rather than replace -- see parse_comparisons().
    partial_reason = ""
    verdicts = []
    print(f"phase 3: {len(units)} comparison(s)", flush=True)
    for platform, key, comp in units:
        col_a, col_b, slot_fw, structural = COMPARISONS[comp]
        plat = PLATFORM_DIR[platform]
        slot = cp.review_slot("gemini", slot_fw)
        if a.resume:
            existing = ((by_name.get(key, {}).get("platforms", {}).get(plat, {}) or {}).get(slot) or {})
            if existing.get("status"):
                continue
        scores = pixel_pair(plat, col_a, col_b, key, themes)
        auto = None if structural else auto_verdict(scores)
        # A byte-identical maui-vs-port pair is an INTEGRITY failure (phase 1) — never auto-green it.
        suspect = (plat, key, themes[0]) in suspect_cells or \
            any((plat, key, t) in suspect_cells for t in themes)
        if auto is not None:
            status, review = auto
            source = "pixel"
        elif a.no_judge:
            # Third value is the motion verdict block; None here, because `scores` holds raw
            # {ssim, diff_pct} pairs with no `verdict` key for it to aggregate. See run_comparison.py's
            # matching call.
            status, review, _motion = pixel_score.classify(scores)
            source = "pixel"
        elif a.limit and client is not None and client.calls >= a.limit:
            # Budget spent. Keep going — the remaining pairs the PIXELS can settle are still free and
            # still worth recording; only the ones needing a call are skipped. Say so once.
            if not limit_hit:
                limit_hit = True
                partial_reason = f"--limit {a.limit} reached, so pairs needing an API call were skipped"
                print(f"  --limit {a.limit} reached: pairs needing a call are skipped from here "
                      f"(re-run with --resume to continue)", flush=True)
            continue
        else:
            if client is None:
                client = Client([a.model] if a.model else gemini.MODELS, a.timeout)
            try:
                status, review = judge_pair(client, plat, col_a, col_b, key, themes, structural)
                source = "gemini"
            except gemini.Quota as q:
                partial_reason = f"the run stopped early: {q}"
                print(f"  STOP: {q} — reporting what is scored so far "
                      f"(section 3 will MERGE, not overwrite)", flush=True)
                break
            except gemini.Failed as e:
                print(f"  ! {plat}/{key}/{comp}: {e}", flush=True)
                continue
        if not structural and scores:
            review = f"{review} [{pixel_note(scores)}]" if pixel_note(scores) else review
        if suspect:
            review = f"[phase 1: this page shares a frame with another page — verify] {review}"
        verdicts.append({"platform": plat, "key": key, "comparison": comp, "status": status,
                         "review": review, "source": source, "slot": slot})
        print(f"  {plat:12} {key:26} {comp:10} {status:6} ({source})", flush=True)

    if a.commit_board:
        wrote = 0
        for v in verdicts:
            fw = COMPARISONS[v["comparison"]][2]
            if cp.write_review(board, v["key"], v["platform"], "gemini", fw, v["status"], v["review"]):
                wrote += 1
        cp.save_comparison(board)
        print(f"wrote {wrote} verdict(s) into comparison.json")

    report = Path(a.report)
    write_report(report, a, findings, missing, tally, meas, verdicts, partial_reason)
    print(f"\nreport -> {report.relative_to(CPP) if report.is_relative_to(CPP) else report}")
    print(f"{len(findings)} wrong-page finding(s), {len(meas)} measurement gap(s), "
          f"{len(verdicts)} comparison(s) scored"
          + (f", {client.calls} Gemini call(s)" if client else ""))
    return len(findings)


if __name__ == "__main__":
    sys.exit(main())
