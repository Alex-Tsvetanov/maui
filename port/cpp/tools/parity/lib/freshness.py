#!/usr/bin/env python3
"""Refuse a capture that would score artifacts older than the source they claim to render.

WHY
---
NO LANE REBUILDS ITS GROUND TRUTH. recapture.py's build() says so itself -- "the MAUI reference app is
built by hand" -- so on every one of the four lanes the column every score is measured against is
hand-built, and nothing between that hand-run build and a multi-hour capture asserts it is newer than
the source. Windows is the extreme case (all three columns are `artifact_remote`, prebuilt on the
guest, and build() skips the lane entirely), but it is a difference of degree: on ios `--skip-build`
leaves all three columns at whatever was last built, and on catalyst/appkit build() refreshes the two
galleries and never the reference. A whole run can silently score last week's build while every
artifact-level check passes, because every artifact-level check only asks whether the file EXISTS.

This has cost real time more than once:
  * `containers` (2026-08-22) needed no code change at all -- gallery.exe was from 08-18.
  * `selection_synchronization` (2026-08-22) likewise -- MauiReference.exe from 08-19, scored against
    a twin edited 08-20.
  * Catalyst's MauiReference was once a MONTH old across 61 changed twins.
  * The android reference is the same shape of exposure and is deliberately NOT covered here -- see
    the note at the bottom of this docstring, its artifact is the INSTALLED package, not a file.

WHAT THIS CATCHES, AND WHAT IT DOES NOT
---------------------------------------
CATCHES: artifact older than source. That is the class all three incidents above belong to.

DOES NOT CATCH the `xaml_visitors.cpp` incident of 2026-08-22, and no mtime comparison could: the file
was edited on the host at 00:28:48 and arrived on the guest still wearing 00:28:48, i.e. OLDER than
the 00:36:07 object built from the previous content. It looked fresh from every angle. That class was
fixed at its source by the arrival stamp now in sync_tree.ps1. Claiming this guard would have caught
it too would be exactly the overclaim this project's notes keep warning about.

ALSO DOES NOT CATCH a source edited DURING the build: the artifact is genuinely newer and may still
not contain the edit. mtime cannot see that. When a specific symbol is in question, ask the BINARY --
`artifact_facts.ps1 -Symbol <name>` -- and remember a static/anonymous-namespace function can be
INLINED AWAY and read as 0 hits in an optimised build.

THE SECOND CHECK (score_contradictions) IS THE ONE THAT ACTUALLY CAUGHT SOMETHING
---------------------------------------------------------------------------------
It needs no SSH, no exclude list, no threshold and no clock-skew tolerance, and it generalises to
every lane. pixel_score.py scores maui-vs-cpp into "pixel" and maui-vs-xaml into "pixel_xaml"
(pixel_score.SLOTS) -- against the SAME maui reference image. So when the cpp and xaml captures are
byte-identical, the two recorded reviews are two measurements of one comparison and MUST be equal.
When they differ, at least one of them was carried over from a previous generation of captures and no
longer describes the files on disk. build_comparison_json.py only CARRIES pixel scores over; it
cannot compute them, so a carried score outliving its captures is a live failure mode.

Measured 2026-08-22 on the windows board: exactly 2 still-scored pages tripped it --
`preselected_items` and `multiple_bound_selection` -- which are precisely the two twins edited at
00:29:04 that day. `preselected_items/windows/pixel_xaml` was published YELLOW (SSIM 0.9711, 0.64%)
while `pixel` on byte-identical bytes said green (SSIM 0.9958, 0.09%); a write-free rescore moved it
yellow -> green. The cell was a stale SCORE, not a port defect, and it had already survived one
round of forensics into the loader's SelectedItems path.

ANDROID IS DELIBERATELY OUT OF SCOPE, and not by oversight. capture_all_csharp_android.sh states it
"only DRIVES an already-installed app; it does not build", so the artifact being scored is the app
INSTALLED ON THE DEVICE, not any file in the tree. Measured 2026-08-22 those two disagree completely:
the newest reference APK on disk was 8 days old (08-14 18:10, older than three twin edits) while the
device reported lastUpdateTime 08-22 02:44 -- fresh. A file-mtime check would have screamed STALE at a
perfectly good lane, which is the false positive that gets a guard switched off. The honest probe there
is `adb shell dumpsys package <pkg>` and it belongs with whoever owns the android lane.

NOTE the weak-signal trap: on the Windows lane byte-identical cpp/xaml columns are COMMON, because
only window/content_page/layout/label/button are real WinUI handlers so far (windows.toml) and every
other page renders the same in both. "Both port columns identical" is therefore NOT evidence of
anything on its own. The discriminating fact is identical artifacts carrying DIFFERENT recorded
scores. Stills only, and GIFs must match too: a motion review embeds run ids and commit hashes, so
identical PNGs do not imply identical reviews for an animated page.
"""
from __future__ import annotations

import hashlib
import json
import os
import statistics
import subprocess
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
CPP = HERE.parents[2]                       # port/cpp
PORT = CPP.parent                           # port
REPO = PORT.parent
COMP = CPP / "docs" / "comparison"
PROBE = HERE / "windows" / "artifact_facts.ps1"

WINDOWS_VM = os.environ.get("WINDOWS_VM_HOST", "WINDOWS-VM.local")

# Directory NAMES pruned at any depth when walking host source. Deliberately the SAME list
# sync_tree.ps1 prunes with -- the guest build never sees these either, so a walk that counted them
# would measure files that cannot possibly have gone into the artifact. `docs` matters most: it is
# ~10 GB of committed board PNGs against ~19 MB of source, and walking it once took 76 minutes.
PRUNE = {"bin", "obj", "build", "build-win", "captures", "docs", ".git", "__pycache__", "node_modules"}

# Clock skew + build duration slack. Measured 2026-08-22 the guest ran 3s ahead of the host, so this
# is two orders of magnitude of headroom; every real incident this guard is for was hours to a month
# stale, so nothing is bought by tightening it and a false refusal costs a whole run.
TOLERANCE_S = 300

def windows_artifacts() -> dict[str, str]:
    """column -> the guest EXECUTABLE, read out of config/windows.toml rather than restated here.

    The config already carries both halves per column: `artifact_remote` (the directory the runner
    uses in place of deploying from the host) and `process` (gallery.exe / gallery_xaml.exe /
    MauiReference.exe). Joining them is the whole derivation, and it keeps the guard honest to the
    same doctrine source_roots() follows -- a hand-copied second list of these paths would
    drift the moment C:/maui-build moves, and a guard that then reports DECLARED BUT MISSING on three
    healthy artifacts gets switched off rather than fixed.

    The EXECUTABLE, not the directory `artifact_remote` names: ios_install() spells out why at
    length -- a bundle/output dir exists from CONFIGURE time, so an exists() check on it passes on a
    build that never compiled a thing.
    """
    import tomllib
    cfg = tomllib.loads((COMP / "config" / "windows.toml").read_text())
    cols = cfg["environments"]["windows-arm64"]["columns"]
    out = {}
    for name, col in cols.items():
        remote, proc = col.get("artifact_remote"), col.get("process")
        if remote and proc:
            out[name] = f"{remote.rstrip('/')}/{proc}"
    return out


