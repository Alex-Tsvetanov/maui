#!/usr/bin/env python3
"""THE parity tool: recapture the board's screenshots and re-measure it, for any slice you ask for.

    tools/parity/recapture.py                                   # everything, emulators visible
    tools/parity/recapture.py --platforms ios,android
    tools/parity/recapture.py --frameworks cpp --themes dark --examples button,entry
    tools/parity/recapture.py --plan                            # resolve every output path, touch nothing

Slice it with (all of them default to "everything"):
    --platforms   android,ios,macos,windows
    --frameworks  maui_xaml,cpp,cpp_xaml
    --themes      light,dark
    --examples    <key>[,<key>…]   (the 172 keys in lib/page_keys.txt)
    --visible     yes|no           whether the mobile emulators get a window you can watch

Every example is announced BEFORE it is captured and again AFTER, with its elapsed time:

    BEGIN platform=ios framework=cpp theme=light example=button kind=png
    END   platform=ios framework=cpp theme=light example=button kind=png took=6.2s

WHAT RUNS WHERE (one lane at a time, never in parallel — the macOS and Windows VMs each have ONE guest
agent and ONE scratch shot.png, so concurrent runs overwrite each other's frames):

  ios      in-process, via lib/capture_ios.py (simctl).
  android  the three build+install+capture scripts in lib/ (they own the aapt2/d8 APK pipeline).
           `maui_xaml` only DRIVES an already-installed MauiReference APK — see that script's header
           for the one-time `dotnet build` + `adb install`.
  macos    TWO sub-lanes on the macOS VM: Catalyst (columns maui_xaml/cpp/cpp_xaml) and AppKit
           (columns appkit_cpp/appkit_xaml — AppKit has no MAUI column, so --frameworks maui_xaml is
           a no-op there). Both go through docs/comparison/tools/run_comparison.py over SSH.
  windows  the same runner against config/windows.toml; the guest builds its own artifacts.

INTERACTION (every lane): a page with a scenario in docs/comparison/scenarios/ is DRIVEN — tapped,
typed into, scrolled — instead of being photographed at rest. A page with none gets one idle
screenshot, as always (~155 of the 172). Two things differ per lane, and neither is cosmetic:

  WHAT THE BOARD PUBLISHES. On the VM lanes the scenario is copied into a per-lane scratch dir (see
  seed_scenarios), the runner shoots EVERY step, and import_run_captures.py publishes the `initial`
  one — so driving a page adds frames to the run without changing its canonical still. iOS has no
  multi-frame run dir: capture_still drives and THEN shoots, so there the board still IS the driven
  frame. Android is driven in the GIF pass only — its still pass is a shell script with no injection
  hook at all — so its board PNG stays at rest.

  WHICH COORDINATES ARE PORTABLE. A point whose |x| and |y| are BOTH <= 1.0 is a FRACTION of the
  target surface (the presented window on a desktop lane, the display on a phone), scaled at
  execution time by whichever layer knows the real geometry; anything larger is absolute pixels in
  that surface, and therefore only means what it says on the lane it was calibrated for. So an
  absolute scenario is seeded only onto a lane whose window actually contains its points
  (seed_scenarios) and is never sent to a device (device_scenarios) — skipped BY NAME in the log,
  because a tap that lands on the wrong widget is worse than one that never fires. A mixed pair like
  [0.5, 300] would scale one axis and not the other: a hard error everywhere.

ANIMATED PAGES (the keys in ANIMATED) get a still AND a GIF, on every platform — each lane records
the motion the way its capture path allows:
  ios      `simctl io recordVideo` -> ffmpeg (smooth video, --gif-secs long)
  android  a burst of `screencap` stills in its own pass, which also has to turn the device's
           ANIMATION SCALES back on: the still pass pins them to 0 for determinism, and nothing moves
           under that pin. (screenrecord is not used — this emulator returns a 1-frame mp4 with no
           timebase, which ffmpeg turns into zero output frames.)
  macos    a BURST of --gif-frames stills --gif-interval apart, assembled into a slideshow GIF. The
  windows  burst rides along in the same runner pass as the still (run_comparison.py honours a
           per-STEP `settle`), so it costs shots, not a second deploy.
This matters beyond looks — find_capture() and build_comparison_json.py both prefer a `.gif` over a
`.png`, so an animated page refreshed as a PNG alone keeps rendering the PREVIOUS run's GIF. Every
lane therefore DELETES the old GIF before recording: a missing GIF falls back to the fresh still,
which is honest; a stale GIF is a lie.

EVERY LANE ALSO LEAVES A RUN DIRECTORY, and an animated page is therefore scored frame by frame on
every platform (lib/motion_score.py) rather than judged on one resting frame. The VM lanes get theirs
from the E2E runner; the two device lanes write RUN_DIR here, in the same layout and the same stamp
format — see run_unit() for what the frame names mean on a lane whose recording has no steps in it.
This is ADDITIONAL evidence: every board path stays exactly where it was. Run dirs are gitignored,
per-run, and NOTHING prunes them (84 present as of this change) — deleting one destroys the only
frames a score can be recomputed from, so no policy is added here.

AFTERWARDS (unless --no-measure) the board is rebuilt and re-measured for the platforms you captured:
comparison.json -> pixel scores -> artifact sizes -> time-to-first-frame -> README.

EXIT CODE is the number of failed steps, so `echo $?` is a real verdict. A lane that fails does NOT
stop the others — a broken Windows VM should not cost you the Android numbers.
"""
from __future__ import annotations

import argparse
import json
import os
import shutil
import subprocess
import sys
import tempfile
import threading
import time
import tomllib
import traceback
from datetime import datetime
from pathlib import Path

HERE = Path(__file__).resolve().parent
LIB = HERE / "lib"
CPP = HERE.parents[1]                      # port/cpp
PORT = CPP.parent                          # port
REPO = PORT.parent
COMP = CPP / "docs" / "comparison"
CTOOLS = COMP / "tools"
SCENARIOS = COMP / "scenarios"             # the AUTHORED interaction scenarios (button, entry, …)
sys.path.insert(0, str(LIB))

PLATFORMS = ("android", "ios", "macos", "windows")
FRAMEWORKS = ("maui_xaml", "cpp", "cpp_xaml")
THEMES = ("light", "dark")

# user-facing framework -> the capture column each lane writes.
IOS_APP = {"maui_xaml": "maui", "cpp": "cpp", "cpp_xaml": "xaml"}
ANDROID_SCRIPT = {"maui_xaml": ("capture_all_csharp_android.sh", "maui"),
                  "cpp": ("build_android_apphost.sh", "cpp"),
                  "cpp_xaml": ("build_android_apphost_xaml.sh", "xaml")}
# (platform, lane, config, env, {framework: column}, board-platform-dir)
VM_LANES = [
    ("macos", "catalyst", "local.toml", "macos-arm64",
     {"maui_xaml": "maui_xaml", "cpp": "cpp", "cpp_xaml": "cpp_xaml"}, "maccatalyst"),
    ("macos", "appkit", "local.toml", "macos-appkit",
     {"cpp": "appkit_cpp", "cpp_xaml": "appkit_xaml"}, "maccatalyst"),
    ("windows", "windows", "windows.toml", "windows-arm64",
     {"maui_xaml": "maui_xaml", "cpp": "cpp", "cpp_xaml": "cpp_xaml"}, "windows"),
]
# run_comparison column -> the framework DIRECTORY import_run_captures.py copies it into.
COL_TO_DIR = {"maui_xaml": "maui", "cpp": "cpp", "cpp_xaml": "xaml",
              "appkit_cpp": "appkit_cpp", "appkit_xaml": "appkit_xaml"}

# Pages a single still cannot represent (was capture_all.ANIMATED) — recorded as a GIF on every
# platform. The purely-interactive ones (gesture/pan/pointer) still only show the idle state: nothing
# taps them, so their GIF is honest but motionless.
ANIMATED = {
    "activity_indicator", "animation", "carousel_page", "swipe_refresh", "empty_view_load_simulate",
    "swipe_gesture", "swipe_item_position", "gestures", "pan_gesture_events", "pointer_gesture",
    "ios_pan_gesture", "ios_swipe_transition", "ios_blur_effect", "chrome",
}

MAC_VM = os.environ.get("MAC_VM_USER", "testinguser") + "@" + \
    os.environ.get("MAC_VM_HOST", "Testings-Virtual-Machine.local")
IOS_UDID = os.environ.get("MAUI_SIM_UDID", "C4926671-2FA7-428E-B4A4-480692EE742B")
ANDROID_SERIAL = os.environ.get("MAUI_ANDROID_SERIAL", "emulator-5554")
# Seconds of SILENCE that mean a step is wedged rather than slow. Not a runtime budget: the VM
# sweeps legitimately run for hours, and every step here reports each page as it finishes.
IDLE_TIMEOUT = int(os.environ.get("PARITY_IDLE_TIMEOUT", "900"))
# Consecutive (page, column, theme) units that captured NOTHING before the lane is declared
# wedged. 9 = three whole pages across three columns — well past a one-off bad frame.
DROP_STREAK_LIMIT = int(os.environ.get("PARITY_DROP_STREAK", "9"))

FAILED: list[str] = []
LOG_DIR = COMP / "_recapture_logs"
RUN_ID = datetime.now().strftime("%Y-%m-%d-%H%M%S")

# --- the RUN DIRECTORY of the two device lanes: the full-resolution, per-step frames a motion score
# is computed from (lib/motion_score.py). Same stamp format and same layout the E2E runner already
# writes for the VM lanes (tools/run_comparison.py: `%Y-%m-%d-%H_%M_%S`, then
# <run>/<tag>/<platform>/<column>/NNNN.png + NNNN.json), REUSED rather than reinvented: motion_score
# globs `20??-??-??-*` and orders the hits by NAME, so a second convention would not merely look
# different — `_` sorts after every digit, so `2026-08-05-010229` would rank as older than every
# runner dir and "the newest run" would stop meaning what it says.
# ONE dir per invocation, shared by both device lanes (their frames differ by <platform>).
# The name is computed here but the directory is NOT created here: lib/motion_score.py imports this
# module for ANIMATED/burst_frames and records "no mkdir at module level" as why that is free, and
# --plan/--selftest must not leave an empty run dir behind in a tree nothing prunes.
RUN_DIR = COMP / datetime.now().strftime("%Y-%m-%d-%H_%M_%S")
# The keyword each capture module takes its destination through. They differ, and deliberately so:
# lib/capture_ios.py takes the UNIT directory and refuses one whose name is not the column it was
# asked to shoot, while lib/capture_android.py takes the run ROOT and derives <key>/android/<column>/
# from the app it already has. Each lane below speaks its own module's contract — a
# lowest-common-denominator argument invented here would be a third contract to keep in step with two.
IOS_RUN_KW, ANDROID_RUN_KW = "run_unit", "run_dir"


# --------------------------------------------------------------------------- logging
_RUN_LOG = None


def log(msg: str) -> None:
    """Print to the terminal AND append to <RUN_ID>.log.

    The per-example timeline used to exist only on stdout, so closing the terminal lost the entire
    history of a multi-hour run — and nothing outside that terminal could tell whether the run was
    progressing. The file is line-buffered and append-only, so `tail -f` works and a monitor can read
    it without touching the run.
    """
    global _RUN_LOG
    line = f"{datetime.now():%H:%M:%S} | {msg}"
    print(line, flush=True)
    try:
        if _RUN_LOG is None:
            LOG_DIR.mkdir(parents=True, exist_ok=True)
            _RUN_LOG = (LOG_DIR / f"{RUN_ID}.log").open("a", buffering=1)
        _RUN_LOG.write(line + "\n")
    except OSError:
        pass          # a logging failure must never take down a capture run


def begin(platform: str, framework: str, theme: str, example: str, kind: str) -> float:
    log(f"BEGIN platform={platform} framework={framework} theme={theme} example={example} kind={kind}")
    return time.time()


def end(platform: str, framework: str, theme: str, example: str, kind: str, t0: float,
        extra: str = "") -> None:
    log(f"END   platform={platform} framework={framework} theme={theme} example={example} "
        f"kind={kind} took={time.time() - t0:.1f}s{(' ' + extra) if extra else ''}")


def fail(what: str) -> None:
    FAILED.append(what)
    log(f"!! FAILED {what}")


