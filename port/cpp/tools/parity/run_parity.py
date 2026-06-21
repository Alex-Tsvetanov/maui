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

# Quota-aware model cascade (model:rpm), best-quality-first. Each full-flash model has only ~20 RPD,
# so they're spent first (best bucketing on the early pages), then the 500-RPD gemini-3.1-flash-lite
# carries the bulk, then 2.5-flash-lite. On a model's quota (HTTP 429) the driver rotates to the next
# model and RETRIES the page; only when ALL are exhausted does it fall back to Claude. rpm sets the
# inter-call pace (60/rpm). Override with --models "m1:rpm1,m2:rpm2,...".
# NOTE: every entry must be a REAL API model id (verify via the ListModels endpoint). 'gemini-3-flash'
# (no -preview suffix) returns 404 and previously killed the sweep — rotate-on-404 now guards that, but the
# cascade is kept to ids confirmed present for this key. gemini-3.1-flash-lite is the 500-RPD workhorse that
# clears a full 172-page sweep; the 20-RPD premiums precede it (best bucketing first) and 429→rotate.
DEFAULT_CASCADE = "gemini-3.5-flash:5,gemini-2.5-flash:5,gemini-3.1-flash-lite:15,gemini-2.5-flash-lite:10,gemini-2.0-flash-lite:30"


def parse_cascade(spec: str) -> list[tuple[str, float]]:
    out = []
    for part in spec.split(","):
        part = part.strip()
        if not part:
            continue
        if ":" in part:
            name, rpm = part.rsplit(":", 1)
            out.append((name.strip(), float(rpm)))
        else:
            out.append((part, 5.0))  # assume conservative 5 RPM if unspecified
    return out


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
    ap.add_argument("--model", default=None,
                    help="use ONLY this model (disables the cascade); e.g. gemini-3.1-flash-lite")
    ap.add_argument("--models", default=DEFAULT_CASCADE,
                    help='quota-aware cascade "m1:rpm1,m2:rpm2,..." (best-first); ignored if --model is set')
    ap.add_argument("--commit-board", action="store_true",
                    help="ALSO merge light/dark verdicts into parity_status.json (default: review-only, non-destructive)")
    ap.add_argument("--gen-readme", action="store_true", help="with --commit-board, also regenerate README.md")
    ap.add_argument("--limit", type=int, default=0, help="cap number of pages (0 = no cap)")
    ap.add_argument("--delay", type=float, default=0.0,
                    help="minimum seconds between calls (in addition to the per-model 60/rpm pace)")
    args = ap.parse_args()

    cascade = [(args.model, 15.0)] if args.model else parse_cascade(args.models)

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
    log("  cascade: " + " -> ".join(f"{m}({int(r)}rpm)" for m, r in cascade))
    verdicts, judged, errored, missing, fallback = [], [], [], [], []
    quota_hit = False
    mi = 0          # index into the cascade of the model currently in use
    last_call = 0.0  # time.monotonic() of the previous API call, for pacing

    for i, key in enumerate(keys):
        rc, verdict = None, None
        # Try the current model; on quota, rotate to the next model and retry the SAME page.
        while mi < len(cascade):
            model, rpm = cascade[mi]
            interval = max(args.delay, (60.0 / rpm * 1.05) if rpm > 0 else 0.0)
            wait = interval - (time.monotonic() - last_call)
            if last_call and wait > 0:
                time.sleep(wait)
            rc, verdict = run_one(key, args.root, model)
            last_call = time.monotonic()
            if rc == EXIT_QUOTA:
                log(f"  {model} quota exhausted -> cascading to next model")
                mi += 1
                continue
            break
        if mi >= len(cascade):
            quota_hit = True
            fallback = keys[i:]  # all models spent: this page + the rest -> Claude fallback
            log(f"  ALL Gemini models exhausted at '{key}' -> {len(fallback)} page(s) to Claude fallback")
            break
        if rc == 0 and verdict:
            verdict["severity"] = severity_of(verdict["light"], verdict["dark"])
            verdicts.append(verdict)
            judged.append(key)
            nq, nd = len(verdict.get("maui_quirks", [])), len(verdict.get("port_diffs", []))
            log(f"  [{i + 1}/{len(keys)}] {key}: L={verdict['light']} D={verdict['dark']} "
                f"(quirks={nq} diffs={nd}) [{verdict.get('model', cascade[mi][0])}]")
            if args.commit_board:
                entry = status.get(key, {})
                entry.update({
                    "light": verdict["light"], "dark": verdict["dark"],
                    "light_note": verdict.get("light_note", ""), "dark_note": verdict.get("dark_note", ""),
                    "severity": verdict["severity"],
                })
                status[key] = entry
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
