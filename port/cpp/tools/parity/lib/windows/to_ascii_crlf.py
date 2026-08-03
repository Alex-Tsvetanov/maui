#!/usr/bin/env python3
"""Convert a script to ASCII + CRLF for the Windows guest, ATOMICALLY.

Why this is a tool and not two inline lines: the obvious version,
    open(path, 'w', encoding='ascii').write(convert(text))
TRUNCATES the destination before the encode runs, so a single stray non-ASCII character leaves a
ZERO-BYTE file. That happened here: an ellipsis in a comment emptied a PowerShell script, which then
parsed fine, exited 0 and printed nothing -- a "successful" run that did nothing at all, which is far
harder to spot than a crash.

ASCII-only matters because PowerShell 5.1 reads a BOM-less file as ANSI, so a UTF-8 em-dash decodes to
bytes containing a double quote and desyncs string parsing (phantom "Missing closing '}'" errors).

Usage: to_ascii_crlf.py <file> [<file>...]
"""
from __future__ import annotations

import os
import sys

# Typographic characters that keep sneaking in from prose-style comments.
REPLACEMENTS = {
    "—": "-", "–": "-", "…": "...", "’": "'", "‘": "'",
    "“": '"', "”": '"', "·": "-", "→": "->", "≥": ">=",
    "≤": "<=", "×": "x", "✓": "ok",
}


def convert(text: str) -> tuple[str, list[str]]:
    for src, dst in REPLACEMENTS.items():
        text = text.replace(src, dst)
    leftovers = sorted({c for c in text if ord(c) > 127})
    return text.replace("\r\n", "\n").replace("\n", "\r\n"), leftovers


def main(argv: list[str]) -> int:
    if not argv:
        print(__doc__.strip().splitlines()[-1], file=sys.stderr)
        return 2
    rc = 0
    for path in argv:
        with open(path, encoding="utf-8") as fh:
            text = fh.read()
        converted, leftovers = convert(text)
        if leftovers:
            # Refuse rather than mangle: an unmapped character means a human should decide.
            print(f"{path}: UNMAPPED non-ASCII {[(c, hex(ord(c))) for c in leftovers]}", file=sys.stderr)
            rc = 1
            continue
        # Encode FIRST, then write. The destination is never opened for writing until the bytes exist,
        # so a failure cannot leave a truncated (or empty) file behind.
        data = converted.encode("ascii")
        tmp = path + ".tmp"
        with open(tmp, "wb") as fh:
            fh.write(data)
        os.replace(tmp, path)
        print(f"{path}: {len(data)} bytes, ASCII + CRLF")
    return rc


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
