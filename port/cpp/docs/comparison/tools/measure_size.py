#!/usr/bin/env python3
"""Measure deployed artifact size per platform per column, decomposed and build-config-tagged.

Answers H1 (see PREDICTIONS.md): does the managed reference ship a materially larger binary than
the native implementation, and *which component* accounts for the difference?

Why this is more than `du -sh`:

  1. **A size number without its build configuration is worthless.** A Debug managed build compared
     against an optimized native build (or the reverse) is a strawman, and that is the first thing a
     reviewer attacks. Every record here carries `build_config` and a `release_grade` flag; the
     README refuses to present a headline ratio unless BOTH sides of a comparison are release-grade.
  2. **The interesting result is the decomposition, not the total.** "Managed is bigger" is
     predictable a priori. "N MB of the delta is runtime + base libraries, M MB is framework code,
     and on the Windows lane — where both columns need the same platform runtime — the delta
     collapses" is a finding, because it *attributes* the difference to a component rather than
     vaguely to a language.
  3. **Unstripped symbol tables dwarf everything else.** The Catalyst native binary measured 58.7 MB
     on disk, of which 39.8 MB was `__LINKEDIT` (560k symbols). Reporting that as "the native app's
     size" would be a fabrication. Native records therefore carry a measured `stripped_bytes` (an
     actual `strip -S -x` on a copy), not an estimate.

Usage:
    python3 tools/measure_size.py                     # all configured environments
    python3 tools/measure_size.py --env macos-arm64   # one lane
    python3 tools/measure_size.py --json out.json     # alternate output path

Writes `measurements.json` next to `comparison.json` (merging into the `size` key, leaving any
`runtime` measurements written by the board run untouched). `gen_readme.py` renders it.
"""
from __future__ import annotations

import argparse
import json
import os
import shutil
import subprocess
import sys
import tempfile
import tomllib
from pathlib import Path

HERE = Path(__file__).resolve().parent
COMPARISON = HERE.parent
REPO = COMPARISON.parents[3]          # …/maui
CONFIGS = [COMPARISON / "config" / "local.toml", COMPARISON / "config" / "windows.toml"]
OUT = COMPARISON / "measurements.json"

# Extension → decomposition bucket. Anything unmatched lands in "other".
MANAGED_EXT = {".dll", ".exe", ".pdb", ".aotdata", ".dylib.dSYM"}
RESOURCE_EXT = {".png", ".jpg", ".jpeg", ".gif", ".svg", ".ttf", ".otf", ".xml", ".json",
                ".plist", ".storyboardc", ".nib", ".car", ".strings", ".icns", ".ico"}
NATIVE_EXT = {".dylib", ".so", ".a", ".framework"}


def sh(*cmd, **kw) -> str:
    """Run a command, return stdout, empty string on any failure (these are all optional probes)."""
    try:
        return subprocess.run(cmd, capture_output=True, text=True, timeout=120, **kw).stdout
    except Exception:
        return ""


# Build intermediates that live INSIDE a CMake target directory. Several columns point `artifact`
# at a CMake target dir rather than a bundle (the apple preset emits a plain executable + resources
# beside it), and that directory also holds CMakeFiles/ — 636 MB of object files for one column.
# Counting those would inflate the native side by an order of magnitude and produce a number that
# is not merely imprecise but backwards.
SKIP_DIRS = {"CMakeFiles", "__pycache__"}
SKIP_SUFFIX = {".o", ".obj", ".d", ".ninja_deps", ".ninja_log", ".tlog", ".ilk"}
SKIP_NAMES = {"CMakeCache.txt", "cmake_install.cmake", "CTestTestfile.cmake", "Makefile",
              "build.ninja", "compile_commands.json", ".ninja_log", ".ninja_deps"}


def walk_files(root: Path):
    if root.is_file():
        yield root
        return
    for dirpath, dirnames, filenames in os.walk(root):
        dirnames[:] = [d for d in dirnames if d not in SKIP_DIRS and not d.endswith(".dSYM")]
        for f in filenames:
            if f in SKIP_NAMES or Path(f).suffix in SKIP_SUFFIX:
                continue
            p = Path(dirpath) / f
            if p.is_file() and not p.is_symlink():
                yield p


