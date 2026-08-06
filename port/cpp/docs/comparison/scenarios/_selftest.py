#!/usr/bin/env python3
"""Check every scenario in this directory against the runner's own step validators AND the two lanes'
capture rects.  Run it directly:  python3 port/cpp/docs/comparison/scenarios/_selftest.py

WHY THIS EXISTS: a scenario coordinate has exactly one failure mode, and it is silent.  A click that
misses lands somewhere harmless, the agent reports ok, and the next frame comes back identical to the
previous one — which is indistinguishable from a page that does not react.  That is the same bug the
whole interaction vocabulary was added to fix, so the aim is a hit rather than plausible.

WHAT IT ACTUALLY CHECKS
  1. The file parses and every step passes run_comparison.py's OWN validators (imported, not
     reimplemented — step_point / drag_endpoints / drag_options).  That catches a missing `at`, a zero
     `dy`, an unknown direction, a zero-length drag, a bad steps/duration.
  2. Every resolved point — a click target, both ends of a drag/swipe — lands inside the INTERSECTION
     of the two lanes' content rects.  Measured from run sidecars, not assumed:
         macOS (maccatalyst)  window_bounds = [128, 30, 1024, 800]
         Windows              window_bounds = [244,  0, 1024, 800]
     Both lanes read the SAME scenario files (recapture.seed_scenarios copies this directory into each
     lane's scratch dir) and there is no per-lane calibration key, so a point outside the intersection
     is off-window on one of them.  The y floor additionally clears the macOS title bar: a DRAG that
     starts there moves the window and invalidates the capture rect for the rest of the unit.

WHAT IT CANNOT CHECK, by construction: whether a point hits the RIGHT CONTROL.  The band is ~900x740;
a control row is ~30px tall.  button.toml's [756, 171] is inside the band and lands solidly on
"Clicked" on Windows, but on macOS it resolves to image (628, 141) — the last pixel of the "Clicked"
button, with "Command" starting at 142.  entry.toml's [756, 111] is inside the band too and on macOS
misses its field.  Both PASS here.  A green run means "on-window and shaped correctly", never
"verified to hit the target"; only a real capture proves that.

NOT A TOML LINT: `[[steps]]` blocks appended by recapture.write_gif_scenarios (the GIF burst) carry no
action and are skipped like any other idle step.
"""
import importlib.util
import sys
import tomllib
from pathlib import Path

HERE = Path(__file__).resolve().parent
RUNNER = HERE.parent / "tools" / "run_comparison.py"

# Lane capture rects, from any run's frame sidecars (docs/comparison/<run>/<key>/<plat>/<col>/*.json).
LANES = {"maccatalyst": (128, 30, 1024, 800), "windows": (244, 0, 1024, 800)}
# The board dir a lane publishes into is NOT the key its per-lane coordinates are written under.
# run_comparison.for_lane keys on the ENVIRONMENT name because local.toml gives Catalyst and AppKit the
# same `platform = "maccatalyst"`, so platform cannot address them separately. Kept here so this gate
# resolves `at_<env>` exactly as the runner will — a per-lane override nobody can check offline is the
# same silent-miss hazard the rest of this file exists to close.
LANE_ENV = {"maccatalyst": "macos-arm64", "windows": "windows-x64"}
TITLE_BAR = 34          # macOS traffic-light strip, in window-local pixels — never a drag origin
# Scenarios authored before the sidecar rects were measured: an off-band point in one of these is a
# WARNING, not a failure — reporting them is in scope, editing them is not.
LEGACY = {"button", "entry", "scroll_view", "web_view", "hybrid_web_view"}


def _load_runner():
    """Import run_comparison.py by path — it is a script in a sibling dir, not an installed module."""
    spec = importlib.util.spec_from_file_location("run_comparison", RUNNER)
    mod = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(mod)
    return mod