# --------------------------------------------------------------------------- child processes
def run_step(name: str, cmd: list[str], env: dict | None = None,
             timeout: int = IDLE_TIMEOUT, on_line=None) -> int:
    """Run a child, tee its output to a per-step log, hand every line to `on_line`.

    PYTHONUNBUFFERED is forced: Python block-buffers stdout when it is a pipe, which turned the whole
    per-page progress display into a lie once already (6 minutes in, log empty, 364 PNGs on disk).

    `timeout` is an IDLE timeout — seconds with NO output — not a deadline for the whole step. A
    deadline cannot be chosen: the step that motivated one is a 172-page VM sweep whose honest runtime
    is 5-6 hours, and a fixed 3h cap killed both VM lanes mid-board after they had already captured a
    complete light theme. Silence is what actually distinguishes a wedged step (a VM reboot that never
    returns, an agent waiting on a window that will never appear) from a slow one, and every step here
    narrates itself page by page.

    The kill is TERM to the process GROUP first — run_comparison.py has `finally` handlers that stop
    the guest agent and restore the guest's theme, and a leaked agent breaks the NEXT run — escalating
    to KILL only if it is still there 20s later.
    """
    LOG_DIR.mkdir(parents=True, exist_ok=True)
    slug = "".join(c if c.isalnum() else "-" for c in name)[:60]
    step_log = LOG_DIR / f"{RUN_ID}-{slug}.log"
    e = dict(os.environ, PYTHONUNBUFFERED="1")
    e.update(env or {})
    log(f"--- {name}   (log {step_log.name})")
    p = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, env=e, text=True,
                         bufsize=1, start_new_session=True)
    last_output = [time.time()]
    stalled = threading.Event()

    def watchdog():
        while p.poll() is None:
            if time.time() - last_output[0] > timeout:
                stalled.set()
                log(f"      !! no output for {timeout}s — terminating '{name}'")
                for sig, wait in ((15, 20), (9, 0)):
                    try:
                        os.killpg(p.pid, sig)
                    except ProcessLookupError:
                        return
                    if not wait or p.poll() is not None:
                        return
                    time.sleep(wait)
                return
            time.sleep(15)

    watcher = threading.Thread(target=watchdog, daemon=True)
    watcher.start()
    # LINE-buffered: the whole point of a per-step log is that it can be tailed WHILE the step runs.
    # With the default 8KB block buffer the file sits at 0 bytes through a multi-minute deploy phase,
    # which reads exactly like a wedged step — and that is the failure mode this tool exists to expose.
    with step_log.open("w", buffering=1) as fh:
        for line in p.stdout:
            last_output[0] = time.time()
            fh.write(line)
            line = line.rstrip()
            if on_line is not None:
                if on_line(line):
                    # The handler decided the step is producing garbage rather than progress. The
                    # idle watchdog cannot see this: a lane dropping every frame is NOISY, not silent.
                    log(f"      !! aborting '{name}' — the handler reported it is not capturing")
                    try:
                        os.killpg(p.pid, 15)
                    except ProcessLookupError:
                        pass
                    break
            elif "!" in line:
                log(f"      {line[:160]}")   # drops/warnings stay visible on the terminal
    rc = p.wait()
    if rc != 0:
        why = f"STALLED >{timeout}s" if stalled.is_set() else f"rc={rc}"
        fail(f"{name} ({why}, see {step_log})")
    return rc


def marker_reader(platform: str, fw_of_column: dict[str, str], example_kind):
    """Translate a child's `@@PARITY BEGIN/END <key> <col> <theme>` markers into our own log lines.

    The runner and the Android scripts emit these; without them we could only learn an example had
    started once its first frame LANDED, which is not "print before every example".
    """
    state: dict = {"open": None, "t0": 0.0, "dropped_streak": 0, "dropped_here": False}

    def close(status: str = "") -> None:
        if state["open"] is None:
            return
        key, fw, theme, kind = state["open"]
        end(platform, fw, theme, key, kind, state["t0"], status)
        state["open"] = None

    def on_line(line: str) -> bool:
        """Returns True to ask run_step to abort the whole step."""
        if line.startswith("@@PARITY "):
            parts = line.split()
            phase, key, col, theme = parts[1], parts[2], parts[3], parts[4]
            fw = fw_of_column.get(col, col)
            kind = example_kind(key)
            if phase == "BEGIN":
                close("(no END — step failed)")   # a launch failure skips the END marker
                state["t0"] = begin(platform, fw, theme, key, kind)
                state["open"] = (key, fw, theme, kind)
                state["dropped_here"] = False
            else:
                close()
                # A wedged guest still narrates: it announces every page and drops every frame, at
                # full speed, for hours. Consecutive all-dropped units are the signal. Measured: the
                # guest locked its screen 20 minutes in and the next 40 pages x 3 columns all dropped
                # while the run looked healthy.
                state["dropped_streak"] = state["dropped_streak"] + 1 if state["dropped_here"] else 0
                if state["dropped_streak"] >= DROP_STREAK_LIMIT:
                    log(f"      !! {state['dropped_streak']} consecutive units captured NOTHING — "
                        f"the guest is wedged (a locked screen has no window to present)")
                    return True
        elif "DROPPED" in line:
            state["dropped_here"] = True
            log(f"      {line[:160]}")
        elif "!" in line:
            log(f"      {line[:160]}")
        return False

    on_line.close = close   # type: ignore[attr-defined]
    return on_line


# --------------------------------------------------------------------------- selection
def all_examples() -> list[str]:
    return [ln.strip() for ln in (LIB / "page_keys.txt").read_text().splitlines()
            if ln.strip() and not ln.startswith("#")]


def kind_for(platform: str, key: str) -> str:
    """What an example produces. Animated pages get BOTH a still and a GIF on every platform — the
    still because everything that reads PNGs still needs one, the GIF because the board prefers it."""
    return "png+gif" if key in ANIMATED else "png"


def board_path(platform: str, column: str, key: str, theme: str, ext: str) -> Path:
    plat_dir = {"macos": "maccatalyst"}.get(platform, platform)
    return COMP / "captures" / plat_dir / COL_TO_DIR.get(column, column) / f"{key}_{theme}.{ext}"


def plan_rows(platforms, frameworks, themes, examples):
    """Every (platform, framework, theme, example) the run will capture, with its output path."""
    rows = []
    for platform in platforms:
        for fw in frameworks:
            if platform == "ios":
                lanes = [("ios", IOS_APP[fw])]
            elif platform == "android":
                lanes = [("android", ANDROID_SCRIPT[fw][1])]
            else:
                lanes = [(lane, cols[fw]) for p, lane, _c, _e, cols, _d in VM_LANES
                         if p == platform and fw in cols]
            for lane, column in lanes:
                for theme in themes:
                    for key in examples:
                        kind = kind_for(platform, key)
                        # For an animated page the GIF is the path that matters: the board renders it
                        # in preference to the still that is written alongside it.
                        ext = "gif" if kind == "png+gif" else "png"
                        rows.append((platform, lane, fw, theme, key, kind,
                                     board_path(platform, column, key, theme, ext)))
    return rows


# --------------------------------------------------------------------------- devices
def ensure_ios_sim(visible: bool) -> bool:
    booted = subprocess.run(["xcrun", "simctl", "list", "devices", "booted"],
                            capture_output=True, text=True).stdout
    if IOS_UDID not in booted:
        log(f"      booting iOS simulator {IOS_UDID}")
        subprocess.run(["xcrun", "simctl", "boot", IOS_UDID], capture_output=True)
    if visible:
        # `-g` = open in the BACKGROUND: show the window without raising it over the operator's work.
        # Cosmetic ONLY on iOS in both senses — shots come from the device framebuffer via `simctl io`
        # and touches go through idb's companion socket to CoreSimulator, so neither the window nor its
        # focus is in any capture path. A plain `open -a Simulator` took the foreground once per lane,
        # which is the same class of problem as the host-cursor injection this lane just removed: a
        # capture run must never take over the machine it runs on. If it costs nothing to be visible,
        # it must also cost nothing to be ignored.
        subprocess.run(["open", "-g", "-a", "Simulator"], capture_output=True)
    for _ in range(40):   # booted != able to serve screenshots
        r = subprocess.run(["xcrun", "simctl", "io", IOS_UDID, "screenshot", "--type=png",
                            "/tmp/_iosready.png"], capture_output=True)
        if r.returncode == 0:
            Path("/tmp/_iosready.png").unlink(missing_ok=True)
            return True
        time.sleep(3)
    fail("ios: simulator never served a screenshot")
    return False


def android_sdk_root() -> str | None:
    """The first candidate root that actually HAS platform-tools/adb.

    Deliberately not `which adb`, and deliberately not trusting $ANDROID_HOME on its own: this machine
    has an Android-Studio-shaped ~/Library/Android/sdk with no platform-tools in it, and tools that
    took the env var at its word died on a path that does not exist. Same probe order, and the same
    MAUI_ANDROID_SDK_ROOT override, as tools/android-emu-lib.sh — the two must agree or the Python
    lane and the shell capture scripts would drive different SDKs.
    """
    for candidate in (os.environ.get("MAUI_ANDROID_SDK_ROOT"), os.environ.get("ANDROID_HOME"),
                      os.environ.get("ANDROID_SDK_ROOT"),
                      "/opt/homebrew/share/android-commandlinetools",
                      str(Path.home() / "Library" / "Android" / "sdk")):
        if candidate and os.access(os.path.join(candidate, "platform-tools", "adb"), os.X_OK):
            return candidate
    return None


def ensure_android_emulator(visible: bool) -> bool:
    root = android_sdk_root()
    if root is None:
        fail("android: no SDK with platform-tools/adb found "
             "(looked at $MAUI_ANDROID_SDK_ROOT, $ANDROID_HOME, $ANDROID_SDK_ROOT, "
             "/opt/homebrew/share/android-commandlinetools, ~/Library/Android/sdk)")
        return False
    adb = os.path.join(root, "platform-tools", "adb")
    state = subprocess.run([adb, "-s", ANDROID_SERIAL, "get-state"],
                           capture_output=True, text=True).stdout.strip()
    if state == "device":
        if visible:
            ps = subprocess.run(["pgrep", "-fl", "qemu-system"], capture_output=True, text=True).stdout
            if "-no-window" in ps:
                # An already-running headless emulator CANNOT be given a window; say so rather than
                # leaving the user waiting for one that will never appear.
                log(f"      !! the running emulator is HEADLESS (-no-window) — it cannot be watched. "
                    f"To see it: adb -s {ANDROID_SERIAL} emu kill, then re-run.")
        return True
    avd = os.environ.get("MAUI_AVD", "maui-test")
    emulator = os.path.join(root, "emulator", "emulator")
    if not os.access(emulator, os.X_OK):
        fail(f"android: {root} has adb but no emulator binary at {emulator} — "
             f"start the '{avd}' AVD yourself, or install the emulator package in that SDK")
        return False
    args = [emulator, "-avd", avd, "-no-snapshot-save", "-no-boot-anim"]
    if not visible:
        args.append("-no-window")
    log(f"      starting Android emulator '{avd}'{'' if visible else ' (headless)'}")
    LOG_DIR.mkdir(parents=True, exist_ok=True)
    subprocess.Popen(args, stdout=(LOG_DIR / f"{RUN_ID}-emulator.log").open("w"),
                     stderr=subprocess.STDOUT)
    for _ in range(60):
        if subprocess.run([adb, "-s", ANDROID_SERIAL, "get-state"], capture_output=True,
                          text=True).stdout.strip() == "device":
            break
        time.sleep(5)
    for _ in range(60):   # `device` precedes boot completion; capturing before it gives blank frames
        done = subprocess.run([adb, "-s", ANDROID_SERIAL, "shell", "getprop", "sys.boot_completed"],
                              capture_output=True, text=True).stdout.strip()
        if done == "1":
            subprocess.run([adb, "-s", ANDROID_SERIAL, "shell", "input", "keyevent", "KEYCODE_WAKEUP"],
                           capture_output=True)
            return True
        time.sleep(5)
    fail("android: emulator never reported boot_completed")
    return False


def mac_vm_clean() -> None:
    """A leftover app from a killed run holds a window the agent will happily photograph."""
    subprocess.run(["ssh", "-o", "BatchMode=yes", "-o", "ConnectTimeout=10", MAC_VM,
                    "pkill -x gallery; pkill -x gallery_xaml; pkill -f MauiReference; exit 0"],
                   capture_output=True)