def is_macho(p: Path) -> bool:
    try:
        with open(p, "rb") as fh:
            return fh.read(4) in (b"\xcf\xfa\xed\xfe", b"\xce\xfa\xed\xfe", b"\xca\xfe\xba\xbe")
    except OSError:
        return False


def linkedit_bytes(binary: Path) -> int:
    """Size of the __LINKEDIT segment — symbol tables + debug info, i.e. what `strip` removes."""
    for line in sh("size", "-m", str(binary)).splitlines():
        if "Segment __LINKEDIT:" in line:
            try:
                return int(line.split(":")[1].strip())
            except ValueError:
                return 0
    return 0


def measured_stripped_bytes(binary: Path) -> int | None:
    """ACTUAL stripped size: copy, `strip -S -x`, measure. Not an estimate — the estimate
    (total − __LINKEDIT) is wrong because strip also rewrites the load commands."""
    if not shutil.which("strip") or not is_macho(binary):
        return None
    with tempfile.TemporaryDirectory() as td:
        tmp = Path(td) / binary.name
        try:
            shutil.copy2(binary, tmp)
            subprocess.run(["strip", "-S", "-x", str(tmp)], capture_output=True, timeout=120)
            return tmp.stat().st_size
        except Exception:
            return None


def bucket(p: Path) -> str:
    ext = p.suffix.lower()
    if ext in MANAGED_EXT:
        return "managed"
    if ext in RESOURCE_EXT:
        return "resources"
    if ext in NATIVE_EXT:
        return "native_libs"
    if is_macho(p) or os.access(p, os.X_OK):
        return "code"
    return "other"


def detect_build_config(artifact: Path) -> tuple[str, bool, str]:
    """→ (label, release_grade, how_detected).

    `release_grade` gates the README's headline ratio. It is deliberately conservative: an EMPTY
    CMAKE_BUILD_TYPE is NOT release-grade (no -O, no NDEBUG), which is the current state of the
    native Catalyst build and the reason the raw on-disk numbers cannot be published as-is.
    """
    parts = {s.lower() for s in artifact.parts}
    if "debug" in parts:
        return "Debug", False, "path segment"
    if "release" in parts:
        return "Release", True, "path segment"

    # CMake tree: walk up for the build dir's cache.
    for anc in [artifact, *artifact.parents][:8]:
        cache = anc / "CMakeCache.txt"
        if cache.exists():
            for line in cache.read_text(errors="replace").splitlines():
                if line.startswith("CMAKE_BUILD_TYPE:"):
                    val = line.split("=", 1)[1].strip()
                    if not val:
                        return "(unset)", False, f"{cache.name}: CMAKE_BUILD_TYPE empty — no -O, no NDEBUG"
                    return val, val.lower() in ("release", "relwithdebinfo", "minsizerel"), cache.name
            return "(unset)", False, f"{cache.name}: no CMAKE_BUILD_TYPE"
    return "unknown", False, "not determined"


def measure_artifact(path: Path) -> dict:
    files = list(walk_files(path))
    buckets: dict[str, int] = {}
    for f in files:
        try:
            buckets[bucket(f)] = buckets.get(bucket(f), 0) + f.stat().st_size
        except OSError:
            pass
    total = sum(buckets.values())

    # The main executable: largest Mach-O, or the file itself for a bare-executable artifact.
    machos = sorted((f for f in files if is_macho(f)), key=lambda f: f.stat().st_size, reverse=True)
    rec: dict = {"total_bytes": total, "file_count": len(files), "buckets": buckets}
    if machos:
        main = machos[0]
        symbols = linkedit_bytes(main)
        stripped = measured_stripped_bytes(main)
        rec["main_binary"] = {
            "name": main.name,
            "bytes": main.stat().st_size,
            "linkedit_bytes": symbols,
            "stripped_bytes": stripped,
        }
        if stripped is not None:
            # What the bundle WOULD weigh with the main binary stripped. The honest native number.
            rec["total_bytes_stripped"] = total - main.stat().st_size + stripped
    return rec


