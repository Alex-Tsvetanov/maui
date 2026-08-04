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
    in through the Simulator's own WINDOW — see "Driving the device" below.

DRIVING THE DEVICE (scenario steps)
-----------------------------------
`run_steps()` replays a page's step dicts against the device — before the still, and DURING the GIF
recording. Touches are injected HOST-SIDE: `cliclick` posts real mouse/key events onto the Simulator
WINDOW, which forwards them to the device. The alternatives were measured on this machine, not assumed:

  * `xcrun simctl` — no input subcommand of any kind (read its full subcommand list). A dead end.
  * `idb` (`idb ui tap/swipe/text`) — the robust, window-free path, but NOT installed (`which idb` ->
    not found), and its Xcode-26 support is unverified. This is the upgrade if the window path rots.
  * the port's own DevFlow HTTP agent (docs/DEVFLOW_PROTOCOL.md) — reachable (a simulator process
    shares the host's 127.0.0.1), but it exists ONLY in the C++ galleries; the MAUI reference column
    has no agent. Driving 2 of 3 columns makes every scenario page a guaranteed FALSE RED, so it
    cannot be the parity driver. (Its `/tap` is also `i_button`-gated: a tap recognizer on a Label or
    Grid answers `found:true, activated:false`.)
  * an XCUITest runner — a signed test host per app; the heaviest option for the smallest gain.

cliclick is the only zero-install path that is APP-AGNOSTIC, which is the entire requirement: all three
columns must receive the identical gesture or the comparison means nothing. Verified on this machine —
the C++ gallery's `button` page went Taps: 0 -> 1, the MAUI reference's first button lit its pressed
state under a held drag, `scroll_view` scrolled under a swipe, and `t:abc` landed glyphs in `entry`.

GEOMETRY (measured — do NOT guess an inset): the Simulator window is not the device screen. On Xcode 26
the window (412x884 pt here) is a 52pt title bar + the drawn device bezel + the screen, and the user's
window scale (~0.90) shrinks the screen inside it, so no fixed inset survives. The screen rect is read
at RUNTIME from the Accessibility tree: the window child whose SUBROLE is `iOSContentGroup` IS the
screen (measured 363x789 pt at (1578,138) for a 1206x2622 framebuffer — aspect 0.4601 vs 0.45996).
Everything maps through that rect, so a moved, rescaled or different-sized device still lands correctly.
An early "uniform 5pt border" model looked perfectly plausible and tapped the WRONG ROW — the class of
bug in `cpp-capture-fabricates-plausible-data`, which is why every mapped point is bounds-checked.

Interaction REQUIRES the Simulator window open and frontmost (`recapture.py --visible yes`; the
"cosmetic ONLY on iOS" note there stops being true for scenario pages) plus Accessibility permission
for this process. Both are checked and a failure RAISES — the lane already turns that into one failed
page, whereas a silent no-op reproduces the very bug this exists to fix: 12 byte-identical burst frames
from a page nobody ever touched.

Apps (bundle + env contract):
  maui -> dev.mauicpp.mauireference     MAUI_COMPARE_PAGE -> port/maui-reference/captures/ios/<key>_<theme>.png
  cpp  -> dev.maui-cpp.ios-gallery      MAUI_SAMPLE_PAGE  -> docs/comparison/captures/ios/cpp/<key>_<theme>.png
  xaml -> dev.maui-cpp.ios-gallery-xaml MAUI_SAMPLE_PAGE  -> docs/comparison/captures/ios/xaml/<key>_<theme>.png

