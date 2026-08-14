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
from datetime import datetime, timezone
from pathlib import Path

HERE = Path(__file__).resolve().parent
COMPARISON = HERE.parent
REPO = COMPARISON.parents[3]          # …/maui
CONFIGS = [COMPARISON / "config" / "local.toml", COMPARISON / "config" / "windows.toml",
           # Size-only declarations for the iOS/Android lanes. Those are captured through
           # recapture.py's own lane table rather than run_comparison.py, so they had no config at
           # all and were missing from the size table entirely -- not even an em-dash row.
           COMPARISON / "config" / "mobile.toml"]
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


# ------------------------------------------------------------------ the Windows (guest-side) lane
# Windows artifacts are built ON the guest and cannot be reached from here, so until now the three
# Windows rows in the README were literal em-dashes. Measuring the HOST-side `artifact` instead is not
# an option and never was: it points at a SOURCE directory, and doing so once produced 1170.9 MB for
# the managed column against 2.2 MB for the native one -- a fabricated 500x ratio that would have gone
# into the README as the study's headline.
#
# The split of labour is deliberate. The guest does ENUMERATION ONLY (walk the tree, report name and
# size); every judgement -- bucketing, the debug split, build-config grading -- happens here, on the
# same code path the local lanes use. Nothing about "what counts as resources" gets a second
# implementation that can drift from the first.
#
# PE debug info is the one genuine difference from Mach-O, and it makes the Windows number STRONGER
# rather than weaker. On Apple platforms symbols live INSIDE the binary, so an honest figure needs a
# real `strip` on a copy. On Windows they live in separate .pdb files that are never deployed, so
# excluding them is exact rather than estimated -- no strip, no approximation.
WINDOWS_DEBUG_EXT = {".pdb", ".ipdb", ".iobj"}
WINDOWS_CODE_EXT = {".exe", ".dll", ".winmd", ".so", ".pyd"}
WINDOWS_RESOURCE_EXT = RESOURCE_EXT | {".pri", ".xbf", ".resw", ".resjson", ".ttc", ".xaml"}


def artifact_built_at(path: Path, newest_walked: float | None) -> tuple[str, str]:
    """→ (ISO date, how it was determined). A size is only as current as the binary it came from.

    WHY THIS IS NOT JUST st_mtime. The Android Release APK reports mtime AND birthtime of
    1981-01-01 01:01:02 -- the ZIP epoch, written deliberately by reproducible Android packaging.
    Its own stamp therefore says nothing at all about when it was built, while its sibling
    MauiReference.dll in the same directory reads 2026-08-11. Taking the file's timestamp at face
    value would have dated the study's only release-grade mobile row to 1981.

    This exists because the session that added these rows was itself about a stale tree publishing a
    day of wrong scores. A number whose artifact cannot be dated is the same failure one layer up.
    """
    try:
        own = path.stat().st_mtime
    except OSError:
        return "unknown", "artifact not stat-able"
    candidates = [t for t in (own, newest_walked) if t]
    best, how = max(candidates), "newest measured file"
    # Any pre-2000 stamp is a build-determinism artifact, not a date. Fall back to the directory the
    # artifact sits in, which the build tool writes normally.
    if best < 946684800:  # 2000-01-01
        sibs = []
        try:
            sibs = [p.stat().st_mtime for p in path.parent.iterdir() if p.is_file()]
        except OSError:
            pass
        newer = [t for t in sibs if t >= 946684800]
        if newer:
            return (datetime.fromtimestamp(max(newer), tz=timezone.utc).strftime("%Y-%m-%d"),
                    "sibling file (artifact carries a reproducible-build epoch stamp)")
        return "unknown", "artifact carries a reproducible-build epoch stamp"
    return datetime.fromtimestamp(best, tz=timezone.utc).strftime("%Y-%m-%d"), how


def bucket_windows(name: str) -> str:
    """Bucket a PE-lane file by extension alone -- the guest reported a name and a size, nothing more.

    Note this does NOT reuse bucket(): that one puts .exe/.dll/.pdb in "managed" (correct for the
    Catalyst lanes, where a .dll only appears in the managed column) and probes the local filesystem
    via is_macho/os.access. On Windows BOTH columns are .exe + .dll, so that split would label the
    native C++ gallery "managed" and quietly invert the study's headline.
    """
    ext = os.path.splitext(name)[1].lower()
    if ext in WINDOWS_DEBUG_EXT:
        return "debug"
    if ext in WINDOWS_CODE_EXT:
        return "code"
    if ext in WINDOWS_RESOURCE_EXT:
        return "resources"
    return "other"