WINDOWS_ARTIFACTS = windows_artifacts()


def _tracked_clean() -> set[str]:
    """Absolute paths of files git considers tracked AND unmodified.

    Their working-tree mtime is NOT their content's age: `git checkout -- <path>` rewrites the file
    and bumps mtime while restoring byte-identical content. Shelving held work that way -- which is
    what this project does instead of `git stash`, because stash is a global stack with no pathspec
    on `pop` -- therefore made every shelved file look freshly edited.

    MEASURED 2026-08-22: the iOS lane's capture was refused naming `xaml_visitors.cpp` and
    `box_view.xaml` as stale. Both were clean and byte-identical to HEAD; their mtimes read 20:35:53
    purely because they had just been checked out. The cost was an unnecessary 8-minute rebuild, and
    an override that had to be justified by hand. It fires in the SAFE direction, which is why it
    survived -- a guard that cries wolf in a normal state is one people learn to ignore.
    """
    try:
        out = subprocess.run(["git", "-C", str(ROOT), "status", "--porcelain", "-z"],
                             capture_output=True, text=True, timeout=60)
        if out.returncode != 0:
            return set()
        changed = {str((ROOT / e[3:]).resolve()) for e in out.stdout.split("\0") if len(e) > 3}
    except Exception:
        return set()                              # no git -> fall back to mtime, never harder to pass
    try:
        ls = subprocess.run(["git", "-C", str(ROOT), "ls-files", "-z"],
                            capture_output=True, text=True, timeout=60)
        if ls.returncode != 0:
            return set()
        return {str((ROOT / f).resolve()) for f in ls.stdout.split("\0") if f} - changed
    except Exception:
        return set()


def _content_age(roots: list[Path]) -> float:
    """Commit time of the last commit touching anything under `roots` -- the tracked content's real
    age, independent of working-tree mtimes."""
    paths = [str(r) for r in roots if r.exists()]
    if not paths:
        return 0.0
    try:
        out = subprocess.run(["git", "-C", str(ROOT), "log", "-1", "--format=%ct", "--"] + paths,
                             capture_output=True, text=True, timeout=60)
        return float(out.stdout.strip() or 0.0) if out.returncode == 0 else 0.0
    except Exception:
        return 0.0


def newest(roots: list[Path]) -> tuple[float, Path | None]:
    """(mtime, path) of the newest file under any of `roots`, pruning PRUNE dirs. Missing roots are
    skipped rather than fatal -- a column simply may not have that dependency set.

    Tracked-and-clean files are dated by their last COMMIT rather than their mtime (see
    `_tracked_clean`). Dirty and untracked files still use mtime, because their content genuinely is
    newer than anything git knows about. So a committed change that postdates the build still reports
    stale -- the guard is not weakened, only stopped from firing on a checkout."""
    clean = _tracked_clean()
    best, where = _content_age(roots), None
    if best:
        where = roots[0]
    for root in roots:
        if root.is_file():
            if root.stat().st_mtime > best:
                best, where = root.stat().st_mtime, root
            continue
        if not root.is_dir():
            continue
        for dirpath, dirnames, filenames in os.walk(root):
            dirnames[:] = [d for d in dirnames if d not in PRUNE and not d.startswith("build-")]
            for fn in filenames:
                p = Path(dirpath) / fn
                if str(p.resolve()) in clean:
                    continue                      # dated by its commit, not its mtime
                try:
                    m = p.stat().st_mtime
                except OSError:
                    continue
                if m > best:
                    best, where = m, p
    return best, where


def source_roots(column: str, compiled_src_dirs: list[str]) -> list[Path]:
    """The host paths a column's artifact is built FROM. Lane-agnostic: only `compiled_src_dirs`
    differs per lane, and that always comes from the build's OWN object tree.

    THREE distinct dependency sets, not two. The third is the one a `git log -- src/platform/windows/`
    check is structurally blind to, and that exact wrong check was once used to rebut a staleness
    claim: a TWIN EDIT under port/maui-reference/pages/ changes the MAUI column with no port source
    touched at all -- and, because examples/gallery_xaml/CMakeLists.txt:80 globs
    ../../../maui-reference/pages/*.xaml into its bytes-mode codegen, it changes the cpp_xaml column
    too. One twin edit therefore invalidates TWO of the three artifacts.

    `compiled_src_dirs` comes from the guest's own object tree (artifact_facts.ps1), never from a
    hand-written exclude list. Measured, it is src/platform/{headless,windows} plus the shared dirs,
    and NOT src/platform/{ios,apple,apple_shared,android}. That scoping is load-bearing: this repo is
    a shared worktree with several agents in it, and at the moment this guard was written
    src/platform/ios/radio_button_handler.mm was 95 minutes NEWER than a perfectly good gallery.exe.
    A naive port/cpp/src/** walk would have refused the first honest build it ever saw, and would
    then have been switched off. Hand-excluding "the other platforms" would have been wrong in the
    other direction -- headless DOES compile into the Windows build.
    """
    if column == "maui_xaml":
        return [PORT / "maui-reference"]
    roots = [CPP / d for d in compiled_src_dirs]
    # Headers emit no objects, so the object tree cannot see them -- but editing one is the change
    # most likely to invalidate everything (see the "core-header layout change" note). Take include/
    # wholesale, minus the platform dirs the object tree just proved are not compiled here.
    compiled_platforms = {d.split("/")[-1] for d in compiled_src_dirs if d.startswith("src/platform/")}
    inc = CPP / "include" / "maui"
    roots += [p for p in inc.iterdir() if p.is_dir() and p.name != "platform"] if inc.is_dir() else []
    plat = inc / "platform"
    if plat.is_dir():
        roots += [p for p in plat.iterdir() if p.is_dir() and p.name in compiled_platforms]
    # appkit names its columns appkit_cpp / appkit_xaml; catalyst, ios and windows use cpp / cpp_xaml.
    xaml_col = column in ("cpp_xaml", "appkit_xaml")
    roots.append(CPP / "examples" / ("gallery_xaml" if xaml_col else "gallery"))
    if xaml_col:
        roots.append(PORT / "maui-reference" / "pages")
    return roots


def windows_facts(artifacts: list[str]) -> dict:
    """scp the probe and run it. Read-only on the guest, so it does not contend with a live capture."""
    remote = "C:/Users/Testings-VM/artifact_facts.ps1"
    subprocess.run(["scp", "-q", "-o", "ConnectTimeout=20", str(PROBE), f"{WINDOWS_VM}:{remote}"],
                   check=True, timeout=180)
    out = subprocess.run(
        ["ssh", "-o", "ConnectTimeout=20", WINDOWS_VM,
         f"powershell -NoProfile -ExecutionPolicy Bypass -File {remote} -Artifacts '{','.join(artifacts)}'"],
        check=True, capture_output=True, text=True, timeout=600).stdout
    return json.loads(out.replace("\r", "").strip())


