#!/usr/bin/env python3
"""iOS simulator capture engine (was capture_ios_clean.py — the WS-E clean capture).

recapture.py owns the per-example loop; this module owns the iOS-specific traps, which are the whole
reason it is not three lines of `simctl`:

  * SpringBoard "◀ back to <app>" overlay — leaks into `simctl io screenshot` ONLY on the first launch
    after switching FROM a different app. A same-app relaunch does not re-trigger it, so every theme
    pass starts with a discarded warm-up launch (`warmup()`).
  * THEME IS A DEVICE SETTING, not an app env var: MAUI_APPEARANCE / MAUI_THEME both map to
    UserAppTheme, which OVERRIDES the OS — setting one would prove the override works rather than that
    the app follows the system. So `set_theme()` drives the simulator and the env var is NOT sent.
    That also makes theme the OUTER loop for the caller: flipping it per page would be ~364 device
    appearance changes on a full board.
  * STATUS BAR pinned for the whole run (`pin()` / `unpin()`): iOS captures are full-screen, so an
    unpinned clock/battery/signal scores as a diff on every single page.
  * TCC: a simctl subprocess cannot write into ~/Documents, so every shot stages in /tmp and Python
    copies the bytes into the repo.
  * .NET SPLASH frames: a screenshot can succeed and still be a picture of the WRONG SCREEN. Escalating
    relaunch, and the frame is DROPPED rather than banked (a splash scores as an enormous port defect
    on a page the port may render perfectly).
  * INTERACTION: `simctl` has NO input-injection subcommand at all (read its full subcommand list), so
    a page that only changes under a tap/swipe/keystroke is otherwise photographed at rest. Touches go
    in through `idb` — see "Driving the device" below.

DRIVING THE DEVICE (scenario steps)
-----------------------------------
`run_steps()` replays a page's step dicts against the device — AFTER the at-rest still and again for
the reacted frame (`capture_still(still_first=True)`, which is what a driven page gets), or before the
still on the animated path, and DURING the GIF recording. Touches are injected with **idb** (`idb ui tap/swipe/text`), which speaks to the simulator's
own HID layer over the companion socket. THE HOST POINTER IS NEVER TOUCHED, and that is a hard
requirement, not a nicety: this lane previously drove the Simulator WINDOW with `cliclick` plus an
`osascript … activate`, so a board run seized the cursor and the foreground app for its whole duration
— the machine could not be used while it ran, and any stray user click landed mid-gesture and corrupted
a frame. Every sibling lane already met that bar (Android injects through `adb shell input`, and the
macOS/Windows guests run their agent over SSH, so the cursor that moves belongs to the VM). Measured
here: three `idb ui tap`s took the cpp gallery's `button` readout 0 -> 3 with the host cursor at
(512,393) before AND after, and no app changed focus.

The alternatives were measured on this machine, not assumed:

  * `xcrun simctl` — no input subcommand of any kind (read its full subcommand list). A dead end.
  * `cliclick` on the Simulator window — works, but costs the operator the machine (above). Gone.
  * the port's own DevFlow HTTP agent (docs/DEVFLOW_PROTOCOL.md) — reachable (a simulator process
    shares the host's 127.0.0.1), but it exists ONLY in the C++ galleries; the MAUI reference column
    has no agent. Driving 2 of 3 columns makes every scenario page a guaranteed FALSE RED, so it
    cannot be the parity driver. (Its `/tap` is also `i_button`-gated: a tap recognizer on a Label or
    Grid answers `found:true, activated:false`.)
  * an XCUITest runner — a signed test host per app; the heaviest option for the smallest gain.

idb is APP-AGNOSTIC, which is the entire requirement: all three columns must receive the identical
gesture or the comparison means nothing.

GEOMETRY — there is none to resolve. idb addresses the DEVICE, so there is no window rect, no bezel, no
window scale and no Accessibility tree in the path. MEASURED: the whole proof above ran with the
Simulator NOT frontmost and never activated. Inferred but NOT measured (the companion talks to
CoreSimulator, not to Simulator.app, so window state should be irrelevant): that it also works with the
window closed or on another Space — untested because quitting Simulator.app shuts the device down.
What DOES matter is the unit:

  **idb ui speaks POINTS; `simctl io screenshot` writes PIXELS.** On this device that is a factor of
  3.0 (402x874 points, 1206x2622 pixels), and it is the single most likely source of a tap that lands
  on the wrong widget. So this module works in POINTS end to end — one unit space, no conversion to get
  wrong — and reads the size from `idb describe` per device rather than pinning 402x874, because the
  next device is another size. A coordinate read off a board PNG is a PIXEL and must be divided by the
  density before it can be used here; prefer a 0..1 fraction, which needs no such arithmetic and is the
  only form that means the same thing on every lane.

A failure RAISES — the lane already turns that into one failed page, whereas a silent no-op reproduces
the very bug this exists to fix: 12 byte-identical burst frames from a page nobody ever touched. idb
itself will NOT do that for us: a tap at (99999,99999) and a zero-length swipe both exit 0, so the
bounds and zero-length guards below are load-bearing, not decorative.

RUN-DIRECTORY EVIDENCE (the full-resolution frames motion_score.py scores)
--------------------------------------------------------------------------
The board's PNG + 400px GIF are the human artifacts and they still land exactly where they always did.
On top of them, `capture_still`/`capture_gif` optionally bank FULL-RESOLUTION frames + sidecars into a
run unit — the same evidence the VM lanes leave, in the same shape (motion_score._shots,
recapture.burst_frames, run_comparison.py's sidecar):

    <comp>/<RUN>/<key>/ios/<column>/NNNN.png  +  NNNN.json
        {tag, platform, column, theme, step, frame, commit, captured_at}

Without it an animated iOS page could only ever be judged on ONE resting frame; `motion_score` says so
out loud ("NOT motion-scored … iOS and Android keep no run dir at all") rather than reporting a still
as motion. Nothing here relocates an output: the run dir is ADDITIONAL, and a caller that passes no
`run_unit` gets byte-identically the old behavior.

Two contracts this lane has to meet, both easy to break silently:
  * the banked `initial` frame is a BYTE COPY of the published still (motion_score._is_published_run
    accepts a run only if it holds a byte-identical twin of what captures/ shows) — see `_bank`;
  * every frame carries a MEANINGFUL step name, because motion_score._pair joins the columns by name
    and drops nameless frames on purpose — see `frame_step` for why that name is a TIMESTAMP.

Apps (bundle + env contract):
  maui -> dev.mauicpp.mauireference     MAUI_COMPARE_PAGE -> port/maui-reference/captures/ios/<key>_<theme>.png
  cpp  -> dev.maui-cpp.ios-gallery      MAUI_SAMPLE_PAGE  -> docs/comparison/captures/ios/cpp/<key>_<theme>.png
  xaml -> dev.maui-cpp.ios-gallery-xaml MAUI_SAMPLE_PAGE  -> docs/comparison/captures/ios/xaml/<key>_<theme>.png

Does NOT build or install.
"""
import json
import math
import os
import shutil
import signal
import subprocess
import tempfile
import time
from datetime import datetime

import gif as gifmod
from capture_guard import is_splash
from device_state import clear_ios, pin_ios, set_ios_theme

UDID = os.environ.get("MAUI_SIM_UDID", "C4926671-2FA7-428E-B4A4-480692EE742B")
HERE = os.path.dirname(os.path.abspath(__file__))
CPP = os.path.abspath(os.path.join(HERE, "..", "..", ".."))     # port/cpp
PORT = os.path.abspath(os.path.join(CPP, ".."))                 # port
REF_CAP = os.path.join(PORT, "maui-reference", "captures", "ios")
COMP_CAP = os.path.join(CPP, "docs", "comparison", "captures", "ios")

APPS = {
    "maui": {"bundle": "dev.mauicpp.mauireference", "page": "MAUI_COMPARE_PAGE",
             "out": lambda k, t, e: os.path.join(REF_CAP, f"{k}_{t}.{e}")},
    "cpp": {"bundle": "dev.maui-cpp.ios-gallery", "page": "MAUI_SAMPLE_PAGE",
            "out": lambda k, t, e: os.path.join(COMP_CAP, "cpp", f"{k}_{t}.{e}")},
    "xaml": {"bundle": "dev.maui-cpp.ios-gallery-xaml", "page": "MAUI_SAMPLE_PAGE",
             "out": lambda k, t, e: os.path.join(COMP_CAP, "xaml", f"{k}_{t}.{e}")},
}