Does NOT build or install.
"""
import json
import os
import shutil
import signal
import struct
import subprocess
import tempfile
import time

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
CLICLICK = os.environ.get("MAUI_PARITY_CLICLICK", "cliclick")

# The window child that IS the device screen. `subrole`, not `description` — description reads "group".
# The window is picked BY DEVICE NAME, never `window 1`: with two simulators open, `window 1` can be the
# other device — taps would land there while `simctl` screenshots keep coming from UDID, and the frame
# banked would be a perfectly sharp picture of an untouched page.
AX_SCREEN = ('tell application "System Events" to tell process "Simulator" to get {position, size} of '
             '(first UI element of (first window whose name starts with "%s") whose subrole is '
             '"iOSContentGroup")')
AX_FRONT = 'tell application "System Events" to get name of first process whose frontmost is true'

# A gesture STARTING within this many device pixels of an edge is an OS gesture, not app input: left =
# back, top = Notification Center, bottom = home/app switcher. 60px is ~20pt on a 3x screen. A step that
# really means to pull the shade sets `edge = true`; everything else is rejected, because the frame it
# would produce (a half-open Notification Center over the page) is plausible enough to survive review.
# What this does NOT cover, and no constant can: the STATUS BAR / Dynamic Island band is far taller
# than 60px (roughly the top ~55-60pt, i.e. ~170px at 3x — device-dependent, not measured here), so a
# point below the margin can still be on system chrome rather than on the page. swipe_refresh's
# `at = [700, 120]` is exactly that: legal here, but ~40pt down, which on this device is inside the
# island. A pull origin has to clear it — one more reason to author fractions off the board still.
EDGE_PX = 60
SWIPE_MOVES = 8            # dm: waypoints; enough for UIScrollView to see a velocity, cheap to emit
STEP_SETTLE = 0.6          # let the UI react before the next step / the shot

# The axis form of a drag/swipe, byte-identical to run_comparison.SWIPE_DIRECTIONS and
# capture_android._SWIPE_DIRECTIONS. One vocabulary, four lanes: `direction = "left"` has to mean the
# same gesture here as it does on the guest agents, or a scenario file stops being portable — which is
# exactly what happened while this module implemented a SMALLER verb set than the files it is fed.
_SWIPE_DIRECTIONS = {"up": (0, -1), "down": (0, 1), "left": (-1, 0), "right": (1, 0)}
# The shared vocabulary's default distance is 300px at the desktop lanes' 1024x800 capture geometry.
# On a 1206x2622 framebuffer that is a ninth of the screen, so — exactly like the Android lane — the
# default is a FRACTION of the axis, which is the same board-scale flick on any device.
_SWIPE_FRACTION = 0.25
# A touch screen has no pointer, so there is nothing to park and nothing to leave hovering. Reported,
# never substituted: a tap is a DIFFERENT gesture and would manufacture a pressed/activated state a
# hover never causes — a fake reaction is worse than a missing one.
_HOVER_SKIP = ("hover: iOS is a touch screen with no pointer to park — and a tap is a different "
               "gesture, not a substitute")


def _osa(script: str) -> str:
    r = subprocess.run(["osascript", "-e", script], capture_output=True, text=True)
    return r.stdout.strip()


def _name_from_devices(devices: dict, udid: str) -> str:
    """The device's name out of `simctl list devices -j` (a runtime -> [device] map)."""
    for runtime in devices.get("devices", {}).values():
        for dev in runtime:
            if dev.get("udid") == udid:
                return dev["name"]
    raise RuntimeError(f"simulator {udid} is not in `simctl list devices`")


_NAMES: dict[str, str] = {}
_SIZES: dict[str, tuple[int, int]] = {}


def device_name(udid: str = UDID) -> str:
    if udid not in _NAMES:
        out = subprocess.run(["xcrun", "simctl", "list", "devices", "-j"],
                             capture_output=True, text=True).stdout
        _NAMES[udid] = _name_from_devices(json.loads(out or "{}"), udid)
    return _NAMES[udid]


def screen_rect(udid: str = UDID) -> tuple[float, float, float, float]:
    """The device screen's rect in host screen points, from the Simulator's AX tree. Raises if absent."""
    out = _osa(AX_SCREEN % device_name(udid))
    try:
        x, y, w, h = (float(v) for v in out.split(", "))
    except ValueError:
        raise RuntimeError(
            f"no Simulator window for {device_name(udid)!r} to drive: the AX 'iOSContentGroup' element "
            "was not found. Open the Simulator window (recapture.py --visible yes) and grant this "
            "process Accessibility permission (System Settings > Privacy & Security > Accessibility). "
            f"osascript said: {out!r}"
        ) from None
    return x, y, w, h