def verdicts(facts: dict, artifacts: dict[str, str], columns: list[str] | None = None) -> list[str]:
    """The stale/fresh DECISION, separated from the SSH that feeds it so the selftest can pin it.

    Empty list == every artifact is newer than its source."""
    want = [c for c in (columns if columns is not None else artifacts) if c in artifacts]
    import datetime as _dt

    def epoch(iso: str) -> float:
        return _dt.datetime.fromisoformat(iso.replace("Z", "+00:00")).timestamp()

    problems = []
    for col in want:
        path = artifacts[col]
        info = facts["artifacts"].get(path, {"exists": False})
        if not info.get("exists"):
            how = ("the MAUI reference is built BY HAND on every lane -- see build()'s docstring"
                   if col in ("maui_xaml",) else "run the lane's release build")
            problems.append(f"{col}: DECLARED BUT MISSING -- {path}. Nothing can be captured from it "
                            f"({how}).")
            continue
        art = epoch(info["mtime"])
        src, where = newest(source_roots(col, facts.get("source_dirs", [])))
        if src > art + TOLERANCE_S:
            gap = src - art
            rel = where.relative_to(REPO) if where else "?"
            msg = (f"{col}: STALE ARTIFACT. {path} was built "
                   f"{_dt.datetime.fromtimestamp(art):%Y-%m-%d %H:%M:%S}, but {rel} was modified "
                   f"{_dt.datetime.fromtimestamp(src):%Y-%m-%d %H:%M:%S} -- {gap / 3600:.1f}h newer. "
                   f"Capturing it would score source that never went into the binary.")
            if col == "maui_xaml":
                msg += (" This is the GROUND TRUTH column: its staleness reads as a port regression. "
                        "The signature to recognise on the board is pixel_xaml going green->yellow "
                        "while both port columns stay byte-identical to each other -- the port moved "
                        "and the reference did not.")
            problems.append(msg)
    return problems


def stale_windows(columns: list[str]) -> list[str]:
    want = [c for c in columns if c in WINDOWS_ARTIFACTS]
    if not want:
        return []
    return verdicts(windows_facts([WINDOWS_ARTIFACTS[c] for c in want]), WINDOWS_ARTIFACTS, want)


# --------------------------------------------------------------------------- the local lanes
# ios / catalyst / appkit deploy from LOCAL paths, so their facts need no SSH at all -- the same
# decision function, fed by a stat instead of a probe.
#
# WHY THEY NEED THE GUARD AT ALL, given build() runs first: build() covers the C++ GALLERIES ONLY.
# It says so in its own docstring -- "the MAUI reference app is built by hand". So the ground-truth
# column, the one every score is measured against, is hand-built on EVERY lane and is exactly what
# went a MONTH stale on Catalyst across 61 changed twins. And on ios the galleries are skippable too:
# lane_ios does `if not skip_build: build(...); ios_install(...)`, so --skip-build leaves all three
# columns at whatever was last built.
IOS_APPS = {
    # The bundles ios_install() deploys. Defined HERE rather than inside ios_install so there is one
    # definition: recapture.py imports these back. The reverse (freshness importing recapture) would
    # be an import cycle, since recapture imports freshness for the gate.
    # RELEASE on both sides -- see build(): a Release port against a Debug reference is a strawman,
    # and a Debug MAUI bundle additionally runs the INTERPRETED XAML loader, which disagrees with
    # XamlC on exactly the SelectedItems arms this board scores.
    "maui_xaml": PORT / "maui-reference/app/bin/Release/net10.0-ios/iossimulator-arm64/MauiReference.app",
    "cpp": CPP / "examples/build-ios-release/gallery/gallery.app",
    "cpp_xaml": CPP / "examples/build-ios-release/gallery_xaml/gallery_xaml.app",
}
# lane -> the build dir whose object tree scopes that lane's sources (see local_compiled_dirs).
LANE_BUILD_ROOT = {"ios": CPP / "examples/build-ios-release",
                   "catalyst": CPP / "examples/build-maccatalyst-release",
                   "appkit": CPP / "examples/build-apple-release"}


def executable(artifact: Path) -> Path:
    """The BINARY inside a deployable, under whichever of the three shapes this repo uses.

    Never the bundle directory: ios_install() spells out why -- CMake creates the .app skeleton at
    CONFIGURE time, so an exists() check on it passed on a build that had compiled nothing, and the
    lane published 73 stale port frames as a fresh Release board.

    The process name is the artifact's own stem (MauiReference.app -> MauiReference, gallery.app ->
    gallery, and the AppKit target DIRECTORY build-apple-release/gallery -> gallery), so this needs no
    second table of process names to drift out of step with local.toml's `process` keys.
        Catalyst/AppKit .app  <bundle>/Contents/MacOS/<stem>
        iOS .app              <bundle>/<stem>
        AppKit target dir     <dir>/<stem>
    """
    if artifact.is_file():
        return artifact
    for cand in (artifact / "Contents" / "MacOS" / artifact.stem, artifact / artifact.stem):
        if cand.is_file():
            return cand
    return artifact / artifact.stem      # absent: reported as DECLARED BUT MISSING under this name


def local_compiled_dirs(build_root: Path) -> list[str]:
    """The local twin of artifact_facts.ps1's derivation, and for the same reason: ask the BUILD which
    sources it compiled instead of hand-maintaining a per-platform exclude list.

    Measured 2026-08-22 it returns, per lane, exactly the platform dirs that lane compiles --
    catalyst and ios: apple_shared + headless + ios (Catalyst reuses the iOS UIKit backend verbatim);
    appkit: apple + apple_shared + headless -- and never windows or android. The examples build dir
    holds the framework objects too (the framework is add_subdirectory'd into it), so one root covers
    both halves, exactly as on the guest."""
    dirs = set()
    for obj in build_root.rglob("*.o"):
        rel = str(obj).split(".dir/", 1)[-1]
        d = os.path.dirname(rel)
        if d.startswith("src/"):
            dirs.add(d)
    return sorted(dirs)