def band():
    """(x_lo, x_hi, y_lo, y_hi) in ABSOLUTE SCREEN pixels: inside every lane's content rect."""
    x_lo = max(x for x, _, _, _ in LANES.values())
    x_hi = min(x + w for x, _, w, _ in LANES.values())
    y_lo = max(y + TITLE_BAR for _, y, _, _ in LANES.values())
    y_hi = min(y + h for _, y, _, h in LANES.values())
    return x_lo, x_hi, y_lo, y_hi


def points(rc, step, rect=None, env=None):
    """Every screen point a step resolves to, using the RUNNER's validators (which raise on garbage).

    `rect` is the lane's capture rect. Pass it: without one, step_point cannot scale a 0..1 fraction
    and cannot bounds-check an absolute pixel, so both classes of authoring error go unreported."""
    action = step.get("action")
    if action in (None, "type"):
        return []
    if env:
        step = rc.for_lane(step, env)   # resolve exactly as CoordinateDriver.run_action will
    if action in ("click", "hover", "scroll"):
        pt = rc.step_point(step, rect=rect)
        if action == "scroll":
            dy = step.get("dy")
            if not isinstance(dy, int) or dy == 0:
                raise ValueError(f"{step.get('name')!r}: dy must be a non-zero int, got {dy!r}")
        return [pt]
    if action in ("drag", "swipe"):
        rc.drag_options(step)                      # raises on a bad steps/duration
        return list(rc.drag_endpoints(step, rect=rect))
    raise ValueError(f"{step.get('name')!r}: unknown action {action!r}")


# --- does the point land on a CONTROL, or on empty page? ------------------------------------------
# The docstring above says this file cannot check that "by construction". It can, for any lane whose
# MAUI capture is on disk, and not checking it cost sixteen dead pages on the Windows board before
# anyone looked: seven of them aimed at bare background, and because a miss and an inert page produce
# the SAME identical-frames result, the board reported them as parity for weeks.
#
# The rule is deliberately weak in one direction: a FLAT patch that matches the page's own background
# colour is a miss, anything else passes. An empty Entry's interior is flat white too, which is why the
# background-colour agreement is required and not just the flatness — and why a control sitting on a
# same-coloured fill is a false PASS rather than a false alarm. Verified against the measured Windows
# sweep: check_box (512,108) and stepper (215,104) are caught; picker (512,104), which lands on a
# full-width ComboBox, is not. The SAME check clears check_box on maccatalyst, where that page is
# centred and the identical fraction lands on the control — which is how the two lanes were shown to
# need different coordinates rather than sharing one bug.
CAPTURES = HERE.parent / "captures"
PATCH = 30              # half-width: a 60x60 window, wide enough to include a small control's border
FLAT_STDDEV = 3.0       # below this the patch carries no edge at all
BG_TOLERANCE = 6        # per-channel distance at which the patch counts as "the page background"


def _capture(platform: str, key: str):
    """The lane's MAUI light capture, or None when this lane has never shot this page."""
    try:
        from PIL import Image                      # noqa: PLC0415  optional: the rest of the gate runs without it
    except ImportError:
        return None
    p = CAPTURES / platform / "maui" / f"{key}_light.png"
    return Image.open(p).convert("RGB") if p.exists() else None


def lands_on_content(im, x: int, y: int) -> bool:
    """False when (x, y) sits in a flat region painted the page's own background colour."""
    from PIL import ImageStat                      # noqa: PLC0415  guarded by _capture returning None

    w, h = im.size
    if not (0 <= x < w and 0 <= y < h):
        return True                                # off-image is the band check's job, not this one
    patch = im.crop((max(0, x - PATCH), max(0, y - PATCH), min(w, x + PATCH), min(h, y + PATCH)))
    stat = ImageStat.Stat(patch)
    if max(stat.stddev) >= FLAT_STDDEV:
        return True
    # Flat. Is it flat in the PAGE BACKGROUND, or flat inside a large same-coloured control? Compare
    # against a corner well clear of any content — the bottom-left of the frame, which every gallery
    # page leaves empty because its content stacks from the top.
    bg = im.getpixel((min(8, w - 1), h - 9))
    return any(abs(a - b) > BG_TOLERANCE for a, b in zip(stat.mean, bg))


