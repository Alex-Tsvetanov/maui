#!/usr/bin/env python3
"""Promote FRESH MAUI reference captures into the board's maui column.

THE GAP THIS FILLS
------------------
The reference captures live in TWO roots and nothing connected them:

  port/maui-reference/captures/<platform>/<key>_<theme>.png    written by capture_ios_clean.py --app maui
                                                               and friends — the FRESH ground truth
  docs/comparison/captures/<platform>/maui/<key>_<theme>.png   what build_comparison_json.py and
                                                               pixel_score.py actually read

Every capture tool writes the first; every scoring tool reads the second; no script copied between
them. Measured 2026-08-03: a full iOS reference sweep had just written 364 frames, and the board's
column was still the 2-day-old set — so the port would have been scored against a reference captured
under a DIFFERENT theme mechanism, and the resulting reds would have been pure fiction.

port/CLAUDE.md ruling 6 says captures/*/maui/ is never hand-written — only an importer or an explicit
reference capture may write it. This is that importer.

WHAT IT CHECKS
--------------
Copying is the easy part; refusing to copy garbage is the point.
  * refuses to run when the source root has NO frames (the silent-no-op failure that has bitten this
    tree twice — capture_appkit's empty glob and import_run_captures' maui_xaml-only tag scan)
  * refuses a frame that is a .NET startup SPLASH (capture_guard) — that is a capture of the wrong
    screen, not a degraded one, and it scores as an enormous port defect
  * refuses a frame that is blank/flat
  * reports how many frames actually differ, so "promoted 364" never hides "changed 0"

Usage:
    promote_reference_captures.py --platform ios [--dry-run]
"""
from __future__ import annotations

import argparse
import filecmp
import shutil
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
CPP = HERE.parents[2]                         # port/cpp
PORT = CPP.parent                             # port
sys.path.insert(0, str(HERE))
from capture_guard import splash_verdict      # noqa: E402


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--platform", required=True, help="ios | android | maccatalyst | windows")
    ap.add_argument("--dry-run", action="store_true")
    a = ap.parse_args()

    src_dir = PORT / "maui-reference" / "captures" / a.platform
    dst_dir = CPP / "docs" / "comparison" / "captures" / a.platform / "maui"
    if not src_dir.is_dir():
        raise SystemExit(f"no reference capture root at {src_dir}")
    frames = sorted(src_dir.glob("*.png"))
    if not frames:
        raise SystemExit(f"{src_dir} holds NO frames — refusing to report success having promoted "
                         f"nothing (run the reference capture first)")

    rejected, changed, same = [], 0, 0
    for src in frames:
        is_splash, frac, dom = splash_verdict(str(src))
        if is_splash:
            rejected.append(f"{src.name}: .NET SPLASH ({dom} at {frac * 100:.0f}%)")
            continue
        dst = dst_dir / src.name
        if dst.exists() and filecmp.cmp(src, dst, shallow=False):
            same += 1
            continue
        changed += 1
        if not a.dry_run:
            dst_dir.mkdir(parents=True, exist_ok=True)
            shutil.copyfile(src, dst)

    verb = "would promote" if a.dry_run else "promoted"
    print(f"{a.platform}: {len(frames)} reference frame(s); {verb} {changed}, unchanged {same}, "
          f"rejected {len(rejected)}")
    for r in rejected:
        print(f"  ! REJECTED {r}")
    if rejected:
        print("  (rejected frames were NOT copied — re-capture those pages; a splash frame scores as a "
              "huge port defect on a page the port may render perfectly)")
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
