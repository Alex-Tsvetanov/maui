#!/usr/bin/env python3
"""Shared helpers for the README section generators (macOS / Android).

Provides:
  - ROOT / COMP paths (the docs/comparison dir),
  - page_keys(): the canonical 172-key order from page_keys.txt,
  - ios_descriptions(): {key -> Description cell text} parsed out of the existing
    iOS Examples table in README.md (so the macOS/Android tables reuse the exact
    same Description text the iOS rows already carry), and
  - title(): the human label for a key.

The Description column in the iOS table is keyed implicitly by the capture path
(captures/<col>_<theme>/<key>.png|gif) embedded in each row, so we recover the
key from that path and read the 4th pipe-delimited cell as the Description.
"""
import os
import re

ROOT = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", ".."))
COMP = os.path.join(ROOT, "docs", "comparison")
README = os.path.join(COMP, "README.md")
KEYS_FILE = os.path.join(ROOT, "tools", "parity", "page_keys.txt")
PLACEHOLDER = "_placeholder.png"

# captures/<col>_<theme>/<key>.<ext> — col in {maui,cpp,xaml}, ext in {png,gif}
_KEY_RE = re.compile(r"captures/(?:maui|cpp|xaml)_(?:light|dark)/([a-z0-9_]+)\.(?:png|gif)")
_ROW_RE = re.compile(r"^\|\s*\d+\s*\|")


def page_keys():
    """Canonical key order from page_keys.txt."""
    if not os.path.exists(KEYS_FILE):
        return []
    with open(KEYS_FILE, encoding="utf-8") as f:
        return [line.strip() for line in f if line.strip()]


def title(key):
    return key.replace("_", " ").title()


def ios_descriptions():
    """Parse {key -> Description text} from the iOS Examples table in README.md.

    Returns an empty dict if the table can't be located (so callers fall back to
    the page title). The first occurrence of a key wins.
    """
    if not os.path.exists(README):
        return {}
    with open(README, encoding="utf-8") as f:
        lines = f.readlines()
    try:
        start = next(i for i, l in enumerate(lines) if l.startswith("| # | Example | Demo"))
    except StopIteration:
        return {}
    try:
        end = next(i for i, l in enumerate(lines) if "<!-- MACOS:BEGIN -->" in l)
    except StopIteration:
        end = len(lines)
    desc = {}
    for line in lines[start:end]:
        if not _ROW_RE.match(line):
            continue
        m = _KEY_RE.search(line)
        if not m:
            continue
        key = m.group(1)
        if key in desc:
            continue
        cells = line.split("|")
        # cells: ['', ' # ', ' Example ', ' Demo… ', ' Description ', ' Sonnet ', ' Gemini ', '\n']
        desc[key] = cells[4].strip() if len(cells) > 4 else ""
    return desc


def description_for(key, descs):
    """Description text for a key: the iOS row's text if present, else the title."""
    d = descs.get(key, "").strip()
    return d if d else title(key)