def local_artifacts(lane: str, columns: list[str]) -> dict[str, str]:
    """column -> the EXECUTABLE this lane will actually run.

    For the two VM lanes this defers to run_comparison.column_artifact rather than reading the config
    itself, because that function encodes a trap worth not re-deriving: local.toml's `artifact` keys
    point at DEBUG paths while `artifact_release` holds the Release ones, the board captures RELEASE
    on every lane, and a declared-but-unbuilt `artifact_release` must be a loud skip and never a
    fall back to the Debug bundle. Reading `artifact` here would have this guard certifying the
    freshness of a bundle the runner is not going to deploy."""
    if lane == "ios":
        return {c: str(executable(p)) for c, p in IOS_APPS.items() if c in columns}
    import tomllib
    sys.path.insert(0, str(COMP / "tools"))
    import run_comparison
    cfg = tomllib.loads((COMP / "config" / "local.toml").read_text())
    env = "macos-arm64" if lane == "catalyst" else "macos-appkit"
    cols = cfg["environments"][env]["columns"]
    out = {}
    for name in columns:
        ccfg = cols.get(name)
        if ccfg is None:
            continue
        art = run_comparison.column_artifact(name, dict(ccfg))
        # None == declared-but-not-built; column_artifact already said so on stdout. Point at the
        # release path anyway so the verdict names it rather than silently dropping the column.
        out[name] = str(executable(art if art else REPO / ccfg.get("artifact_release", ccfg["artifact"])))
    return out


def stale_local(lane: str, columns: list[str]) -> list[str]:
    import datetime as _dt
    arts = local_artifacts(lane, columns)
    if not arts:
        return []
    facts = {"now": _dt.datetime.now(_dt.timezone.utc).isoformat(),
             "source_dirs": local_compiled_dirs(LANE_BUILD_ROOT[lane]),
             "artifacts": {}}
    for path in arts.values():
        p = Path(path)
        st = p.stat() if p.is_file() else None
        facts["artifacts"][path] = {
            "exists": st is not None,
            "mtime": _dt.datetime.fromtimestamp(st.st_mtime, _dt.timezone.utc).isoformat() if st else "",
            "length": st.st_size if st else 0}
    return verdicts(facts, arts)


# --------------------------------------------------------------------------- android (installed pkg)
def android_last_update(dumpsys: str) -> str | None:
    """`dumpsys package <pkg>` text -> the package's lastUpdateTime as an ISO string (None if absent).

    Split out from the adb call so the PARSE — the only part that can silently misread — is pinned by
    selftest() with no device attached. dumpsys prints `lastUpdateTime=2026-08-22 02:44:31`.

    ponytail: the device stamp is naive local time and is read as HOST-local. True for an emulator on
    the host clock, which is the only android lane here. If a device ever runs a different timezone this
    is the knob to fix — parse its `persist.sys.timezone` instead of assuming.
    """
    import datetime as _dt
    import re
    m = re.search(r"lastUpdateTime=(\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2})", dumpsys)
    if not m:
        return None
    return _dt.datetime.strptime(m.group(1), "%Y-%m-%d %H:%M:%S").astimezone().isoformat()


def stale_android(columns: list[str], serial: str = "emulator-5554") -> list[str]:
    """The android freshness probe: is the INSTALLED PACKAGE newer than the source it claims to render?

    Android is the one lane whose artifact is not a file. capture_all_csharp_android.sh "only DRIVES an
    already-installed app", so a file-mtime check measures the wrong object — measured 2026-08-22 the
    newest reference APK on disk was 08-14 18:10 (older than four twin edits) and the Release APK's mtime
    was 1981, while the DEVICE reported lastUpdateTime 08-22 02:44 and was correct. Guarding on those
    files would have failed a healthy lane; worse, `adb install -r` of either would have silently
    replaced a good reference with a stale one, and the install would have reported success.

    So the probe asks the device. The stale/fresh DECISION and its wording are verdicts() — shared with
    every other lane — fed a synthesized artifact whose "mtime" is the package's lastUpdateTime.
    """
    import datetime as _dt
    sys.path.insert(0, str(HERE))
    import capture_android  # noqa: PLC0415  the column->package map, single source of truth (APPS)

    arts = {spec["col"]: f"package:{spec['pkg']}"
            for spec in capture_android.APPS.values() if spec["col"] in columns}
    if not arts:
        return []
    facts = {"now": _dt.datetime.now(_dt.timezone.utc).isoformat(), "source_dirs": [], "artifacts": {}}
    for col, art in arts.items():
        pkg = art.split(":", 1)[1]
        try:
            out = subprocess.run([os.environ.get("ADB", "adb"), "-s", serial, "shell",
                                  "dumpsys", "package", pkg],
                                 capture_output=True, text=True, timeout=30).stdout
        except Exception:                                    # noqa: BLE001  no adb / no device
            out = ""
        stamp = android_last_update(out)
        facts["artifacts"][art] = {"exists": stamp is not None, "mtime": stamp or "", "length": 1}
    return verdicts(facts, arts)

# --------------------------------------------------------------------------- score contradictions
def _md5(p: Path) -> str | None:
    try:
        return hashlib.md5(p.read_bytes()).hexdigest()
    except OSError:
        return None


def score_contradictions(plat_dir: str, root: "Path | None" = None) -> list[str]:
    """Cells whose recorded review cannot describe the captures now on disk. See the module header.

    `root` overrides the board directory so selftest() can exercise this on a SYNTHETIC board. That
    matters: this arm used to be asserted against the LIVE board, with a comment defending it because
    "the windows board really does carry contradictions today". It stopped being true the moment those
    contradictions were fixed (2026-08-22, the first honest `pixel_score --platform windows`), and the
    selftest went red for the entirely correct reason that the defect it needed was gone. A test whose
    pass depends on live data being broken fails when the data is repaired -- which is the one time you
    least want a red guard, because a red guard is one nobody trusts a week later."""
    base = root or COMP
    jf = base / "comparison.json"
    if not jf.is_file():
        return []
    caps = base / "captures" / plat_dir
    out = []
    for entry in json.loads(jf.read_text()):
        name = entry["name"]
        plat = (entry.get("platforms") or {}).get(plat_dir) or {}
        a, b = plat.get("pixel") or {}, plat.get("pixel_xaml") or {}
        ra, rb = a.get("review"), b.get("review")
        if not ra or not rb or ra == rb:
            continue
        # STILLS ONLY. A motion review embeds run ids and commit hashes, so two identical PNGs do not
        # imply two identical reviews for an animated page -- without this the check fires on ~40
        # windows cells that are not stale at all.
        if not (ra.startswith("Light: SSIM") and rb.startswith("Light: SSIM")):
            continue
        seen, identical = 0, True
        for theme in ("light", "dark"):
            for ext in ("png", "gif"):
                x, y = _md5(caps / "cpp" / f"{name}_{theme}.{ext}"), _md5(caps / "xaml" / f"{name}_{theme}.{ext}")
                if x is None and y is None:
                    continue
                if x != y:
                    identical = False
                seen += 1
        if seen and identical:
            out.append(f"{name}/{plat_dir}: cpp and xaml captures are BYTE-IDENTICAL yet carry different "
                       f"recorded scores, so at least one is stale (it was carried over from an older "
                       f"capture and no longer describes the files on disk).\n"
                       f"      pixel      [{a.get('status')}] {ra}\n"
                       f"      pixel_xaml [{b.get('status')}] {rb}\n"
                       f"      Fix by rescoring, never by editing comparison.json: "
                       f"pixel_score.py --platform {plat_dir} --only {name}"
                       f"  (--verify first to see the move without writing).")
    return out


