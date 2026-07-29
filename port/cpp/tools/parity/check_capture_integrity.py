#!/usr/bin/env python3
"""Catch captures that are lying before they become parity scores.

A capture run reports "captures=1104 failures=0" and that is trusted, but "the file was
written" is not "the file shows what it claims". Two failure modes have both really happened
on this board and neither raised anything:

  CROSS-COLUMN  the same bytes under two different frameworks. Found on windows/header_footer_grid,
                where cpp/{light,dark} and maui/dark were all one file -- so the board was
                comparing a MAUI dark capture against a MAUI light capture and scoring the port
                at 79.84%. That is a pure artifact: no port change could ever have fixed it, and
                an agent was dispatched at it before this check existed.

  SAME-THEME    a page's light and dark captures byte-identical within one column. The theme was
                never applied, so the "dark" score is a second light score. MAUI's own column has
                zero of these, which is what makes it a defect signal rather than a page that
                happens to look theme-neutral -- the page background alone should always differ.

Exit 1 on cross-column duplication (always wrong). Same-theme duplication exits 1 too unless
--allow-same-theme, since a legitimately theme-invariant page is rare enough to be worth naming
explicitly rather than tolerating silently.

Usage: check_capture_integrity.py [--platform windows] [--allow-same-theme key,key] [captures_dir]
"""
from __future__ import annotations

import argparse
import collections
import hashlib
import pathlib
import sys

DEFAULT = pathlib.Path(__file__).resolve().parents[2] / "docs" / "comparison" / "captures"


def scan(root: pathlib.Path) -> dict[str, list[pathlib.Path]]:
    by_hash: dict[str, list[pathlib.Path]] = collections.defaultdict(list)
    for png in sorted(root.rglob("*.png")):
        by_hash[hashlib.md5(png.read_bytes()).hexdigest()].append(png)
    return by_hash


def main(argv: list[str]) -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("captures", nargs="?", default=DEFAULT, type=pathlib.Path)
    ap.add_argument("--platform", help="limit to one platform subdirectory")
    ap.add_argument("--allow-same-theme", default="",
                    help="comma-separated page keys whose light==dark is known and accepted")
    args = ap.parse_args(argv)

    root = args.captures / args.platform if args.platform else args.captures
    if not root.is_dir():
        print(f"no captures under {root}", file=sys.stderr)
        return 2

    allowed = {k for k in args.allow_same_theme.split(",") if k}
    by_hash = scan(root)
    total = sum(len(v) for v in by_hash.values())

    cross: list[list[pathlib.Path]] = []
    same_theme: list[tuple[str, pathlib.Path, pathlib.Path]] = []
    for group in by_hash.values():
        if len(group) < 2:
            continue
        # The framework column is the parent directory: captures/<platform>/<framework>/<key>_<theme>.png
        # Only MAUI-vs-port collisions are contamination. cpp == xaml byte-identical is the GOAL --
        # both are this port rendering the same page two ways (code-first builder and XAML loader),
        # so identical output means the two columns agree perfectly. Flagging that as a defect would
        # report the board's best pages as broken.
        columns = {p.parent.name for p in group}
        if "maui" in columns and len(columns) > 1:
            cross.append(group)
        for light in group:
            if not light.name.endswith("_light.png"):
                continue
            dark = light.with_name(light.name.replace("_light.png", "_dark.png"))
            if dark in group:
                key = light.name[: -len("_light.png")]
                if key not in allowed:
                    same_theme.append((key, light, dark))

    print(f"{total} capture(s) under {root}, {len(by_hash)} unique")
    for group in cross:
        print("CROSS-COLUMN: identical bytes in different framework columns:")
        for p in group:
            print(f"    {p.relative_to(root)}")
    for key, light, _ in same_theme:
        print(f"SAME-THEME: {light.parent.name}/{key} light and dark are identical "
              f"-- the theme was never applied")

    bad = len(cross) + len(same_theme)
    print(f"{len(cross)} cross-column, {len(same_theme)} same-theme problem(s)")
    return 1 if bad else 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