def mac_vm_reboot_and_settle() -> None:
    """LOAD-BEARING for Catalyst: without a clean WindowServer the app windows are not AX-enumerable
    and `present` drops nearly every frame (58 of 62, measured). Bounded by hand because the runner's
    own reboot_before_run has hung for 1h34m with 0 frames captured."""
    def boottime() -> str:
        return subprocess.run(["ssh", "-o", "BatchMode=yes", "-o", "ConnectTimeout=5", MAC_VM,
                               "sysctl -n kern.boottime"], capture_output=True, text=True).stdout.strip()

    # WAIT FOR THE BOOT TIME TO CHANGE, not for SSH to answer. `sudo reboot` returns immediately and
    # sshd stays up for several more seconds, so "ssh answered" is true of the DYING session: measured,
    # this reported "VM back after ~25s" a full 90 seconds before the machine actually came up, and
    # everything done next — the keep-awake, the capture — ran against a system on its way down. The
    # caffeinate died with it and the guest locked itself 20 minutes later, exactly as before.
    before = boottime()
    log("      rebooting the macOS VM (bounded wait)")
    subprocess.run(["ssh", "-o", "BatchMode=yes", "-o", "ConnectTimeout=10", MAC_VM, "sudo reboot"],
                   capture_output=True)
    time.sleep(20)
    for i in range(60):
        now = boottime()
        if now and now != before:
            log(f"      VM rebooted after ~{20 + i * 5}s (kern.boottime changed)")
            break
        time.sleep(5)
    else:
        fail(f"macos: kern.boottime never changed from {before!r} — the VM did not reboot")
        return
    for i in range(30):   # sshd is up long before the GUI session; capturing early = blank frames
        who = subprocess.run(["ssh", "-o", "BatchMode=yes", "-o", "ConnectTimeout=5", MAC_VM,
                              "stat -f%Su /dev/console"], capture_output=True, text=True).stdout.strip()
        if who and who != "root":
            log(f"      console session up ({who}) after ~{i * 10}s")
            mac_vm_keep_awake()
            return
        time.sleep(10)


def mac_vm_keep_awake() -> None:
    """Stop the guest locking its screen mid-sweep.

    A LOCKED screen has no AX-enumerable windows, so `present` fails and EVERY frame is dropped —
    while the run keeps going at full speed producing nothing. Measured: the guest locked itself 20
    minutes after boot and the next 40 pages x 3 columns all dropped. `pmset` was already set to never
    sleep; it is the screen LOCK (idle) that does it, and it cannot be undone over SSH — only the
    reboot's autologin clears it, which is exactly why the first 20 minutes worked.

    So: never let it start. idleTime 0 disables the screensaver, and `caffeinate -u` asserts USER
    activity, which keeps the idle timer from ever reaching the lock threshold. The caffeinate is
    bounded (24h) so a crashed run cannot leave the guest pinned awake forever.
    """
    subprocess.run(["ssh", "-o", "BatchMode=yes", "-o", "ConnectTimeout=10", MAC_VM,
                    "defaults -currentHost write com.apple.screensaver idleTime 0; "
                    "pkill -f 'caffeinate -disu' ; "
                    "nohup caffeinate -disu -t 86400 >/dev/null 2>&1 & exit 0"], capture_output=True)
    log("      guest kept awake (screensaver off + caffeinate -disu)")


# --------------------------------------------------------------------------- builds
def build(platform: str, frameworks: list[str]) -> None:
    """Build only what the selected frameworks need. Android builds inside its capture scripts; the
    Windows guest builds its own artifacts; the MAUI reference app is built by hand (see the Android
    script's header) — this covers the C++ galleries."""
    targets = [t for fw, t in (("cpp", "gallery"), ("cpp_xaml", "gallery_xaml")) if fw in frameworks]
    if not targets:
        return
    preset, exdir = {"ios": ("ios", "build-ios"),
                     "catalyst": ("maccatalyst", "build-maccatalyst"),
                     "appkit": ("apple", "build-apple")}[platform]
    run_step(f"{platform}: build framework + {' '.join(targets)}", ["bash", "-c",
             f"cd '{CPP}' && cmake --build --preset {preset} && "
             f"cmake --install build/{preset} --prefix /tmp/maui-prefix-{preset} && "
             f"cmake --build examples/{exdir} --target {' '.join(targets)}"], timeout=3600)


def ios_install(frameworks: list[str]) -> None:
    apps = {"maui_xaml": PORT / "maui-reference/app/bin/Debug/net10.0-ios/iossimulator-arm64/MauiReference.app",
            "cpp": CPP / "examples/build-ios/gallery/gallery.app",
            "cpp_xaml": CPP / "examples/build-ios/gallery_xaml/gallery_xaml.app"}
    for fw in frameworks:
        app = apps[fw]
        if not app.exists():
            fail(f"ios: {fw} app not built at {app}")
            continue
        run_step(f"ios: install {fw}", ["xcrun", "simctl", "install", IOS_UDID, str(app)], timeout=900)


# --------------------------------------------------------------------------- run directory
def run_unit(key: str, plat_dir: str, column: str, root: Path | None = None) -> Path:
    """<run>/<tag>/<plat_dir>/<column>/ — where ONE (page, platform, column) unit's frames go.

    `column` is the RUNNER column (maui_xaml / cpp / cpp_xaml), NOT the capture-directory name
    (maui / cpp / xaml) that IOS_APP and ANDROID_SCRIPT hand out, and which is sitting in a
    conveniently-named local at both call sites. motion_score.find_frames resolves the board's
    framework dir through FW_TO_COL and looks the frames up under the runner column, so passing
    `app` here writes a full run directory nobody ever reads while every animated cell goes on
    reporting "no run directory" — a lane that looks wired and scores nothing. The selftest pins the
    three parts of this path for exactly that reason.

    WHY THE FRAMES OF A DEVICE LANE CAN BE PAIRED AT ALL — the assumption this file is the one that
    guarantees. A VM run's frames are discrete named scenario steps, and motion_score._pair joins the
    columns BY NAME precisely so that a dropped frame cannot re-align the tail of a sequence. A device
    recording has no step boundaries: iOS is one continuous `simctl io recordVideo` mp4, Android a
    `screencap` burst whose scenario runs on a concurrent thread. Naming those frames by capture index
    would be index pairing wearing a name. So both modules name a frame for its NORMALIZED MOMENT —
    its offset into the recording (capture_ios.frame_step) or its nominal sample of a stated schedule
    (capture_android.step_name) — which is a real join key ONLY IF the two columns recorded the same
    page for the same length of time. THEY DO, and this is where that is decided: the loops below hand
    every column of a page ONE `gif_secs` (and one `frame_count`) straight off the one argv, so the
    k-th sample is the same moment of the same animation in each column. A caller that ever recorded
    two columns of one page for different durations would make those names a lie.
    GAP, stated rather than hidden: the sidecar shape (tag/platform/column/theme/step/frame/commit/
    captured_at) carries no duration, so nothing downstream can DERIVE that refusal from a frame. Both
    modules mitigate it in the only place they can — the step name itself carries the schedule
    (`gif01@4s/12f`) or the millisecond offset — so two unequal recordings mostly fail to pair rather
    than pair wrongly, and the residue surfaces as motion_score's unpaired-frame count.
    """
    d = (root or RUN_DIR) / key / plat_dir / column
    d.mkdir(parents=True, exist_ok=True)
    return d


def accepts_run_kw(lane: str, kw: str, *fns) -> bool:
    """Do this lane's capture functions take the run-directory destination? Asked ONCE per lane.

    The parameter lives in lib/capture_ios.py / lib/capture_android.py. If it is absent or renamed,
    this file must not crash on page 1 of 1032 — but it must not shrug either: the run dir would stay
    empty and every animated cell would keep the "NOT motion-scored" label with nothing saying why.
    So it costs one named failed step and the lane captures on, board stills and GIFs unaffected.
    """
    import inspect  # noqa: PLC0415  one-off tolerance probe

    missing = [f"{fn.__module__}.{fn.__name__}" for fn in fns
               if kw not in inspect.signature(fn).parameters]
    if missing:
        fail(f"{lane}: {', '.join(missing)} take no `{kw}=` parameter, so this lane writes NO run "
             f"directory and its animated pages CANNOT be motion-scored (lib/motion_score.py). "
             f"The board stills and GIFs are unaffected.")
        return False
    return True


def ios_run_kw(key: str, column: str, ok: bool) -> dict:
    """The unit destination for one iOS capture call — or nothing, when the module cannot take one.

    Evaluated lazily rather than per lane, so a lane that cannot be scored also never creates the
    empty run directory that would advertise frames it does not hold."""
    return {IOS_RUN_KW: run_unit(key, "ios", column)} if ok else {}


def android_run_kw(ok: bool) -> dict:
    """The run ROOT for one Android capture call. capture_android.write_run_unit builds
    <key>/android/<column>/ from the app it is already given; handing it the unit as well would state
    the same fact twice and let the two disagree.

    NOTE the Android lane does NOT seed its own `initial` frame from here even though its still is
    shot by a shell script with no run-dir hook (lib/build_android_apphost*.sh): write_run_unit copies
    the PUBLISHED still into the unit itself, which is what satisfies motion_score._is_published_run,
    and it does so in the same write as the burst. Adding a second copy here would put a lone
    provenance frame in the unit on the paths where that function bails out — a newer run that
    matches the board still but can pair nothing, shadowing the older complete run underneath it."""
    return {ANDROID_RUN_KW: str(RUN_DIR)} if ok else {}


# --------------------------------------------------------------------------- lanes
def lane_ios(frameworks, themes, examples, visible, skip_build, settle, gif_secs) -> None:
    log("=== LANE ios")
    if not ensure_ios_sim(visible):
        return
    if not skip_build:
        build("ios", frameworks)
        ios_install(frameworks)

    import capture_ios

    # WHAT THIS LANE WILL DRIVE, resolved once (the scenarios cannot change mid-run) so an unusable
    # one is reported by name ONCE rather than at every column x theme.
    scen = device_scenarios("ios", examples)
    # Whether this lane can leave a run directory behind — asked once, before the first page.
    unit_ok = accepts_run_kw("ios", IOS_RUN_KW, capture_ios.capture_still, capture_ios.capture_gif)
    capture_ios.pin()
    unset = object()
    restore = unset      # sentinel, NOT `or`: a device with no appearance set reads back falsy, and
    try:                 # `restore or prev` would then record the run's FIRST theme as the pre-run state
        # THEME OUTERMOST — it is a property of the simulator, not of the launch.
        for theme in themes:
            prev = capture_ios.set_theme(theme)
            if restore is unset:
                restore = prev
            for fw in frameworks:
                app = IOS_APP[fw]
                capture_ios.warmup(app, examples[0], settle)
                for key in examples:
                    kind = kind_for("ios", key)
                    steps = scen.get(key)          # None for the ~155 unscripted pages: touch nothing
                    # PAGES THAT GET MOTION-SCORED — which is the ANIMATED set OR a page with an
                    # authored action step, because pixel_score calls motion_score.score_cell for
                    # both (its `driven_pages()`). An UNSCRIPTED still page still gets no unit: ~1000
                    # PNGs per run duplicating bytes captures/ already holds, in a tree nothing prunes.
                    # This used to claim `kind` alone "cannot drift out of step with what gets scored"
                    # — it did drift, the moment pages were driven: 58 of 62 driven cells were scored
                    # from a single still because their frames were never banked. Both halves of the
                    # scorer's trigger have to appear here, and `steps` is the same
                    # scenario-derived answer `driven_pages()` computes.
                    want_unit = unit_ok and (kind == "png+gif" or bool(steps))
                    t0 = begin("ios", fw, theme, key, kind)
                    try:
                        # `still_first` on everything that is not ANIMATED: shoot AT REST, publish
                        # that, then drive and bank the reacted frame in the unit — so the board keeps
                        # the resting render (it was showing post-click switches and checkboxes) and
                        # the unit holds a real before/after.
                        #
                        # DRIVEN WINS OVER ANIMATED. This used to read `kind != "png+gif"`, so a page
                        # that was BOTH animated and driven drove FIRST and only then shot — its
                        # `initial` frame was already post-gesture, every burst frame after it was too,
                        # and no at-rest frame existed anywhere in the run. motion_score then compared
                        # two identical post-gesture states and reported 0 px, correctly, on a page that
                        # had reacted perfectly. Measured on `gestures` (2026-08-06): the scenario logged
                        # `step tapped: ok` on all three columns and not one frame carried that step.
                        # 89261d905a fixed this ordering for the non-animated path; nothing was both
                        # animated and driven at the time, so this branch kept the old behavior unseen.
                        # `or` rather than a swap: still_first can only turn ON here, never off, so an
                        # undriven page of either kind behaves exactly as before.
                        # that recording. `run_unit` is the ADDITIONAL evidence copy either way.
                        out = capture_ios.capture_still(app, key, theme, settle, steps=steps,
                                                        still_first=bool(steps) or kind != "png+gif",
                                                        **ios_run_kw(key, fw, want_unit))
                        if kind == "png+gif":
                            # Both, deliberately: the GIF is what the board renders, the still keeps a
                            # frame for anything that only reads PNGs. The steps run INSIDE the
                            # recording window — a page that only moves when poked has to be poked
                            # while the camera is running.
                            out = capture_ios.capture_gif(app, key, theme, settle, gif_secs,
                                                          steps=steps,
                                                          **ios_run_kw(key, fw, want_unit)) or out
                    except Exception as exc:      # one bad page must not cost the other 171
                        fail(f"ios/{fw}/{theme}/{key}: {exc}")
                        end("ios", fw, theme, key, kind, t0, "ERROR")
                        continue
                    if out is None:
                        fail(f"ios/{fw}/{theme}/{key}: frame DROPPED (splash or screenshot failure)")
                        end("ios", fw, theme, key, kind, t0, "DROPPED")
                    else:
                        end("ios", fw, theme, key, kind, t0, f"-> {Path(out).relative_to(PORT)}")
    finally:
        if restore is not unset and restore:
            capture_ios.set_theme(restore)
        capture_ios.unpin()
    if "maui_xaml" in frameworks:
        # The reference writes to port/maui-reference/captures/ios/; the BOARD reads
        # captures/ios/maui/. Nothing else copies between those two roots.
        run_step("ios: promote reference captures into the board",
                 [sys.executable, str(LIB / "promote_reference_captures.py"), "--platform", "ios"],
                 timeout=600)