def _ssh_python(host: str, user: str, python3: str, script: str) -> str:
    """Run `script` under the guest's Python and return stdout.

    The program is piped to STDIN (`py -`) rather than passed with -c. The guest's DefaultShell is
    PowerShell, so a -c argument containing quotes, backslashes and newlines has to survive two
    incompatible quoting layers; stdin has no quoting layer at all. The artifact path is baked into
    the script text host-side for the same reason -- it contains both `\\` and `:`.
    """
    try:
        r = subprocess.run(["ssh", "-o", "BatchMode=yes", "-o", "ConnectTimeout=10",
                            f"{user}@{host}", f"{python3} -"],
                           input=script.encode(), capture_output=True, timeout=300)
        return r.stdout.decode("utf-8", "replace") if r.returncode == 0 else ""
    except Exception:
        return ""


def _remote_listing(host: str, user: str, python3: str, root: str) -> list | None:
    """[[relative path, size, mtime], ...] for the artifact tree on the guest, or None if unreachable."""
    script = (
        "import os, json\n"
        f"root = {root!r}\n"
        f"SKIP_DIRS = {sorted(SKIP_DIRS)!r}\n"
        f"SKIP_SUFFIX = {sorted(SKIP_SUFFIX)!r}\n"
        f"SKIP_NAMES = {sorted(SKIP_NAMES)!r}\n"
        "out = []\n"
        "if os.path.isdir(root):\n"
        "    for dp, dn, fn in os.walk(root):\n"
        "        dn[:] = [d for d in dn if d not in SKIP_DIRS]\n"
        "        for f in fn:\n"
        "            if f in SKIP_NAMES or os.path.splitext(f)[1] in SKIP_SUFFIX:\n"
        "                continue\n"
        "            p = os.path.join(dp, f)\n"
        "            try:\n"
        "                out.append([os.path.relpath(p, root), os.path.getsize(p), os.path.getmtime(p)])\n"
        "            except OSError:\n"
        "                pass\n"
        "print(json.dumps({'exists': os.path.isdir(root), 'files': out}))\n"
    )
    raw = _ssh_python(host, user, python3, script)
    if not raw.strip():
        return None
    try:
        return json.loads(raw.strip().splitlines()[-1])
    except (json.JSONDecodeError, IndexError):
        return None


def _remote_build_config(host: str, user: str, python3: str, artifact: str) -> tuple[str, bool, str]:
    """Build config for a guest artifact, mirroring detect_build_config's rules.

    A path segment settles it for the MSBuild column (bin/Debug/...). The CMake columns have no such
    segment, so CMakeCache.txt has to be read ON the guest -- and an EMPTY CMAKE_BUILD_TYPE stays
    NOT release-grade here exactly as it is locally, since it means no -O and no NDEBUG.
    """
    parts = {s.lower() for s in artifact.replace("\\", "/").split("/")}
    if "debug" in parts:
        return "Debug", False, "path segment"
    if "release" in parts:
        return "Release", True, "path segment"
    script = (
        "import os, json\n"
        f"a = {artifact!r}.replace('\\\\', '/')\n"
        "cur, found = a, None\n"
        "for _ in range(8):\n"
        "    c = os.path.join(cur, 'CMakeCache.txt')\n"
        "    if os.path.isfile(c):\n"
        "        found = c\n"
        "        break\n"
        "    nxt = os.path.dirname(cur)\n"
        "    if nxt == cur:\n"
        "        break\n"
        "    cur = nxt\n"
        "val = None\n"
        "if found:\n"
        "    for line in open(found, errors='replace'):\n"
        "        if line.startswith('CMAKE_BUILD_TYPE:'):\n"
        "            val = line.split('=', 1)[1].strip()\n"
        "            break\n"
        "print(json.dumps({'cache': found, 'value': val}))\n"
    )
    raw = _ssh_python(host, user, python3, script)
    try:
        got = json.loads(raw.strip().splitlines()[-1])
    except Exception:
        return "unknown", False, "not determined (remote probe failed)"
    if not got.get("cache"):
        return "unknown", False, "not determined (no CMakeCache.txt on the guest)"
    val = got.get("value")
    if not val:
        return "(unset)", False, "CMakeCache.txt: CMAKE_BUILD_TYPE empty - no -O, no NDEBUG"
    return val, val.lower() in ("release", "relwithdebinfo", "minsizerel"), "CMakeCache.txt (guest)"


