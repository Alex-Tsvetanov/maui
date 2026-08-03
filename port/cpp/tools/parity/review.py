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

  1. INTEGRITY (free).  Hashes the capture tree. A `maui` frame byte-identical to a `cpp`/`xaml` frame
     is NOT parity — it is one column's capture banked under another column's name (the runner's
     `or bounds` failure mode), and it is reported as a FATAL inconsistency. cpp==xaml is expected and
     merely counted. Missing captures are listed (builder columns are exempt on non-twin pages).
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

EXIT CODE = the number of FATAL integrity inconsistencies (0 = the capture tree is self-consistent).
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
    """Hash every capture; return (fatal findings, the exact cells they hit, missing cells, tally).

    The second value is keyed on (platform, key, port column) rather than re-parsed out of the
    formatted findings: matching those by string prefix silently blanks every SHORTER key in a family
    (`border` would inherit `border_playground`'s failure).
    """
    twins = twin_keys()
    fatal: list[str] = []
    fatal_pairs: set[tuple[str, str, str]] = set()
    missing: list[str] = []
    tally: Counter = Counter()
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
                for port_col in ("cpp", "xaml"):
                    if digests.get("maui") and digests.get("maui") == digests.get(port_col):
                        # Byte-identical across DIFFERENT frameworks is impossible from rendering; it
                        # means one column's frame was banked under another column's name.
                        fatal.append(f"{plat}/{key}_{theme}: maui and {port_col} captures are "
                                     f"BYTE-IDENTICAL — one column's frame is filed under the other")
                        fatal_pairs.add((plat, key, port_col))
                if digests.get("cpp") and digests.get("cpp") == digests.get("xaml"):
                    tally["cpp==xaml (expected)"] += 1
    return fatal, fatal_pairs, missing, tally


# --------------------------------------------------------------------------- measurements
SIZE_ENV = {"macos-arm64": ("maccatalyst", ("maui_xaml", "cpp", "cpp_xaml")),
            "macos-appkit": ("maccatalyst", ("appkit_cpp", "appkit_xaml")),
            "windows-x64": ("windows", ("maui_xaml", "cpp", "cpp_xaml"))}


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
def write_report(path: Path, args, fatal, missing, tally, meas, verdicts) -> None:
    lines = [f"# Parity inconsistencies — {datetime.now():%Y-%m-%d %H:%M}", "",
             f"`review.py --platforms {args.platforms} --comparisons {args.comparisons}"
             f"{' --no-judge' if args.no_judge else ''}`", "",
             "## 1. Capture integrity", ""]
    if fatal:
        lines += [f"**{len(fatal)} FATAL** — a MAUI frame byte-identical to a port frame is a "
                  "mis-filed capture, not parity:", ""]
        lines += [f"- {f}" for f in fatal] + [""]
    else:
        lines += ["No cross-framework byte-identical captures. ✅", ""]
    lines += [f"- captures hashed: {sum(v for k, v in tally.items() if '/' in k)}",
              f"- cpp==xaml cells (expected, same renderer): {tally.get('cpp==xaml (expected)', 0)}",
              f"- missing cells: {len(missing)}", ""]
    if missing:
        lines += ["<details><summary>missing cells</summary>", ""]
        lines += [f"- {m}" for m in missing[:200]]
        if len(missing) > 200:
            lines.append(f"- …and {len(missing) - 200} more")
        lines += ["", "</details>", ""]

    lines += ["## 2. Measurements coverage", ""]
    lines += ([f"- {m}" for m in meas] if meas else ["Every captured platform/column is measured. ✅"])
    lines += ["", "## 3. Comparisons", "",
              "| platform | page | comparison | status | source | detail |",
              "| --- | --- | --- | --- | --- | --- |"]
    for v in verdicts:
        detail = v["review"].replace("|", "/")[:160]
        lines.append(f"| {v['platform']} | {v['key']} | {v['comparison']} | {v['status']} | "
                     f"{v['source']} | {detail} |")
    counts = Counter((v["comparison"], v["status"]) for v in verdicts)
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
    print("review selftest: slots + appkit scoping + pixel prefilter OK")
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
    fatal, fatal_pairs, missing, tally = phase_integrity(platforms, keys, themes)
    for f in fatal:
        print(f"  FATAL {f}", flush=True)
    print(f"phase 2: measurements coverage", flush=True)
    meas = phase_measurements(platforms)
    for m in meas:
        print(f"  gap   {m}", flush=True)

    board = cp.load_comparison()
    by_name = {p.get("name"): p for p in board}
    client = None
    limit_hit = False
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
        if col_a == "maui" and (plat, key, col_b) in fatal_pairs:
            status, review, source = "blank", "capture integrity failure — see phase 1", "integrity"
        elif auto is not None:
            status, review = auto
            source = "pixel"
        elif a.no_judge:
            status, review = pixel_score.classify(scores)
            source = "pixel"
        elif a.limit and client is not None and client.calls >= a.limit:
            # Budget spent. Keep going — the remaining pairs the PIXELS can settle are still free and
            # still worth recording; only the ones needing a call are skipped. Say so once.
            if not limit_hit:
                limit_hit = True
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
                print(f"  STOP: {q} — reporting what is scored so far", flush=True)
                break
            except gemini.Failed as e:
                print(f"  ! {plat}/{key}/{comp}: {e}", flush=True)
                continue
        if not structural and scores:
            review = f"{review} [{pixel_note(scores)}]" if pixel_note(scores) else review
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
    write_report(report, a, fatal, missing, tally, meas, verdicts)
    print(f"\nreport -> {report.relative_to(CPP) if report.is_relative_to(CPP) else report}")
    print(f"{len(fatal)} fatal integrity finding(s), {len(meas)} measurement gap(s), "
          f"{len(verdicts)} comparison(s) scored"
          + (f", {client.calls} Gemini call(s)" if client else ""))
    return len(fatal)


if __name__ == "__main__":
    sys.exit(main())