def lane_android(frameworks, themes, examples, visible, gif_secs, gif_frames) -> None:
    log("=== LANE android")
    if not ensure_android_emulator(visible):
        return
    # ANIMATED **or DRIVEN** — the same trigger pixel_score uses, for the same reason. Gating this on
    # ANIMATED alone meant the ~24 pages with an authored scenario were never driven on Android at
    # all: not mis-ordered, simply skipped, so their cells could only ever score a resting frame. That
    # is the fifth instance of one shape — a hard-coded list standing between work and the tool meant
    # to see it (sanitizers gated on CXX never saw .mm; motion scoring gated on ANIMATED never saw
    # driven pages; frame banking gated on the same list never kept their frames; the VM lane seeded
    # an empty scenario dir). Keying on the scenarios themselves is what stops it recurring here.
    drivable = [k for k in examples if k in ANIMATED or (SCENARIOS / f"{k}.toml").is_file()]
    # ONLY THE GIF PASS IS DRIVABLE HERE. The still pass below is build_android_apphost*.sh — a shell
    # pipeline with no injection hook — so a driven page's board PNG stays AT REST on Android. That is
    # the honest outcome and it sidesteps the reacted-still defect iOS had: the still is never driven,
    # so it cannot be published post-action. Resolved once so the log line is not repeated per theme.
    scen = device_scenarios("android (GIF pass only — the still pass is a shell script)", drivable)
    animated = drivable
    for theme in themes:
        for fw in frameworks:
            script, column = ANDROID_SCRIPT[fw]
            reader = marker_reader("android", {column: fw}, lambda k: "png")
            run_step(f"android: {fw} ({theme})",
                     ["bash", str(LIB / script), *examples],
                     env={"MAUI_APPEARANCE": theme}, on_line=reader)
            reader.close("(script exited)")   # type: ignore[attr-defined]
        if animated:
            android_gifs(frameworks, theme, animated, gif_secs, gif_frames, scen)


def android_gifs(frameworks, theme, animated, gif_secs, gif_frames, scen) -> None:
    """screenrecord pass for the animated pages. Separate from the still pass because the shell
    scripts capture exactly one frame per page — and because their exit trap has already put the
    device's night mode back, so this pass has to set it again for the theme it is recording."""
    import capture_android
    import device_state

    unit_ok = accepts_run_kw("android", ANDROID_RUN_KW, capture_android.capture_gif)
    prev = capture_android.set_theme(theme)
    prev_anim = capture_android.animations()   # restore what we found, not a guess at what it was
    # PIN THE STATUS BAR, exactly as the still pass does. Its exit trap has already un-pinned it, so a
    # live clock/battery/wifi ticks between burst frames — and that alone passes the "frames differ"
    # test. Measured: of 24 GIFs the first run produced, only activity_indicator moved in the PAGE
    # (225-262 rows); every other one changed just rows 21-31 at the clock and the wifi icon. A GIF of
    # a ticking clock is not an animation, it is a still with a bug.
    device_state.pin_android(capture_android.SERIAL)
    capture_android.set_animations(True)       # pin_android zeroes them; the recording needs them on
    try:
        for fw in frameworks:
            app = ANDROID_SCRIPT[fw][1]
            for key in animated:
                t0 = begin("android", fw, theme, key, "gif")
                try:
                    # steps run on a background thread INSIDE the burst (see capture_android
                    # .capture_gif), so the frames straddle the gesture instead of following it.
                    out = capture_android.capture_gif(app, key, theme, secs=gif_secs,
                                                      frame_count=gif_frames, steps=scen.get(key),
                                                      **android_run_kw(unit_ok))
                except Exception as exc:
                    fail(f"android/{fw}/{theme}/{key}: gif: {exc}")
                    end("android", fw, theme, key, "gif", t0, "ERROR")
                    continue
                if out is None:
                    # NOT a failed step: the still from the main pass stands, and drop_stale() has
                    # already removed any older GIF, so the board shows a fresh PNG rather than a lie.
                    log(f"      no recording for {key} ({fw}/{theme}) — still stands, GIF skipped")
                    end("android", fw, theme, key, "gif", t0, "SKIPPED")
                else:
                    end("android", fw, theme, key, "gif", t0, f"-> {Path(out).relative_to(COMP)}")
    finally:
        device_state.clear_android(capture_android.SERIAL)
        capture_android.set_animations(prev_anim)   # after clear_android, which forces them back to 1
        capture_android.set_theme(prev)


def _points(step: dict) -> dict[str, list[float]]:
    """The step's `at`/`to` values that are SHAPED like coordinates, KEYED — `at` is the gesture's
    start and `to` its end, and the two are not interchangeable to anything downstream.

    Anything malformed is left alone on purpose: run_comparison.step_point and capture_android
    .input_argv already raise on it, loudly, at the point of use, and scenarios/_selftest.py catches
    it at authoring time. This module only has to classify what is there.

    PER-LANE KEYS COUNT TOO. `at_<env>` / `to_<env>` (run_comparison.for_lane) are real coordinates on
    the lane they name, so a file whose only ABSOLUTE pair sits in one of them must classify as
    absolute — otherwise coordinate_space calls the file fractional, seed_scenarios waves it onto every
    lane including the ones that never pin a window, and out_of_rect never sees the pair it exists to
    bounds-check. Keyed by the ORIGINAL key name so callers reading `.get("at")` are unaffected.
    """
    out = {}
    for key, pt in step.items():
        if key not in ("at", "to") and not key.startswith(("at_", "to_")):
            continue
        if (isinstance(pt, (list, tuple)) and len(pt) == 2
                and all(isinstance(v, (int, float)) for v in pt)):
            out[key] = [float(pt[0]), float(pt[1])]
    return out


def coordinate_space(steps: list[dict]) -> str:
    """"fraction" | "absolute" | "none" — which surface a scenario's points are authored in.

    THE PORTABLE AUTHORING CONTRACT, of which capture_android.to_pixels is the reference
    implementation: a pair whose |x| and |y| are BOTH <= 1.0 is a fraction of the target surface,
    anything larger is absolute pixels in it, and a MIXED pair is an error rather than a guess —
    scaling one axis and not the other produces a point nobody authored.

    A scenario is "fraction" only if EVERY pair is one. A single absolute pair pins the whole file to
    the geometry it was calibrated for, so the conservative classification is the correct one.
    """
    space = "none"
    for step in steps:
        for pt in _points(step).values():        # start or end: both have to mean the same thing
            frac = [abs(v) <= 1.0 for v in pt]
            if any(frac) and not all(frac):
                raise ValueError(f"step {step.get('name', '?')!r}: mixed coordinate {pt!r} — both "
                                 f"values must be fractions (<=1.0) or both pixels, never one of each")
            space = "fraction" if all(frac) and space in ("none", "fraction") else "absolute"
    return space


def skip_scenario(lane: str, key: str, why: str) -> None:
    """One greppable shape for every un-replayable scenario. Silence here is the whole bug class: a
    scenario that does not run must cost a log line, not a frame that looks like it reacted."""
    log(f"      !! SCENARIO SKIPPED {lane}/{key}: {why}")


def read_steps(key: str) -> list[dict]:
    """A page's authored steps, or [] if it has no scenario. Raises on a file the runner would die on."""
    f = SCENARIOS / f"{key}.toml"
    return tomllib.loads(f.read_text()).get("steps", []) if f.is_file() else []


def lane_geometry(cfg: str, envname: str) -> tuple[bool, dict]:
    """(present, rect) for a VM lane, out of the SAME config file run_comparison.py will be handed.

    Read rather than hard-coded per lane so a geometry change lands in one place. The defaults mirror
    run_comparison.Env.__init__; the selftest pins both presented lanes' resolved rects to the numbers
    scenarios/_selftest.py measured off real run sidecars, so a renamed config key or a drift in these
    defaults fails there rather than by taps landing somewhere else on the guest.
    """
    cap = tomllib.loads((COMP / "config" / cfg).read_text())["environments"][envname].get("capture", {})
    return cap.get("present", True), {"x": 128, "y": 30, "w": 1024, "h": 800, **cap.get("geometry", {})}


def out_of_rect(steps: list[dict], rect: dict, envname: str | None = None) -> str | None:
    """The first gesture START that lands outside a lane's presented window, phrased as a skip reason.

    The `at` point only, deliberately — and BY NAME, never "the first coordinate in the step": `at` is
    the start, `to` is the end, and a step carrying only `to` would otherwise be validated as if its
    end were its start. The start decides what the gesture touches, while an end that runs off the
    edge is simply a SHORTER drag, which is what a real finger does — the split both capture_ios.plan
    (strict start, clamped end) and capture_android.input_argv already encode. The strict both-ends
    check belongs to scenarios/_selftest.py, the AUTHORING gate; this is the execution gate and can
    afford to be laxer.

    Half-open on the far edge, matching capture_android.input_argv's `0 <= x < w`: a window w px wide
    ends at x = w-1, so a point AT w is already the first pixel of whatever is next to it.
    """
    x0, y0 = rect["x"], rect["y"]
    x1, y1 = x0 + rect["w"], y0 + rect["h"]
    for step in steps:
        if not step.get("action"):
            continue
        pts = _points(step)
        # The start THIS lane will actually use: its own `at_<env>` when the step carries one, else the
        # portable `at`. Checking the portable one regardless would bounds-check a coordinate the lane
        # is never going to send, and — worse — let the override it DOES send through unchecked.
        start = pts.get(f"at_{envname}") if envname else None
        if start is None:
            start = pts.get("at")
        if start and not (x0 <= start[0] < x1 and y0 <= start[1] < y1):
            which = f"at_{envname}" if envname and f"at_{envname}" in pts else "at"
            return (f"step {step.get('name', '?')!r} starts at {start} ({which}), outside this lane's "
                    f"window [{x0},{y0} {rect['w']}x{rect['h']}] — those coordinates belong to "
                    f"another lane")
    return None


