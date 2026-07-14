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
COL_TO_FW = {"maui_xaml": "maui", "cpp": "cpp", "cpp_xaml": "xaml"}

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
tags = sorted({p.split("/")[-4] for p in glob.glob(f"{run}/*/{platform}/maui_xaml/*.png")})
for tag in tags:
    for col, fw in COL_TO_FW.items():
        for theme in ("light", "dark"):
            src = initial_frame(tag, col, theme)
            dst = os.path.join(HERE, "captures", platform, fw, f"{tag}_{theme}.png")
            if src:
                os.makedirs(os.path.dirname(dst), exist_ok=True)
                shutil.copyfile(src, dst)
                copied += 1
            elif col != "cpp":  # cpp legitimately absent for non-twin pages
                missing.append(f"{tag}/{fw}/{theme}")
print(f"imported {copied} captures for {len(tags)} pages into captures/{platform}/{{maui,cpp,xaml}}/")
if missing:
    print(f"  {len(missing)} missing (non-cpp): {missing[:12]}{' …' if len(missing) > 12 else ''}")
