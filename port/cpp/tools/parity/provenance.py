#!/usr/bin/env python3
"""Is the board's number trustworthy? — a one-command answer.

WHY THIS EXISTS. On 2026-08-13 the android lane fell from 262 green to 197 the moment every one of its
cells was measured from a SINGLE capture run against HEAD. Nothing regressed; the 65 greens had never
been earned. They came from two habits this board makes easy and never flagged:

  1. STALE MOTION PROVENANCE. A rebuild refreshes the still captures. MOTION frames live in gitignored
     run directories, so a cell can score its animation against a binary that no longer exists — 70
     android cells were being scored against a commit that had been REVERTED.
  2. BEST-OF-N DRIFT. A board stitched from many PARTIAL runs keeps, per cell, whichever run's picture
     landed last. That total is the best of N runs, which no single binary can reproduce.

Both are invisible in the headline number, which is exactly what makes them expensive: you cannot
subtract from 172 with any confidence if you do not know how many binaries the row was measured
against. This prints that, per lane, so "we have N left to fix" becomes a statement you can defend.

READ IT LIKE THIS:
  runs = 1        the lane is measured from one binary. The number means what it says.
  runs = 2-3      mostly consolidated; treat the total as approximate.
  runs > 3        an UPPER BOUND, not a measurement. Consolidate before planning against it.

Consolidating is one command per lane:
    python3 tools/parity/recapture.py --platforms <lane> --themes light,dark
and then verify the run actually happened (exit 0 is not evidence): a NEW run dir must appear under
docs/comparison/ AND comparison.json must change. A pass that moves the board >5% with no code change
is a failed run, not a measurement.

Usage:  python3 tools/parity/provenance.py [--json]
"""
import collections
import json
import os
import re
import sys

COMP = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..", "docs", "comparison")
BOARD = os.path.normpath(os.path.join(COMP, "comparison.json"))
LANES = ("android", "ios", "maccatalyst", "windows")
COLUMNS = ("pixel", "pixel_xaml")
# A lane measured from more runs than this is reported as an upper bound rather than a count. 3 is not
# a tuned threshold — it is "one run, plus a re-drive or two", which is the most a lane can accumulate
# while still describing a single binary.
TRUSTED_RUNS = 3


def scan(board_path=BOARD):
    """-> {lane: {"green":int, "total":int, "runs":Counter}} straight from the committed board."""
    with open(board_path, encoding="utf-8") as handle:
        rows = json.load(handle)
    out = {lane: {"green": 0, "total": 0, "runs": collections.Counter()} for lane in LANES}
    for row in rows:
        for lane, platform in (row.get("platforms") or {}).items():
            if lane not in out:
                continue
            for column in COLUMNS:
                cell = platform.get(column)
                if not cell or not cell.get("status"):
                    continue
                out[lane]["total"] += 1
                out[lane]["green"] += cell["status"] == "green"
                # The motion block names the run its FRAMES came from. Cells with no motion block are
                # still-only and carry no separate provenance, so they are not counted against the lane.
                found = re.search(r"run (\d{4}-\d{2}-\d{2}-\d{2}_\d{2}_\d{2})", cell.get("review", ""))
                if found:
                    out[lane]["runs"][found.group(1)] += 1
    return out


def main() -> int:
    data = scan()
    if "--json" in sys.argv:
        print(json.dumps({k: {"green": v["green"], "total": v["total"],
                              "runs": dict(v["runs"])} for k, v in data.items()}, indent=2))
        return 0

    print(f"{'lane':13s} {'green':>9s}  {'runs':>4s}  verdict")
    print("-" * 62)
    suspect = []
    for lane in LANES:
        entry = data[lane]
        runs = len(entry["runs"])
        if runs == 0:
            verdict = "no motion cells"
        elif runs == 1:
            verdict = "TRUSTWORTHY — one binary"
        elif runs <= TRUSTED_RUNS:
            verdict = "approximate"
        else:
            oldest = min(entry["runs"]) if entry["runs"] else "?"
            verdict = f"UPPER BOUND — spans {runs} runs, oldest {oldest[:10]}"
            suspect.append(lane)
        print(f"{lane:13s} {entry['green']:4d}/{entry['total']:<4d} {runs:>4d}  {verdict}")

    if suspect:
        print()
        print("These lanes are NOT a count you can subtract from 172. Consolidate each with:")
        for lane in suspect:
            print(f"    python3 tools/parity/recapture.py --platforms {lane} --themes light,dark")
        print("Then confirm a NEW run dir appeared and comparison.json changed — exit 0 is not evidence.")
    return 1 if suspect else 0


if __name__ == "__main__":
    raise SystemExit(main())