def device_size(udid: str = UDID) -> tuple[int, int]:
    """The framebuffer size in pixels — the unit scenario coordinates are authored in.

    Cached: it cannot change during a run, and the uncached form would shell a `simctl io screenshot`
    at every driven page — including from inside capture_gif's live `recordVideo`.
    """
    if udid not in _SIZES:
        stage = os.path.join(tempfile.gettempdir(), "parity_ios_calibrate.png")
        if not _screenshot(stage, udid):
            raise RuntimeError("could not screenshot the device to calibrate interaction coordinates")
        with open(stage, "rb") as f:
            _SIZES[udid] = struct.unpack(">II", f.read(24)[16:24])   # PNG IHDR
        os.remove(stage)
    return _SIZES[udid]


def _scale(v: float, span: int) -> int:
    """One axis: |v| <= 1 is a FRACTION of `span`, anything larger is already device pixels."""
    return round(v * span) if abs(v) <= 1.0 else round(v)


def to_pixels(at, shot: tuple[int, int]) -> tuple[int, int]:
    """Resolve a scenario `at`/`to` pair to DEVICE PIXELS. See the COORDINATE SPACE note in plan().

    Same rule and same rejection as capture_android.to_pixels — deliberately, because both lanes read
    the SAME scenario files and a second contract would mean a coordinate that lands in two places."""
    if (not isinstance(at, (list, tuple)) or len(at) != 2
            or not all(isinstance(v, (int, float)) for v in at)):
        # Every coordinate in the module funnels through here, so this is the one place that has to
        # turn a malformed/missing `at` into the ValueError the callers are documented to raise — a
        # bare KeyError/TypeError out of a step reads as a crash, not as a scenario authoring error.
        raise ValueError(f"at/to must be [x, y] (fractions of the screen, or device pixels), got {at!r}")
    x, y = float(at[0]), float(at[1])
    frac = [abs(v) <= 1.0 for v in (x, y)]
    if any(frac) and not all(frac):
        raise ValueError(f"mixed coordinate {list(at)!r}: both values must be fractions (<=1.0) or "
                         f"both device pixels — a mixed pair would scale one axis and not the other")
    return _scale(x, shot[0]), _scale(y, shot[1])