def collect(env_filter: str | None) -> dict:
    out: dict = {}
    for cfg_path in CONFIGS:
        if not cfg_path.exists():
            continue
        cfg = tomllib.loads(cfg_path.read_text())
        for env_name, env in (cfg.get("environments") or {}).items():
            if env_filter and env_name != env_filter:
                continue
            for col, ccfg in (env.get("columns") or {}).items():
                art = ccfg.get("artifact")
                if not art:
                    continue
                # The Windows lane builds ON the guest: `artifact_remote` is the real artifact and
                # the host-side `artifact` is explicitly "unused while artifact_remote is set" — it
                # points at a SOURCE directory. Measuring it produced 1170.9 MB for the managed
                # column and 2.2 MB for the native one: a fabricated 500x ratio that would have gone
                # straight into the README as the study's headline. A lane we cannot measure from
                # here is recorded as unmeasured; it is not measured wrongly.
                if ccfg.get("artifact_remote"):
                    out.setdefault(env_name, {})[col] = {
                        "artifact": ccfg["artifact_remote"],
                        "exists": False,
                        "remote_only": True,
                        "note": "built on the guest — needs guest-side measurement over SSH "
                                "(host `artifact` is a source path, not the built artifact)",
                        "build_config": "unknown",
                        "release_grade": False,
                        "build_config_detected_from": "n/a (remote)",
                        "config_file": cfg_path.name,
                    }
                    continue
                # PREFER a release-grade artifact when the column declares one. `artifact` stays
                # untouched because run_comparison.py DEPLOYS from it — the parity board and the size
                # study need different builds (the board wants the binaries its committed frames were
                # captured from; the study wants a build a reviewer would accept), and conflating them
                # would silently rebaseline the board. Which key was used is recorded in every record,
                # so a number can never be quoted without knowing which build produced it.
                art_key = "artifact"
                if ccfg.get("artifact_release"):
                    cand = ccfg["artifact_release"]
                    cand_p = (REPO / cand) if not Path(cand).is_absolute() else Path(cand)
                    if cand_p.exists():
                        art, art_key = cand, "artifact_release"
                    else:
                        # Declared but missing: say so rather than silently falling back to the Debug
                        # artifact and labelling the result release-grade.
                        out.setdefault(env_name, {})[col] = {
                            "artifact": cand,
                            "artifact_key": "artifact_release",
                            "exists": False,
                            "note": "artifact_release declared but not built — run the release build "
                                    "or drop the key; NOT falling back to the debug artifact",
                            "build_config": "unknown",
                            "release_grade": False,
                            "build_config_detected_from": "n/a (missing)",
                            "config_file": cfg_path.name,
                        }
                        continue
                p = (REPO / art) if not Path(art).is_absolute() else Path(art)
                label, release, how = detect_build_config(p)
                rec = {
                    "artifact": art,
                    "artifact_key": art_key,
                    "exists": p.exists(),
                    "build_config": label,
                    "release_grade": release,
                    "build_config_detected_from": how,
                    "config_file": cfg_path.name,
                }
                if p.exists():
                    rec.update(measure_artifact(p))
                out.setdefault(env_name, {})[col] = rec
    return out


def main(argv=None) -> int:
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--env", help="measure only this environment (e.g. macos-arm64)")
    ap.add_argument("--json", type=Path, default=OUT, help=f"output path (default {OUT.name})")
    a = ap.parse_args(argv)

    sizes = collect(a.env)
    if not sizes:
        print("no artifacts configured (checked "
              f"{', '.join(c.name for c in CONFIGS if c.exists())})", file=sys.stderr)
        return 1

    doc = json.loads(a.json.read_text()) if a.json.exists() else {}
    doc["size"] = sizes
    doc["size_measured_at"] = subprocess.run(["date", "-u", "+%Y-%m-%dT%H:%M:%SZ"],
                                             capture_output=True, text=True).stdout.strip()
    a.json.write_text(json.dumps(doc, indent=1, ensure_ascii=False) + "\n")

    for env_name, cols in sizes.items():
        print(f"\n{env_name}")
        for col, r in cols.items():
            if r.get("remote_only"):
                print(f"  {col:12} REMOTE   {r['artifact']}  (guest-side, not measured)")
                continue
            if not r["exists"]:
                print(f"  {col:12} MISSING  {r['artifact']}")
                continue
            mb = r["total_bytes"] / 2**20
            s = r.get("total_bytes_stripped")
            extra = f" (stripped {s / 2**20:.1f} MB)" if s else ""
            flag = "" if r["release_grade"] else "  ⚠ NOT release-grade"
            print(f"  {col:12} {mb:7.1f} MB{extra}  [{r['build_config']}]{flag}")
    print(f"\nwrote {a.json}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
