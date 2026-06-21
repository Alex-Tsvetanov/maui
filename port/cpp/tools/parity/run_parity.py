#!/usr/bin/env python3
"""Batch parity sweep — the DEFAULT iOS parity comparator (Gemini), with Claude fallback.

Runs `gemini_compare.py` for each requested page, merges the verdicts into
`docs/comparison/parity_status.json` (preserving any extra fields like `severity`/`flags`),
and reports which pages still need Claude's own vision.

Fallback contract (this is the whole point):
  * When Gemini hits a quota / rate limit on any page, that page's `gemini_compare.py`
    exits 75. This driver STOPS the Gemini sweep at that point, and writes EVERY remaining
    (not-yet-judged) page — including the one that hit quota — to `parity_fallback.json`.
    The loop agent then judges that fallback set with its OWN vision (the
    `parity_assess_wf.js` workflow or direct image reads) and re-runs the README generator.
  * Pages whose images are missing (exit 2) or that hard-error (exit 1) are recorded
    separately and also surfaced for fallback; they don't stop the sweep.

A machine-readable summary is printed as the LAST stdout line (JSON), so a workflow/agent
capturing stdout can parse the fallback set directly.

Usage:
  run_parity.py --all                 # every page in parity_status.json (FIX_ORDER first)
  run_parity.py button label grid     # specific pages
  run_parity.py --all --gen-readme    # also regenerate README.md afterwards
"""
from __future__ import annotations

import argparse
import json
import os
import subprocess
import sys
import time

HERE = os.path.dirname(os.path.abspath(__file__))
# Per-page comparator; overridable so a different backend can be swapped in (and for tests).
COMPARE = os.environ.get("PARITY_COMPARE_BIN", os.path.join(HERE, "gemini_compare.py"))
CMP_ROOT = os.path.normpath(os.path.join(HERE, "..", "..", "docs", "comparison"))
STATUS_PATH = os.path.join(CMP_ROOT, "parity_status.json")
FALLBACK_PATH = os.path.join(CMP_ROOT, "parity_fallback.json")
REVIEW_PATH = os.path.join(CMP_ROOT, "parity_review.json")
GEN_README = os.path.join(CMP_ROOT, "gen_parity_readme.py")
GEN_REVIEW = os.path.join(HERE, "gen_review_md.py")

EXIT_QUOTA = 75
EXIT_MISSING = 2

SEV_RANK = {"match": 0, "minor": 1, "diff": 2, "cpp_blank": 3, "cs_blank": 3}


def log(msg: str) -> None:
    print(msg, file=sys.stderr, flush=True)


def severity_of(light: str, dark: str) -> str:
    worst = light if SEV_RANK.get(light, 0) >= SEV_RANK.get(dark, 0) else dark
    return "diff" if worst in ("cpp_blank", "cs_blank") else worst


def load_status(path: str) -> dict:
    with open(path, "r", encoding="utf-8") as fh:
        return json.load(fh)


def write_status(status: dict, path: str) -> None:
    with open(path, "w", encoding="utf-8") as fh:
        json.dump(status, fh, indent=2, sort_keys=True)  # matches existing format (no trailing nl)


def ordered_keys(status: dict) -> list[str]:
    """All status keys, FIX_ORDER pages first (so a quota stop covers the high-value tail)."""
    try:
        import importlib.util
        spec = importlib.util.spec_from_file_location("gen_parity_readme", GEN_README)
        mod = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(mod)
        fix = [k for k in getattr(mod, "FIX_ORDER", []) if k in status]
        rest = sorted(k for k in status if k not in set(fix))
        return fix + rest
    except Exception:
        return sorted(status)


def run_one(key: str, root: str, model: str | None) -> tuple[int, dict | None]:
    env = dict(os.environ)
    if model:
        env["GEMINI_MODEL"] = model
    proc = subprocess.run(
        [sys.executable, COMPARE, key, "--root", root],
        capture_output=True, text=True, env=env,
    )
    if proc.returncode == 0:
        try:
            return 0, json.loads(proc.stdout.strip())
        except json.JSONDecodeError:
            log(f"  {key}: exit 0 but unparseable stdout: {proc.stdout[:200]}")
            return 1, None
    # surface the child's stderr (quota detail, missing-image paths, etc.)
    if proc.stderr.strip():
        log(f"  {key}: {proc.stderr.strip().splitlines()[0]}")
    return proc.returncode, None


