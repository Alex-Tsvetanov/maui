#!/usr/bin/env python3
"""Bridge: import an E2E-runner dated run's captures into the canonical captures/<platform>/<fw>/<key>_<theme>.png
layout that build_comparison_json.py + gen_readme.py consume. The runner writes <tag>/<platform>/<column>/NNNN.png
(column ∈ maui_xaml|cpp|cpp_xaml, theme+step in the NNNN.json sidecar); the canonical tree uses fw ∈ maui|cpp|xaml
and <key>_<theme>.png. This copies each column's AT-REST frame per theme into place — the board still is the page
before any input was driven, never a post-click/scroll/swipe one. After running, regenerate:
  python3 tools/build_comparison_json.py && python3 tools/gen_readme.py
Usage: import_run_captures.py <run_dir> [platform=maccatalyst]  |  import_run_captures.py --selftest"""
import sys, os, glob, json, shutil, tomllib

HERE = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))  # docs/comparison
SCENARIOS = os.path.join(HERE, "scenarios")
# The AppKit columns keep their names as framework dirs (captures/maccatalyst/appkit_{cpp,xaml}) — that is
# already the canonical layout build_comparison_json.py declares for maccatalyst, so no rename is wanted.
COL_TO_FW = {"maui_xaml": "maui", "cpp": "cpp", "cpp_xaml": "xaml",
             "appkit_cpp": "appkit_cpp", "appkit_xaml": "appkit_xaml"}

def at_rest_steps(tag, scen_dir=SCENARIOS):
    """The names of `tag`'s steps whose frame is captured before ANY input has been driven.

    Being at rest is a property of the leading PREFIX of the step list, not of a step on its own:
    run_comparison.py drives every step IN ORDER against ONE launched app, so once step k clicks,
    every later frame shows a reacted page even if those later steps carry no `action` themselves.
    Hence the break rather than a filter. The sidecar records only the step NAME (see the schema in
    run_comparison.py), so the scenario file is what says which names were driven.

    Unnamed steps are deliberately not collected: the runner names them f"step{n}" where n is the
    FRAME number (theme_index * len(steps) + step_index + 1), so the same step is called something
    different in light and dark and cannot be reconstructed from the sidecar. They end up refused,
    which is the safe direction. Every authored scenario names its steps — scenarios/_selftest.py
    warns when one does not.
    """
    f = os.path.join(scen_dir, f"{tag}.toml")
    if not os.path.isfile(f):
        return {"initial"}  # load_scenario's default for a page with no scenario: one idle screenshot
    try:
        with open(f, "rb") as fh:
            steps = tomllib.load(fh).get("steps") or [{"name": "initial"}]
    except Exception as e:
        # Loud, not skipped: guessing here means guessing which frame is the board still. The runner
        # parses the same file, so a run that produced frames cannot legitimately hit this.
        raise SystemExit(f"{f}: unreadable scenario — cannot tell which frame is at rest: {e}")
    at_rest, first_action = set(), len(steps)
    for i, s in enumerate(steps):
        if s.get("action"):
            first_action = i
            break
        if "name" in s:
            at_rest.add(s["name"])
    # "initial" is always at rest unless this tag reuses it after an action. It is load_scenario's
    # default step name, so a run that did NOT use this scenario still produces a frame called
    # "initial" — which happens whenever recapture.py's per-lane geometry gate skips an absolute
    # scenario (macos-appkit gets none at all). Without this, at_rest for scroll_view is {"top"}, the
    # skipped lane's one honest idle frame is named "initial", nothing matches, and the page is
    # refused as missing forever. The post-action subtraction below still governs, so a scenario that
    # names a post-action step "initial" is not let through by this.
    at_rest.add("initial")
    # A name REUSED after the first action is a post-action frame wearing an at-rest name; since the
    # sidecar carries the name alone, such a name can never be trusted again for this tag.
    return at_rest - {s.get("name") for s in steps[first_action:]}