def plan(steps: list[dict], rect: tuple[float, float, float, float],
         shot: tuple[int, int]) -> list[tuple[list[str], float]]:
    """Translate step dicts into (cliclick argv-tail, sleep-after) pairs. Pure — the self-check drives it.

    A step is the same dict shape the other lanes take:
        {"name": "initial", "settle": 0.25}                         no action -> hold, then shoot
        {"action": "click"|"tap", "at": [x, y]}                     tap
        {"action": "swipe"|"drag", "at": [x, y], "to": [x, y], "duration": 0.35, "steps": 20}
        {"action": "swipe"|"drag", "at": [x, y], "direction": "left", "distance": 600}
        {"action": "scroll", "at": [x, y], "dy": -400}              the VM lanes' wheel step, as a drag
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
        1206x2622 framebuffer, on a 1080x2340 emulator and in a 1024x800 desktop window, i.e. the only
        one a single scenario file can use to drive every lane. Mixed pairs are a hard error.
      * Anything larger is ABSOLUTE DEVICE PIXELS as seen in the captured PNG — the coordinates you
        read straight off the board image, not points and not host-screen pixels.
      Fractions resolve against the FRAMEBUFFER, then map through `rect` like any other pixel. That is
      the same host point as scaling by `rect` directly (the mapping is linear: rx + f*px_w*sx ==
      rx + f*rw), and it keeps the EDGE_PX guard and the end clamp in the units they were measured in.
    """
    rx, ry, rw, rh = rect
    px_w, px_h = shot
    sx, sy = rw / px_w, rh / px_h
    if abs(sx - sy) / sx > 0.01:
        # The screen rect and the framebuffer disagree on aspect: a rotated device, the wrong window, or
        # a Simulator layout this mapping does not understand. Every tap would be off; refuse to guess.
        raise RuntimeError(f"screen rect {rect} does not match framebuffer {shot} (scale {sx:.4f} vs {sy:.4f})")

    def start_point(at, edge: bool) -> tuple[int, int]:
        """A gesture's FIRST point, in device pixels: strict. An off-screen or edge start is a bug,
        never a clamp — it decides what the gesture touches."""
        x, y = to_pixels(at, shot)
        margin = 0 if edge else EDGE_PX
        if not (margin <= x <= px_w - margin and margin <= y <= px_h - margin):
            raise ValueError(f"step point {list(at)} -> ({x},{y}) is outside the drivable area of a "
                             f"{px_w}x{px_h} screen (edge margin {margin}px)")
        return x, y

    def end_point(x, y) -> tuple[int, int]:
        """A drag's END, in device pixels: CLAMPED, because a finger cannot leave the glass. A `dy`
        that overshoots the screen is a normal way to say "scroll as far as this drag goes", not an
        authoring error."""
        return min(max(round(x), 0), px_w), min(max(round(y), 0), px_h)

    def host(x, y) -> tuple[float, float]:
        """Device pixel -> host screen point, through the AX-measured screen rect."""
        return rx + x * sx, ry + y * sy

    def drag(p0, p1, seconds, moves) -> list[str]:
        (x0, y0), (x1, y1) = host(*p0), host(*p1)
        ms = max(1, int(seconds * 1000 / moves))
        cmds = [f"m:{x0:.0f},{y0:.0f}", f"dd:{x0:.0f},{y0:.0f}"]
        for i in range(1, moves + 1):
            cmds += [f"w:{ms}", f"dm:{x0 + (x1 - x0) * i / moves:.0f},"
                                f"{y0 + (y1 - y0) * i / moves:.0f}"]
        return cmds + [f"du:{x1:.0f},{y1:.0f}"]

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
            x, y = host(*start_point(step.get("at"), bool(step.get("edge"))))
            out.append(([f"c:{x:.0f},{y:.0f}"], hold))
        elif action in ("swipe", "scroll", "drag"):
            x0, y0 = start_point(step.get("at"), bool(step.get("edge")))
            if action == "scroll":
                # `scroll` is the VM lanes' wheel step; on glass the same intent IS a drag by (dx, dy)
                # — the finger moves the way the wheel moves the content, so the sign convention
                # carries over. dx/dy scale per axis by the same |v| <= 1 rule as a coordinate pair.
                if "dy" not in step:
                    raise ValueError(f"step {step.get('name', action)!r}: scroll needs dy (device "
                                     f"pixels, or a fraction of the height)")
                x1, y1 = x0 + _scale(float(step.get("dx", 0)), px_w), y0 + _scale(float(step["dy"]), px_h)
            elif "to" in step:
                x1, y1 = to_pixels(step["to"], shot)
            else:
                # The axis form. Missing/unknown `direction` used to be a bare KeyError on step["to"];
                # this module's contract is ValueError for a malformed step, and the callers rely on it.
                direction = str(step.get("direction", "")).lower()
                if direction not in _SWIPE_DIRECTIONS:
                    raise ValueError(f"step {step.get('name', action)!r}: needs to = [x, y] or "
                                     f"direction = one of {sorted(_SWIPE_DIRECTIONS)} "
                                     f"(got {step.get('direction')!r})")
                dx, dy = _SWIPE_DIRECTIONS[direction]
                span = px_w if dx else px_h
                dist = _scale(float(step["distance"]), span) if "distance" in step \
                    else round(_SWIPE_FRACTION * span)
                x1, y1 = x0 + dx * dist, y0 + dy * dist
            x1, y1 = end_point(x1, y1)
            if (x1, y1) == (x0, y0):
                # Press-hold-release at ONE point is a click. cliclick would run it, report success, and
                # the frame would come back identical — the silent no-op this whole section exists to
                # prevent, wearing a different hat. Every sibling lane raises here too.
                raise ValueError(f"step {step.get('name', action)!r}: zero-length {action} at "
                                 f"({x0},{y0}) is a click, not a pan (clamped to the {px_w}x{px_h} "
                                 f"screen?)")
            moves = max(2, int(step.get("steps", SWIPE_MOVES)))   # <2 presses and releases at one point
            out.append((drag((x0, y0), (x1, y1), float(step.get("duration", 0.35)), moves), hold))
        elif action == "type":
            # Goes to the FOCUSED field, so a `type` step almost always follows a `click` on it. iOS
            # autocapitalization applies to the keystrokes exactly as it would to a human: "abc" lands
            # as "Abc" in a default Entry. Author the expected text, or set the field's Keyboard.
            # Measured: the text also lands MARKED (autocorrect-highlighted) and uncommitted — the
            # `entry` page showed "Abc" highlighted with its LENGTH readout still 0. A scenario that
            # needs the committed value has to dismiss the candidate; the tool for that is cliclick's
            # `kp:return`, which is deliberately NOT wired here until a page actually needs it.
            if "text" not in step:
                raise ValueError(f"step {step.get('name', action)!r}: type needs text = \"…\"")
            out.append(([f"t:{step['text']}"], hold))
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
    # DISABLED BY DEFAULT — this path drives the HOST's pointer, and that is not an acceptable cost.
    # cliclick posts real mouse events onto the Simulator window and osascript activates it, so a
    # capture run seizes the cursor and the foreground app for its whole duration: the machine cannot
    # be used while the board runs, and any stray user click lands in the middle of a gesture and
    # corrupts the frame. Every other lane already avoids this — Android injects through `adb shell
    # input` (no pointer at all), and the macOS/Windows guests run their agent over SSH so the cursor
    # that moves is the VM's, not the operator's. iOS must reach the same bar before it is re-enabled.
    #
    # The two acceptable replacements, in preference order:
    #   1. idb — `brew install idb-companion && pipx install fb-idb`, then `idb ui tap/swipe/text`.
    #      HID injection straight into the simulator: app-agnostic (all three columns get the same
    #      gesture, which is what keeps a scenario from manufacturing a red), device-point coordinates
    #      so no window geometry to resolve, and no cursor or focus involvement whatsoever.
    #   2. DevFlow on both frameworks — Microsoft's agent + CLI for maui_xaml, the port's in-app HTTP
    #      agent for cpp/cpp_xaml. In-process, so also cursor-free, but it needs the port's /tap
    #      widened past its i_button dynamic_cast plus new /swipe, /scroll and /text routes, and the
    #      MauiReference app does not host an agent yet.
    if os.environ.get("MAUI_PARITY_IOS_HOST_CURSOR") != "1":
        raise RuntimeError(
            "iOS interaction is disabled: the only implemented path drives the HOST pointer and "
            "steals focus, making the machine unusable for the length of a run. Install idb "
            "(`brew install idb-companion && pipx install fb-idb`) and switch this lane to `idb ui`, "
            "or route it through DevFlow on both frameworks. Set MAUI_PARITY_IOS_HOST_CURSOR=1 only "
            "for a deliberate, attended experiment on a machine you are not using.")
    tool = shutil.which(CLICLICK)
    if tool is None:
        raise RuntimeError(f"{CLICLICK!r} not found — `brew install cliclick` (or set MAUI_PARITY_CLICLICK)")
    commands = plan(steps, screen_rect(udid), device_size(udid))

    _osa('tell application "Simulator" to activate')
    time.sleep(0.6)
    front = _osa(AX_FRONT)
    if front != "Simulator":
        # Every coordinate is a SCREEN coordinate: if some other window is in front, the clicks land in
        # it. Refusing is not just about a bad capture — it is about not poking the user's other apps.
        raise RuntimeError(f"Simulator did not come to the front (frontmost is {front!r}); not injecting")

    # zip, so each emitted command is reported against the STEP that produced it: a step which emits
    # nothing is either an idle marker or a skipped hover, and those must not look alike in the log.
    for step, (cmds, after) in zip(steps, commands, strict=True):
        if cmds:
            # -r puts the pointer back where the user left it; -w is cliclick's inter-event pause.
            r = subprocess.run([tool, "-r", "-w", "20", *cmds], capture_output=True, text=True)
            if r.returncode != 0:
                raise RuntimeError(f"cliclick {cmds} failed: {r.stderr.strip()[:200]}")
        name = step.get("name", step.get("action", "?"))
        print(f"      step {name}: {step_status(step, cmds)}", flush=True)
        time.sleep(after)