def seed_scenarios(scen_dir: Path, lane: str, present: bool, rect: dict,
                   envname: str | None = None) -> list[str]:
    """Start a lane's scenario dir from the AUTHORED scenarios in docs/comparison/scenarios/.

    This dir used to be created EMPTY, and `--scenarios <dir>` is the runner's WHOLE scenario source
    (its own default is exactly that authored dir), so handing it an empty one silently disabled every
    authored scenario on the macOS and Windows lanes: `button` was never tapped, `entry` never typed
    into, `scroll_view` never scrolled, and web_view/hybrid_web_view lost the 5s settle that stops
    MAUI's own column racing WebView2's init. The pages were photographed at rest and the reacted state
    the scenario exists to capture was never reached.

    PER-LANE GEOMETRY GATE. One authored dir feeds every VM lane, but the lanes do not share a rect:
    Catalyst presents at [128,30 1024x800], Windows at [244,0 1024x800], and macos-appkit does not
    present AT ALL — its 480x752 window sits wherever the WM put it (System Events sees no windows
    there, so it is captured by Quartz window id instead). An absolute point calibrated for the
    Catalyst window is a real click somewhere else entirely on AppKit, which is worse than no click:
    it fabricates a reacted frame nobody can explain. So an absolute scenario is seeded only onto a
    lane that pins a window CONTAINING its start points; a fractional one is valid everywhere and is
    always seeded, as is a scenario with no coordinates at all (web_view's settle-only file, whose
    whole job is the 5s that stops the WebView2 race).

    Only `*.toml` is copied, so a README sitting next to them cannot become a scenario.

    NO THEME REGRESSION, despite what button.toml's `themes = ["light"]` looks like: run_comparison.py's
    run_env reads `themes_override or scenario["themes"]`, and every lane here passes `--themes`, which
    IS that override. The authored `themes` only ever applies to a bare runner invocation.
    """
    scen_dir.mkdir(parents=True, exist_ok=True)
    names = []
    for f in sorted(SCENARIOS.glob("*.toml")):
        try:
            steps = tomllib.loads(f.read_text()).get("steps", [])
            space = coordinate_space(steps)
        except (tomllib.TOMLDecodeError, ValueError) as exc:
            # A mixed pair (or a file the runner cannot even parse) is an AUTHORING error, not a lane
            # mismatch: it is wrong on every lane, so it costs a failed step rather than a skip line.
            fail(f"scenario {f.name}: {exc}")
            continue
        why = None
        if space == "absolute":
            why = (out_of_rect(steps, rect, envname) if present else
                   "absolute screen coordinates, but this lane never PINS its window (present=false) "
                   "— there is no fixed rect they could be relative to; re-author as 0..1 fractions")
        if why:
            skip_scenario(lane, f.stem, why)
            continue
        shutil.copy2(f, scen_dir / f.name)
        names.append(f.stem)
    return names


def device_scenarios(lane: str, examples: list[str]) -> dict[str, list[dict]]:
    """{page: steps} for the pages a DEVICE lane (ios/android) can actually be driven through.

    Same authored dir seed_scenarios reads — there is one scenario source, not a desktop one and a
    mobile one. Two kinds of page are dropped rather than replayed:

      * a settle-only scenario (web_view, hybrid_web_view): there is nothing to inject, so the page
        keeps its single idle screenshot exactly as the ~155 unscripted ones do. Not a skip line —
        nothing failed.
      * an ABSOLUTE scenario: those coordinates are calibrated for a 1024x800 desktop window, and a
        phone display is neither that size nor that shape. capture_android would reject most of them
        as off-display, but not all — swipe_refresh's y=120 IS on a 1080x2340 screen, inside the
        status bar, where a downward drag pulls the notification shade instead of the page. So they
        are refused here, by name, rather than replayed into whatever happens to be under them.

    The fractional form is what makes a scenario portable, and it is the one the device lanes resolve
    against real device geometry (capture_android.to_pixels, capture_ios.plan).
    """
    out: dict[str, list[dict]] = {}
    skipped: list[str] = []
    for key in examples:
        try:
            steps = read_steps(key)
            space = coordinate_space(steps)
        except (tomllib.TOMLDecodeError, ValueError) as exc:
            fail(f"scenario {key}.toml: {exc}")     # hard everywhere, exactly as on the VM lanes
            continue
        if not any(s.get("action") for s in steps):
            continue                                 # nothing to inject: one idle screenshot, as ever
        if space == "absolute":
            skip_scenario(lane, key, "absolute desktop coordinates cannot be replayed on a device "
                                     "with different geometry; re-author as 0..1 fractions")
            skipped.append(key)
            continue
        out[key] = steps
    log(f"      scenarios {lane}: driven={sorted(out) or '(none)'} skipped={skipped or '(none)'}")
    return out


def write_gif_scenarios(scen_dir: Path, examples, themes, frames: int, interval: float) -> list[str]:
    """Give every animated page a burst of no-action frames, COMPOSED onto whatever it already has.

    With no authored scenario: the `initial` still, then the burst. With one (seeded by
    seed_scenarios): its steps are kept and the burst is APPENDED AFTER THE LAST of them — the last
    step is the one that set the page in motion, so the frames that record that motion have to follow
    it, not replace it. Replacing was the old behavior and it silently disarmed any page that was both
    animated and driven.

    The compose is a TEXT append, not a parse-and-rewrite: `[[steps]]` is an array-of-tables, so more
    entries at the end of the file simply extend it. That keeps the authored `themes`, `settle`,
    `timeout_seconds`, the calibration comments, and any key a later scenario adds — none of which a
    re-serializer would know to carry, and stdlib has no TOML *writer* to do it with.

    The burst rides along in the SAME runner pass as the still — run_comparison.py honours a per-STEP
    `settle`, so the still keeps the full --settle and the burst runs at `interval`. Without that, a
    GIF would cost a second deploy at a lower global --settle (and the VM lanes' deploy is the
    expensive part, not the shots).
    """
    animated = [k for k in examples if k in ANIMATED]
    burst = "\n\n".join(f'[[steps]]\nname = "gif{i:02d}"\nsettle = {interval}'
                        for i in range(1, frames + 1))
    for key in animated:
        f = scen_dir / f"{key}.toml"
        head = f.read_text().rstrip() if f.is_file() else (
            f'# generated by tools/parity/recapture.py — GIF burst for an animated page\n'
            f'tag = "{key}"\nthemes = {list(themes)!r}\n\n[[steps]]\nname = "initial"')
        f.write_text(f"{head}\n\n"
                     f"# --- appended by tools/parity/recapture.py: the GIF burst. These follow the LAST\n"
                     f"# step above, which is the one whose motion they are here to record.\n"
                     f"{burst}\n")
    return animated


def burst_frames(unit_dir: Path, theme: str, driven: bool | None = None) -> list[str]:
    """The PNGs one (tag, column, theme) unit contributes to its GIF, in CAPTURE ORDER. Pure.

    EVERY frame except the one the board publishes as the still. This used to be the opposite — an
    INCLUSION of steps whose name starts with "gif" — which threw away the ACTION frames, the only
    ones that differ on a driven page. carousel_page composes to [initial, paged-left, gif01..gif12],
    so the GIF was handed 12 identical post-settle frames, gif.py correctly deleted it as a
    non-animation, and the log blamed the encoder. The motion is in the before/after, not in the
    settle that follows it.

    The at-rest frame is DROPPED rather than led with because it is temporally distant: it is taken
    after the full --settle while the burst runs at --gif-interval, so on the ~13 animated pages with
    no authored scenario at all it would read as a stutter at the top of an otherwise even loop.
    Which frame that is mirrors import_run_captures.initial_frame exactly — the step named `initial`,
    else the theme's first — so the GIF and the board still can never disagree about what "at rest"
    was on a given page.
    """
    shots: list[tuple[str, str]] = []
    for sidecar in sorted(unit_dir.glob("*.json")):     # NNNN.json: sorted IS capture order
        try:
            meta = json.loads(sidecar.read_text())
        except (OSError, json.JSONDecodeError):
            continue
        png = sidecar.with_suffix(".png")
        if meta.get("theme") == theme and png.exists():
            shots.append((str(meta.get("step", "")), str(png)))
    # AN EXPLICIT IN-BURST BEFORE WINS, and settles what `initial` means per lane. A unit carrying an
    # `at-rest` step was shot with one (capture_android.write_run_unit): that frame belongs to THIS
    # recording, while `initial` there is a byte copy of the published still from a different launch,
    # kept only as motion_score._is_published_run's provenance witness. Lead with the real BEFORE and
    # drop the copy — checked FIRST because the name-based `driven` test below would otherwise see
    # `at-rest`, call the unit driven, and keep BOTH.
    #
    # On the VM and iOS lanes nothing writes `at-rest`, so this is inert there and `initial` keeps its
    # existing meaning: a genuine first frame of the run.
    prov = next((i for i, (step, _) in enumerate(shots) if step == "initial"), None)
    if any(step == "at-rest" for step, _ in shots):
        return [png for i, (_, png) in enumerate(shots) if i != prov]

    at_rest = next((i for i, (step, _) in enumerate(shots) if step == "initial"), 0)
    # DRIVEN pages keep the at-rest frame; UNDRIVEN ones drop it. The stutter rationale above only
    # holds when every frame is a burst frame taken at --gif-interval. On a page with an action step
    # the at-rest frame is the only BEFORE: the runner shoots each step after its settle, so by the
    # time the action frame is taken the transition has already finished and every retained frame is
    # the same post-action state. Dropping it leaves gif.py nothing to distinguish, so it deletes the
    # GIF and the log blames the encoder — the exact 12-identical-frames symptom this pass exists to
    # kill, reproduced on the only pages that actually gesture.
    # WHETHER THIS UNIT WAS DRIVEN. Inferred from the step names by default, which works wherever the
    # runner labels each frame with the SCENARIO step that produced it (the VM and iOS lanes do).
    #
    # IT DOES NOT WORK ON ANDROID, and silently. capture_android's burst is TIME-based: it labels its
    # frames `gif01@4s/12f` … `gif12@4s/12f` no matter what the driver thread is doing, so a page whose
    # scenario step is called `tapped` produces a unit whose steps are only `initial` + gifNN — and the
    # test below then calls a DRIVEN page undriven and throws away its at-rest frame. That frame is the
    # only BEFORE a time-labelled burst has.
    #
    # MEASURED on run 2026-08-07-10_31_49, android/gestures: the unit holds 13 frames and this returned
    # 12. MAUI survived by luck — its tap landed DURING the burst, so gif01 still differed from gif02+
    # (2985 px) — while the port's landed before gif01, leaving its 12 surviving frames identical and
    # scoring "port frozen, 0 px" against a page that measurably reacts (2985 px with the at-rest frame
    # kept, verified by direct adb injection). That is a fabricated MOTION MISMATCH.
    #
    # So callers that KNOW may say so. motion_score does: a page with an authored scenario is driven,
    # whatever the frames happen to be called.
    if driven is None:
        driven = any(step and step != "initial" and not step.startswith("gif") for step, _ in shots)
    if driven:
        return [png for _, png in shots]
    return [png for i, (_, png) in enumerate(shots) if i != at_rest]


def assemble_vm_gifs(run_dir: Path, plat_dir: str, animated, columns, themes, interval: float) -> None:
    """Turn each (tag, column, theme) burst in a finished run into captures/…/<key>_<theme>.gif."""
    import gif as gifmod

    fps = max(1, min(10, round(1.0 / max(interval, 0.1))))
    for key in animated:
        for col in columns:
            for theme in themes:
                frames = burst_frames(run_dir / key / plat_dir / col, theme)
                out = COMP / "captures" / plat_dir / COL_TO_DIR.get(col, col) / f"{key}_{theme}.gif"
                gifmod.drop_stale(str(out))
                if gifmod.frames_to_gif(frames, str(out), fps=fps):
                    log(f"      gif {plat_dir}/{COL_TO_DIR.get(col, col)}/{key}_{theme}.gif "
                        f"({len(frames)} frames @ {fps}fps)")
                elif frames:
                    fail(f"{plat_dir}/{col}/{theme}/{key}: gif assembly failed ({len(frames)} frames)")
                else:
                    # An animated page whose unit produced NO frame beyond the still. Silence here read
                    # as "GIF done" for as long as the burst selector was broken, which is the whole
                    # reason this pass exists — so it costs a failed step, not nothing.
                    fail(f"{plat_dir}/{col}/{theme}/{key}: no burst frames in the run — nothing to "
                         f"animate, so the board falls back to the still")