def measure_remote_artifact(host: str, user: str, python3: str, artifact: str,
                            process: str | None) -> dict:
    """The guest-side twin of measure_artifact(), for a Windows PE tree."""
    listing = _remote_listing(host, user, python3, artifact)
    if listing is None:
        return {"artifact": artifact, "exists": False, "remote_only": True,
                "note": "guest unreachable over SSH - not measured (NOT zero)",
                "build_config": "unknown", "release_grade": False,
                "build_config_detected_from": "n/a (unreachable)"}
    if not listing.get("exists"):
        return {"artifact": artifact, "exists": False, "remote_only": True,
                "note": "artifact directory does not exist on the guest - build it first",
                "build_config": "unknown", "release_grade": False,
                "build_config_detected_from": "n/a (absent)"}

    files = [(rel, size) for rel, size, _ in listing["files"]]
    newest = max((m for _, _, m in listing["files"]), default=None)
    buckets: dict[str, int] = {}
    for rel, size in files:
        b = bucket_windows(rel)
        buckets[b] = buckets.get(b, 0) + size
    total = sum(buckets.values())
    cfg_label, release_grade, how = _remote_build_config(host, user, python3, artifact)

    rec: dict = {
        "artifact": artifact, "exists": True, "measured_on": "guest (ssh)",
        "total_bytes": total, "file_count": len(files), "buckets": buckets,
        "build_config": cfg_label, "release_grade": release_grade,
        "build_config_detected_from": how,
        # Dated from the guest's own newest measured file: the host cannot stat the artifact at all.
        "built_at": (datetime.fromtimestamp(newest, tz=timezone.utc).strftime("%Y-%m-%d")
                     if newest else "unknown"),
        "built_at_from": "newest measured file (guest)",
    }
    # The deployable size. Exact, not an estimate: .pdb files are simply not shipped.
    debug_bytes = buckets.get("debug", 0)
    rec["total_bytes_stripped"] = total - debug_bytes

    # Prefer the column's declared process over "largest .exe" -- the runner already names the binary
    # it launches, and on the managed column the biggest .exe is not necessarily it.
    exes = [(rel, size) for rel, size in files if rel.lower().endswith(".exe")]
    main = None
    if process:
        main = next((e for e in exes if os.path.basename(e[0]).lower() == process.lower()), None)
    if main is None and exes:
        main = max(exes, key=lambda e: e[1])
    if main:
        stem = os.path.splitext(main[0])[0].lower()
        pdb = next((size for rel, size in files if os.path.splitext(rel)[0].lower() == stem
                    and rel.lower().endswith(".pdb")), 0)
        rec["main_binary"] = {"name": os.path.basename(main[0]), "bytes": main[1],
                              "linkedit_bytes": pdb,     # the PE analogue: symbols in a sidecar .pdb
                              "stripped_bytes": main[1]}  # a PE carries no symbols to strip
    return rec


# ------------------------------------------------------------------------------------ the APK lane
# An APK is a ZIP, so `du` on it reports one number and attributes nothing. That is not good enough
# here, because the raw Android figures point the OPPOSITE way to every other lane: the port's Debug
# apphost measures ~360 MB against the MAUI reference's ~81 MB. Reported as a bare total that reads
# "the native port is 4x LARGER", which would be a real finding only if it were about shipped code --
# and it is not. It is unstripped DWARF inside lib/*.so. Splitting the archive is what separates
# "native code is bigger" (false) from "this Debug build carries its debug info" (true).
#
# Sizes are COMPRESSED (what the file actually weighs), so the buckets sum to the APK on disk.
APK_MANAGED_PREFIX = ("assemblies/",)
APK_RESOURCE_PREFIX = ("res/", "assets/")