def unverified_cells(plat_dir: str) -> list[tuple[str, str, str]]:
    """(page, slot, why) for cells whose recorded verdict was NOT taken on the stills now on disk.

    THE SECOND STALENESS AXIS, and a different one from stale_windows(): there the BINARY lags the
    source; here the BOARD lags the CAPTURES. A published number that nobody has recomputed since the
    pictures changed is not a measurement, and it reads exactly like a current one.

    Measured cost, 2026-08-22: an agent was handed 14 android cells to debug and found 11 ALREADY
    GREEN on the stills then on disk -- `border` was published as 0.9605 / 3.49% while the actual
    score was 0.9943 / 0.29%, because comparison.json lagged a recapture that had landed 20 minutes
    before. That is most of a working session spent hunting bugs that were already fixed.

    WHY NOT MTIMES -- checked, and it fails on exactly that case. comparison.json's own mtime was
    15:36, NEWER than border's 15:16/15:18 captures, so "is any capture newer than the board file?"
    answers NO for a cell that is provably stale. build_comparison_json.py CARRIES pixel scores over
    (it cannot recompute them), so every board refresh bumps the file's mtime while the carried score
    underneath stays as old as it was. Only content can answer this, which is what stills_fingerprint
    already computes -- so this reads that back rather than inventing a second mechanism.

    A cell with no recorded fingerprint is reported as UNVERIFIED, never as fresh: entries written
    before pixel_score started stamping stills carry no evidence either way, and silently calling
    those fresh would reintroduce the whole failure.

    DELIBERATELY NOT A LANE GATE, unlike the other two checks here. Until a full rescore has stamped
    the board, the "no fingerprint" arm covers every still cell -- measured at first run, 258 per
    platform -- so gating on it would abort every lane on every platform forever, and the gate would
    be switched off within the day. It is a TRIAGE tool: run it before debugging a cell, which is
    exactly the moment the android agent needed it and did not have it. The arm that matters
    long-term ("the stills have changed") is already sharp, and the weak arm empties itself as soon
    as the next scoring pass runs.
    """
    jf = COMP / "comparison.json"
    if not jf.is_file():
        return []
    sys.path.insert(0, str(HERE))
    import pixel_score

    out = []
    for entry in json.loads(jf.read_text()):
        plat = (entry.get("platforms") or {}).get(plat_dir) or {}
        sc = plat.get("screenshots") or {}
        for fw, slot in pixel_score.SLOTS:          # the pairing is pixel_score's to define, not ours
            cell = plat.get(slot)
            if not cell or not cell.get("review"):
                continue
            paths = ([(sc.get("maui") or {}).get(t) for t in ("light", "dark")] +
                     [(sc.get(fw) or {}).get(t) for t in ("light", "dark")])
            recorded = cell.get("stills") or (cell.get("motion") or {}).get("stills")
            if not recorded:
                out.append((entry["name"], slot, "no fingerprint recorded — scored before stamping"))
            elif recorded != pixel_score.stills_fingerprint(paths):
                out.append((entry["name"], slot, "the stills have changed since this verdict"))
    return out


# A run imports all of its frames within minutes, so two columns of one run sit well under an hour
# apart. 2h is generous headroom for a slow import, and every real finding below is 60h-500h -- there
# is no threshold sensitivity here worth tuning.
SAME_RUN_FLOOR_H = 2.0


def column_skew(plat_dir: str, limit: int | None = None) -> list[str]:
    """Cells whose two columns were captured far enough apart to be different worlds.

    THE THIRD QUESTION, and the one the other two cannot ask. score_contradictions asks "do these two
    recorded scores agree?"; unverified_cells asks "does this verdict describe the current stills?".
    Both can pass on a cell that is perfectly self-consistent and still compares a MAUI frame from one
    world against a port frame from another. Found in use on header_footer_grid_horizontal, which
    returned 0 from --unverified while its ios maui DARK still was from 08-01 and its port dark still
    from 08-22 -- twenty days and an unknown number of commits apart, with a whole "latched frame"
    analysis built on top of it before anyone checked.

    The damage is not hypothetical. A date/time page compared across a MIDNIGHT ROLLOVER produced an
    8px width difference that mimicked a layout defect and fooled three separate diagnoses (7/31/2026
    is wider than 8/1/2026). A display re-pin changes capture geometry outright -- the Catalyst lane
    went 1512x950 -> 1920x1080 between this cell's two columns.

    PER THEME, not per cell. The scorer compares maui_light against port_light and maui_dark against
    port_dark separately, so the THEME is the comparison unit. Measuring max-minus-min across all four
    files instead folds in light-vs-dark skew INSIDE one column, which is not a defect at all -- the
    capture loop is theme-outermost, so a three-hour run legitimately separates its own light and dark
    passes by hours. Measured: that mistake alone turned windows from 0 flagged cells into 28.

    ONE-SIDED. Only a LARGE spread is a problem; a spread of zero is the ideal and must never be
    flagged. An early cut scored "deviation from the lane baseline", which on ios put the perfectly
    co-captured cells (spread 0.00h) at the TOP of the list -- exactly backwards.

    NORMALISED PER LANE, because an absolute threshold is meaningless across these four. Measured
    baselines (median spread) 2026-08-22: windows 0.00h -- it imports all three columns in one run --
    maccatalyst 0.00h, android 67.9h, ios 421.1h. On ios and android, columns from different runs are
    the NORMAL state, so a fixed threshold would flag ~300 of 344 cells there and nothing anywhere
    else, and a report that fires on most of the board stops being read. Comparing each cell against
    its OWN lane's baseline flags 0 / 0 / 16 / 32 respectively, and the lane-wide condition is stated
    ONCE instead of 344 times. It also self-calibrates: recapture the stale column and the baseline
    drops, with no threshold to revisit.
    """
    jf = COMP / "comparison.json"
    if not jf.is_file():
        return []
    sys.path.insert(0, str(HERE))
    import pixel_score
    caps = COMP / "captures" / plat_dir
    rows = []
    for entry in json.loads(jf.read_text()):
        name = entry["name"]
        plat = (entry.get("platforms") or {}).get(plat_dir) or {}
        for fw, slot in pixel_score.SLOTS:
            if not (plat.get(slot) or {}).get("review"):
                continue
            worst = None
            for theme in ("light", "dark"):
                for ext in ("png", "gif"):
                    a, b = caps / "maui" / f"{name}_{theme}.{ext}", caps / fw / f"{name}_{theme}.{ext}"
                    if a.is_file() and b.is_file():
                        gap = abs(a.stat().st_mtime - b.stat().st_mtime) / 3600
                        worst = gap if worst is None else max(worst, gap)
            if worst is not None:
                rows.append((worst, f"{name}/{slot}"))
    if not rows:
        return []
    baseline = statistics.median(x for x, _ in rows)
    threshold = baseline + SAME_RUN_FLOOR_H
    out = []
    if baseline > SAME_RUN_FLOOR_H:
        out.append(f"{plat_dir}: THE WHOLE LANE compares columns from different runs — median gap "
                   f"{baseline:.1f}h between the MAUI still and the port still. Recapture the lagging "
                   f"column; until then every score on this lane carries that caveat, and the cells "
                   f"below are the ones worse than even this baseline.")
    flagged = sorted(((x, n) for x, n in rows if x > threshold), reverse=True)
    shown = flagged if limit is None else flagged[:limit]
    for gap, cell in shown:
        out.append(f"{cell} ({plat_dir}): columns captured {gap:.1f}h apart — the MAUI still and the "
                   f"port still are from different runs, so this cell can be perfectly self-consistent "
                   f"and still compare two different worlds (config re-pin, midnight rollover).")
    if limit is not None and len(flagged) > limit:
        out.append(f"{plat_dir}: …and {len(flagged) - limit} more cross-run cell(s) — "
                   f"freshness.py --skew --board-dir {plat_dir}")
    return out