def main() -> int:
    rc = _load_runner()
    x_lo, x_hi, y_lo, y_hi = band()
    errors, warnings, checked = [], [], 0
    blind = []

    for f in sorted(HERE.glob("*.toml")):
        try:
            scen = tomllib.loads(f.read_text())
        except Exception as e:                     # noqa: BLE001 — report, don't crash the sweep
            errors.append(f"{f.name}: not valid TOML: {e}")
            continue
        if scen.get("tag") != f.stem:
            errors.append(f"{f.name}: tag = {scen.get('tag')!r} must equal the filename stem "
                          f"{f.stem!r} (load_scenario looks the file up BY TAG)")
        steps = scen.get("steps", [])
        # The board still comes from import_run_captures.initial_frame: the frame whose step is named
        # 'initial', else the theme's FIRST frame. So the hard invariant is that step 1 carries no
        # action (otherwise the checked-in PNG silently becomes a post-interaction shot); the name is
        # a convention that makes the choice explicit instead of relying on the ordering fallback.
        if not steps or steps[0].get("action"):
            errors.append(f"{f.name}: the FIRST step must have no `action` — it is the frame that "
                          f"becomes the board still, and a driven one silently changes the checked-in "
                          f"PNG for every page that previously got a single idle screenshot")
        elif steps[0].get("name") != "initial":
            # Convention, not correctness: import_run_captures.at_rest_steps derives the at-rest
            # prefix from THIS file, so any action-free first-step name is recognised by name. It
            # stays a warning because "initial" is what every other scenario and the no-scenario
            # default call that frame, and a reader matching sidecars to steps by eye expects it.
            warnings.append(f"{f.name}: first step is named {steps[0].get('name')!r}, not 'initial' "
                            f"— recognised correctly, but it breaks the convention every other "
                            f"scenario and the no-scenario default follow")
        # Resolve against EACH lane's real rect rather than against a lane-agnostic intersection band.
        # The band was the right check while every scenario was absolute screen pixels: one number had
        # to be simultaneously valid on two differently-positioned windows, so the intersection WAS the
        # constraint. A fractional scenario has no fixed pixel to test — 0.85 is a different pixel per
        # lane, and in-bounds on all of them by construction — so testing it against the band reports a
        # failure for the one authoring form that cannot have this bug. step_point does the real check
        # (and raises) once it is given a rect, which is exactly what its docstring says --selftest is
        # for. What remains lane-specific is the title bar: a drag ORIGIN on it moves the window and
        # corrupts the capture rect for the rest of the unit, so that stays an explicit test.
        for step in steps:
            for lane, (lx, ly, lw, lh) in sorted(LANES.items()):
                rect = {"x": lx, "y": ly, "w": lw, "h": lh}
                try:
                    pts = points(rc, step, rect, LANE_ENV.get(lane))
                except Exception as e:             # noqa: BLE001
                    (warnings if f.stem in LEGACY else errors).append(f"{f.name} [{lane}]: {e}")
                    continue
                im = _capture(lane, f.stem)
                if im is None:
                    blind.append(f"{f.stem} [{lane}]")
                for i, (x, y) in enumerate(pts):
                    checked += 1
                    # Only the START of a gesture is tested: a drag END may legitimately finish over
                    # empty page.
                    #
                    # A CLICK on background is always dead — there is nothing under the cursor to
                    # receive it. A SCROLL or DRAG is not: it acts on whatever scrollable ancestor is
                    # beneath, and the inside of a ScrollView is mostly blank by nature. clip_gallery
                    # aims at flat background on both desktop lanes and scrolls perfectly well there
                    # (15.93% of frame in MAUI vs 16.25% in the port), so failing it would be the gate
                    # lying about a page that works. Hence: hard error for click/hover, warning for the
                    # rest — still worth saying, because carousel_page and swipe_refresh aim at blank
                    # too and those two really are dead on Windows.
                    if i == 0 and im is not None and not lands_on_content(im, x - lx, y - ly):
                        needs_target = step.get("action") in ("click", "hover")
                        msg = (f"{f.name} [{lane}]: step {step.get('name')!r} ({step.get('action')}) "
                               f"aims at [{x}, {y}] = image ({x - lx}, {y - ly}), which is FLAT PAGE "
                               f"BACKGROUND in this lane's MAUI capture — "
                               + ("a click there lands on nothing, the agent reports ok, and the frame "
                                  "comes back identical, which the board cannot tell apart from a page "
                                  "that does not react"
                                  if needs_target else
                                  "which is fine IF a scrollable ancestor is underneath, and dead if "
                                  "not; confirm against the page's motion score before trusting it"))
                        (errors if needs_target and f.stem not in LEGACY else warnings).append(msg)
                    if i == 0 and lane == "maccatalyst" and y < ly + TITLE_BAR:
                        msg = (f"{f.name} [{lane}]: step {step.get('name')!r} STARTS at [{x}, {y}], "
                               f"on the title bar (y < {ly + TITLE_BAR}) — a drag from there moves "
                               f"the WINDOW and invalidates the capture rect for every remaining "
                               f"frame of the unit, not just this step")
                        (warnings if f.stem in LEGACY else errors).append(msg)

    for w in warnings:
        print(f"WARN  {w}")
    for e in errors:
        print(f"FAIL  {e}")
    if blind:
        # Say what was NOT checked. A gate that silently skips half its inputs reads as a pass.
        print(f"note  no MAUI capture, content check skipped: {len(blind)} — {', '.join(blind[:6])}"
              f"{' ...' if len(blind) > 6 else ''}")
    print(f"{'FAIL' if errors else 'ok'}: {checked} point(s) across "
          f"{len(list(HERE.glob('*.toml')))} scenario(s), {len(warnings)} warning(s)")
    return 1 if errors else 0


