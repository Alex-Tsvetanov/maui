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


def points(rc, step, rect=None):
    """Every screen point a step resolves to, using the RUNNER's validators (which raise on garbage).

    `rect` is the lane's capture rect. Pass it: without one, step_point cannot scale a 0..1 fraction
    and cannot bounds-check an absolute pixel, so both classes of authoring error go unreported."""
    action = step.get("action")
    if action in (None, "type"):
        return []
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


def main() -> int:
    rc = _load_runner()
    x_lo, x_hi, y_lo, y_hi = band()
    errors, warnings, checked = [], [], 0

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
                    pts = points(rc, step, rect)
                except Exception as e:             # noqa: BLE001
                    (warnings if f.stem in LEGACY else errors).append(f"{f.name} [{lane}]: {e}")
                    continue
                for i, (x, y) in enumerate(pts):
                    checked += 1
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