def capture_still(app: str, key: str, theme: str, settle: float, udid: str = UDID,
                  steps: list[dict] | None = None) -> str | None:
    """Launch + settle + screenshot. Returns the written path, or None if the frame was DROPPED."""
    out = out_path(app, key, theme, "png")
    os.makedirs(os.path.dirname(out), exist_ok=True)
    stage = os.path.join(tempfile.gettempdir(), f"parity_{app}_{key}_{theme}.png")
    launch(app, key, udid)
    time.sleep(settle)
    run_steps(steps, udid)
    if not _screenshot(stage, udid):
        return None
    shutil.copyfile(stage, out)
    os.remove(stage)
    if not is_splash(out):
        return out
    for extra in (4.0, 8.0, 16.0):
        launch(app, key, udid)
        time.sleep(settle + extra)
        run_steps(steps, udid)   # a relaunch discards the reacted state — re-drive, or the retry frame is idle
        if not _screenshot(stage, udid):
            continue
        shutil.copyfile(stage, out)
        os.remove(stage)
        if not is_splash(out):
            return out
    os.remove(out)   # still a splash — drop it rather than bank a known-bad frame
    return None


def capture_gif(app: str, key: str, theme: str, settle: float, record_secs: float = 4.0,
                udid: str = UDID, steps: list[dict] | None = None) -> str | None:
    """Record a short mp4 and convert it to a paletted GIF — for pages a single still cannot represent.

    The GIF is not a nicety: comparison_paths.find_capture() and build_comparison_json.py both prefer
    `.gif` over `.png`, so refreshing an animated page's PNG alone leaves the board rendering the
    PREVIOUS run's GIF behind a green log line.

    `steps` run INSIDE the recording window — a page that only moves when poked has to be poked while
    the camera is rolling, or the GIF is `record_secs` of a still page (which is exactly how 13 pages
    ended up with 12 byte-identical frames and no GIF at all).
    """
    gif = out_path(app, key, theme, "gif")
    os.makedirs(os.path.dirname(gif), exist_ok=True)
    gifmod.drop_stale(gif)
    mp4 = os.path.join(tempfile.gettempdir(), f"parity_{app}_{key}_{theme}.mp4")
    launch(app, key, udid)
    time.sleep(settle)
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
    ok = gifmod.video_to_gif(mp4, gif)
    if os.path.exists(mp4):
        os.remove(mp4)
    return gif if ok else None