def _demo() -> None:
    """Assert the band check actually rejects the things it claims to."""
    rc = _load_runner()
    x_lo, x_hi, y_lo, y_hi = band()
    assert (x_lo, x_hi, y_lo, y_hi) == (244, 1152, 64, 800), (x_lo, x_hi, y_lo, y_hi)
    # A fraction (the portable authoring form) used to ROUND TO THE ORIGIN here — a silent breakage
    # this check was written to pin. step_point now scales fractions against the presented window
    # rect instead, so with no rect to scale against it must REFUSE rather than guess: a fraction
    # that quietly became [0, 0] is the same do-nothing-and-report-success class as the rest of this
    # pass. Asserting the raise is what keeps the old behaviour from creeping back.
    try:
        bad = points(rc, {"name": "f", "action": "click", "at": [0.5, 0.4]})
        raise AssertionError(f"a fraction with no rect must raise, got {bad}")
    except ValueError:
        pass
    assert not (x_lo <= 0 <= x_hi)
    # button.toml's calibration is Windows-only: on macOS x=756 is 116px right of where it was aimed.
    assert points(rc, {"name": "b", "action": "click", "at": [756, 171]}) == [[756, 171]]
    for bad in ({"name": "z", "action": "drag", "at": [700, 120], "to": [700, 120]},
                {"name": "d", "action": "swipe", "at": [700, 120], "direction": "sideways"},
                {"name": "s", "action": "scroll", "at": [700, 120], "dy": 0},
                {"name": "u", "action": "wiggle"}):
        try:
            points(rc, bad)
        except ValueError:
            continue
        raise AssertionError(f"expected {bad['name']!r} to be rejected")
    print("ok: _demo")


if __name__ == "__main__":
    if "--demo" in sys.argv:
        _demo()
    sys.exit(main())