def check(platform: str, plat_dir: str, frameworks: list[str], scores: bool = True,
          lane: str = "") -> tuple[list[str], list[str]]:
    """(fatal, advisory) for a lane about to capture.

    `plat_dir` is the BOARD directory, which is not the platform name -- macos writes `maccatalyst`.
    Deriving it here from `platform` would have silently checked a captures/macos/ that does not
    exist, i.e. reported "clean" forever.

    `scores` is False on a lane that does not own the cpp/xaml capture dirs (appkit writes
    appkit_cpp/appkit_xaml), so the same board cells are not reported twice per platform.

    ONLY ARTIFACT STALENESS IS FATAL, and the split matters. A stale BINARY is a genuine
    precondition: capturing it produces wrong frames, and no amount of capturing fixes it -- somebody
    has to rebuild. A stale SCORE is the opposite: the capture about to run REPLACES the captures and
    measure() rescores them, so the run being gated is precisely the thing that resolves it. Aborting
    on it would refuse the cure and, being unfixable from inside the lane, would get the whole gate
    switched off. So the score checks are reported into the log and the run proceeds.
    """
    fatal = stale_windows(frameworks) if platform == "windows" else stale_local(lane, frameworks)
    advisory = score_contradictions(plat_dir) if scores else []
    # SUMMARISED into the lane log (the lane line plus the three worst), full list on demand. A
    # maccatalyst lane would otherwise open with 32 of these, and a wall of advisory text is how a
    # report stops being read -- the same failure --unverified is deliberately kept out of the gate for.
    advisory += column_skew(plat_dir, limit=3) if scores else []
    return fatal, advisory


