#!/usr/bin/env python3
"""Render PARITY_REVIEW.md from parity_review.json — a human-verification doc.

This MIRRORS docs/comparison/README.md (same 4-image grid, same page order) but is a SEPARATE,
non-authoritative file: it shows Gemini's independent verdicts split into two buckets per page —
**MAUI quirks** (MAUI-side imperfections, subject to a human ruling) and **Port diffs** (genuine
C++ issues to fix). You verify this BEFORE any verdict touches the tracked board or any fix is made.

Written to docs/comparison/PARITY_REVIEW.md so the relative image links resolve like the README.
"""
from __future__ import annotations

import argparse
import importlib.util
import json
import os

HERE = os.path.dirname(os.path.abspath(__file__))
CMP_ROOT = os.path.normpath(os.path.join(HERE, "..", "..", "docs", "comparison"))
REVIEW_JSON = os.path.join(CMP_ROOT, "parity_review.json")
OUT_MD = os.path.join(CMP_ROOT, "PARITY_REVIEW.md")
GEN_README = os.path.join(CMP_ROOT, "gen_parity_readme.py")

EMOJI = {"match": "🟢", "minor": "🟡", "diff": "🔴", "cpp_blank": "⬛", "cs_blank": "⬜"}

# Keyword buckets for aggregating MAUI quirks into discussion categories.
# (Colors are NOT a quirk category — per ruling, MAUI's render is the color source of truth, so color
#  deltas are port_diffs to fix, not MAUI imperfections.)
QUIRK_CATEGORIES = [
    ("Whole-screen padding / margins", ("inset", "padding", "margin", "edge-to-edge", "gap to", "screen edge")),
    ("Top/bottom cropping", ("top", "bottom", "crop", "cut", "shorter", "more of the page")),
    ("Harness chrome (card / nav / status bar)", ("card", "harness", "navigation", "nav bar", "status bar", "notch", "clock", "battery")),
]


def load_genmod():
    spec = importlib.util.spec_from_file_location("gen_parity_readme", GEN_README)
    mod = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(mod)
    return mod


def categorize(line: str) -> str:
    low = line.lower()
    for name, kws in QUIRK_CATEGORIES:
        if any(kw in low for kw in kws):
            return name
    return "Other / uncategorised"


def main() -> None:
    ap = argparse.ArgumentParser()
    ap.add_argument("--review", default=REVIEW_JSON)
    ap.add_argument("--out", default=OUT_MD)
    args = ap.parse_args()

    with open(args.review, "r", encoding="utf-8") as fh:
        data = json.load(fh)
    verdicts = {v["key"]: v for v in data.get("verdicts", [])}
    summary = data.get("summary", {})

    gen = load_genmod()
    fix_order = [k for k in gen.FIX_ORDER if k in verdicts]
    rest = sorted(k for k in verdicts if k not in set(fix_order))
    order = fix_order + rest

    # Aggregate quirks for the discussion section.
    cats: dict[str, list[tuple[str, str]]] = {}
    n_port = 0
    for k in order:
        v = verdicts[k]
        for q in v.get("maui_quirks", []):
            cats.setdefault(categorize(q), []).append((k, q))
        if v.get("port_diffs"):
            n_port += 1

    o: list[str] = []
    o.append("# C++ port vs .NET MAUI — parity REVIEW (Gemini-judged, for verification)")
    o.append("")
    o.append("> **Not authoritative.** Independent Google-Gemini vision pass. These verdicts are NOT in the tracked "
             "board (`parity_status.json` / `README.md`) — verify them here first.")
    o.append(">")
    o.append("> Each page's differences are split into two buckets: **🛠 Port diffs** (genuine C++ issues to fix) and "
             "**🧩 MAUI quirks** (MAUI-side imperfections — *subject to your ruling*; they do NOT drive the verdict). "
             "Rule on the quirk categories below; rulings get recorded in the comparison policy so the loop stops "
             "re-litigating them.")
    o.append("")
    o.append(f"**Pages judged: {summary.get('judged', len(order))}** · {n_port} with port diffs · "
             f"{sum(len(x) for x in cats.values())} MAUI-quirk notes across {len(cats)} categories.")
    if summary.get("fallback"):
        o.append("")
        o.append(f"**Deferred to Claude fallback ({len(summary['fallback'])}):** "
                 + ", ".join(summary["fallback"][:30]) + (" …" if len(summary["fallback"]) > 30 else ""))
    o.append("")

    # --- MAUI imperfection categories (the discussion agenda) ---
    o.append("## MAUI imperfection categories seen — RULE ON EACH")
    o.append("")
    o.append("For each category decide: **ignore** (MAUI imperfection, port need not match) · **match** (port should "
             "replicate it) · **case-by-case**. Rulings → comparison policy in `port/CLAUDE.md` / `port/PROJECT.md`.")
    o.append("")
    for name, _ in QUIRK_CATEGORIES + [("Other / uncategorised", ())]:
        items = cats.get(name)
        if not items:
            continue
        o.append(f"### {name}  ({len(items)} page(s)) — _ruling: TBD_")
        for k, q in items[:40]:
            o.append(f"- **{gen.title(k)}**: {q}")
        if len(items) > 40:
            o.append(f"- … and {len(items) - 40} more")
        o.append("")

    # --- Per-page review (mirrors README.md grid order) ---
    o.append("## Per-page review")
    o.append("")
    for i, k in enumerate(order, 1):
        v = verdicts[k]
        lt, dk = v["light"], v["dark"]
        o.append(f"### {i}. {gen.title(k)} — {EMOJI.get(lt, '?')} L:{lt} · {EMOJI.get(dk, '?')} D:{dk}")
        port = v.get("port_diffs", [])
        quirks = v.get("maui_quirks", [])
        if port:
            o.append("**🛠 Port diffs (fix):**")
            o.extend(f"- {d}" for d in port)
        else:
            o.append("**🛠 Port diffs (fix):** _none — " + (v.get("light_note") or "identical") + "_")
        if quirks:
            o.append("")
            o.append("**🧩 MAUI quirks (discuss):**")
            o.extend(f"- {q}" for q in quirks)
        o.append("")
        o.append("| MAUI (light) | C++ (light) | MAUI (dark) | C++ (dark) |")
        o.append("| --- | --- | --- | --- |")
        o.append(f"| ![](csharp_ios_light/{k}.png) | ![](cpp_ios_light/{k}.png) "
                 f"| ![](csharp_ios_dark/{k}.png) | ![](cpp_ios_dark/{k}.png) |")
        o.append("")

    with open(args.out, "w", encoding="utf-8") as fh:
        fh.write("\n".join(o))
    print(f"Wrote {os.path.relpath(args.out)} ({len(order)} pages)")


if __name__ == "__main__":
    main()