def lane_vm(platform: str, frameworks, themes, examples, settle, gif_frames, gif_interval) -> None:
    for p, lane, cfg, envname, cols, plat_dir in VM_LANES:
        if p != platform:
            continue
        columns = [cols[fw] for fw in frameworks if fw in cols]
        if not columns:
            log(f"=== LANE {platform}/{lane}: no selected framework has a column here — skipped")
            continue
        log(f"=== LANE {platform}/{lane}")
        if platform == "macos":
            mac_vm_clean()
            if lane == "catalyst":
                mac_vm_reboot_and_settle()
            else:
                mac_vm_keep_awake()   # AppKit does not reboot; it inherits a hours-old session
        # Scenarios dir per lane: SEEDED from the authored scenarios, then the animated pages get their
        # GIF burst merged on top. A page with neither still gets exactly one idle shot. Regenerated
        # each lane (rmtree first) so a stale scenario from a previous run cannot leak in.
        scen_dir = LOG_DIR / f"scenarios-{RUN_ID}-{lane}"
        shutil.rmtree(scen_dir, ignore_errors=True)
        # The gate needs the lane's REAL rect, so it comes from the config the runner is about to be
        # handed rather than from a table here that could drift out of step with it.
        present, rect = lane_geometry(cfg, envname)
        seeded = seed_scenarios(scen_dir, f"{platform}/{lane}", present, rect, envname)
        animated = write_gif_scenarios(scen_dir, examples, themes, gif_frames, gif_interval)
        # Say which pages this lane will actually DRIVE. In a five-hour log this one line is how you
        # confirm the scenarios reached the guest without reading any code.
        driven = [s for s in seeded if s in examples]
        log(f"      scenarios: driven={driven or '(none selected)'} gif-burst={animated or '(none)'}")
        fw_of_column = {c: fw for fw, c in cols.items()}
        reader = marker_reader(f"{platform}({lane})", fw_of_column, lambda k: kind_for(platform, k))
        rc = run_step(f"{platform}/{lane}: capture", [
            sys.executable, "-u", str(CTOOLS / "run_comparison.py"),
            "--config", str(COMP / "config" / cfg), "--env", envname,
            "--columns", ",".join(columns), "--themes", ",".join(themes),
            "--only", ",".join(examples), "--settle", str(settle),
            "--scenarios", str(scen_dir),
        ], on_line=reader, timeout=int(os.environ.get("PARITY_VM_IDLE_TIMEOUT", "1800")))
        reader.close("(runner exited)")   # type: ignore[attr-defined]
        # IMPORT EVEN WHEN THE CAPTURE STEP FAILED. The runner writes frames incrementally and loops
        # theme-OUTERMOST, so a lane that dies partway has still finished whole themes. Skipping the
        # import on rc!=0 threw away a complete light theme plus ~80% of dark on BOTH VM lanes — six
        # hours of captures left in the run directory and never published. import_run_captures.py
        # copies each tag's `initial` frame per theme and simply reports the ones that are absent.
        # …and only run dirs that hold frames FOR THIS LANE'S PLATFORM. Same invocation, the device
        # lanes now write a run dir of their own (RUN_DIR) whose units are `*/ios` and `*/android`;
        # without this filter a runner that died before creating its own would hand THAT one to
        # import_run_captures. A partially-captured runner run still matches — it has <tag>/<plat>/ —
        # so the deliberate import-anyway path below is untouched.
        runs = sorted((r for r in COMP.glob("20??-??-??-*") if next(r.glob(f"*/{plat_dir}"), None)),
                      key=lambda p2: p2.stat().st_mtime)
        if not runs:
            fail(f"{platform}/{lane}: the runner produced no run directory")
            continue
        if rc != 0:
            log(f"      capture step failed — importing the {rc and 'partial ' or ''}frames it did "
                f"produce from {runs[-1].name}")
        run_step(f"{platform}/{lane}: import run into canonical captures",
                 [sys.executable, str(CTOOLS / "import_run_captures.py"), str(runs[-1]), plat_dir],
                 timeout=900)
        if animated:
            assemble_vm_gifs(runs[-1], plat_dir, animated, columns, themes, gif_interval)


# --------------------------------------------------------------------------- measure
def measure(platforms: list[str]) -> None:
    log("=== MEASURE (board rebuild + scoring)")
    # ORDER MATTERS: build_comparison_json.py only CARRIES OVER pixel scores, it cannot compute them.
    run_step("board: refresh comparison.json", [sys.executable, str(CTOOLS / "build_comparison_json.py")],
             timeout=900)
    score_plats = []
    for p in platforms:
        score_plats += ["maccatalyst"] if p == "macos" else [p]
    for p in score_plats:
        run_step(f"board: pixel_score {p}",
                 [sys.executable, str(LIB / "pixel_score.py"), "--platform", p], timeout=2700)
    run_step("board: measure artifact sizes", [sys.executable, str(CTOOLS / "measure_size.py")],
             timeout=900)
    ttff = []
    for p in platforms:
        ttff += ["maccatalyst", "appkit"] if p == "macos" else [p]
    args: list[str] = []
    for p in ttff:
        args += ["--platform", p]
    run_step("measure: time-to-first-frame",
             [sys.executable, str(CTOOLS / "measure_runtime.py"), "--metric", "ttff", *args],
             timeout=5400)
    run_step("board: regenerate README", [sys.executable, str(CTOOLS / "gen_readme.py")], timeout=900)


# --------------------------------------------------------------------------- main
def csv_arg(value: str, valid: tuple[str, ...], what: str) -> list[str]:
    items = [v.strip() for v in value.split(",") if v.strip()]
    bad = [v for v in items if v not in valid]
    if bad:
        shown = ", ".join(valid[:12]) + (" …" if len(valid) > 12 else "")
        raise SystemExit(f"unknown {what}: {', '.join(bad)} (valid: {shown})")
    return [v for v in valid if v in items]     # canonical order, de-duplicated