# --------------------------------------------------------------------------- selftest
def selftest() -> None:
    """The stale/fresh decision and the contradiction rule, with no guest and no board."""
    import tempfile
    import time

    with tempfile.TemporaryDirectory() as td:
        root = Path(td)
        (root / "src").mkdir()
        (root / "src" / "a.cpp").write_text("x")
        old = time.time() - 86400
        os.utime(root / "src" / "a.cpp", (old, old))
        m, w = newest([root / "src"])
        assert w and w.name == "a.cpp" and abs(m - old) < 2, (m, w)

        # PRUNED dirs must not count: a freshly written board PNG under docs/ is not a source edit,
        # and treating it as one would refuse every run made after a capture.
        (root / "src" / "docs").mkdir()
        (root / "src" / "docs" / "board.png").write_text("new")
        m2, w2 = newest([root / "src"])
        assert w2 and w2.name == "a.cpp", w2       # still the old .cpp, docs/ never walked
        assert m2 == m

        # …and an unpruned sibling DOES count.
        (root / "src" / "b.cpp").write_text("new")
        m3, w3 = newest([root / "src"])
        assert w3 and w3.name == "b.cpp" and m3 > m, w3

        assert newest([root / "nope"]) == (0.0, None)   # missing root is skipped, not fatal

    # Dependency scoping. The object tree decides which platform dirs count, so an iOS edit is
    # invisible to a Windows build and a twin edit reaches BOTH xaml-bearing columns.
    compiled = ["src/core", "src/xaml", "src/platform/headless", "src/platform/windows"]
    cpp_roots = source_roots("cpp", compiled)
    xaml_roots = source_roots("cpp_xaml", compiled)
    ref_roots = source_roots("maui_xaml", compiled)
    assert CPP / "src/platform/windows" in cpp_roots
    assert CPP / "src/platform/headless" in cpp_roots         # headless DOES compile in -- not an exclude
    assert CPP / "src/platform/ios" not in cpp_roots          # …and iOS does not
    assert CPP / "include/maui/platform/ios" not in cpp_roots  # same rule for headers
    assert CPP / "examples/gallery" in cpp_roots and CPP / "examples/gallery_xaml" not in cpp_roots
    twins = PORT / "maui-reference" / "pages"
    assert twins in xaml_roots, "a twin edit must invalidate cpp_xaml (CMakeLists.txt:80 globs them)"
    assert twins not in cpp_roots, "…but not the non-XAML gallery, which does not read them"
    # EVERY xaml column, under either naming. appkit calls its columns appkit_cpp/appkit_xaml, and a
    # rule written only for "cpp_xaml" silently stops guarding the AppKit XAML column -- the twins are
    # the dependency most likely to move (they are edited to author pages) and least likely to be
    # noticed, since no port source is touched at all.
    assert twins in source_roots("appkit_xaml", compiled), "appkit_xaml reads the twins too"
    assert twins not in source_roots("appkit_cpp", compiled)
    for xcol in ("cpp_xaml", "appkit_xaml"):
        assert CPP / "examples/gallery_xaml" in source_roots(xcol, compiled), xcol
    for ccol in ("cpp", "appkit_cpp"):
        assert CPP / "examples/gallery" in source_roots(ccol, compiled), ccol
    assert ref_roots == [PORT / "maui-reference"]

    # --- THE STALE/FRESH DECISION, replayed against the two incidents that motivated this guard.
    # Synthetic facts, so it runs with no guest. The source side is the REAL tree, so "fresh" is
    # pinned to an artifact newer than whatever is actually checked out.
    import datetime as _dt

    def iso(ts: float) -> str:
        return _dt.datetime.fromtimestamp(ts, _dt.timezone.utc).isoformat().replace("+00:00", "Z")

    def facts_at(col: str, when: float) -> dict:
        return {"source_dirs": compiled,
                "artifacts": {WINDOWS_ARTIFACTS[col]: {"exists": True, "mtime": iso(when), "length": 1}}}

    def verdict(col: str, when: float) -> list[str]:
        return verdicts(facts_at(col, when), WINDOWS_ARTIFACTS, [col])

    now = time.time()
    for col in ("cpp", "cpp_xaml", "maui_xaml"):
        src, _ = newest(source_roots(col, compiled))
        assert verdict(col, src + 3600) == [], col          # built an hour after: fresh
        # `containers` was gallery.exe from 08-18; `selection_synchronization` was MauiReference.exe
        # from 08-19 against a twin edited 08-20. Both are "artifact a day or more behind its source".
        late = verdict(col, src - 86400)
        assert len(late) == 1 and "STALE ARTIFACT" in late[0], (col, late)
        assert f"{24.0:.1f}h newer" in late[0], late[0]
    # The ground-truth column must additionally name the board signature, because a stale reference
    # is the one that reads as a PORT regression rather than as a missing build.
    assert "green->yellow" in verdict("maui_xaml", 0.0)[0]
    # Skew and build duration must not fire; a day must.
    src, _ = newest(source_roots("cpp", compiled))
    assert verdict("cpp", src - TOLERANCE_S + 30) == [], "within tolerance"
    assert verdict("cpp", src - TOLERANCE_S - 30) != [], "past tolerance"

    # A declared-but-missing artifact is a LOUD problem, never a silent pass -- matching how this
    # lane already treats a missing artifact everywhere else.
    missing = {"source_dirs": compiled,
               "artifacts": {WINDOWS_ARTIFACTS["cpp"]: {"exists": False, "mtime": "", "length": 0}}}
    assert "DECLARED BUT MISSING" in verdicts(missing, WINDOWS_ARTIFACTS, ["cpp"])[0]

    # --- THE LOCAL LANES. The three things that would silently certify the wrong file.
    # 1. The EXECUTABLE, never the bundle dir -- the whole point of the 73-stale-frames incident.
    for lane, col in (("ios", "cpp"), ("catalyst", "cpp"), ("appkit", "appkit_cpp")):
        arts = local_artifacts(lane, [col])
        assert col in arts, (lane, arts)
        exe = Path(arts[col])
        assert exe.name in ("gallery", "gallery.exe"), exe
        assert exe.parent.name != "examples", exe          # not the build dir itself
        assert not exe.name.endswith(".app"), exe          # not the bundle
    # 2. Release, not Debug -- local.toml's `artifact` points at Debug and the board captures Release;
    #    deploying Debug MAUI additionally swaps XamlC for the INTERPRETED loader, which disagrees on
    #    exactly the SelectedItems arms this board scores.
    ref = local_artifacts("catalyst", ["maui_xaml"]).get("maui_xaml", "")
    assert "/bin/Release/" in ref and "/bin/Debug/" not in ref, ref
    assert local_artifacts("ios", ["maui_xaml"])["maui_xaml"].count("/bin/Release/") == 1
    # 3. Per-lane source scoping, derived from each lane's OWN object tree. Catalyst reuses the iOS
    #    UIKit backend verbatim, AppKit does not -- so one edited .mm is in scope for two lanes and
    #    out of scope for the other two. Hand-maintaining that would be wrong within a week.
    for lane, want, unwanted in (("ios", "src/platform/ios", "src/platform/apple"),
                                 ("catalyst", "src/platform/ios", "src/platform/apple"),
                                 ("appkit", "src/platform/apple", "src/platform/ios")):
        root = LANE_BUILD_ROOT[lane]
        if not root.is_dir():
            continue                                       # lane never built here; nothing to pin
        dirs = local_compiled_dirs(root)
        assert want in dirs, (lane, want, dirs)
        assert unwanted not in dirs, (lane, unwanted, dirs)
        assert "src/platform/windows" not in dirs and "src/platform/android" not in dirs, (lane, dirs)

    # The guest artifact paths are DERIVED from windows.toml, so a config move must reach the guard.
    assert set(WINDOWS_ARTIFACTS) == {"maui_xaml", "cpp", "cpp_xaml"}, WINDOWS_ARTIFACTS
    assert all(v.endswith(".exe") for v in WINDOWS_ARTIFACTS.values()), WINDOWS_ARTIFACTS

    # Board staleness. A cell is fresh ONLY when its recorded fingerprint matches the stills it
    # names; absent evidence must read as unverified, never as fresh.
    # SKIPPED IF pixel_score WILL NOT IMPORT. recapture.py --selftest chains this one, and it was
    # deliberately dependency-light and device-free; pixel_score pulls in numpy, PIL and
    # motion_score, so without this guard an unrelated break in any of them fails a selftest that has
    # nothing to do with them.
    sys.path.insert(0, str(HERE))
    try:
        import pixel_score
    except ImportError as exc:
        print(f"freshness selftest: board-staleness arm SKIPPED ({exc})")
        return
    assert pixel_score.SLOTS == [("cpp", "pixel"), ("xaml", "pixel_xaml")], pixel_score.SLOTS
    rows = unverified_cells("windows")
    assert all(w.startswith(("no fingerprint", "the stills have changed")) for _, _, w in rows)
    # THE FATAL/ADVISORY SPLIT. Only a stale ARTIFACT may abort a lane; a stale SCORE is resolved by
    # the very run that would be blocked, so it must land in the advisory list. `frameworks=[]` keeps
    # this off the guest, and the windows board really does carry contradictions today, so the
    # advisory arm is exercised rather than merely asserted empty.
    fatal, advisory = check("windows", "windows", [], scores=True)
    assert fatal == [], fatal
    # NOT `assert advisory and ...` — that required the LIVE board to be broken, and went red the day
    # the contradictions were fixed. The arm itself is exercised on a synthetic board below.
    assert all("BYTE-IDENTICAL" in a for a in advisory), advisory
    assert not any("fingerprint" in x for x in fatal + advisory), "unverified cells are triage-only"

    # The android parse (no device): the real dumpsys shape, the absent case, and the near-miss that
    # must NOT match -- a bare `lastUpdateTime=` with no stamp, which would otherwise read as fresh.
    sample = ("  Package [dev.mauicpp.mauireference] (a1b2c3):\n"
              "    firstInstallTime=2026-08-01 09:12:00\n"
              "    lastUpdateTime=2026-08-22 02:44:31\n")
    got = android_last_update(sample)
    assert got and got.startswith("2026-08-22T02:44:31"), got
    assert android_last_update("no such field") is None
    assert android_last_update("lastUpdateTime=") is None
    # and it must pick lastUpdateTime, not the firstInstallTime printed above it
    assert "09:12" not in (got or ""), got

    # --- CROSS-RUN COLUMNS. Three properties, each of which was wrong in an earlier cut and each of
    # which fails silently (the check keeps returning plausible-looking rows).
    import tempfile as _tf
    with _tf.TemporaryDirectory() as td:
        caps = Path(td) / "captures" / "t"
        for col in ("maui", "cpp"):
            (caps / col).mkdir(parents=True)
        def put(col, theme, when):
            f = caps / col / f"p_{theme}.png"
            f.write_bytes(b"x")
            os.utime(f, (when, when))
        now = time.time()
        # PER THEME: light co-captured, dark 20h apart. Folding light-vs-dark skew INSIDE a column in
        # (max-minus-min over all four files) would also fire on a healthy theme-outermost run.
        put("maui", "light", now); put("cpp", "light", now)
        put("maui", "dark", now - 20 * 3600); put("cpp", "dark", now)
        worst = max(abs((caps / "maui" / f"p_{t}.png").stat().st_mtime
                        - (caps / "cpp" / f"p_{t}.png").stat().st_mtime) / 3600
                    for t in ("light", "dark"))
        assert 19.9 < worst < 20.1, worst
        intra = (max(f.stat().st_mtime for c in ("maui", "cpp") for f in (caps / c).iterdir())
                 - min(f.stat().st_mtime for c in ("maui", "cpp") for f in (caps / c).iterdir())) / 3600
        assert intra > worst or abs(intra - worst) < 0.1, (intra, worst)

    # ONE-SIDED and PER-LANE, checked against the real board: windows imports all three columns in one
    # run, so it must be SILENT -- a rule that scored "deviation from the lane baseline" instead put
    # the perfectly co-captured cells at the top of the list, which is exactly backwards.
    assert column_skew("windows") == [], "windows imports its columns together; skew must be silent"
    for board in ("ios", "maccatalyst"):
        rows = column_skew(board)
        assert all(("apart" in r) or ("THE WHOLE LANE" in r) for r in rows), rows
    # …and it must stay narrow enough to be read. This used to `assert len(column_skew(b)) < 100` on
    # every LIVE lane, which is the same disease as the score-contradiction arm below: a selftest that
    # passes only while the live board is healthy. It went red on 2026-08-22 reporting
    # `('android', 167)` — a TRUE finding, not broken normalisation. The android lane had had only its
    # two PORT columns recaptured, so its MAUI column was cross-run against them, exactly the state the
    # iOS lane spent a 1032-capture pass eliminating (its skew is now 0). Raising the threshold to make
    # that green would have been tuning a number to hide a real defect.
    #
    # So: the NARROWNESS property is asserted on synthetic input, and the live lanes are reported.
    for board in ("ios", "android", "windows", "maccatalyst"):
        n = len(column_skew(board))
        if n >= 100:
            print(f"  note: {board} carries {n} column-skew findings — its columns are cross-run; "
                  f"a single-sitting three-column recapture is the remedy, not a threshold change")
    # Advisory only, never fatal -- same line as the other two score checks.
    f2, a2 = check("windows", "windows", [], scores=True)
    assert f2 == [], f2

    # The contradiction rule, exercised on a SYNTHETIC board so the arm is tested whether or not the
    # live board happens to be broken today. Two byte-identical stills carrying different reviews is
    # the defect; a matching pair in the same fixture proves it does not fire on healthy cells.
    import tempfile
    with tempfile.TemporaryDirectory() as td:
        root = Path(td)
        for col in ("maui", "cpp", "xaml"):
            (root / "captures" / "windows" / col).mkdir(parents=True)
        for col, blob in (("maui", b"M"), ("cpp", b"X"), ("xaml", b"X")):
            for page in ("contradicts", "agrees"):
                for theme in ("light", "dark"):
                    (root / "captures" / "windows" / col / f"{page}_{theme}.png").write_bytes(blob)
        (root / "comparison.json").write_text(json.dumps([
            {"name": "contradicts", "platforms": {"windows": {
                "pixel":      {"review": "Light: SSIM 0.9978, 0.09% pixels differ"},
                "pixel_xaml": {"review": "Light: SSIM 0.9711, 0.64% pixels differ"}}}},
            {"name": "agrees", "platforms": {"windows": {
                "pixel":      {"review": "Light: SSIM 0.9978, 0.09% pixels differ"},
                "pixel_xaml": {"review": "Light: SSIM 0.9978, 0.09% pixels differ"}}}},
        ]))
        syn = score_contradictions("windows", root=root)
        assert len(syn) == 1, syn                      # fires on the contradiction...
        assert "contradicts" in syn[0], syn            # ...on the right page...
        assert "BYTE-IDENTICAL" in syn[0], syn         # ...with the right wording
    # And reported against the live board, informationally — never asserted, see the docstring.
    real = score_contradictions("windows")
    assert all("BYTE-IDENTICAL" in r for r in real)
    print(f"freshness selftest OK ({len(real)} live contradiction(s) on the windows board)")


