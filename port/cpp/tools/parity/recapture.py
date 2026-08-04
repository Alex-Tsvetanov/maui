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
    ("windows", "windows", "windows.toml", "windows-x64",
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
        # Cosmetic ONLY on iOS: shots come from the device framebuffer via `simctl io`, never the host
        # screen, so the window cannot affect what is captured.
        subprocess.run(["open", "-a", "Simulator"], capture_output=True)
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


# --------------------------------------------------------------------------- lanes
def lane_ios(frameworks, themes, examples, visible, skip_build, settle, gif_secs) -> None:
    log("=== LANE ios")
    if not ensure_ios_sim(visible):
        return
    if not skip_build:
        build("ios", frameworks)
        ios_install(frameworks)

    import capture_ios

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
                    t0 = begin("ios", fw, theme, key, kind)
                    try:
                        out = capture_ios.capture_still(app, key, theme, settle)
                        if kind == "png+gif":
                            # Both, deliberately: the GIF is what the board renders, the still keeps a
                            # frame for anything that only reads PNGs.
                            out = capture_ios.capture_gif(app, key, theme, settle, gif_secs) or out
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
    animated = [k for k in examples if k in ANIMATED]
    for theme in themes:
        for fw in frameworks:
            script, column = ANDROID_SCRIPT[fw]
            reader = marker_reader("android", {column: fw}, lambda k: "png")
            run_step(f"android: {fw} ({theme})",
                     ["bash", str(LIB / script), *examples],
                     env={"MAUI_APPEARANCE": theme}, on_line=reader)
            reader.close("(script exited)")   # type: ignore[attr-defined]
        if animated:
            android_gifs(frameworks, theme, animated, gif_secs, gif_frames)


def android_gifs(frameworks, theme, animated, gif_secs, gif_frames) -> None:
    """screenrecord pass for the animated pages. Separate from the still pass because the shell
    scripts capture exactly one frame per page — and because their exit trap has already put the
    device's night mode back, so this pass has to set it again for the theme it is recording."""
    import capture_android
    import device_state

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
                    out = capture_android.capture_gif(app, key, theme, secs=gif_secs,
                                                     frame_count=gif_frames)
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


def write_gif_scenarios(scen_dir: Path, examples, themes, frames: int, interval: float) -> list[str]:
    """A scenario per animated page: the `initial` still, then a burst of no-action frames.

    The burst rides along in the SAME runner pass as the still — run_comparison.py honours a per-STEP
    `settle`, so the still keeps the full --settle and the burst runs at `interval`. Without that, a
    GIF would cost a second deploy at a lower global --settle (and the VM lanes' deploy is the
    expensive part, not the shots).
    """
    animated = [k for k in examples if k in ANIMATED]
    for key in animated:
        steps = ['[[steps]]\nname = "initial"']
        steps += [f'[[steps]]\nname = "gif{i:02d}"\nsettle = {interval}' for i in range(1, frames + 1)]
        (scen_dir / f"{key}.toml").write_text(
            f'# generated by tools/parity/recapture.py — GIF burst for an animated page\n'
            f'tag = "{key}"\nthemes = {list(themes)!r}\n\n' + "\n\n".join(steps) + "\n")
    return animated


def assemble_vm_gifs(run_dir: Path, plat_dir: str, animated, columns, themes, interval: float) -> None:
    """Turn each (tag, column, theme) burst in a finished run into captures/…/<key>_<theme>.gif."""
    import gif as gifmod

    fps = max(1, min(10, round(1.0 / max(interval, 0.1))))
    for key in animated:
        for col in columns:
            for theme in themes:
                frames = []
                for sidecar in sorted((run_dir / key / plat_dir / col).glob("*.json")):
                    try:
                        meta = json.loads(sidecar.read_text())
                    except (OSError, json.JSONDecodeError):
                        continue
                    if meta.get("theme") == theme and str(meta.get("step", "")).startswith("gif"):
                        png = sidecar.with_suffix(".png")
                        if png.exists():
                            frames.append(str(png))
                out = COMP / "captures" / plat_dir / COL_TO_DIR.get(col, col) / f"{key}_{theme}.gif"
                gifmod.drop_stale(str(out))
                if gifmod.frames_to_gif(frames, str(out), fps=fps):
                    log(f"      gif {plat_dir}/{COL_TO_DIR.get(col, col)}/{key}_{theme}.gif "
                        f"({len(frames)} frames @ {fps}fps)")
                elif frames:
                    fail(f"{plat_dir}/{col}/{theme}/{key}: gif assembly failed ({len(frames)} frames)")


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
        # Scenarios dir per lane: empty (one idle shot per page) except for the animated pages, which
        # get a generated burst scenario. Regenerated each lane so a stale one can't leak in.
        scen_dir = LOG_DIR / f"scenarios-{RUN_ID}-{lane}"
        shutil.rmtree(scen_dir, ignore_errors=True)
        scen_dir.mkdir(parents=True)
        animated = write_gif_scenarios(scen_dir, examples, themes, gif_frames, gif_interval)
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
        runs = sorted(COMP.glob("20??-??-??-*"), key=lambda p2: p2.stat().st_mtime)
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

    scen = Path(tempfile.mkdtemp())
    animated = write_gif_scenarios(scen, ["button", "animation"], ["light"], 3, 0.25)
    body = (scen / "animation.toml").read_text()
    parsed = tomllib.loads(body)
    shutil.rmtree(scen, ignore_errors=True)
    assert animated == ["animation"] and not (scen / "button.toml").exists()
    steps = parsed["steps"]
    # The still keeps the full --settle (no per-step key); only the burst frames override it.
    assert steps[0] == {"name": "initial"} and len(steps) == 4, steps
    assert all(s["settle"] == 0.25 and s["name"].startswith("gif") for s in steps[1:]), steps

    seen: list[str] = []
    real_log, sys.modules[__name__].log = log, seen.append          # type: ignore[attr-defined]
    try:
        r = marker_reader("ios", {"cpp": "cpp_xaml"}, lambda k: kind_for("ios", k))
        r("@@PARITY BEGIN button cpp light")
        r("@@PARITY BEGIN entry cpp light")     # a BEGIN with no END = the previous example failed
        r.close("(exited)")                     # type: ignore[attr-defined]
    finally:
        sys.modules[__name__].log = real_log    # type: ignore[attr-defined]
    assert len(seen) == 4 and "framework=cpp_xaml theme=light example=button" in seen[0], seen
    assert "no END" in seen[1] and "(exited)" in seen[3], seen
    print("recapture selftest: mapping + markers OK")
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
                    help="device-free check of the column mapping and the progress markers; exits")
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
    (LOG_DIR / "empty-scenarios").mkdir(parents=True, exist_ok=True)  # no scenario => one idle shot
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