if __name__ == "__main__":
    # `python3 lib/capture_ios.py` — the coordinate mapping, checked against MEASURED values so a
    # regression is caught without a device. RECT/SHOT are the real numbers read off this machine
    # (Xcode 26, iPhone17-265, window scale ~0.90), and (600,558) is the `button` page's "Clicked"
    # button, which was verified to bump the C++ gallery's readout from Taps: 0 to Taps: 1 when
    # clicked at screen (1759,306) — the value asserted below.
    RECT, SHOT = (1578.0, 138.0, 363.0, 789.0), (1206, 2622)

    assert plan([{"action": "click", "at": [600, 558]}], RECT, SHOT) == [(["c:1759,306"], STEP_SETTLE)]
    assert plan([{"name": "initial"}], RECT, SHOT) == [([], 0.0)]              # no action -> settle only
    assert plan([{"name": "gif01", "settle": 0.25}], RECT, SHOT) == [([], 0.25)]
    assert plan([{"action": "type", "text": "abc"}], RECT, SHOT) == [(["t:abc"], STEP_SETTLE)]
    assert plan([{"action": "wait", "duration": 2.0}], RECT, SHOT) == [([], 2.0)]

    swipe, = plan([{"action": "swipe", "at": [600, 1900], "to": [600, 800], "duration": 0.4}], RECT, SHOT)
    assert swipe[0][:2] == ["m:1759,710", "dd:1759,710"], swipe
    assert swipe[0][-1] == "du:1759,379", swipe
    assert swipe[0].count("w:50") == SWIPE_MOVES, swipe                        # 0.4s / 8 moves = 50ms

    # `scroll` is a relative drag, and its END clamps to the glass instead of failing.
    assert plan([{"action": "scroll", "at": [600, 1900], "dy": -1100, "duration": 0.4}],
                RECT, SHOT)[0][0] == swipe[0]
    assert plan([{"action": "scroll", "at": [600, 1900], "dy": -9999}], RECT, SHOT)[0][0][-1] == "du:1759,138"

    # ---- the shared vocabulary, which this module used to implement only a subset of.
    # `drag` IS `swipe` on glass: same argv, or scenarios/swipe_refresh.toml cannot run here at all.
    assert plan([{"action": "drag", "at": [600, 1900], "to": [600, 800], "duration": 0.4}],
                RECT, SHOT)[0][0] == swipe[0]
    # The direction/distance form (scenarios/carousel_page.toml), and its 0.25-of-the-axis default.
    assert plan([{"action": "swipe", "at": [600, 1900], "direction": "up", "distance": 1100,
                  "duration": 0.4}], RECT, SHOT)[0][0] == swipe[0]
    left, = plan([{"action": "swipe", "at": [600, 1900], "direction": "left"}], RECT, SHOT)
    assert left[0][-1] == "du:1668,710", left     # default 0.25 * 1206 = 302px left of x=600
    # `steps` is the desktop agents' move-event count, honoured (a CarouselView pages on the moves in
    # between) and floored at 2 — one move would press and release at a single point.
    assert sum(c.startswith("dm:") for c in
               plan([{"action": "drag", "at": [600, 1900], "to": [600, 800], "steps": 20}],
                    RECT, SHOT)[0][0]) == 20
    assert sum(c.startswith("dm:") for c in
               plan([{"action": "drag", "at": [600, 1900], "to": [600, 800], "steps": 1}],
                    RECT, SHOT)[0][0]) == 2
    # hover: emitted as nothing at all, NOT as a tap — and reported, since the log line is the only
    # record that the step existed. A hover silently turned into a tap would fake a pressed state.
    hover = {"action": "hover", "at": [600, 558]}
    assert plan([hover], RECT, SHOT) == [([], STEP_SETTLE)]
    assert step_status(hover, []).startswith("SKIPPED"), step_status(hover, [])
    assert step_status({"name": "initial"}, []) == "idle"
    assert step_status({"action": "click", "at": [600, 558]}, ["c:1759,306"]) == "ok"

    # FRACTIONS are the portable authoring form: a fraction must resolve to the identical gesture as
    # the device pixels it names. This is what lets ONE scenario file drive iOS, Android and both VMs.
    assert to_pixels([0.5, 0.2], SHOT) == (603, 524)
    assert to_pixels([603, 524], SHOT) == (603, 524)      # >1 is already device pixels
    assert to_pixels([1.0, 1.0], SHOT) == SHOT            # the 1.0 boundary is a fraction
    assert plan([{"action": "click", "at": [0.5, 0.2]}], RECT, SHOT) == \
        plan([{"action": "click", "at": [603, 524]}], RECT, SHOT)
    # ...and the scalars scale on their own axis by the same rule: 0.72*2622 = 1888, 0.42*2622 = 1101.
    assert plan([{"action": "scroll", "at": [0.5, 0.72], "dy": -0.42}], RECT, SHOT) == \
        plan([{"action": "scroll", "at": [603, 1888], "dy": -1101}], RECT, SHOT)
    for mixed in ([0.5, 300], [300, 0.5]):                # one axis scaled, one not
        try:
            plan([{"action": "click", "at": mixed}], RECT, SHOT)
            raise AssertionError(f"mixed coordinate {mixed} was accepted")
        except ValueError:
            pass

    # A drag that goes nowhere is a CLICK, and every sibling lane raises on it. Both shapes: authored
    # zero-length, and clamped-to-nothing (which needs `edge` to get a start point on the boundary).
    for nowhere in ({"action": "drag", "at": [600, 1900], "to": [600, 1900]},
                    {"action": "swipe", "at": [600, 0], "edge": True, "direction": "up"},
                    {"action": "scroll", "at": [600, 1900], "dy": 0}):
        try:
            plan([nowhere], RECT, SHOT)
            raise AssertionError(f"zero-length gesture {nowhere!r} was accepted")
        except ValueError:
            pass
    try:
        plan([{"action": "swipe", "at": [600, 1900], "direction": "sideways"}], RECT, SHOT)
        raise AssertionError("expected an unknown-direction rejection")
    except ValueError:
        pass
    try:
        plan([{"action": "swipe", "at": [600, 1900]}], RECT, SHOT)   # no `to`, no `direction`
        raise AssertionError("expected a ValueError, not a bare KeyError")
    except ValueError:
        pass
    for shapeless in ({"action": "click"},                                  # no `at` at all
                      {"action": "drag", "at": [600], "to": [600, 800]},    # not a pair
                      {"action": "scroll", "at": [600, 1900]},              # no dy
                      {"action": "type"}):                                  # no text
        try:
            plan([shapeless], RECT, SHOT)
            raise AssertionError(f"malformed coordinate {shapeless!r} was accepted")
        except ValueError:
            pass

    for bad, why in [({"action": "click", "at": [600, 10]}, "top edge -> Notification Center"),
                     ({"action": "click", "at": [600, 2620]}, "bottom edge -> home gesture"),
                     ({"action": "click", "at": [20, 900]}, "left edge -> back gesture"),
                     ({"action": "click", "at": [600, 9999]}, "off screen")]:
        try:
            plan([bad], RECT, SHOT)
            raise AssertionError(f"expected a rejection: {why}")
        except ValueError:
            pass
    assert plan([{"action": "click", "at": [600, 10], "edge": True}], RECT, SHOT)   # opt-in still allowed

    fake = {"devices": {"com.apple.CoreSimulator.SimRuntime.iOS-26-5":
                        [{"udid": "OTHER", "name": "iPad"}, {"udid": UDID, "name": "iPhone17-265"}]}}
    assert _name_from_devices(fake, UDID) == "iPhone17-265"      # picks the window to drive, not window 1
    try:
        _name_from_devices(fake, "NOPE")
        raise AssertionError("expected an unknown-udid rejection")
    except RuntimeError:
        pass

    try:
        plan([{"action": "pinch"}], RECT, SHOT)
        raise AssertionError("expected unknown-action rejection")
    except ValueError:
        pass
    try:
        plan([{"action": "click", "at": [1, 1]}], RECT, (2622, 1206))   # landscape shot vs portrait rect
        raise AssertionError("expected an aspect-mismatch rejection")
    except RuntimeError:
        pass

    # EVERY checked-in scenario, replayed through plan() — the real files, not fixtures. These are read
    # by four lanes, and this one used to implement a strictly SMALLER verb set than the vocabulary they
    # are authored in: `drag` raised "unknown scenario action" (swipe_refresh) and the direction form
    # raised a bare KeyError (carousel_page), so two pages could never be driven on iOS at all. A
    # scenario that the shared vocabulary accepts must never again be un-runnable here without this
    # failing. NOTE what it does NOT prove: the older files carry Mac-calibrated absolute pixels that
    # happen to fit a 1206x2622 framebuffer, so they plan a valid gesture at the WRONG pixel — only a
    # fraction is portable, and only a real capture proves a hit.
    import tomllib
    scenarios = os.path.join(CPP, "docs", "comparison", "scenarios")
    broken = []
    for name in sorted(f for f in os.listdir(scenarios) if f.endswith(".toml")):
        with open(os.path.join(scenarios, name), "rb") as fh:
            steps = tomllib.load(fh).get("steps", [])
        try:
            print(f"  {name}: {len(plan(steps, RECT, SHOT))} step(s) planned")
        except Exception as exc:                              # noqa: BLE001 — report all, not the first
            broken.append(f"{name}: {type(exc).__name__}: {exc}")
            print(f"  {name}: FAILED — {type(exc).__name__}: {exc}")
    assert not broken, broken

    print("capture_ios self-check OK")