def selftest() -> int:
    """Device-free check of the two things that silently write to the WRONG PLACE if they break:
    the framework->column->directory mapping, and the marker translation the VM/Android lanes log by."""
    rows = plan_rows(["macos"], ["maui_xaml", "cpp"], ["dark"], ["button"])
    got = {(r[1], r[2], str(r[6].relative_to(COMP))) for r in rows}
    assert got == {
        ("catalyst", "maui_xaml", "captures/maccatalyst/maui/button_dark.png"),
        ("catalyst", "cpp", "captures/maccatalyst/cpp/button_dark.png"),
        ("appkit", "cpp", "captures/maccatalyst/appkit_cpp/button_dark.png"),
    }, got   # AppKit has no MAUI column: maui_xaml must fan out to catalyst ONLY, never to appkit_*
    for plat, want in (("ios", "captures/ios/xaml/animation_light.gif"),
                       ("android", "captures/android/xaml/animation_light.gif"),
                       ("windows", "captures/windows/xaml/animation_light.gif")):
        row = plan_rows([plat], ["cpp_xaml"], ["light"], ["animation"])[0]
        assert row[5] == "png+gif" and row[6] == COMP / want, row   # animated => GIF on every platform
    assert plan_rows(["ios"], ["cpp"], ["light"], ["button"])[0][5] == "png"

    # --- scenarios. A page with no scenario must still get exactly ONE idle shot, the authored
    # scenarios must SURVIVE into the lane dir, and the GIF burst must COMPOSE with them.
    scen = Path(tempfile.mkdtemp())
    try:
        animated = write_gif_scenarios(scen, ["button", "animation"], ["light"], 3, 0.25)
        steps = tomllib.loads((scen / "animation.toml").read_text())["steps"]
        assert animated == ["animation"] and not (scen / "button.toml").exists()
        # The still keeps the full --settle (no per-step key); only the burst frames override it.
        assert steps[0] == {"name": "initial"} and len(steps) == 4, steps
        assert all(s["settle"] == 0.25 and s["name"].startswith("gif") for s in steps[1:]), steps
    finally:
        shutil.rmtree(scen, ignore_errors=True)

    # --- the portable coordinate contract, which three separate gates below depend on.
    assert coordinate_space([{"name": "initial"}]) == "none"
    assert coordinate_space([{"action": "click", "at": [0.5, 0.2]}]) == "fraction"
    assert coordinate_space([{"action": "click", "at": [756, 171]}]) == "absolute"
    # one absolute pair pins the WHOLE file to one geometry, wherever in it that pair sits
    assert coordinate_space([{"action": "click", "at": [0.5, 0.2]},
                             {"action": "swipe", "at": [0.5, 0.5], "to": [700, 200]}]) == "absolute"
    for mixed in ([0.5, 300], [300, 0.5]):
        try:
            coordinate_space([{"action": "click", "at": mixed}])
        except ValueError:
            pass
        else:
            raise AssertionError(f"mixed coordinate {mixed} was accepted — it would scale one axis")

    # --- what the window gate judges, and what it must NOT. `at` is the start and `to` the end; a
    # step carrying only `to` has no start to judge (the runner rejects it as malformed at the point of
    # use), and gating it as though its end were one would refuse a scenario for the wrong reason.
    cat = {"x": 128, "y": 30, "w": 1024, "h": 800}
    assert out_of_rect([{"action": "click", "at": [200, 100]}], cat) is None
    assert out_of_rect([{"action": "swipe", "to": [9999, 9999]}], cat) is None          # end-only
    assert out_of_rect([{"action": "swipe", "at": [200, 100], "to": [9999, 9999]}], cat) is None
    assert out_of_rect([{"name": "initial", "at": [9999, 9999]}], cat) is None           # no action
    assert out_of_rect([{"action": "click", "at": [1400, 100]}], cat)                    # off-window
    # --- per-lane overrides (run_comparison.for_lane) ---------------------------------------------
    # The lane bounds-checks the point IT will send, not the portable one. Without the envname arm
    # both of these are wrong in the dangerous direction: the first passes a lane a coordinate that is
    # off its window, the second refuses a file whose override is exactly what makes it valid.
    off = [{"action": "click", "at": [200, 100], "at_macos-arm64": [1400, 100]}]
    assert out_of_rect(off, cat, "macos-arm64"), "an off-window override must be caught"
    assert out_of_rect(off, cat) is None, "with no envname only the portable `at` is judged"
    fix = [{"action": "click", "at": [1400, 100], "at_macos-arm64": [200, 100]}]
    assert out_of_rect(fix, cat, "macos-arm64") is None, "the override rescues the step for its lane"
    assert out_of_rect(fix, cat), "other lanes still see the portable coordinate, and it is off-window"
    # An absolute pair hiding in an override must still pin the WHOLE file to absolute, or
    # seed_scenarios waves it onto lanes that never pin a window.
    assert coordinate_space([{"action": "click", "at": [0.5, 0.2],
                              "at_windows-arm64": [756, 171]}]) == "absolute"
    # half-open on the far edge, as capture_android.input_argv is: 128+1024 = 1152 is the first pixel
    # PAST a window whose last column is 1151.
    assert out_of_rect([{"action": "click", "at": [1151, 829]}], cat) is None
    assert out_of_rect([{"action": "click", "at": [1152, 100]}], cat)
    assert out_of_rect([{"action": "click", "at": [200, 830]}], cat)

    # --- the lane rects the gate runs on. These are the numbers scenarios/_selftest.py records as
    # MEASURED from real run sidecars; pinning them here catches both a renamed config key and a
    # drift in the defaults duplicated from run_comparison.Env.
    assert lane_geometry("local.toml", "macos-arm64") == (True, {"x": 128, "y": 30, "w": 1024, "h": 800})
    assert lane_geometry("windows.toml", "windows-arm64") == (True, {"x": 244, "y": 0, "w": 1024, "h": 800})
    appkit_present, appkit_rect = lane_geometry("local.toml", "macos-appkit")
    assert appkit_present is False and appkit_rect["w"] == 480, (appkit_present, appkit_rect)

    scen = Path(tempfile.mkdtemp())
    try:
        seeded = seed_scenarios(scen, "catalyst", True, {"x": 128, "y": 30, "w": 1024, "h": 800})
        # (a) the authored scenarios reach the lane dir INTACT. This is the whole defect: the dir was
        #     seeded EMPTY, so --scenarios pointed the runner at nothing and no page was ever driven.
        assert "button" in seeded, seeded
        assert (scen / "button.toml").read_bytes() == (SCENARIOS / "button.toml").read_bytes()
        # …and still ASK for an interaction on the far side of the copy. Deliberately not pinned to the
        # exact step list: scenarios are authored freely, only "it drives something" is the invariant.
        btn = tomllib.loads((scen / "button.toml").read_text())["steps"]
        assert any(s.get("action") for s in btn), f"button.toml no longer taps anything: {btn}"
        # The frame the BOARD publishes must be the page AT REST: import_run_captures.py takes the step
        # named `initial`, else the theme's first frame. An action in that frame republishes a reacted
        # state as the page's canonical still — a silent board corruption, and the reason this asserts
        # over the REAL authored dir rather than a fixture (new scenarios land there without touching
        # this file). Parsing every file here also catches a scenario the runner would die loading.
        # SCOPE: this is a VM-LANE invariant, and only a VM lane can honour it — it holds because the
        # runner shoots every step into a run dir and the import picks one out. iOS has no run dir:
        # capture_still drives and then shoots the single board frame, so there the published still is
        # the DRIVEN state by construction. Do not "extend" this assertion to the device lanes; it
        # would be asserting something their capture path cannot express.
        for f in sorted(SCENARIOS.glob("*.toml")):
            s = tomllib.loads(f.read_text())["steps"]
            published = next((st for st in s if st.get("name") == "initial"), s[0])
            assert not published.get("action"), f"{f.name}: the published frame performs an action"

        # (b) an animated page that ALSO has an authored scenario keeps its actions IN ORDER and gets
        #     the burst after them, with its own themes/settle untouched.
        (scen / "animation.toml").write_text(
            'tag = "animation"\nthemes = ["dark"]\nsettle = 5.0\n\n'
            '[[steps]]\nname = "initial"\n\n'
            '[[steps]]\nname = "go"\naction = "click"\nat = [10, 20]\n\n'
            '[[steps]]\nname = "more"\naction = "scroll"\nat = [10, 20]\ndy = -400\n')
        animated = write_gif_scenarios(scen, ["button", "animation"], ["light"], 2, 0.25)
        comp = tomllib.loads((scen / "animation.toml").read_text())
        assert animated == ["animation"] and comp["themes"] == ["dark"] and comp["settle"] == 5.0, comp
        assert [s["name"] for s in comp["steps"]] == ["initial", "go", "more", "gif01", "gif02"], comp
        assert comp["steps"][1] == {"name": "go", "action": "click", "at": [10, 20]}, comp
        # a non-animated authored page is not rewritten at all
        assert (scen / "button.toml").read_bytes() == (SCENARIOS / "button.toml").read_bytes()
    finally:
        shutil.rmtree(scen, ignore_errors=True)

    # --- (a) which frames a burst contributes. The bug was an INCLUSION of `step` names starting with
    # "gif", which dropped the action frame — the only one that differs — and left the encoder holding
    # 12 identical stills. Driven here as a pure function over a fake run tree so the check cannot
    # touch the real board (assemble_vm_gifs drop_stale()s a live GIF before it writes one).
    run = Path(tempfile.mkdtemp())
    try:
        unit = run / "carousel_page" / "maccatalyst" / "cpp"
        unit.mkdir(parents=True)
        for i, (step, theme) in enumerate([("initial", "light"), ("paged-left", "light"),
                                           ("gif01", "light"), ("initial", "dark")], start=1):
            (unit / f"{i:04d}.json").write_text(json.dumps({"step": step, "theme": theme}))
            (unit / f"{i:04d}.png").write_bytes(b"png")
        # DRIVEN page: every frame is kept, at-rest one included. Keeping only the action frame and
        # the burst was the second form of the same bug — the runner shoots each step AFTER its
        # settle, so the transition is over before the action frame is taken and all of them show the
        # same post-action state. The at-rest frame is the only BEFORE, and without it gif.py sees no
        # distinct frames, deletes the GIF, and the log blames the encoder.
        got = [Path(p).name for p in burst_frames(unit, "light")]
        assert got == ["0001.png", "0002.png", "0003.png"], got
        assert burst_frames(unit, "dark") == []         # only a still: no burst, and no GIF to claim
        # Same, with the at-rest step named something other than `initial` (scroll_view names its
        # first step `top`): still driven, so still every frame.
        (unit / "0001.json").write_text(json.dumps({"step": "top", "theme": "light"}))
        assert [Path(p).name for p in burst_frames(unit, "light")] == \
            ["0001.png", "0002.png", "0003.png"]

        # (a1) AN EXPLICIT `at-rest` STEP SUPERSEDES THE PROVENANCE COPY. This is the Android shape:
        # 0001 is a byte copy of the published still (motion_score._is_published_run's witness, shot
        # under different device state) and 0002 is the burst's own BEFORE. The witness must stay in
        # the unit — find_frames rejects the whole run without it — and must NOT be scored as a frame.
        # Substituting one for the other was tried and reverted twice; this pins the third shape.
        (unit / "0001.json").write_text(json.dumps({"step": "initial", "theme": "light"}))
        (unit / "0002.json").write_text(json.dumps({"step": "at-rest", "theme": "light"}))
        got = [Path(p).name for p in burst_frames(unit, "light")]
        assert got == ["0002.png", "0003.png"], got
    finally:
        shutil.rmtree(run, ignore_errors=True)

    # --- (a2) UNDRIVEN animated page: the at-rest frame is still DROPPED. This is the ~13 pages with
    # no authored scenario, where that frame is taken after the full --settle while the burst runs at
    # --gif-interval — leading with it reads as a stutter at the top of an otherwise even loop. The
    # driven/undriven split is the whole point: keep the BEFORE only when something happened after it.
    run = Path(tempfile.mkdtemp())
    try:
        unit = run / "activity_indicator" / "maccatalyst" / "cpp"
        unit.mkdir(parents=True)
        for i, step in enumerate(["initial", "gif01", "gif02"], start=1):
            (unit / f"{i:04d}.json").write_text(json.dumps({"step": step, "theme": "light"}))
            (unit / f"{i:04d}.png").write_bytes(b"png")
        got = [Path(p).name for p in burst_frames(unit, "light")]
        assert got == ["0002.png", "0003.png"], got
    finally:
        shutil.rmtree(run, ignore_errors=True)

    # --- (b)+(c) the two gates, over a SYNTHETIC scenario dir: no checked-in scenario is mixed or
    # off-window today, and these paths must be exercised before one is. Both gates share
    # coordinate_space, so this also pins the two OUTCOMES apart: a skip is a log line (the page keeps
    # its honest idle frame), a mixed pair is a failed step (it is wrong on every lane).
    src, dest, seen = Path(tempfile.mkdtemp()), Path(tempfile.mkdtemp()), []
    (src / "frac.toml").write_text('tag = "frac"\n[[steps]]\nname = "initial"\n\n'
                                   '[[steps]]\nname = "tap"\naction = "click"\nat = [0.5, 0.4]\n')
    (src / "far.toml").write_text('tag = "far"\n[[steps]]\nname = "initial"\n\n'
                                  '[[steps]]\nname = "tap"\naction = "click"\nat = [1400, 400]\n')
    # `near` is the case the AppKit rule exists for and the off-window rule cannot catch: absolute, and
    # numerically INSIDE that lane's 480x752 — but the lane never pins its window, so the number is
    # relative to nothing and the click lands wherever the WM left it. A gate that only compared
    # against the rect would seed this and tap a stranger.
    (src / "near.toml").write_text('tag = "near"\n[[steps]]\nname = "initial"\n\n'
                                   '[[steps]]\nname = "tap"\naction = "click"\nat = [200, 300]\n')
    (src / "mixed.toml").write_text('tag = "mixed"\n[[steps]]\nname = "initial"\n\n'
                                    '[[steps]]\nname = "tap"\naction = "click"\nat = [0.5, 300]\n')
    (src / "idle.toml").write_text('tag = "idle"\nsettle = 5.0\n[[steps]]\nname = "initial"\n')
    failed_before = len(FAILED)
    real_log, real_scen = log, SCENARIOS
    sys.modules[__name__].log = seen.append                         # type: ignore[attr-defined]
    sys.modules[__name__].SCENARIOS = src                           # type: ignore[attr-defined]
    try:
        # A presented lane keeps what its own window contains; `far` is 1400 > 128+1024 = off-window,
        # while `near` is inside it and rides along.
        assert set(seed_scenarios(dest / "catalyst", "catalyst", True,
                                  {"x": 128, "y": 30, "w": 1024, "h": 800})) == {"frac", "idle", "near"}
        # An UNPINNED lane (macos-appkit) has no rect at all, so NO absolute scenario is valid there —
        # not even `near`, which its 480x752 numerically contains.
        assert set(seed_scenarios(dest / "appkit", "appkit", False, appkit_rect)) == {"frac", "idle"}
        # and the return value is not a claim: the runner reads the DIR, so that is what must match.
        assert {p.stem for p in (dest / "catalyst").glob("*.toml")} == {"frac", "idle", "near"}
        assert {p.stem for p in (dest / "appkit").glob("*.toml")} == {"frac", "idle"}
        # A device gets fractions only; `idle` is dropped for having nothing to inject, which is not a
        # skip — it is a page keeping the single idle screenshot all ~155 unscripted ones get.
        assert set(device_scenarios("ios", ["frac", "far", "near", "mixed", "idle"])) == {"frac"}
    finally:
        sys.modules[__name__].log = real_log                        # type: ignore[attr-defined]
        sys.modules[__name__].SCENARIOS = real_scen                 # type: ignore[attr-defined]
        shutil.rmtree(src, ignore_errors=True)
        shutil.rmtree(dest, ignore_errors=True)
    # Every skip names its lane AND its page: in a five-hour log an anonymous "skipped 1" is no better
    # than the silence this pass exists to remove.
    for lane in ("catalyst", "appkit", "ios"):
        assert any(f"SCENARIO SKIPPED {lane}/far" in ln for ln in seen), (lane, seen)
    assert len(FAILED) - failed_before == 3, FAILED[failed_before:]   # mixed: 2 seeds + 1 device pass
    del FAILED[failed_before:]      # a selftest must not inflate the run's exit code

    # …and the same gates over the REAL authored dir, as INVARIANTS rather than a fixed page list, so
    # they keep meaning while the scenarios are re-authored from absolute to fractional coordinates.
    failed_before, seen = len(FAILED), []
    sys.modules[__name__].log = seen.append                         # type: ignore[attr-defined]
    try:
        real_seeded = {}
        for lane, (present, rect) in (("appkit", (appkit_present, appkit_rect)),
                                      ("catalyst", lane_geometry("local.toml", "macos-arm64")),
                                      ("windows", lane_geometry("windows.toml", "windows-arm64"))):
            d = Path(tempfile.mkdtemp())
            try:
                real_seeded[lane] = set(seed_scenarios(d, lane, present, rect))
            finally:
                shutil.rmtree(d, ignore_errors=True)
        real_device = set(device_scenarios("ios", all_examples()))
    finally:
        sys.modules[__name__].log = real_log                        # type: ignore[attr-defined]
    assert not FAILED[failed_before:], FAILED[failed_before:]    # no authored scenario is malformed
    for lane, got in real_seeded.items():
        # a settle-only scenario carries no geometry, so it reaches EVERY lane — web_view's 5s is the
        # only thing keeping MAUI's own column from racing WebView2's init.
        assert "web_view" in got, (lane, got)
    for key in real_seeded["appkit"] | real_device:
        # the unpinned AppKit window and a phone display share exactly one requirement: nothing
        # absolute may reach them.
        assert coordinate_space(read_steps(key)) != "absolute", key
    assert real_seeded["appkit"] <= real_seeded["catalyst"], real_seeded  # unpinned is the strictest

    # The device wiring itself. `steps=` IS fix (b), and a device-free check cannot drive a simulator —
    # but it can prove the keyword still exists on the far side, which is the difference between the
    # injection code being reachable and being dead. These modules only read env vars at import, so
    # importing them here costs nothing and needs no device. A rename over there would otherwise show
    # up as a TypeError on the guest, once per page, hours into a sweep.
    import inspect

    import capture_android
    import capture_ios
    for fn in (capture_ios.capture_still, capture_ios.capture_gif, capture_android.capture_gif):
        assert "steps" in inspect.signature(fn).parameters, f"{fn.__module__}.{fn.__name__}"
    # …and that this file actually PASSES it. Deliberately a source check, not a behavioural one: the
    # behaviour needs a booted simulator, and the defect being guarded is precisely a call that
    # type-checks, runs, logs success and injects nothing. A dropped `steps=` is invisible to every
    # other assertion here, which is how it survived into a shipped lane once already.
    # Counted per call site — iOS drives BOTH its still and its GIF, and losing either one is a whole
    # class of page that quietly goes back to being photographed at rest.
    for fn, needle, want in ((lane_ios, "steps=steps", 2), (android_gifs, "steps=scen.get(key)", 1)):
        assert inspect.getsource(fn).count(needle) == want, (fn.__name__, needle, want)
    # …and WHICH SIDE OF THE SHOT they run on. A DRIVEN page must shoot at rest FIRST; without that,
    # its board still is the frame AFTER the click — the board was showing flipped switches and ticked
    # checkboxes as their resting render. Source check for the same reason as above: it needs a booted
    # simulator to observe, and the failure mode is a published frame that looks perfectly plausible.
    #
    # THIS ASSERTION USED TO PIN `still_first=kind != "png+gif"`, i.e. it pinned the ANIMATED-only rule
    # as spec — and that rule was the bug: a page that was BOTH animated and driven drove before it
    # shot, so no at-rest frame existed and motion_score read 0 px on a page that reacted perfectly
    # (measured on `gestures`, 2026-08-06 — `step tapped: ok` on all three columns, zero frames carrying
    # that step). The assertion faithfully protected the defect. It now pins the property that was
    # actually wanted: a page with STEPS shoots first, whatever its kind.
    assert "still_first" in inspect.signature(capture_ios.capture_still).parameters
    assert inspect.getsource(lane_ios).count('still_first=bool(steps) or kind != "png+gif"') == 1
    # The unit gate must cover BOTH halves of what pixel_score motion-scores (ANIMATED or driven), or
    # the frames it scores are never banked in the first place.
    assert inspect.getsource(lane_ios).count('kind == "png+gif" or bool(steps)') == 1

    # --- the RUN DIRECTORY the device lanes leave for lib/motion_score.py. Same shape of check and
    # for the same reason: a dropped destination type-checks, runs, logs success, writes no evidence,
    # and every animated cell keeps reporting the "no run directory" string it reports today — so the
    # only thing that can catch it without a device is that the call site says so.
    for fn, needle, want in ((lane_ios, "**ios_run_kw(key, fw, want_unit)", 2),
                             (android_gifs, "**android_run_kw(unit_ok)", 1)):
        assert inspect.getsource(fn).count(needle) == want, (fn.__name__, needle, want)
    # …each preceded by ONE tolerance probe, asked before the lane captures anything.
    for fn, want in ((lane_ios, 'accepts_run_kw("ios", IOS_RUN_KW'),
                     (android_gifs, 'accepts_run_kw("android", ANDROID_RUN_KW')):
        assert inspect.getsource(fn).count(want) == 1, (fn.__name__, want)
    # The keywords are the two capture modules' OWN, and they are not the same word; this file must
    # not drift into a house name for either. (A soft check on purpose — see the tolerance block
    # below: the run is designed to survive their absence, so this must not be a hard failure.)
    for kw, fn in ((IOS_RUN_KW, capture_ios.capture_gif), (ANDROID_RUN_KW, capture_android.capture_gif)):
        if kw not in inspect.signature(fn).parameters:
            print(f"  NOTE {fn.__module__}.{fn.__name__} has no `{kw}=` yet — the device lanes will "
                  f"capture normally and report that they cannot be motion-scored")

    # The tolerance path. The keyword is added by two SIBLING changes to the capture modules; until
    # (and if) they land, this file must neither crash on page 1 of 1032 nor go quiet — an empty run
    # dir and an unexplained "NOT motion-scored" on every animated cell is the worse outcome.
    failed_before, seen = len(FAILED), []
    sys.modules[__name__].log = seen.append                          # type: ignore[attr-defined]
    try:
        assert accepts_run_kw("probe", IOS_RUN_KW, lambda app, key: None) is False
        # …and the lane then passes NOTHING and captures on, rather than crashing or creating a run
        # directory it will never write a frame into.
        assert ios_run_kw("animation", "cpp", False) == {} and android_run_kw(False) == {}
    finally:
        sys.modules[__name__].log = real_log                          # type: ignore[attr-defined]
    assert len(FAILED) - failed_before == 1 and IOS_RUN_KW in FAILED[-1], FAILED[failed_before:]
    del FAILED[failed_before:]      # a selftest must not inflate the run's exit code

    # The unit path itself. The column must be the RUNNER column (maui_xaml/cpp/cpp_xaml); both call
    # sites have the CAPTURE-dir name (maui/cpp/xaml) in a local called `app`, and using it would be
    # invisible everywhere else in this file.
    unit_root = Path(tempfile.mkdtemp())
    try:
        for plat_dir in ("ios", "android"):
            for fw in FRAMEWORKS:
                got = run_unit("animation", plat_dir, fw, unit_root)
                assert got.parts[-3:] == ("animation", plat_dir, fw), got
                assert got.relative_to(unit_root).parts[0] == "animation", got   # <run>/<tag>/… first
                assert got.is_dir(), got
        # …and that the real one is named the way motion_score._run_dirs globs AND ORDERS: `20??-??-??-*`
        # sorted by NAME. The runner's stamp is the only format that sorts correctly against itself.
        assert RUN_DIR.parent == COMP, RUN_DIR
        assert datetime.strptime(RUN_DIR.name, "%Y-%m-%d-%H_%M_%S"), RUN_DIR.name
        # A real runner stamp out of this tree, so the ORDERING claim is pinned and not just the
        # format: a same-day `%H%M%S` name compares LESS than this one and would rank as older.
        assert RUN_DIR.name > "2026-08-04-18_42_46", RUN_DIR.name
        assert not RUN_DIR.exists(), f"{RUN_DIR} was created by a --selftest/--plan run"

        # …and that "the framework IS the column" — the assumption both device lanes pass `fw` on.
        # A device lane has no VM_LANES row to look a column up in: it hands the framework name
        # straight through, which is only right because the board's framework DIRECTORY maps back to
        # exactly that column. Read out of motion_score's own table, in the direction it reads it.
        import motion_score  # noqa: PLC0415  selftest-only; it imports THIS module at its top level

        for fw in FRAMEWORKS:
            assert motion_score.FW_TO_COL[COL_TO_DIR[fw]] == fw, fw
    finally:
        shutil.rmtree(unit_root, ignore_errors=True)

    seen = []
    sys.modules[__name__].log = seen.append                          # type: ignore[attr-defined]
    try:
        r = marker_reader("ios", {"cpp": "cpp_xaml"}, lambda k: kind_for("ios", k))
        r("@@PARITY BEGIN button cpp light")
        r("@@PARITY BEGIN entry cpp light")     # a BEGIN with no END = the previous example failed
        r.close("(exited)")                     # type: ignore[attr-defined]
    finally:
        sys.modules[__name__].log = real_log    # type: ignore[attr-defined]
    assert len(seen) == 4 and "framework=cpp_xaml theme=light example=button" in seen[0], seen
    assert "no END" in seen[1] and "(exited)" in seen[3], seen
    print("recapture selftest: mapping + markers + scenarios + coordinate gates + gif burst + "
          "device-lane run dir OK")
    return 0


