#!/usr/bin/env python3
"""Refuse a capture that would score artifacts older than the source they claim to render.

WHY
---
The Windows lane declares `artifact_remote` for all three columns, so recapture.py's build() skips it
outright ("the Windows guest builds its own artifacts") and the guest builds are run BY HAND via
build_gallery_windows.ps1 / build_maui_reference.ps1. The lane log goes straight from
`=== LANE windows/windows` to `--- windows/windows: capture`: there is no build stage between a
hand-run build and a three-hour capture, and therefore nothing that asserts the binaries are newer
than the source. A whole run can silently score last week's build while every artifact-level check
passes, because every artifact-level check only asks whether the file EXISTS.

This has cost real time more than once:
  * `containers` (2026-08-22) needed no code change at all -- gallery.exe was from 08-18.
  * `selection_synchronization` (2026-08-22) likewise -- MauiReference.exe from 08-19, scored against
    a twin edited 08-20.
  * Catalyst's MauiReference was once a MONTH old across 61 changed twins.
It is NOT Windows-specific: recapture.py rebuilds neither MauiReference nor the galleries on ANY lane.

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
    same doctrine windows_source_roots() follows -- a hand-copied second list of these paths would
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


def newest(roots: list[Path]) -> tuple[float, Path | None]:
    """(mtime, path) of the newest file under any of `roots`, pruning PRUNE dirs. Missing roots are
    skipped rather than fatal -- a column simply may not have that dependency set."""
    best, where = 0.0, None
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
                try:
                    m = p.stat().st_mtime
                except OSError:
                    continue
                if m > best:
                    best, where = m, p
    return best, where


def windows_source_roots(column: str, compiled_src_dirs: list[str]) -> list[Path]:
    """The host paths a Windows column's artifact is built FROM.

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
    roots.append(CPP / "examples" / ("gallery" if column == "cpp" else "gallery_xaml"))
    if column == "cpp_xaml":
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


def verdicts(facts: dict, columns: list[str]) -> list[str]:
    """The stale/fresh DECISION, separated from the SSH that feeds it so the selftest can pin it.

    Empty list == every artifact is newer than its source."""
    want = [c for c in columns if c in WINDOWS_ARTIFACTS]
    import datetime as _dt

    def epoch(iso: str) -> float:
        return _dt.datetime.fromisoformat(iso.replace("Z", "+00:00")).timestamp()

    problems = []
    for col in want:
        path = WINDOWS_ARTIFACTS[col]
        info = facts["artifacts"].get(path, {"exists": False})
        if not info.get("exists"):
            problems.append(f"{col}: DECLARED BUT MISSING on the guest -- {path}. Build it "
                            f"({'build_maui_reference.ps1' if col == 'maui_xaml' else 'build_gallery_windows.ps1'}).")
            continue
        art = epoch(info["mtime"])
        src, where = newest(windows_source_roots(col, facts.get("source_dirs", [])))
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
    return verdicts(windows_facts([WINDOWS_ARTIFACTS[c] for c in want]), want)


# --------------------------------------------------------------------------- score contradictions
def _md5(p: Path) -> str | None:
    try:
        return hashlib.md5(p.read_bytes()).hexdigest()
    except OSError:
        return None


def score_contradictions(plat_dir: str) -> list[str]:
    """Cells whose recorded review cannot describe the captures now on disk. See the module header."""
    jf = COMP / "comparison.json"
    if not jf.is_file():
        return []
    caps = COMP / "captures" / plat_dir
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


def check(platform: str, plat_dir: str, frameworks: list[str], scores: bool = True) -> tuple[list[str], list[str]]:
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
    fatal = stale_windows(frameworks) if platform == "windows" else []
    advisory = score_contradictions(plat_dir) if scores else []
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
    cpp_roots = windows_source_roots("cpp", compiled)
    xaml_roots = windows_source_roots("cpp_xaml", compiled)
    ref_roots = windows_source_roots("maui_xaml", compiled)
    assert CPP / "src/platform/windows" in cpp_roots
    assert CPP / "src/platform/headless" in cpp_roots         # headless DOES compile in -- not an exclude
    assert CPP / "src/platform/ios" not in cpp_roots          # …and iOS does not
    assert CPP / "include/maui/platform/ios" not in cpp_roots  # same rule for headers
    assert CPP / "examples/gallery" in cpp_roots and CPP / "examples/gallery_xaml" not in cpp_roots
    twins = PORT / "maui-reference" / "pages"
    assert twins in xaml_roots, "a twin edit must invalidate cpp_xaml (CMakeLists.txt:80 globs them)"
    assert twins not in cpp_roots, "…but not the non-XAML gallery, which does not read them"
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

    now = time.time()
    for col in ("cpp", "cpp_xaml", "maui_xaml"):
        src, _ = newest(windows_source_roots(col, compiled))
        assert verdicts(facts_at(col, src + 3600), [col]) == [], col          # built an hour after: fresh
        # `containers` was gallery.exe from 08-18; `selection_synchronization` was MauiReference.exe
        # from 08-19 against a twin edited 08-20. Both are "artifact a day or more behind its source".
        late = verdicts(facts_at(col, src - 86400), [col])
        assert len(late) == 1 and "STALE ARTIFACT" in late[0], (col, late)
        assert f"{24.0:.1f}h newer" in late[0], late[0]
    # The ground-truth column must additionally name the board signature, because a stale reference
    # is the one that reads as a PORT regression rather than as a missing build.
    assert "green->yellow" in verdicts(facts_at("maui_xaml", 0.0), ["maui_xaml"])[0]
    # Skew and build duration must not fire; a day must.
    src, _ = newest(windows_source_roots("cpp", compiled))
    assert verdicts(facts_at("cpp", src - TOLERANCE_S + 30), ["cpp"]) == [], "within tolerance"
    assert verdicts(facts_at("cpp", src - TOLERANCE_S - 30), ["cpp"]) != [], "past tolerance"

    # A declared-but-missing artifact is a LOUD problem, never a silent pass -- matching how this
    # lane already treats a missing artifact everywhere else.
    missing = {"source_dirs": compiled,
               "artifacts": {WINDOWS_ARTIFACTS["cpp"]: {"exists": False, "mtime": "", "length": 0}}}
    assert "DECLARED BUT MISSING" in verdicts(missing, ["cpp"])[0]

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
    assert advisory and all("BYTE-IDENTICAL" in a for a in advisory), advisory
    assert not any("fingerprint" in x for x in fatal + advisory), "unverified cells are triage-only"

    # The contradiction rule, exercised on the real board.
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
    ap.add_argument("--scores-only", action="store_true", help="skip the guest probe (no SSH)")
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
        fatal, advisory = check(a.platform, board, a.columns.split(","))
    for p in fatal:
        print(f"  !! {p}")
    for p in advisory:
        print(f"  ~~ {p}")
    print(f"{len(fatal)} stale artifact(s), {len(advisory)} stale score(s) on {a.platform}")
    return 1 if fatal or advisory else 0


if __name__ == "__main__":
    raise SystemExit(main())