def bucket_apk(name: str) -> str:
    low = name.lower()
    if low.startswith("lib/") and low.endswith(".so"):
        return "native_libs"
    if low.startswith(APK_MANAGED_PREFIX) or low.endswith(".dll"):
        return "managed"
    if low.endswith(".dex"):
        return "code"
    if low.startswith(APK_RESOURCE_PREFIX) or low == "resources.arsc":
        return "resources"
    return "other"


def measure_apk(path: Path) -> dict:
    """Decompose an .apk by reading its zip directory. No extraction, no android tooling."""
    import zipfile  # stdlib; imported here so the module still loads where APKs are irrelevant

    buckets: dict[str, int] = {}
    entries = 0
    biggest_so = ("", 0)
    try:
        with zipfile.ZipFile(path) as z:
            for i in z.infolist():
                if i.is_dir():
                    continue
                entries += 1
                buckets[bucket_apk(i.filename)] = buckets.get(bucket_apk(i.filename), 0) + i.compress_size
                if i.filename.lower().endswith(".so") and i.compress_size > biggest_so[1]:
                    biggest_so = (i.filename, i.compress_size)
    except (zipfile.BadZipFile, OSError) as exc:
        return {"total_bytes": path.stat().st_size, "file_count": 1, "buckets": {},
                "note": f"could not read the APK zip directory: {exc}"}

    rec: dict = {
        # The APK's real on-disk size, NOT the sum of buckets: the zip's own central directory and
        # alignment padding are part of what ships and belong in the headline number.
        "total_bytes": path.stat().st_size,
        "file_count": entries,
        "buckets": buckets,
    }
    if biggest_so[0]:
        rec["main_binary"] = {"name": os.path.basename(biggest_so[0]), "bytes": biggest_so[1],
                              "linkedit_bytes": 0, "stripped_bytes": None}
    rec["built_at"], rec["built_at_from"] = artifact_built_at(path, None)
    return rec


def measure_artifact(path: Path) -> dict:
    if path.is_file() and path.suffix.lower() == ".apk":
        return measure_apk(path)
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
    newest = max((f.stat().st_mtime for f in files), default=None)
    rec["built_at"], rec["built_at_from"] = artifact_built_at(path, newest)
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
                    conn = (env.get("connection") or {})
                    host, user = conn.get("host"), conn.get("user")
                    if not host or not user:
                        out.setdefault(env_name, {})[col] = {
                            "artifact": ccfg["artifact_remote"], "exists": False, "remote_only": True,
                            "note": "no [connection] host/user in the config - cannot measure remotely",
                            "build_config": "unknown", "release_grade": False,
                            "build_config_detected_from": "n/a (remote)",
                            "config_file": cfg_path.name,
                        }
                        continue
                    py = (env.get("tools") or {}).get("python3") or "py"
                    rec = measure_remote_artifact(host, user, py, ccfg["artifact_remote"],
                                                  ccfg.get("process"))
                    rec["remote_only"] = True
                    rec["config_file"] = cfg_path.name
                    out.setdefault(env_name, {})[col] = rec
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
    # A FULL run is authoritative and replaces the map, so a lane dropped from the configs stops being
    # reported. A --env run must MERGE: it only looked at one lane, and overwriting the map with that
    # single result silently deleted the other lanes' rows from the README -- five macOS rows would go
    # blank on any `--env windows-x64` iteration, which is the very command this lane invites.
    if a.env:
        merged = doc.get("size") or {}
        merged.update(sizes)
        doc["size"] = merged
    else:
        doc["size"] = sizes
    doc["size_measured_at"] = subprocess.run(["date", "-u", "+%Y-%m-%dT%H:%M:%SZ"],
                                             capture_output=True, text=True).stdout.strip()
    a.json.write_text(json.dumps(doc, indent=1, ensure_ascii=False) + "\n")

    for env_name, cols in sizes.items():
        print(f"\n{env_name}")
        for col, r in cols.items():
            # remote_only marks WHERE it was measured, not WHETHER. A guest artifact that was actually
            # walked falls through to the normal reporting below; only an unreachable or absent one
            # is called out here.
            if r.get("remote_only") and not r.get("exists"):
                print(f"  {col:12} REMOTE   {r['artifact']}  ({r.get('note', 'not measured')})")
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