def initial_frame(tag, col, theme, scen_dir=SCENARIOS):
    """(png, None) for the frame safe to publish as this page's canonical AT-REST still, else
    (None, why) — where `why` is None only when the run holds no frame at all for this cell.

    NEVER returns a post-action frame. The old rule was "the sidecar named 'initial', else the first
    surviving frame of this theme", which was harmless while every page was a single idle screenshot.
    Now that pages drive input it is board corruption: run_comparison.py drops frames on three
    separate paths (failed present, failed pull, wrong window size — one sweep reported ~400), and
    when the frame that drops is the `initial` one, that fallback silently copies the post-click /
    post-scroll / post-swipe frame over captures/<platform>/<fw>/<key>_<theme>.png. It reads on the
    board as a port bug that does not exist, and it destroys the page's last honest capture to do it.

    So the only frame published is one the SCENARIO proves was taken before any input: the earliest
    surviving frame whose step is in the at-rest prefix. Anything else is reported and left
    unpublished, which keeps the previous capture — stale is recoverable, a lie is not.

    The rule is "an at-rest step" rather than the narrower "the step named 'initial', or the theme's
    only surviving frame" because scenarios/scroll_view.toml names its first step "top" and has two
    steps: on a HEALTHY run both frames survive, so an only-one-frame rule would refuse that page
    forever. `initial` is a convention (scenarios/_selftest.py warns, not errors, when a first step is
    named otherwise) — the scenario, not the name, is what says whether anything was driven.
    """
    at_rest = at_rest_steps(tag, scen_dir)
    shots = []  # (png, step name) for this theme, in frame order — NNNN.json sorts as NNNN
    for j in sorted(glob.glob(f"{run}/{tag}/{platform}/{col}/*.json")):
        try:
            m = json.load(open(j))
        except Exception:
            continue
        png = j[:-5] + ".png"
        if m.get("theme") == theme and os.path.exists(png):
            shots.append((png, m.get("step")))
    for png, step in shots:
        # Frame order == step order within a theme (n = theme_index * len(steps) + step_index + 1),
        # so the first hit is the earliest at-rest frame: `initial` itself whenever it survived.
        if step in at_rest:
            return png, None
    if not shots:
        return None, None  # nothing captured for this cell at all — the ordinary "missing" case
    # Two causes, and this tool cannot tell them apart — so it claims neither: (a) the at-rest frame
    # DROPPED and what survived is post-action, (b) the run predates the current <tag>.toml, so its
    # step names mean nothing here (an old run dir re-imported after a scenario was added does exactly
    # this). Both resolve the same way — re-capture the page — and both must keep the previous still.
    return None, (f"survivors {[s for _, s in shots]} are none of {tag}'s at-rest steps "
                  f"{sorted(at_rest)}: either that frame dropped and these are post-action, or this "
                  f"run predates the scenario. Re-capture the page")

