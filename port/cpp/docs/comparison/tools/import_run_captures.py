#!/usr/bin/env python3
"""Bridge: import an E2E-runner dated run's captures into the canonical captures/<platform>/<fw>/<key>_<theme>.png
layout that build_comparison_json.py + gen_readme.py consume. The runner writes <tag>/<platform>/<column>/NNNN.png
(column ∈ maui_xaml|cpp|cpp_xaml, theme+step in the NNNN.json sidecar); the canonical tree uses fw ∈ maui|cpp|xaml
and <key>_<theme>.png. This copies each column's 'initial' frame per theme into place. After running, regenerate:
  python3 tools/build_comparison_json.py && python3 tools/gen_readme.py
Usage: import_run_captures.py <run_dir> [platform=maccatalyst]"""
import sys, os, glob, json, shutil

run = sys.argv[1].rstrip("/")
platform = sys.argv[2] if len(sys.argv) > 2 else "maccatalyst"
HERE = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))  # docs/comparison
# The AppKit columns keep their names as framework dirs (captures/maccatalyst/appkit_{cpp,xaml}) — that is
# already the canonical layout build_comparison_json.py declares for maccatalyst, so no rename is wanted.
COL_TO_FW = {"maui_xaml": "maui", "cpp": "cpp", "cpp_xaml": "xaml",
             "appkit_cpp": "appkit_cpp", "appkit_xaml": "appkit_xaml"}

def initial_frame(tag, col, theme):
    """The col's PNG whose sidecar theme==theme and step=='initial' (fallback: first matching theme)."""
    best = None
    for j in sorted(glob.glob(f"{run}/{tag}/{platform}/{col}/*.json")):
        try:
            m = json.load(open(j))
        except Exception:
            continue
        if m.get("theme") == theme:
            png = j[:-5] + ".png"
            if os.path.exists(png):
                if m.get("step") == "initial":
                    return png
                best = best or png
    return best

copied, missing = 0, []
# Discover tags AND columns from whatever the run actually produced. This used to glob the maui_xaml
# column specifically, which silently imported NOTHING for any run without one — the macos-appkit env has
# only appkit_cpp/appkit_xaml, so a full 718-frame sweep reported "imported 0 captures for 0 pages" and
# exited 0. A no-op that reports success is the worst failure mode this tree has: it reads as "nothing
# changed" rather than "your import does nothing".
frames = glob.glob(f"{run}/*/{platform}/*/*.png")
tags = sorted({p.split("/")[-4] for p in frames})
present_cols = {p.split("/")[-2] for p in frames}
if not tags:
    raise SystemExit(f"no {platform} frames under {run} — refusing to report success having imported "
                     f"nothing (looked for {run}/<tag>/{platform}/<column>/*.png)")
unknown = present_cols - set(COL_TO_FW)
if unknown:
    raise SystemExit(f"run contains column(s) with no COL_TO_FW mapping: {sorted(unknown)} — add them "
                     f"rather than silently dropping their frames")
for tag in tags:
    for col, fw in COL_TO_FW.items():
        if col not in present_cols:
            continue  # not part of THIS run's env (e.g. no maui_xaml in the appkit env)
        for theme in ("light", "dark"):
            src = initial_frame(tag, col, theme)
            dst = os.path.join(HERE, "captures", platform, fw, f"{tag}_{theme}.png")
            if src:
                os.makedirs(os.path.dirname(dst), exist_ok=True)
                shutil.copyfile(src, dst)
                copied += 1
            elif col not in ("cpp", "appkit_cpp"):  # builder columns are absent for non-twin pages
                missing.append(f"{tag}/{fw}/{theme}")
print(f"imported {copied} captures for {len(tags)} pages into "
      f"captures/{platform}/{{{','.join(sorted(COL_TO_FW[c] for c in present_cols))}}}/")
if missing:
    print(f"  {len(missing)} missing (non-builder): {missing[:12]}{' …' if len(missing) > 12 else ''}")