def main(argv=None) -> int:
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0],
                                 formatter_class=argparse.RawDescriptionHelpFormatter,
                                 epilog="\n".join(__doc__.splitlines()[2:]))
    ap.add_argument("--platforms", default=",".join(PLATFORMS), help=f"default: {','.join(PLATFORMS)}")
    ap.add_argument("--frameworks", default=",".join(FRAMEWORKS), help=f"default: {','.join(FRAMEWORKS)}")
    ap.add_argument("--themes", default=",".join(THEMES), help="default: light,dark")
    ap.add_argument("--examples", default="all", help="comma-separated page keys (default: all 172)")
    ap.add_argument("--visible", choices=("yes", "no"), default="yes",
                    help="give the mobile emulators a window you can watch (default yes). iOS: "
                         "cosmetic — shots come from the device framebuffer either way. Android: "
                         "load-bearing at START-up; an already-running -no-window emulator cannot be "
                         "given a window. No effect on macos/windows (they run on a VM's own display).")
    ap.add_argument("--plan", action="store_true",
                    help="print every (platform, framework, theme, example) -> output path and exit; "
                         "no device, no build, no capture")
    ap.add_argument("--selftest", action="store_true",
                    help="device-free check of the column mapping, the progress markers and the "
                         "scenario seed/compose; exits")
    ap.add_argument("--skip-build", action="store_true",
                    help="capture with whatever is already built (iOS/macOS; the Android scripts "
                         "always build their own APK)")
    ap.add_argument("--no-measure", action="store_true", help="capture only; skip the board re-measure")
    ap.add_argument("--measure-only", action="store_true",
                    help="skip capturing and ONLY re-measure the board. Use this to finish two lanes "
                         "that were captured in parallel: the measure phase rewrites comparison.json, "
                         "measurements.json and README.md, so two concurrent runs would clobber each "
                         "other's scores. Capture with --no-measure, then run this once for both.")
    ap.add_argument("--settle", type=float, default=4.0, help="seconds to settle after each launch")
    ap.add_argument("--gif-secs", type=float, default=4.0,
                    help="seconds of motion recorded for an animated page (ios/android)")
    ap.add_argument("--gif-frames", type=int, default=12,
                    help="frames in an animated page's burst on the macos/windows VM lanes")
    ap.add_argument("--gif-interval", type=float, default=0.3,
                    help="seconds between those burst frames")
    a = ap.parse_args(argv)
    if a.selftest:
        return selftest()

    platforms = csv_arg(a.platforms, PLATFORMS, "platform")
    frameworks = csv_arg(a.frameworks, FRAMEWORKS, "framework")
    themes = csv_arg(a.themes, THEMES, "theme")
    known = all_examples()
    examples = known if a.examples == "all" else csv_arg(a.examples, tuple(known), "example")
    if not (platforms and frameworks and themes and examples):
        raise SystemExit("nothing selected")

    rows = plan_rows(platforms, frameworks, themes, examples)
    if a.plan:
        for platform, lane, fw, theme, key, kind, path in rows:
            print(f"{platform:8} {lane:9} {fw:10} {theme:5} {key:28} {kind:14} "
                  f"{path.relative_to(COMP)}")
        print(f"\n{len(rows)} capture(s): platforms={platforms} frameworks={frameworks} "
              f"themes={themes} examples={len(examples)}")
        return 0

    # The toolchain env every lane needs (cmake/vcpkg, the Android SDK, the Homebrew dotnet).
    os.environ.setdefault("VCPKG_ROOT", str(Path.home() / "vcpkg"))
    # ANDROID_HOME is CORRECTED rather than defaulted. A pointing-at-nothing value is worse than an
    # absent one: an Android-Studio-shaped ~/Library/Android/sdk with no platform-tools in it is a real
    # configuration on this machine, and honouring it would send every android step to a path that does
    # not exist. android_sdk_root() picks the first root that actually has adb.
    sdk = android_sdk_root()
    if sdk:
        if os.environ.get("ANDROID_HOME") not in (None, sdk):
            log(f"note: ANDROID_HOME={os.environ['ANDROID_HOME']} has no platform-tools/adb — "
                f"using {sdk} (same probe order as tools/android-emu-lib.sh)")
        os.environ["ANDROID_HOME"] = sdk
        # Children resolve `adb` off PATH (lib/capture_android.py) — put the WORKING one first.
        os.environ["PATH"] = f"{sdk}/platform-tools:/opt/homebrew/bin:{os.environ['PATH']}"
    else:
        os.environ["PATH"] = f"/opt/homebrew/bin:{os.environ['PATH']}"

    started = time.time()
    log(f"RECAPTURE {RUN_ID} — platforms={platforms} frameworks={frameworks} themes={themes} "
        f"examples={len(examples)} visible={a.visible}")
    visible = a.visible == "yes"

    # Strictly sequential. The macOS VM's Catalyst and AppKit lanes share ONE guest agent and ONE
    # scratch shot.png; two runs at once destroy each other's frames.
    for platform in (() if a.measure_only else platforms):
        # A lane that throws must not cost you the other three — that is the whole point of a run this
        # long, and an unhandled exception in the FIRST lane (a missing SDK, an unreachable VM) would
        # otherwise abandon a multi-hour job seconds after it started.
        try:
            if platform == "ios":
                lane_ios(frameworks, themes, examples, visible, a.skip_build, a.settle, a.gif_secs)
            elif platform == "android":
                lane_android(frameworks, themes, examples, visible, a.gif_secs, a.gif_frames)
            else:
                if platform == "macos" and not a.skip_build:
                    build("catalyst", frameworks)     # no-op unless cpp / cpp_xaml is selected
                    build("appkit", frameworks)
                lane_vm(platform, frameworks, themes, examples, a.settle, a.gif_frames, a.gif_interval)
        except KeyboardInterrupt:
            raise
        except Exception as exc:
            fail(f"{platform}: lane aborted — {type(exc).__name__}: {exc}")
            traceback.print_exc()

    if not a.no_measure or a.measure_only:
        measure(platforms)

    mins, secs = divmod(int(time.time() - started), 60)
    log(f"SUMMARY — {mins}m {secs}s, {len(FAILED)} failed step(s)")
    for f in FAILED:
        log(f"  {f}")
    if FAILED:
        log("NOTE: a run that exits 0 is not evidence the frames are correct — check the captures.")
    return len(FAILED)


if __name__ == "__main__":
    sys.exit(main())