def _selftest():
    """python3 tools/import_run_captures.py --selftest — pure: no run dir, no VM, no board writes.

    Covers the three cases the old `best = best or png` fallback got wrong, plus the ~155-page
    no-scenario path it must not disturb. Everything goes through initial_frame itself, so a future
    rewrite of the rule is checked rather than just the helper it happens to call today.
    """
    global run, platform  # initial_frame reads the run root the same way the script body sets it
    import tempfile  # noqa: PLC0415  selftest-only
    ok = True

    def shot(d, n, theme, step):
        """One captured frame: the PNG plus the sidecar fields run_comparison.py writes at ~line 848."""
        os.makedirs(d, exist_ok=True)
        open(os.path.join(d, f"{n:04d}.png"), "wb").close()
        with open(os.path.join(d, f"{n:04d}.json"), "w") as fh:
            json.dump({"theme": theme, "step": step, "frame": n}, fh)

    def check(what, got, want):
        nonlocal ok
        if got != want:
            print(f"  FAIL {what}: got {got!r}, want {want!r}")
            ok = False

    with tempfile.TemporaryDirectory() as tmp:
        platform = "maccatalyst"
        run = os.path.join(tmp, "run")
        scen = os.path.join(tmp, "scenarios")
        os.makedirs(scen)
        # A driven page, same shape as the authored scenarios/button.toml: idle still, then a tap.
        with open(os.path.join(scen, "driven.toml"), "w") as fh:
            fh.write('tag = "driven"\n\n[[steps]]\nname = "initial"\n\n[[steps]]\n'
                     'name = "after-tap"\naction = "click"\nat = [756, 171]\n')
        # …and one whose at-rest step is NOT called "initial" (scroll_view.toml really does this).
        with open(os.path.join(scen, "renamed.toml"), "w") as fh:
            fh.write('tag = "renamed"\n\n[[steps]]\nname = "top"\n\n[[steps]]\n'
                     'name = "scrolled"\naction = "scroll"\nat = [756, 400]\ndy = -300\n')

        def cell(tag, frames):
            d = os.path.join(run, tag, platform, "cpp")
            for n, (th, st) in frames:
                shot(d, n, th, st)
            return d

        # (1) initial present, action frame alongside it -> the initial frame, never the reacted one.
        d = cell("driven", [(1, ("light", "initial")), (2, ("light", "after-tap"))])
        src, why = initial_frame("driven", "cpp", "light", scen)
        check("initial present", (os.path.basename(src or ""), why), ("0001.png", None))

        # (2) THE BUG: initial DROPPED (the runner `continue`s before writing a sidecar), the tapped
        #     frame survived. Must refuse — publishing it would bank a reacted page as the still.
        os.remove(os.path.join(d, "0001.png"))
        os.remove(os.path.join(d, "0001.json"))
        src, why = initial_frame("driven", "cpp", "light", scen)
        check("initial dropped, action frame survives", src, None)
        check("…and names the survivor it refused", bool(why) and "after-tap" in why, True)

        # (3) a single surviving frame that drove nothing is publishable even when it is not called
        #     "initial" — and (4) both frames surviving still publishes the at-rest one, not the
        #     scrolled one, which is what a healthy scroll_view run looks like.
        cell("renamed", [(1, ("light", "top"))])
        src, why = initial_frame("renamed", "cpp", "light", scen)
        check("single action-free frame", (os.path.basename(src or ""), why), ("0001.png", None))
        cell("renamed", [(2, ("light", "scrolled"))])
        check("healthy 2-step page", os.path.basename(initial_frame("renamed", "cpp", "light", scen)[0]),
              "0001.png")

        # (5) no frames at all for the cell -> the ordinary "missing" report, NOT a refusal (a lane
        #     that never reached this page must not read as board corruption).
        check("nothing captured", initial_frame("driven", "cpp", "dark", scen), (None, None))

        # (6) a page with no scenario file — ~155 of the board's 172 — keeps its one idle screenshot.
        cell("plain", [(1, ("light", "initial"))])
        check("no scenario", os.path.basename(initial_frame("plain", "cpp", "light", scen)[0]), "0001.png")
        check("no scenario at-rest set", at_rest_steps("plain", scen), {"initial"})

        # (7) a name reused AFTER an action is untrustworthy: the sidecar carries the name alone.
        with open(os.path.join(scen, "reused.toml"), "w") as fh:
            fh.write('tag = "reused"\n\n[[steps]]\nname = "initial"\n\n[[steps]]\nname = "tap"\n'
                     'action = "click"\nat = [1, 1]\n\n[[steps]]\nname = "initial"\n')
        check("name reused after an action", at_rest_steps("reused", scen), set())

    # (8) the REAL authored scenarios: every one must still have a publishable at-rest step, so a new
    #     scenario that drives on its first step trips here rather than on the next board sweep.
    for f in sorted(glob.glob(os.path.join(SCENARIOS, "*.toml"))):
        tag = os.path.basename(f)[:-5]
        check(f"scenarios/{tag}.toml has an at-rest step", bool(at_rest_steps(tag)), True)

    print("import_run_captures selftest:", "OK" if ok else "FAILED")
    return 0 if ok else 1


if sys.argv[1:2] == ["--selftest"]:
    sys.exit(_selftest())
if len(sys.argv) < 2:
    raise SystemExit(__doc__)
run = sys.argv[1].rstrip("/")
platform = sys.argv[2] if len(sys.argv) > 2 else "maccatalyst"

copied, missing, refused = 0, [], []
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
            src, why = initial_frame(tag, col, theme)
            dst = os.path.join(HERE, "captures", platform, fw, f"{tag}_{theme}.png")
            if src:
                os.makedirs(os.path.dirname(dst), exist_ok=True)
                shutil.copyfile(src, dst)
                copied += 1
            elif why:
                # NEVER squelched, not even for the builder columns the `missing` list skips below: a
                # refusal means this cell keeps its PREVIOUS still, so the run is not the refresh it
                # otherwise looks like, and only the log can say so.
                refused.append(f"{tag}/{fw}/{theme}: {why}")
            elif col not in ("cpp", "appkit_cpp"):  # builder columns are absent for non-twin pages
                missing.append(f"{tag}/{fw}/{theme}")
print(f"imported {copied} captures for {len(tags)} pages into "
      f"captures/{platform}/{{{','.join(sorted(COL_TO_FW[c] for c in present_cols))}}}/")
if missing:
    print(f"  {len(missing)} missing (non-builder): {missing[:12]}{' …' if len(missing) > 12 else ''}")
if refused:
    # Non-zero exit + a '!' on every line: tools/parity/recapture.py echoes '!' lines to the terminal
    # and records a non-zero step in its FAILED summary, so this cannot pass as a clean import.
    print(f"  ! {len(refused)} cell(s) NOT published — no AT-REST frame survived, and a post-action "
          f"frame must never become a page's still; the previous capture is retained:")
    for r in refused:
        print(f"      ! {r}")
    sys.exit(1)
