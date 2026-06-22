#!/usr/bin/env python3
"""Merge a batch of fresh Claude parity verdicts into docs/comparison/parity_status.json.
Reads build/claude_batch.json: {key: {"light":s,"dark":s,"light_note":..,"dark_note":..}}.
Preserves the file format (indent=1, ensure_ascii=False, top-level key order), canonical within-entry
key order, computes severity (worst of light/dark), stamps reviewed=<DATE>, appends keys to
build/claude_done.txt (progress tracker, deduped)."""
import json, os, sys
from collections import OrderedDict

HERE = os.path.dirname(os.path.abspath(__file__))
CMP = os.path.normpath(os.path.join(HERE, "..", "..", "docs", "comparison"))
STATUS = os.path.join(CMP, "parity_status.json")
BATCH = os.path.normpath(os.path.join(HERE, "..", "..", "build", "claude_batch.json"))
DONE = os.path.normpath(os.path.join(HERE, "..", "..", "build", "claude_done.txt"))
DATE = sys.argv[1] if len(sys.argv) > 1 else "2026-06-23"
RANK = {"match": 0, "minor": 1, "diff": 2, "blank": 3}
ORDER = ["light", "dark", "severity", "light_note", "dark_note", "reviewed"]

status = json.load(open(STATUS))
batch = json.load(open(BATCH))
for key, v in batch.items():
    light, dark = v["light"], v["dark"]
    sev = light if RANK.get(light, 0) >= RANK.get(dark, 0) else dark
    cur = status.get(key, {})
    merged = {**cur, "light": light, "dark": dark, "severity": sev,
              "light_note": v.get("light_note", ""), "dark_note": v.get("dark_note", ""), "reviewed": DATE}
    if "flags" in v:  # explicit flag list, e.g. ["maui_broken"] / ["needs_gif"]; [] clears
        if v["flags"]:
            merged["flags"] = v["flags"]
        else:
            merged.pop("flags", None)
    # keep any extra existing fields (e.g. flags) after the canonical ones
    extra = {k: merged[k] for k in merged if k not in ORDER}
    status[key] = OrderedDict([(k, merged[k]) for k in ORDER if k in merged] + list(extra.items()))

with open(STATUS, "w") as f:
    json.dump(status, f, indent=1, ensure_ascii=False)
    f.write("\n")
done = set()
if os.path.exists(DONE):
    done = set(l.strip() for l in open(DONE) if l.strip())
done |= set(batch)
open(DONE, "w").write("\n".join(sorted(done)) + "\n")
print(f"merged {len(batch)} verdict(s); {len(done)} total reviewed this pass")