def out_path(app: str, key: str, theme: str, ext: str = "png") -> str:
    return APPS[app]["out"](key, theme, ext)


def pin(udid: str = UDID) -> None:
    pin_ios(udid)


def unpin(udid: str = UDID) -> None:
    clear_ios(udid)


def set_theme(theme: str, udid: str = UDID) -> str:
    """Set the SIMULATOR's appearance; returns the previous value (restore it when the run ends)."""
    return set_ios_theme(theme, udid)


def launch(app: str, key: str, udid: str = UDID) -> None:
    env = dict(os.environ)
    env[f"SIMCTL_CHILD_{APPS[app]['page']}"] = key
    subprocess.run(["xcrun", "simctl", "launch", "--terminate-running-process", udid, APPS[app]["bundle"]],
                   env=env, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, check=True)


def warmup(app: str, key: str, settle: float, udid: str = UDID) -> None:
    """Foreground the bundle once per theme pass and DISCARD the frame — kills the SpringBoard overlay."""
    launch(app, key, udid)
    time.sleep(settle + 2.0)


def _screenshot(stage: str, udid: str) -> bool:
    # Retry: a screenshot fired mid display-transition fails transiently.
    for _ in range(4):
        r = subprocess.run(["xcrun", "simctl", "io", udid, "screenshot", "--type=png", stage],
                           stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        if r.returncode == 0 and os.path.exists(stage):
            return True
        time.sleep(1.0)
    return False


# --------------------------------------------------------------------------- interaction
# fb-idb's client. NOT resolved off PATH first: pipx puts it in ~/.local/bin, which a non-login shell
# does not have, and it must run under python 3.11 — 3.14 crashes it (asyncio.get_event_loop) and 3.9
# cannot install it (dataclass slots). The pipx venv shebang pins the right interpreter, so addressing
# the binary by path is also what keeps the interpreter right.
IDB = os.environ.get("MAUI_PARITY_IDB", os.path.expanduser("~/.local/bin/idb"))
IDB_INSTALL = ("brew install idb-companion && pipx install --python python3.11 fb-idb "
               "(or point MAUI_PARITY_IDB at an existing client)")

# A gesture STARTING within this many POINTS of an edge is an OS gesture, not app input: left = back,
# top = Notification Center, bottom = home/app switcher. 20pt is the 60px this was measured at on a 3x
# screen. A step that really means to pull the shade sets `edge = true`; everything else is rejected,
# because the frame it would produce (a half-open Notification Center over the page) is plausible enough
# to survive review. What this does NOT cover, and no constant can: the STATUS BAR / Dynamic Island band
# is far taller than the margin (roughly the top ~55-60pt), so a point below it can still be on system
# chrome rather than on the page. A pull origin has to clear that band — one more reason to author
# fractions against the board still rather than guessing a y.
EDGE_PT = 20
SWIPE_MOVES = 8            # touch points along the line; enough for UIScrollView to see a velocity
STEP_SETTLE = 0.6          # let the UI react before the next step / the shot

# The axis form of a drag/swipe, byte-identical to run_comparison.SWIPE_DIRECTIONS and
# capture_android._SWIPE_DIRECTIONS. One vocabulary, four lanes: `direction = "left"` has to mean the
# same gesture here as it does on the guest agents, or a scenario file stops being portable — which is
# exactly what happened while this module implemented a SMALLER verb set than the files it is fed.
_SWIPE_DIRECTIONS = {"up": (0, -1), "down": (0, 1), "left": (-1, 0), "right": (1, 0)}
# The shared vocabulary's default distance is 300px at the desktop lanes' 1024x800 capture geometry.
# On an 874pt-tall screen that is a third of it, so — exactly like the Android lane — the default is a
# FRACTION of the axis, which is the same board-scale flick on any device.
_SWIPE_FRACTION = 0.25
# A touch screen has no pointer, so there is nothing to park and nothing to leave hovering. Reported,
# never substituted: a tap is a DIFFERENT gesture and would manufacture a pressed/activated state a
# hover never causes — a fake reaction is worse than a missing one.
_HOVER_SKIP = ("hover: iOS is a touch screen with no pointer to park — and a tap is a different "
               "gesture, not a substitute")


_POINTS: dict[str, tuple[int, int]] = {}
_CONNECTED: set[str] = set()


def idb_bin() -> str:
    """The idb client, or an actionable RuntimeError. Never falls back to a no-op."""
    tool = IDB if os.access(IDB, os.X_OK) else shutil.which(os.path.basename(IDB))
    if tool is None:
        raise RuntimeError(f"idb not found at {IDB!r} and not on PATH — install it: {IDB_INSTALL}")
    return tool


def _idb(*args: str, timeout: float = 60.0) -> subprocess.CompletedProcess:
    return subprocess.run([idb_bin(), *args], capture_output=True, text=True, timeout=timeout)


def connect(udid: str = UDID) -> None:
    """Attach a companion to `udid`. Once per run — spawning one per step would cost ~a second each.

    Reconnecting an already-connected target is measured to exit 0, so the cache is an optimisation,
    not a correctness crutch."""
    if udid in _CONNECTED:
        return
    r = _idb("connect", udid)
    if r.returncode != 0:
        raise RuntimeError(f"`idb connect {udid}` failed (rc={r.returncode}): "
                           f"{(r.stderr or r.stdout).strip()[:300]}")
    _CONNECTED.add(udid)


def device_points(udid: str = UDID) -> tuple[int, int]:
    """The screen size in POINTS — the unit `idb ui` takes, and the unit this module works in.

    Cached: it cannot change during a run, and the uncached form would shell out at every driven page,
    including from inside capture_gif's live `recordVideo`. NO DEFAULT on failure: a hardcoded 402x874
    would drive the next device at the wrong scale while reporting success, which is exactly the
    fabricate-plausible-data failure this lane keeps re-learning."""
    if udid not in _POINTS:
        connect(udid)
        r = _idb("describe", "--udid", udid, "--json")
        try:
            dim = json.loads(r.stdout)["screen_dimensions"]
            _POINTS[udid] = (int(dim["width_points"]), int(dim["height_points"]))
        except (ValueError, KeyError, TypeError):
            raise RuntimeError(
                f"could not read the point size of {udid} from `idb describe --json` (rc={r.returncode}): "
                f"{(r.stderr or r.stdout).strip()[:300]}") from None
    return _POINTS[udid]


def _scale(v: float, span: int) -> int:
    """One axis: |v| <= 1 is a FRACTION of `span`, anything larger is already device points."""
    return round(v * span) if abs(v) <= 1.0 else round(v)


def to_points(at, size: tuple[int, int]) -> tuple[int, int]:
    """Resolve a scenario `at`/`to` pair to DEVICE POINTS. See the COORDINATE SPACE note in plan().

    Same rule and same rejection as capture_android.to_pixels — deliberately, because both lanes read
    the SAME scenario files and a second contract would mean a coordinate that lands in two places.
    (Only the ABSOLUTE unit differs, and it has to: Android's injector takes pixels, idb takes points.)
    """
    if (not isinstance(at, (list, tuple)) or len(at) != 2
            or not all(isinstance(v, (int, float)) for v in at)):
        # Every coordinate in the module funnels through here, so this is the one place that has to
        # turn a malformed/missing `at` into the ValueError the callers are documented to raise — a
        # bare KeyError/TypeError out of a step reads as a crash, not as a scenario authoring error.
        raise ValueError(f"at/to must be [x, y] (fractions of the screen, or device points), got {at!r}")
    x, y = float(at[0]), float(at[1])
    frac = [abs(v) <= 1.0 for v in (x, y)]
    if any(frac) and not all(frac):
        raise ValueError(f"mixed coordinate {list(at)!r}: both values must be fractions (<=1.0) or "
                         f"both device points — a mixed pair would scale one axis and not the other")
    return _scale(x, size[0]), _scale(y, size[1])


def plan(steps: list[dict], size: tuple[int, int]) -> list[tuple[list[str], float]]:
    """Translate step dicts into (`idb ui` argv-tail, sleep-after) pairs. Pure — the self-check drives it.

    A step is the same dict shape the other lanes take:
        {"name": "initial", "settle": 0.25}                         no action -> hold, then shoot
        {"action": "click"|"tap", "at": [x, y]}                     tap
        {"action": "swipe"|"drag", "at": [x, y], "to": [x, y], "duration": 0.35, "steps": 20}
        {"action": "swipe"|"drag", "at": [x, y], "direction": "left", "distance": 0.5}
        {"action": "scroll", "at": [x, y], "dy": -0.4}              the VM lanes' wheel step, as a drag
        {"action": "type", "text": "abc"}                           into whatever has focus
        {"action": "hover", "at": [x, y]}                           SKIPPED — no pointer on glass
        {"action": "wait", "duration": 1.0}                         hold for an animation
    `drag` and `swipe` are ONE verb on glass: press, interpolated moves, release is what both mean —
    only the desktop agents' DEFAULTS differ. Keeping them apart here cost the lane an entire page:
    scenarios/swipe_refresh.toml raised "unknown scenario action: 'drag'" and never ran.
    Any step may carry `settle` = seconds to hold AFTER it (default STEP_SETTLE for an action, 0 for a
    bare marker step) — that is how the GIF burst steps recapture.py composes express their spacing.
    `steps` (the desktop agents' move-event count) is honoured, not ignored: a CarouselView pages on
    the INTERMEDIATE moves, so a scenario asking for 20 of them must not silently get 8.

    COORDINATE SPACE — `at` / `to`, and the `dx`/`dy`/`distance` scalars:
      * A pair whose |x| and |y| are BOTH <= 1.0 is a FRACTION of the screen (0,0 = top-left,
        1,1 = bottom-right). PREFER THIS FORM — it is the only one that means the same thing on this
        402x874 point surface, on a 1080x2340 emulator and in a 1024x800 desktop window, i.e. the only
        one a single scenario file can use to drive every lane. Mixed pairs are a hard error.
      * Anything larger is ABSOLUTE DEVICE POINTS, the unit `idb ui` takes. NOT the pixels you read off
        a board PNG: this device is 3x, so a pixel read off the image has to be divided by the density
        first. (recapture.device_scenarios refuses absolute scenarios on device lanes outright, so in
        practice only the fractional form ever reaches this lane — which is the point.)
      Fractions resolve against the POINT size from `idb describe`, so the EDGE_PT guard and the end
      clamp stay in the one unit everything here is expressed in.
    """
    w, h = size

    def start_point(at, edge: bool) -> tuple[int, int]:
        """A gesture's FIRST point, in device points: strict. An off-screen or edge start is a bug,
        never a clamp — it decides what the gesture touches."""
        x, y = to_points(at, size)
        margin = 0 if edge else EDGE_PT
        if not (margin <= x <= w - margin and margin <= y <= h - margin):
            raise ValueError(f"step point {list(at)} -> ({x},{y}) is outside the drivable area of a "
                             f"{w}x{h} point screen (edge margin {margin}pt)")
        return x, y

    def end_point(x, y) -> tuple[int, int]:
        """A drag's END, in device points: CLAMPED, because a finger cannot leave the glass. A `dy`
        that overshoots the screen is a normal way to say "scroll as far as this drag goes", not an
        authoring error."""
        return min(max(round(x), 0), w), min(max(round(y), 0), h)

    def drag(p0, p1, seconds, moves) -> list[str]:
        """`idb ui swipe` interpolates for us; `--delta` is the SPACING between the touch points it
        synthesises, so the scenario's `steps` (a COUNT, as the desktop agents mean it) becomes
        length/steps. Floored at 1 — a delta of 0 has no defined point list.

        UNIT CAVEAT: idb's help calls delta "pixels" while x/y are points, and that was not measured.
        If it really is pixels, a 3x device gets ~3x MORE intermediate touch points than `steps` asked
        for — which errs the safe way (a CarouselView pages on the intermediate moves, so more of them
        helps). The self-check pins the ARGV; it does not pin the resulting touch-point count."""
        (x0, y0), (x1, y1) = p0, p1
        delta = max(1, round(math.hypot(x1 - x0, y1 - y0) / moves))
        return ["swipe", str(x0), str(y0), str(x1), str(y1),
                "--duration", f"{seconds:g}", "--delta", str(delta)]

    out: list[tuple[list[str], float]] = []
    for step in steps:
        action = step.get("action")
        hold = float(step.get("settle", STEP_SETTLE if action else 0.0))
        if not action:
            out.append(([], hold))                                  # a bare {"name": …} = settle only
        elif action == "hover":
            # Nothing to emit, and deliberately NOT a tap (see _HOVER_SKIP). run_steps() prints the
            # reason, so the step is visibly skipped in the log rather than vanishing into a frame that
            # looks like a page which declined to react.
            out.append(([], hold))
        elif action in ("click", "tap"):
            x, y = start_point(step.get("at"), bool(step.get("edge")))
            tap = ["tap", str(x), str(y)]
            # A LONG press, when the step asks for one. MEASURED on iOS 26.5: an instantaneous
            # `idb ui tap` does NOT actuate a UISwitch — neither column moves — while the SAME point
            # with `--duration 0.5` toggles MAUI's switch (12428 px). So a zero-duration tap is not
            # "the tool cannot drive switches"; it is a press too brief for that control to accept.
            # The desktop lanes ignore this key and click normally, which is correct there.
            if step.get("duration"):
                tap += ["--duration", f"{float(step['duration']):g}"]
            out.append((tap, hold))
        elif action in ("swipe", "scroll", "drag"):
            x0, y0 = start_point(step.get("at"), bool(step.get("edge")))
            if action == "scroll":
                # `scroll` is the VM lanes' wheel step; on glass the same intent IS a drag by (dx, dy).
                # SIGN, spelled out because it is the easy one to invert: a NEGATIVE dy means the finger
                # travels UP the glass (the end y is smaller), which drags the content up and reveals
                # the rows BELOW — the same thing a negative wheel delta does on the desktop lanes, and
                # byte-identical to capture_android's `y2 = y + _scale(dy, h)`.
                # dx/dy scale per axis by the same |v| <= 1 rule as a coordinate pair.
                if "dy" not in step:
                    raise ValueError(f"step {step.get('name', action)!r}: scroll needs dy (device "
                                     f"points, or a fraction of the height)")
                x1, y1 = x0 + _scale(float(step.get("dx", 0)), w), y0 + _scale(float(step["dy"]), h)
            elif "to" in step:
                x1, y1 = to_points(step["to"], size)
            else:
                # The axis form. Missing/unknown `direction` used to be a bare KeyError on step["to"];
                # this module's contract is ValueError for a malformed step, and the callers rely on it.
                direction = str(step.get("direction", "")).lower()
                if direction not in _SWIPE_DIRECTIONS:
                    raise ValueError(f"step {step.get('name', action)!r}: needs to = [x, y] or "
                                     f"direction = one of {sorted(_SWIPE_DIRECTIONS)} "
                                     f"(got {step.get('direction')!r})")
                dx, dy = _SWIPE_DIRECTIONS[direction]
                span = w if dx else h
                dist = _scale(float(step["distance"]), span) if "distance" in step \
                    else round(_SWIPE_FRACTION * span)
                x1, y1 = x0 + dx * dist, y0 + dy * dist
            x1, y1 = end_point(x1, y1)
            if (x1, y1) == (x0, y0):
                # Press-hold-release at ONE point is a click. MEASURED: `idb ui swipe 200 200 200 200`
                # exits 0, so idb would run it, report success, and the frame would come back identical
                # — the silent no-op this whole section exists to prevent, wearing a different hat.
                # Every sibling lane raises here too.
                raise ValueError(f"step {step.get('name', action)!r}: zero-length {action} at "
                                 f"({x0},{y0}) is a click, not a pan (clamped to the {w}x{h} "
                                 f"point screen?)")
            moves = max(2, int(step.get("steps", SWIPE_MOVES)))   # <2 collapses the line to its ends
            out.append((drag((x0, y0), (x1, y1), float(step.get("duration", 0.35)), moves), hold))
        elif action == "type":
            # Goes to the FOCUSED field, so a `type` step almost always follows a `click` on it. No
            # shell filter, and do NOT copy capture_android's `_TEXT_SAFE` regex here: that guard exists
            # because `adb shell input` goes through a REMOTE SHELL that eats metacharacters, while idb
            # is exec'd as an argv list with no shell in the path, so the string arrives verbatim. The
            # one hostile shape left is a LEADING "--", which idb's argparse rejects with a nonzero exit
            # — run_steps raises on that, so it is a loud failure, not the silent-no-op class.
            # What DOES transform it is the iOS keyboard itself, identically for any injector:
            # autocapitalization applies exactly as it would to a human ("abc" lands as "Abc" in a
            # default Entry — author the expected text, or set the field's Keyboard), and the text lands
            # MARKED (autocorrect-highlighted) and uncommitted: the `entry` page showed "Abc"
            # highlighted with its LENGTH readout still 0. A scenario that needs the committed value has
            # to dismiss the candidate; the tool for that is `idb ui key <code>`, deliberately NOT wired
            # here until a page actually needs it.
            if "text" not in step:
                raise ValueError(f"step {step.get('name', action)!r}: type needs text = \"…\"")
            out.append((["text", str(step["text"])], hold))
        elif action == "wait":
            out.append(([], float(step.get("duration", step.get("settle", 1.0)))))
        else:
            raise ValueError(f"unknown scenario action: {action!r}")
    return out


def step_status(step: dict, cmds: list[str]) -> str:
    """What run_steps prints for a step that emitted `cmds`. Split out so the self-check can assert it:
    the ONLY record that a hover was skipped is this line, and an untested log line is a silent drop."""
    if cmds:
        return "ok"
    return f"SKIPPED ({_HOVER_SKIP})" if step.get("action") == "hover" else "idle"


def run_steps(steps: list[dict] | None, udid: str = UDID) -> None:
    """Drive the device through `steps`. No steps -> touches NOTHING (the 159 unscripted pages).

    Observed once in ~10 runs: the FIRST gesture after a cold launch is swallowed (the app has not
    attached its recognizers yet) and the page stays idle. The lever is the scenario's own `settle`,
    not a retry here — a step that lands and legitimately changes nothing must stay visible as the
    finding it is, not be papered over by re-poking until the pixels move.
    """
    if not steps:
        return
    # NOTHING here touches the host: no cursor is moved, no app is activated, nothing is brought to the
    # front. The Simulator window need not even be open. That is the point — see the module docstring.
    connect(udid)                                   # once per run, not per step
    commands = plan(steps, device_points(udid))

    # zip, so each emitted command is reported against the STEP that produced it: a step which emits
    # nothing is either an idle marker or a skipped hover, and those must not look alike in the log.
    for step, (cmds, after) in zip(steps, commands, strict=True):
        if cmds:
            # `--udid` last: idb's argparse takes options after positionals, and appending keeps plan()
            # pure (device-free, so the self-check can assert its argv without a simulator).
            # Bounded so a wedged companion can never outlive the recording it is driving.
            r = _idb("ui", *cmds, "--udid", udid,
                     timeout=float(step.get("duration", 0.35)) + 30)
            if r.returncode != 0:
                raise RuntimeError(f"idb ui {' '.join(cmds)} failed (rc={r.returncode}): "
                                   f"{(r.stderr or r.stdout).strip()[:200]}")
        name = step.get("name", step.get("action", "?"))
        print(f"      step {name}: {step_status(step, cmds)}", flush=True)
        time.sleep(after)


# --------------------------------------------------------------------------- run-dir evidence
# This module's app key -> the RUNNER's column name, which is what the run dir and the sidecar speak
# (motion_score.FW_TO_COL maps the board's framework dirs maui/cpp/xaml back onto these). Derived from
# `app` so a caller cannot mismatch the two; still overridable, since the column is the contract.
_COLUMN = {"maui": "maui_xaml", "cpp": "cpp", "xaml": "cpp_xaml"}
# How many full-res frames one recording contributes. 12 = the VM lanes' --gif-frames default, so an
# iOS sequence is scored over the same number of moments as a maccatalyst one. It is a COUNT, not a
# rate: the sample spacing below is derived from it, so a longer --gif-secs samples further apart
# rather than banking proportionally more megabytes.
MOTION_FRAMES = 12

_COMMIT: list[str] = []


def _commit() -> str:
    """The run's commit for the sidecar — byte-identical in form to run_comparison.git_commit()."""
    if not _COMMIT:
        r = subprocess.run(["git", "-C", CPP, "rev-parse", "--short", "HEAD"],
                           capture_output=True, text=True)
        _COMMIT.append(r.stdout.strip() or "unknown")
    return _COMMIT[0]


def frame_step(i: int, record_secs: float) -> str:
    """The step name of the i-th (0-based) frame extracted from a recording: its OFFSET IN
    MILLISECONDS from the moment the recorder started. Pure — the self-check drives it.

    WHY A TIMESTAMP, AND NOT AN INDEX. motion_score._pair joins the two columns BY STEP NAME exactly
    so that it can never join by index; a frame numbered and then called a name would defeat that in
    one line, because a column that dropped a frame would silently re-align its entire tail. A VM lane
    has real named scenario steps to join on. A `simctl io recordVideo` mp4 has none — it is one
    continuous video — so this lane has to supply a key that MEANS something. Elapsed time does: both
    columns start recording at the same point of the same code path (launch -> settle -> record ->
    0.5s -> the same scenario steps), so "1333 ms after the recorder started" is the same moment of
    the same scenario in either column, and it stays that moment however many frames either produced.

    This is deliberately NOT the "both recorded for the same nominal duration, so the k-th of K
    evenly-spaced samples is the same NORMALIZED moment" rule. Normalizing by duration makes every
    name a function of how long that recording happened to run, so a column whose steps overran would
    have its whole sequence silently re-timed against the other's — and it then needs a bolted-on
    equal-duration check to stay honest. An ABSOLUTE offset needs no equal-duration assumption at all,
    so there is nothing to check and nothing to forget to check: a recording that ran short simply
    stops producing names, its partner's extra frames pair with nothing, and motion_score reports them
    as unpaired (or refuses the cell outright when the overlap is empty). The same is true the other
    way: an overrun keeps sampling at the same spacing, so its extra frames are honestly named for the
    moments they were taken at. The guard IS the key.

    WHAT IT DOES NOT BUY — stated here because the number is otherwise easy to misread: the anchor is
    the RECORDER's start, not the animation's. On a free-running page (activity_indicator's spinner)
    the phase is set by app launch, so two columns can both be correct and still show a different
    spinner angle at the same offset, and the per-frame SSIM is coarse there. The load-bearing
    measurement on such a page is motion_score's per-column SELF-motion — "did this column move at
    all", the frozen-vs-animating finding — which needs no alignment between columns.

    THE `gif` PREFIX IS LOAD-BEARING, not decoration: recapture.burst_frames treats a `gif*` step as a
    burst frame, which is what keeps the temporally-distant `initial` still (a different launch, a
    different recording) out of the scored sequence and out of the self-motion anchor — exactly as on
    the VM lanes, whose burst steps are also named gifNN.
    """
    return f"gif{round(i * 1000.0 * record_secs / MOTION_FRAMES):05d}"


def _unit_dir(run_unit, app: str, column: str | None) -> tuple[str, str]:
    """(directory, column) for a run unit, or a loud rejection.

    The contract is `<comp>/<RUN>/<key>/ios/<column>/`, so the directory's own name IS the column.
    Checking that turns the one likely wiring mistake — handing this the run ROOT, or another lane's
    unit — into an error instead of a pile of frames nothing will ever pair."""
    col = column or _COLUMN[app]
    unit = str(run_unit)
    if os.path.basename(os.path.normpath(unit)) != col:
        raise ValueError(f"run_unit {unit!r} is not column {col!r}'s own directory — the contract is "
                         f"<comp>/<RUN>/<key>/ios/<column>/, not the run root")
    os.makedirs(unit, exist_ok=True)
    return unit, col


def _next_frame_no(unit: str) -> int:
    """The next NNNN in this unit. Both THEMES share one unit dir (the sidecar carries the theme, and
    motion_score._shots filters on it), and on iOS theme is the OUTER loop — so numbering continues
    across the theme flip and across the still/GIF pass rather than restarting and overwriting."""
    used = [int(n[:4]) for n in os.listdir(unit)
            if len(n) == 8 and n.endswith(".png") and n[:4].isdigit()]
    return max(used, default=0) + 1


def _sidecar(unit: str, n: int, col: str, key: str, theme: str, step: str) -> None:
    """NNNN.json beside NNNN.png — the shape run_comparison.py writes, which is the shape
    motion_score._shots / recapture.burst_frames read."""
    with open(os.path.join(unit, f"{n:04d}.json"), "w") as fh:
        json.dump({"tag": key, "platform": "ios", "column": col, "theme": theme, "step": step,
                   "frame": n, "commit": _commit(),
                   "captured_at": datetime.now().astimezone().isoformat()}, fh, indent=2)


def _bank(run_unit, app: str, column: str | None, key: str, theme: str, png: str, step: str) -> str:
    """Copy an already-published PNG into the run unit as the next frame. Returns its path.

    THE BYTES ARE COPIED — the frame is never re-shot. motion_score._is_published_run accepts a run
    only if it holds a byte-identical twin of the still captures/ currently shows, so a second
    screenshot of the same screen (same page, one JPEG-of-a-JPEG's worth of difference) would leave
    every iOS cell refused with "their frames do not match captures/ byte-for-byte"."""
    unit, col = _unit_dir(run_unit, app, column)
    n = _next_frame_no(unit)
    dst = os.path.join(unit, f"{n:04d}.png")
    shutil.copyfile(png, dst)
    _sidecar(unit, n, col, key, theme, step)
    return dst


def _bank_recording(run_unit, app: str, column: str | None, key: str, theme: str,
                    mp4: str, record_secs: float) -> int:
    """Bank MOTION_FRAMES full-resolution frames out of the recording the GIF was just built from.

    THE SAME mp4, decoded again — never a second recording (which would be a different animation) and
    never a downscale. That is the entire point: gif.py renders the human-viewable GIF at 400px wide
    (`_SCALE`), which throws away most of the pixels a score needs, while `simctl io recordVideo`
    writes at the device framebuffer resolution the still is captured at.

    ffmpeg's `fps` filter emits output frame i at t = i/fps, which is the fixed grid `frame_step`
    names; the rate is derived from record_secs so the count stays ~MOTION_FRAMES whatever --gif-secs
    is. RAISES if the extraction produced nothing — a page that banked a GIF and no frames would
    otherwise read on the board as motion-scored-and-fine."""
    unit, col = _unit_dir(run_unit, app, column)
    first = _next_frame_no(unit)
    fps = MOTION_FRAMES / max(record_secs, 0.1)
    r = subprocess.run(["ffmpeg", "-y", "-i", mp4, "-vf", f"fps={fps:g}",
                        "-start_number", str(first), os.path.join(unit, "%04d.png")],
                       capture_output=True, text=True)
    # DROP LEADING FRAMES THAT ARE NOT THE PAGE. capture_still already refuses a splash frame ("a
    # screenshot can succeed and still be a picture of the WRONG SCREEN"), but the motion path banked
    # whatever the recorder caught — and a recording starts right after launch, so on a slow page it
    # opens on the black pre-draw screen. Measured: ios/cpp/pan_gesture_events dark banked three frames
    # at mean brightness 0.4 before the UI appeared at 48.5, and the scorer read black -> rendered as
    # 2,814,930 px of motion (89.02%, worst SSIM 0.1070) against a MAUI column that had settled. That
    # is a FALSE "MOTION MISMATCH" red — a launch artifact reported as a port defect, which is exactly
    # the class of lie this pass exists to remove. The settle before recording is already 4s; the fix
    # is not a longer wait but refusing to score a frame that is not the page.
    raw = []
    n = first
    while os.path.exists(os.path.join(unit, f"{n:04d}.png")):
        raw.append(os.path.join(unit, f"{n:04d}.png"))
        n += 1
    lead = 0
    while lead < len(raw) and is_splash(raw[lead]):
        lead += 1
    if lead:
        for p in raw[:lead]:
            os.remove(p)
        for i, p in enumerate(raw[lead:]):                 # close the gap: NNNN must stay contiguous
            os.rename(p, os.path.join(unit, f"{first + i:04d}.png"))
        raw = [os.path.join(unit, f"{first + i:04d}.png") for i in range(len(raw) - lead)]
    made = len(raw)
    for i in range(made):
        # Named for the moment it was RECORDED at, not its position after trimming: dropping a leading
        # splash must not re-time the rest of the sequence onto the other column's earlier frames.
        _sidecar(unit, first + i, col, key, theme, frame_step(lead + i, record_secs))
    if made >= 2:
        if lead:
            print(f"      run-dir: dropped {lead} leading splash/pre-draw frame(s) from {key}/{theme}")
        return made
    # AN EMPTY RECORDING IS EVIDENCE, NOT AN ERROR — and treating it as one threw the finding away.
    # `simctl io recordVideo` writes an mp4 with NO ENCODED FRAMES when nothing on screen changes
    # (H.264 emits on change), so a page that does not animate yields video:0KiB and ffmpeg extracts
    # nothing. Measured on the first full animated sweep: 37 of 84 units raised here, and the pages
    # that raised were EXACTLY the ones that do not animate — activity_indicator and carousel_page,
    # the only two that move, were the only two absent from the failure list.
    #
    # Raising lost precisely the information the scorer exists to report: those pages ended up with no
    # frames, so they read "NOT motion-scored" instead of "!! NOTHING MOVED". A burst always produces
    # frames whether or not anything moves — identical ones on a static page, which is the honest
    # answer — so it is the right source when the recording declines to give any. Same conclusion the
    # Android lane reached about `adb shell screenrecord`, which returns a 1-frame mp4 on this
    # emulator; that lane has used a screencap burst ever since.
    for stale in range(first, n):
        os.remove(os.path.join(unit, f"{stale:04d}.png"))            # a lone frame cannot be scored
        os.remove(os.path.join(unit, f"{stale:04d}.json"))
    return _bank_burst(run_unit, app, column, key, theme, record_secs, udid=UDID,
                       why=f"recording held {made} frame(s) (rc={r.returncode})")


def _bank_burst(run_unit, app: str, column: str | None, key: str, theme: str, record_secs: float,
                udid: str, why: str) -> int:
    """MOTION_FRAMES screenshots over the same window the recording covered — the fallback source.

    Named on the SAME millisecond grid `frame_step` gives the recording path, so a page captured by
    burst on one column and by recording on the other still pairs: both name a frame for the moment it
    was taken, not for its position in a list. The interval is wall-clock, so a slow shot pushes later
    frames late; that is honest drift, and it is bounded by naming each frame from its OWN measured
    offset rather than from the nominal schedule."""
    unit, col = _unit_dir(run_unit, app, column)
    first = _next_frame_no(unit)
    step = max(record_secs, 0.1) / MOTION_FRAMES
    t0, made = time.monotonic(), 0
    for i in range(MOTION_FRAMES):
        due = t0 + i * step
        slack = due - time.monotonic()
        if slack > 0:
            time.sleep(slack)
        stage = os.path.join(tempfile.gettempdir(), f"parity_burst_{app}_{key}_{theme}.png")
        r = subprocess.run(["xcrun", "simctl", "io", udid, "screenshot", "--type=png", stage],
                           capture_output=True, text=True)
        if r.returncode != 0:
            continue                       # a dropped shot leaves a GAP in the grid, never a shift
        if is_splash(stage):
            os.remove(stage)               # same rule as the recording path: never score a frame that
            continue                       # is not the page (see _bank_recording's splash trim)
        n = first + made
        shutil.copyfile(stage, os.path.join(unit, f"{n:04d}.png"))
        os.remove(stage)
        _sidecar(unit, n, col, key, theme,
                 frame_step(round((time.monotonic() - t0) / step), record_secs))
        made += 1
    if made < 2:
        raise RuntimeError(f"iOS burst fallback banked {made} frame(s) for {key}/{theme} ({why}) — "
                           f"a sequence of fewer than two frames cannot be scored for motion")
    return made


# The step name of the reacted frame `still_first` banks after driving. Not `initial` (that name is
# reserved for the at-rest frame every lane publishes) and not `gif*` (recapture.burst_frames treats
# those as burst frames). Constant, so the two columns of one page pair on it — they run the same
# scenario, so "after the last step" is the same moment of the same page in each.
REACTED_STEP = "driven"


def capture_still(app: str, key: str, theme: str, settle: float, udid: str = UDID,
                  steps: list[dict] | None = None, run_unit=None, column: str | None = None,
                  still_first: bool = False) -> str | None:
    """Launch + settle + screenshot. Returns the written path, or None if the frame was DROPPED.

    `run_unit` (a `<comp>/<RUN>/<key>/ios/<column>/` path) additionally banks the PUBLISHED bytes
    there as this unit's `initial` frame; None keeps the pre-run-dir behavior exactly.

    `steps` DRIVE THE PAGE, and `still_first` decides on which side of the shot:

      * False (the default, and what an ANIMATED page gets): drive, THEN shoot — the published still
        is the REACTED frame. That is deliberate there, because such a page's board artifact is the
        GIF from `capture_gif` and its motion frames come out of that recording.
      * True: shoot AT REST first, publish and bank THAT, then drive and bank the reacted frame
        beside it as `REACTED_STEP`. This is what a driven page needs: the board keeps the resting
        render (a post-click switch is not what "at rest" looks like), and the unit holds a real
        before/after for motion_score instead of the single frame this path used to produce.

    Every VM lane already works the second way — it shoots a step named `initial` with no action and
    import_run_captures publishes that.
    """
    out = out_path(app, key, theme, "png")
    os.makedirs(os.path.dirname(out), exist_ok=True)
    stage = os.path.join(tempfile.gettempdir(), f"parity_{app}_{key}_{theme}.png")
    # The drive that precedes the SHOT. Under `still_first` it is empty and the steps run afterwards —
    # including on every splash retry, whose relaunch would otherwise leave the page undriven.
    pre = None if still_first else steps

    def keep(published: str) -> str:
        """Both return paths funnel through here — the splash retry publishes its own frame, and a
        run dir that recorded only the first attempt would be banking a picture of a splash screen."""
        if run_unit is not None:
            _bank(run_unit, app, column, key, theme, published, "initial")
        return published

    def react(published: str) -> str:
        """`still_first`: the at-rest frame is banked, so NOW drive and bank what the page became.

        Only when there is somewhere to bank it: without a run unit the reacted frame has nothing to
        be compared against and nowhere to live (it must never reach the board path — that is the
        whole point of shooting at rest first), so driving would be pure wall-clock."""
        if not (still_first and steps and run_unit is not None):
            return published
        run_steps(steps, udid)
        if _screenshot(stage, udid):
            if is_splash(stage):
                # Same rule as everywhere else in this module: never bank a frame that is not the
                # page. The unit keeps its at-rest frame, and motion_score reports the missing pair.
                print(f"      run-dir: reacted frame for {key}/{theme} was a splash — not banked")
            else:
                _bank(run_unit, app, column, key, theme, stage, REACTED_STEP)
            os.remove(stage)
        return published

    launch(app, key, udid)
    time.sleep(settle)
    run_steps(pre, udid)
    if not _screenshot(stage, udid):
        return None
    shutil.copyfile(stage, out)
    os.remove(stage)
    if not is_splash(out):
        return react(keep(out))
    for extra in (4.0, 8.0, 16.0):
        launch(app, key, udid)
        time.sleep(settle + extra)
        run_steps(pre, udid)   # a relaunch discards the reacted state — re-drive, or the retry frame is idle
        if not _screenshot(stage, udid):
            continue
        shutil.copyfile(stage, out)
        os.remove(stage)
        if not is_splash(out):
            return react(keep(out))
    os.remove(out)   # still a splash — drop it rather than bank a known-bad frame
    return None


def capture_gif(app: str, key: str, theme: str, settle: float, record_secs: float = 4.0,
                udid: str = UDID, steps: list[dict] | None = None,
                run_unit=None, column: str | None = None) -> str | None:
    """Record a short mp4 and convert it to a paletted GIF — for pages a single still cannot represent.

    The GIF is not a nicety: comparison_paths.find_capture() and build_comparison_json.py both prefer
    `.gif` over `.png`, so refreshing an animated page's PNG alone leaves the board rendering the
    PREVIOUS run's GIF behind a green log line.

    `steps` run INSIDE the recording window — a page that only moves when poked has to be poked while
    the camera is rolling, or the GIF is `record_secs` of a still page (which is exactly how 13 pages
    ended up with 12 byte-identical frames and no GIF at all).

    `run_unit` ALSO banks MOTION_FRAMES full-resolution frames out of that same recording, named by
    their millisecond offset (see `frame_step`) so motion_score can pair the columns by step name and
    score the animation instead of one resting frame. None keeps the pre-run-dir behavior exactly.
    """
    gif = out_path(app, key, theme, "gif")
    os.makedirs(os.path.dirname(gif), exist_ok=True)
    gifmod.drop_stale(gif)
    mp4 = os.path.join(tempfile.gettempdir(), f"parity_{app}_{key}_{theme}.mp4")
    launch(app, key, udid)
    time.sleep(settle)
    if steps:
        # Spawn the companion and read the point size BEFORE the camera rolls: both are cached, so this
        # is free on every later page, and it keeps a first-page companion spawn out of the recording
        # window (where it would eat a chunk of `record_secs` on one arbitrary page).
        device_points(udid)
    proc = subprocess.Popen(["xcrun", "simctl", "io", udid, "recordVideo", "--codec=h264", "--force", mp4],
                            stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    t0 = time.monotonic()
    try:
        # try/finally, not a straight line: a step that raises must not leave a recordVideo running and
        # a half-written mp4 behind — the next page's recording would inherit both.
        if steps:
            time.sleep(0.5)          # let the recorder actually start before anything moves
            run_steps(steps, udid)
        # record_secs is the LENGTH OF THE RECORDING, steps included — not idle time bolted onto the end.
        # A scenario longer than record_secs simply ends the recording when its last step does.
        time.sleep(max(0.0, record_secs - (time.monotonic() - t0)))
    finally:
        proc.send_signal(signal.SIGINT)   # simctl finalizes the mp4 on SIGINT
        try:
            proc.wait(timeout=20)
        except subprocess.TimeoutExpired:
            proc.kill()
    try:
        ok = gifmod.video_to_gif(mp4, gif)
        if run_unit is not None:
            # AFTER the GIF, so the board artifact is on disk before anything else can fail — and
            # unconditionally, because a recording that gif.py rejected as motionless is exactly the
            # case motion_score's "NOTHING MOVED" verdict exists to report, and it needs the frames.
            n = _bank_recording(run_unit, app, column, key, theme, mp4, record_secs)
            print(f"      run-dir: {n} full-res frame(s) banked", flush=True)
    finally:
        # finally, not a straight line: a failed extraction must not leave the mp4 for the next page's
        # recording to inherit (simctl writes to a per-page path, but --force overwrites blindly).
        if os.path.exists(mp4):
            os.remove(mp4)
    return gif if ok else None


if __name__ == "__main__":
    # `python3 lib/capture_ios.py` — the coordinate mapping, checked against MEASURED values so a
    # regression is caught without a device. POINTS is the real size `idb describe` reports for this
    # machine's device (iPhone17-265, iOS 26.5: 402x874 points behind a 1206x2622 framebuffer at
    # density 3.0), which is the unit `idb ui` takes. (200,186) is the `button` page's "Clicked" button
    # — the same widget the old host-cursor path hit at pixel (600,558); 600/3, 558/3.
    POINTS = (402, 874)

    assert plan([{"action": "click", "at": [200, 186]}], POINTS) == [(["tap", "200", "186"], STEP_SETTLE)]
    assert plan([{"action": "tap", "at": [200, 186]}], POINTS)[0][0] == ["tap", "200", "186"]
    assert plan([{"name": "initial"}], POINTS) == [([], 0.0)]                  # no action -> settle only
    assert plan([{"name": "gif01", "settle": 0.25}], POINTS) == [([], 0.25)]
    assert plan([{"action": "type", "text": "abc"}], POINTS) == [(["text", "abc"], STEP_SETTLE)]
    assert plan([{"action": "wait", "duration": 2.0}], POINTS) == [([], 2.0)]

    # A pan: one `idb ui swipe`, in points, with the duration in SECONDS (measured: `--duration 2` takes
    # ~2.4s wall, so it is not milliseconds — at 0.35s a millisecond reading would stall the recording
    # window for 350 SECONDS per step). `--delta` is the spacing between synthesised touch points:
    # 367pt of travel over the default 8 -> 46.
    swipe, = plan([{"action": "swipe", "at": [0.5, 0.72], "to": [0.5, 0.3], "duration": 0.4}], POINTS)
    assert swipe == (["swipe", "201", "629", "201", "262", "--duration", "0.4", "--delta", "46"],
                     STEP_SETTLE), swipe

    # `scroll` is a relative drag. NEGATIVE dy = the finger travels UP = the content moves up = lower
    # rows are revealed, exactly as on the other three lanes; and its END clamps to the glass instead
    # of failing, because a drag off the edge is a SHORTER drag, not an authoring error.
    assert plan([{"action": "scroll", "at": [0.5, 0.72], "dy": -0.42, "duration": 0.4}],
                POINTS)[0][0] == swipe[0]
    assert plan([{"action": "scroll", "at": [0.5, 0.72], "dy": -9999}], POINTS)[0][0][:5] == \
        ["swipe", "201", "629", "201", "0"]
    assert plan([{"action": "scroll", "at": [0.5, 0.4], "dy": 0.3}], POINTS)[0][0][4] == "612", \
        "positive dy must move the finger DOWN"                         # 350 + round(0.3*874) = 612

    # ---- the shared vocabulary, which this module used to implement only a subset of.
    # `drag` IS `swipe` on glass: same argv, or scenarios/swipe_refresh.toml cannot run here at all.
    assert plan([{"action": "drag", "at": [0.5, 0.72], "to": [0.5, 0.3], "duration": 0.4}],
                POINTS)[0][0] == swipe[0]
    # The direction/distance form (scenarios/carousel_page.toml), and its 0.25-of-the-axis default.
    assert plan([{"action": "swipe", "at": [0.5, 0.72], "direction": "up", "distance": 367,
                  "duration": 0.4}], POINTS)[0][0] == swipe[0]
    left, = plan([{"action": "swipe", "at": [0.5, 0.72], "direction": "left"}], POINTS)
    assert left[0][:5] == ["swipe", "201", "629", "101", "629"], left   # 0.25 * 402 = 100pt left of 201
    # `steps` is the desktop agents' MOVE-EVENT COUNT. idb interpolates by spacing rather than by count,
    # so it arrives as `--delta = length / steps`: honoured, not ignored, because a CarouselView pages
    # on the intermediate moves and carousel_page.toml asks for 20 of them (swipe_refresh for 24).
    # Floored at 2 — 1 would collapse the line to its two endpoints.
    assert plan([{"action": "drag", "at": [0.5, 0.72], "to": [0.5, 0.3], "steps": 20}],
                POINTS)[0][0][-1] == "18", "367 / 20"
    assert plan([{"action": "drag", "at": [0.5, 0.72], "to": [0.5, 0.3], "steps": 1}],
                POINTS)[0][0][-1] == "184", "floored to 2: 367 / 2"
    # hover: emitted as nothing at all, NOT as a tap — and reported, since the log line is the only
    # record that the step existed. A hover silently turned into a tap would fake a pressed state.
    hover = {"action": "hover", "at": [200, 186]}
    assert plan([hover], POINTS) == [([], STEP_SETTLE)]
    assert step_status(hover, []).startswith("SKIPPED"), step_status(hover, [])
    assert step_status({"name": "initial"}, []) == "idle"
    assert step_status({"action": "click", "at": [200, 186]}, ["tap", "200", "186"]) == "ok"

    # FRACTIONS are the portable authoring form: a fraction must resolve to the identical gesture as
    # the device points it names. This is what lets ONE scenario file drive iOS, Android and both VMs.
    # NOTE the unit trap: absolute here is POINTS, so a coordinate read off a board PNG (pixels) must
    # be divided by the 3.0 density first. recapture.device_scenarios refuses absolute scenarios on
    # device lanes outright, which is why this is a footnote and not a live hazard.
    assert to_points([0.5, 0.2], POINTS) == (201, 175)
    assert to_points([201, 175], POINTS) == (201, 175)      # >1 is already device points
    assert to_points([1.0, 1.0], POINTS) == POINTS          # the 1.0 boundary is a fraction
    assert plan([{"action": "click", "at": [0.5, 0.2]}], POINTS) == \
        plan([{"action": "click", "at": [201, 175]}], POINTS)
    for mixed in ([0.5, 300], [300, 0.5]):                  # one axis scaled, one not
        try:
            plan([{"action": "click", "at": mixed}], POINTS)
            raise AssertionError(f"mixed coordinate {mixed} was accepted")
        except ValueError:
            pass

    # A drag that goes nowhere is a CLICK, and every sibling lane raises on it — idb does NOT: a
    # measured `idb ui swipe 200 200 200 200` exits 0 and changes no pixel. Both shapes: authored
    # zero-length, and clamped-to-nothing (which needs `edge` to get a start point on the boundary).
    for nowhere in ({"action": "drag", "at": [0.5, 0.72], "to": [0.5, 0.72]},
                    {"action": "swipe", "at": [0.5, 0.0], "edge": True, "direction": "up"},
                    {"action": "scroll", "at": [0.5, 0.72], "dy": 0}):
        try:
            plan([nowhere], POINTS)
            raise AssertionError(f"zero-length gesture {nowhere!r} was accepted")
        except ValueError:
            pass
    try:
        plan([{"action": "swipe", "at": [0.5, 0.72], "direction": "sideways"}], POINTS)
        raise AssertionError("expected an unknown-direction rejection")
    except ValueError:
        pass
    try:
        plan([{"action": "swipe", "at": [0.5, 0.72]}], POINTS)   # no `to`, no `direction`
        raise AssertionError("expected a ValueError, not a bare KeyError")
    except ValueError:
        pass
    for shapeless in ({"action": "click"},                                  # no `at` at all
                      {"action": "drag", "at": [200], "to": [200, 300]},    # not a pair
                      {"action": "scroll", "at": [0.5, 0.72]},              # no dy
                      {"action": "type"}):                                  # no text
        try:
            plan([shapeless], POINTS)
            raise AssertionError(f"malformed coordinate {shapeless!r} was accepted")
        except ValueError:
            pass

    for bad, why in [({"action": "click", "at": [200, 5]}, "top edge -> Notification Center"),
                     ({"action": "click", "at": [200, 870]}, "bottom edge -> home gesture"),
                     ({"action": "click", "at": [8, 300]}, "left edge -> back gesture"),
                     ({"action": "click", "at": [200, 9999]}, "off screen")]:
        try:
            plan([bad], POINTS)
            raise AssertionError(f"expected a rejection: {why}")
        except ValueError:
            pass
    assert plan([{"action": "click", "at": [200, 5], "edge": True}], POINTS)   # opt-in still allowed

    try:
        plan([{"action": "pinch"}], POINTS)
        raise AssertionError("expected unknown-action rejection")
    except ValueError:
        pass

    # The idb client is resolved BY PATH (pipx installs outside a non-login shell's PATH) and its
    # absence must name the install, not surface as "did nothing and reported success".
    _real_idb = IDB
    try:
        IDB = "/nonexistent/idb-xyzzy"
        try:
            idb_bin()
            raise AssertionError("a missing idb was accepted")
        except RuntimeError as exc:
            assert "brew install idb-companion" in str(exc), exc
    finally:
        IDB = _real_idb

    # ---- RUN-DIR EVIDENCE. The naming rule and the byte-identity rule, checked against the REAL
    # consumers (motion_score, recapture.burst_frames) rather than against a restatement of them.
    # The grid: frame i sits at i * record_secs / MOTION_FRAMES, named in milliseconds.
    assert frame_step(0, 4.0) == "gif00000", frame_step(0, 4.0)
    assert frame_step(1, 4.0) == "gif00333", frame_step(1, 4.0)
    assert frame_step(3, 4.0) == "gif01000", frame_step(3, 4.0)
    assert frame_step(MOTION_FRAMES - 1, 4.0) == "gif03667"
    # THE property that makes the key duration-free: the same moment gets the same NAME out of two
    # recordings of different length, so the columns pair on when a frame was taken and never on
    # which frame it happened to be. (Normalized-by-duration naming gets this exactly wrong: it would
    # call 2000ms-of-4s and 4000ms-of-8s the same frame.)
    assert frame_step(2, 4.0) == frame_step(1, 8.0) == "gif00667"
    assert frame_step(6, 4.0) != frame_step(6, 8.0)
    # An overrun keeps the SAME spacing past record_secs — honest names, not a re-timed sequence.
    assert frame_step(MOTION_FRAMES + 3, 4.0) == "gif05000"

    import motion_score  # the actual reader of everything banked below
    from recapture import burst_frames

    with tempfile.TemporaryDirectory() as tmp:
        board = os.path.join(tmp, "board.png")
        with open(board, "wb") as fh:
            fh.write(b"\x89PNG\r\n\x1a\n published-still bytes")
        unit = os.path.join(tmp, "2026-08-05-00_00_00", "demo", "ios", "cpp")

        # A run unit that is not the column's own directory is refused, not filled with orphan frames.
        for wrong in (os.path.dirname(unit), os.path.join(tmp, "run", "demo", "ios", "maui_xaml")):
            try:
                _bank(wrong, "cpp", None, "demo", "light", board, "initial")
                raise AssertionError(f"run_unit {wrong!r} was accepted for column 'cpp'")
            except ValueError:
                pass

        # The still: byte-identical, which is the ONLY thing _is_published_run accepts.
        first = _bank(unit, "cpp", None, "demo", "light", board, "initial")
        assert os.path.basename(first) == "0001.png", first
        assert open(first, "rb").read() == open(board, "rb").read()
        meta = json.load(open(os.path.join(unit, "0001.json")))
        assert set(meta) == {"tag", "platform", "column", "theme", "step", "frame", "commit",
                             "captured_at"}, sorted(meta)
        assert (meta["tag"], meta["platform"], meta["column"], meta["theme"], meta["step"],
                meta["frame"]) == ("demo", "ios", "cpp", "light", "initial", 1)
        assert _COLUMN["xaml"] == "cpp_xaml" == motion_score.FW_TO_COL["xaml"]   # board dir -> column

        # The recording's frames, as _bank_recording would number them, plus a DARK frame: both themes
        # share one unit dir, so numbering has to continue rather than restart onto 0001.
        for i in range(3):
            shutil.copyfile(board, os.path.join(unit, f"{_next_frame_no(unit):04d}.png"))
            _sidecar(unit, _next_frame_no(unit) - 1, "cpp", "demo", "light", frame_step(i, 4.0))
        dark = _bank(unit, "cpp", None, "demo", "dark", board, "initial")
        assert os.path.basename(dark) == "0005.png", dark

        # _shots/burst_frames take a Path; this module is os.path-based end to end, so the conversion
        # happens here rather than dragging pathlib through the capture code.
        import pathlib
        shots = motion_score._shots(pathlib.Path(unit), "light")
        assert [s for s, _, _ in shots] == ["initial", "gif00000", "gif00333", "gif00667"], shots
        assert motion_score._is_published_run(board, shots), "published still not found in the run"
        with open(os.path.join(tmp, "other.png"), "wb") as fh:
            fh.write(b"\x89PNG\r\n\x1a\n a DIFFERENT still")
        assert not motion_score._is_published_run(os.path.join(tmp, "other.png"), shots), \
            "a run that did not produce the published still must be refused"

        # The `gif` prefix, and why it is not cosmetic: burst_frames must drop the temporally-distant
        # `initial` frame (a separate launch and a separate recording) from the scored sequence, or it
        # anchors motion_score's self-motion — the frozen-vs-animating detector — on a frame from
        # another session. Same rule the VM lanes get.
        assert [os.path.basename(p) for p in burst_frames(pathlib.Path(unit), "light")] == \
            ["0002.png", "0003.png", "0004.png"]
        assert [os.path.basename(p) for p in burst_frames(pathlib.Path(unit), "dark")] == []
        # Every banked frame carries a name, so none is dropped by _pair (which discards nameless
        # frames rather than keying them all on "" and pairing by ordinal).
        assert all(s for s, _, _ in shots)
        paired = motion_score._pair([(s, p) for s, p, _ in shots], [(s, p) for s, p, _ in shots])
        assert len(paired) == len(shots), paired

    # ---- A DRIVEN, NON-ANIMATED PAGE (`still_first`): at-rest frame published and banked, reacted
    # frame banked beside it. REACTED_STEP is the load-bearing detail — `initial` is the published
    # still's name and a `gif*` name would make burst_frames treat the frame as a burst frame, so
    # either would leave the sequence with no BEFORE, which is the single-frame bug wearing a run dir.
    assert REACTED_STEP != "initial" and not REACTED_STEP.startswith("gif"), REACTED_STEP
    with tempfile.TemporaryDirectory() as tmp:
        unit = os.path.join(tmp, "2026-08-05-00_00_00", "switch", "ios", "cpp")
        rest, hit = os.path.join(tmp, "rest.png"), os.path.join(tmp, "hit.png")
        for p, b in ((rest, b"\x89PNG\r\n\x1a\n switch OFF"), (hit, b"\x89PNG\r\n\x1a\n switch ON")):
            with open(p, "wb") as fh:
                fh.write(b)
        _bank(unit, "cpp", None, "switch", "light", rest, "initial")
        _bank(unit, "cpp", None, "switch", "light", hit, REACTED_STEP)

        import pathlib
        shots = motion_score._shots(pathlib.Path(unit), "light")
        assert [s for s, _, _ in shots] == ["initial", REACTED_STEP], shots
        # The board still is the AT-REST frame, and this run is the one behind it — the reacted frame
        # must never be what captures/ shows.
        assert motion_score._is_published_run(rest, shots), "the at-rest frame is not the run's still"
        assert not motion_score._is_published_run(hit, [shots[0]]), "the reacted frame was published"
        # Two frames, distinct, and both paired: a real before/after instead of one still.
        assert len(motion_score._pair([(s, p) for s, p, _ in shots],
                                      [(s, p) for s, p, _ in shots])) == 2
        assert len(burst_frames(pathlib.Path(unit), "light")) == 2, "a driven unit keeps its BEFORE"

    # EVERY checked-in scenario, replayed through plan() — the real files, not fixtures. These are read
    # by four lanes, and this one used to implement a strictly SMALLER verb set than the vocabulary they
    # are authored in: `drag` raised "unknown scenario action" (swipe_refresh) and the direction form
    # raised a bare KeyError (carousel_page), so two pages could never be driven on iOS at all. A
    # scenario that the shared vocabulary accepts must never again be un-runnable here without this
    # failing. The surface is deliberately OVERSIZED (the old framebuffer numbers): three of these files
    # still carry Mac-calibrated absolute coordinates, which a device lane refuses upstream
    # (recapture.device_scenarios) — planning them here keeps `click`/`type`/`scroll` inside the verb
    # guard without pretending those numbers would ever be replayed. NOTE what it does NOT prove: an
    # absolute coordinate that fits still names the WRONG spot — only a fraction is portable, and only
    # a real capture proves a hit.
    import tomllib
    BIG = (1206, 2622)
    scenarios = os.path.join(CPP, "docs", "comparison", "scenarios")
    broken = []
    for name in sorted(f for f in os.listdir(scenarios) if f.endswith(".toml")):
        with open(os.path.join(scenarios, name), "rb") as fh:
            steps = tomllib.load(fh).get("steps", [])
        try:
            print(f"  {name}: {len(plan(steps, BIG))} step(s) planned")
        except Exception as exc:                              # noqa: BLE001 — report all, not the first
            broken.append(f"{name}: {type(exc).__name__}: {exc}")
            print(f"  {name}: FAILED — {type(exc).__name__}: {exc}")
    assert not broken, broken

    print("capture_ios self-check OK")