def main() -> None:
    ap = argparse.ArgumentParser(description="Batch Gemini parity sweep with Claude fallback.")
    ap.add_argument("keys", nargs="*", help="page keys; omit with --all")
    ap.add_argument("--all", action="store_true", help="sweep every page in parity_status.json")
    ap.add_argument("--root", default=CMP_ROOT)
    ap.add_argument("--status", default=STATUS_PATH, help="parity_status.json (only touched with --commit-board)")
    ap.add_argument("--review", default=REVIEW_PATH, help="review JSON output (verdicts + buckets)")
    ap.add_argument("--model", default=None, help="override $GEMINI_MODEL")
    ap.add_argument("--commit-board", action="store_true",
                    help="ALSO merge light/dark verdicts into parity_status.json (default: review-only, non-destructive)")
    ap.add_argument("--gen-readme", action="store_true", help="with --commit-board, also regenerate README.md")
    ap.add_argument("--limit", type=int, default=0, help="cap number of pages (0 = no cap)")
    ap.add_argument("--delay", type=float, default=0.0,
                    help="seconds to sleep between pages (use ~5 for full sweeps to stay under the free-tier RPM)")
    args = ap.parse_args()

    status = load_status(args.status)
    if args.all:
        keys = ordered_keys(status)
    elif args.keys:
        keys = args.keys
    else:
        ap.error("provide page keys or --all")
    if args.limit:
        keys = keys[: args.limit]

    log(f"Gemini parity sweep: {len(keys)} page(s)  ({'COMMIT to board' if args.commit_board else 'review-only'})")
    verdicts, judged, errored, missing, fallback = [], [], [], [], []
    quota_hit = False

    for i, key in enumerate(keys):
        if args.delay and i:
            time.sleep(args.delay)
        rc, verdict = run_one(key, args.root, args.model)
        if rc == 0 and verdict:
            verdict["severity"] = severity_of(verdict["light"], verdict["dark"])
            verdicts.append(verdict)
            judged.append(key)
            nq, nd = len(verdict.get("maui_quirks", [])), len(verdict.get("port_diffs", []))
            log(f"  [{i + 1}/{len(keys)}] {key}: L={verdict['light']} D={verdict['dark']} "
                f"(maui_quirks={nq} port_diffs={nd})")
            if args.commit_board:
                entry = status.get(key, {})
                entry.update({
                    "light": verdict["light"], "dark": verdict["dark"],
                    "light_note": verdict.get("light_note", ""), "dark_note": verdict.get("dark_note", ""),
                    "severity": verdict["severity"],
                })
                status[key] = entry
        elif rc == EXIT_QUOTA:
            quota_hit = True
            fallback = keys[i:]  # this page + everything not yet judged -> Claude fallback
            log(f"  QUOTA hit at '{key}' -> {len(fallback)} page(s) deferred to Claude fallback")
            break
        elif rc == EXIT_MISSING:
            missing.append(key)
        else:
            errored.append(key)

    # Everything Gemini couldn't judge is the fallback set the loop should hand to Claude.
    fallback_set = list(dict.fromkeys(fallback + errored + missing))
    summary = {
        "judged": len(judged),
        "errored": errored,
        "missing": missing,
        "quota_hit": quota_hit,
        "fallback": fallback_set,
    }

    # Always write the non-destructive review artifacts (verdicts + buckets) + the fallback list.
    with open(args.review, "w", encoding="utf-8") as fh:
        json.dump({"summary": summary, "verdicts": verdicts}, fh, indent=2)
    with open(FALLBACK_PATH, "w", encoding="utf-8") as fh:
        json.dump(summary, fh, indent=2)
    log(f"Wrote review verdicts -> {os.path.relpath(args.review)}")
    # Render the human-review markdown (mirrors README.md) from the review JSON.
    subprocess.run([sys.executable, GEN_REVIEW, "--review", args.review], check=False)

    if args.commit_board:
        write_status(status, args.status)
        log(f"Committed {len(judged)} verdict(s) to {os.path.relpath(args.status)}")
        if args.gen_readme:
            log("Regenerating README.md ...")
            subprocess.run([sys.executable, GEN_README], check=False)

    log(f"Done. judged={len(judged)} errored={len(errored)} missing={len(missing)} "
        f"quota_hit={quota_hit} fallback={len(fallback_set)}")
    if fallback_set:
        log("Fallback to Claude vision for: " + " ".join(fallback_set[:20])
            + (" ..." if len(fallback_set) > 20 else ""))
    # LAST stdout line = machine-readable summary for a capturing workflow/agent.
    print(json.dumps(summary))


if __name__ == "__main__":
    main()