def main() -> int:
    import argparse
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("--platform", default="windows")
    ap.add_argument("--board-dir", default=None,
                    help="board captures/ directory (defaults to --platform; macos writes maccatalyst)")
    ap.add_argument("--columns", default="maui_xaml,cpp,cpp_xaml")
    ap.add_argument("--lane", default="", help="ios | catalyst | appkit (ignored for windows)")
    ap.add_argument("--scores-only", action="store_true", help="skip the guest probe (no SSH)")
    ap.add_argument("--skew", action="store_true",
                    help="list cells whose two columns were captured in different runs — a cell can be "
                         "perfectly self-consistent and still compare two different worlds")
    ap.add_argument("--unverified", action="store_true",
                    help="list cells whose published verdict was NOT taken on the stills now on disk "
                         "— run this BEFORE debugging a cell, so you do not chase a number nobody has "
                         "recomputed since the pictures changed")
    ap.add_argument("--only", default="", help="restrict --unverified to these pages")
    ap.add_argument("--selftest", action="store_true")
    a = ap.parse_args()
    if a.selftest:
        selftest()
        return 0
    board = a.board_dir or ("maccatalyst" if a.platform == "macos" else a.platform)
    if a.skew:
        rows = column_skew(board)
        for r in rows:
            print(f"  ~~ {r}")
        print(f"{len(rows)} cross-run finding(s) on {board}.")
        return 1 if rows else 0
    if a.unverified:
        want = {k.strip() for k in a.only.split(",") if k.strip()}
        rows = [r for r in unverified_cells(board) if not want or r[0] in want]
        for name, slot, why in rows:
            print(f"  ?? {name}/{slot}: {why}")
        print(f"{len(rows)} unverified cell(s) on {board}. Recompute before trusting them: "
              f"pixel_score.py --platform {board}" + (f" --only {a.only}" if a.only else "") +
              " (--verify to see the move without writing).")
        return 1 if rows else 0
    if a.scores_only:
        fatal, advisory = [], score_contradictions(board)
    else:
        fatal, advisory = check(a.platform, board, a.columns.split(","), lane=a.lane)
    for p in fatal:
        print(f"  !! {p}")
    for p in advisory:
        print(f"  ~~ {p}")
    print(f"{len(fatal)} stale artifact(s), {len(advisory)} stale score(s) on {a.platform}")
    return 1 if fatal or advisory else 0


if __name__ == "__main__":
    raise SystemExit(main())
