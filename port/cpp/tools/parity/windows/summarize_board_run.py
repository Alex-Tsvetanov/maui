#!/usr/bin/env python3
"""Summarize a Windows board run into committed data for the README generator.

The runner's own output lives in a dated, GITIGNORED directory, so nothing about a run survives into
the repo by itself. This distils one run into `docs/comparison/windows_pipeline.json`, which
`tools/gen_readme.py` renders as the README's Windows section -- keeping the README fully generated
("do not edit by hand") rather than hand-patched and silently clobbered on the next regeneration.

WHAT IT DELIBERATELY DOES NOT DO: produce parity verdicts. The Windows column is currently the mingw
Win32 SMOKE app, which paints the same layout for every page (only the title and a page-hash-derived
swatch row change) and is not the gallery. There is also no MAUI Windows reference column to compare
against. So this records PIPELINE evidence -- did every page launch, present at the exact geometry, and
capture a decodable, correctly-sized, page-distinct PNG -- and says so in the data, so the section can
never read as a fidelity result.

The integrity metrics are chosen to catch the failure this project has been bitten by repeatedly: a
capture that looks perfectly legitimate but is the wrong thing. Hence per-page distinctness (a repeated
image means the page env never took effect, or a stale window was photographed) and exact-size checks
(a short frame means present silently failed).

Usage:
    summarize_board_run.py <run-dir> [--out docs/comparison/windows_pipeline.json]
    summarize_board_run.py <run-dir> --print      # inspect without writing
"""
from __future__ import annotations

import argparse
import collections
import hashlib
import json
import os
import sys
from datetime import datetime, timezone
from pathlib import Path

HERE = Path(__file__).resolve().parent
CPP_ROOT = HERE.parents[2]
DEFAULT_OUT = CPP_ROOT / "docs/comparison/windows_pipeline.json"


def summarize(run_dir: Path) -> dict:
    manifest_path = run_dir / "run-manifest.json"
    summary_path = run_dir / "summary.json"
    manifest = json.loads(manifest_path.read_text()) if manifest_path.is_file() else {}
    summary = json.loads(summary_path.read_text()) if summary_path.is_file() else {}

    frames = sorted(run_dir.glob("*/windows/*/*.png"))
    by_page: dict[str, list[Path]] = collections.defaultdict(list)
    for f in frames:
        by_page[f.parts[-4]].append(f)

    sizes: collections.Counter = collections.Counter()
    digests: dict[str, list[str]] = collections.defaultdict(list)
    themes: collections.Counter = collections.Counter()
    steps: collections.Counter = collections.Counter()
    bounds_seen: collections.Counter = collections.Counter()
    undecodable: list[str] = []

    for f in frames:
        # Read the size from the PNG's IHDR directly -- no Pillow dependency, and it also proves the
        # header is well formed (the agent writes PNG bytes itself, so a malformed chunk is possible).
        b = f.read_bytes()
        if not b.startswith(b"\x89PNG\r\n\x1a\n") or b[12:16] != b"IHDR":
            undecodable.append(str(f.relative_to(run_dir)))
            continue
        w = int.from_bytes(b[16:20], "big")
        h = int.from_bytes(b[20:24], "big")
        sizes[f"{w}x{h}"] += 1
        digests[hashlib.sha256(b).hexdigest()].append(str(f.relative_to(run_dir)))
        side = f.with_suffix(".json")
        if side.is_file():
            s = json.loads(side.read_text())
            themes[s.get("theme", "?")] += 1
            steps[s.get("step", "?")] += 1
            wb = s.get("window_bounds")
            bounds_seen[json.dumps(wb)] += 1

    # Duplicate GROUPS matter, not duplicate files: two frames of the same page+theme that differ only
    # by a scenario step SHOULD be identical when the app does not react to input, so a raw duplicate
    # count would look alarming for a correct run. Split same-page from cross-page.
    dup_same_page = 0
    dup_cross_page = []
    for _dig, paths in digests.items():
        if len(paths) < 2:
            continue
        pages = {p.split("/")[0] for p in paths}
        if len(pages) == 1:
            dup_same_page += len(paths) - 1
        else:
            dup_cross_page.append(sorted(paths))

    return {
        "kind": "pipeline-validation",   # NOT a parity result; see the module docstring
        "generated_at": datetime.now(timezone.utc).replace(microsecond=0).isoformat(),
        "run_dir": run_dir.name,
        "run_timestamp": manifest.get("timestamp"),
        "commit": manifest.get("commit"),
        "tags_requested": len(manifest.get("tags") or []),
        "pages_captured": len(by_page),
        "frames_total": len(frames),
        "frames_undecodable": undecodable,
        "dropped_frames": (summary.get("windows-x64") or {}).get("dropped_frames") or [],
        "sizes": dict(sizes),
        "themes": dict(themes),
        "steps": dict(steps),
        "window_bounds": dict(bounds_seen),
        "unique_images": len(digests),
        "duplicate_frames_same_page": dup_same_page,
        "duplicate_frames_cross_page": dup_cross_page,
        "column": "cpp_smoke",
        "caveats": [
            "The Windows column is the mingw-cross Win32 SMOKE app, not the gallery: it paints the same "
            "layout for every page, varying only the title and a page-hash-derived swatch row. These "
            "frames therefore say nothing about MAUI visual fidelity.",
            "There is no MAUI Windows reference column yet, so no scores are computed -- the runner "
            "skips scoring when maui_xaml is absent.",
            "MAUI's Windows backend is WinUI 3 (Microsoft.UI.Xaml), so only a WinUI 3 render can ever "
            "be compared against MAUI. See docs/WINDOWS_TOOLCHAIN.md.",
        ],
    }


def main(argv=None) -> int:
    ap = argparse.ArgumentParser(description="Summarize a Windows board run for the README generator")
    ap.add_argument("run_dir")
    ap.add_argument("--out", default=str(DEFAULT_OUT))
    ap.add_argument("--print", dest="do_print", action="store_true", help="print instead of writing")
    a = ap.parse_args(argv)

    run_dir = Path(a.run_dir)
    if not run_dir.is_dir():
        print(f"error: {run_dir} is not a directory", file=sys.stderr)
        return 2
    data = summarize(run_dir)
    text = json.dumps(data, indent=2) + "\n"
    if a.do_print:
        print(text)
        return 0
    os.makedirs(os.path.dirname(a.out), exist_ok=True)
    Path(a.out).write_text(text)
    print(f"wrote {a.out}: {data['pages_captured']} pages, {data['frames_total']} frames, "
          f"{data['unique_images']} unique images, {len(data['dropped_frames'])} dropped")
    return 0


if __name__ == "__main__":
    sys.exit(main())
