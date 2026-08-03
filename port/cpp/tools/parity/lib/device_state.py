#!/usr/bin/env python3
"""Pin device chrome to a FIXED state so two captures differ only where the PORT differs.

WHY THIS EXISTS
---------------
iOS and Android captures are FULL-SCREEN (1206x2622 and 1080x2340), so the OS status bar is inside
every frame: clock, battery, wifi/cellular bars, and any notification icon. Those change between the
MAUI reference pass and the port pass — which run minutes or hours apart — so every page carried a
guaranteed diff that had nothing to do with the port. Measured on the committed iOS reference set: the
clock read 13:09, 12:51, 13:21, 12:49, 13:16, 12:55, 0:14, 0:26, 0:03 ... across frames, i.e. not just
drifting but spanning different epochs.

Windows and Mac Catalyst are WINDOW-scoped captures (1024x800), so no system clock is in frame and
neither needs pinning. That asymmetry is why this module only implements iOS and Android.

WHAT IT PINS
------------
  clock         9:41 on both platforms (Apple's canonical keynote time; arbitrary but FIXED)
  battery       100% and NOT charging on both — iOS "discharging", Android plugged=false.
                iOS "charged" was rejected: it draws a charging bolt. simctl accepts only
                charging|charged|discharging — "unplugged" is not a value (it failed loudly).
  wifi          shown, full bars
  cellular      shown, full bars, no data-type glyph
  notifications hidden (Android) — an arriving notification icon is a silent per-page diff
  animations    disabled (Android) — a mid-animation frame is nondeterministic by construction

Both pins are RESTORABLE and both restore paths are exercised by --clear. Leaving a device pinned is
harmless for captures but confusing for a human using it, so the capture scripts clear on exit.

USAGE
    python3 device_state.py --ios [--udid <UDID>]        # pin
    python3 device_state.py --android [--serial <S>]     # pin
    python3 device_state.py --ios --android --clear      # restore both

Or from Python:  from device_state import pin_ios, clear_ios, pin_android, clear_android
"""
from __future__ import annotations

import argparse
import os
import subprocess
import sys
import time

# The canonical fixed values. Change them here and BOTH platforms move together — the point is that
# they never change between two capture passes, not what they happen to be.
CLOCK_IOS = "9:41"
CLOCK_ANDROID = "0941"

DEFAULT_ANDROID_SERIAL = os.environ.get("MAUI_ANDROID_SERIAL", "emulator-5554")
ADB = os.environ.get("MAUI_ADB", "adb")


def _run(cmd: list[str], check: bool = False) -> subprocess.CompletedProcess:
    return subprocess.run(cmd, capture_output=True, text=True, check=check)


# --------------------------------------------------------------------------------------- iOS
def _ios_udid(udid: str | None) -> str:
    if udid:
        return udid
    # Fall back to the single booted simulator. Deliberately FAILS when there is more than one rather
    # than guessing: capture_ios_clean.py drives a hardcoded UDID, and pinning a different device than
    # the one being captured would silently leave the captured device unpinned.
    out = _run(["xcrun", "simctl", "list", "devices", "booted"]).stdout
    ids = [ln.split("(")[1].split(")")[0] for ln in out.splitlines() if "Booted" in ln and "(" in ln]
    if len(ids) != 1:
        raise SystemExit(f"expected exactly one booted simulator, found {len(ids)}; pass --udid")
    return ids[0]


def pin_ios(udid: str | None = None) -> str:
    dev = _ios_udid(udid)
    r = _run(["xcrun", "simctl", "status_bar", dev, "override",
              "--time", CLOCK_IOS,
              "--dataNetwork", "wifi",
              "--wifiMode", "active", "--wifiBars", "3",
              "--cellularMode", "active", "--cellularBars", "4",
              "--batteryState", "discharging", "--batteryLevel", "100"])
    if r.returncode != 0:
        raise SystemExit(f"simctl status_bar override failed: {r.stderr.strip()}")
    print(f"  iOS {dev}: status bar pinned (clock {CLOCK_IOS}, full bars, 100% discharging)")
    return dev


def clear_ios(udid: str | None = None) -> None:
    dev = _ios_udid(udid)
    _run(["xcrun", "simctl", "status_bar", dev, "clear"])
    print(f"  iOS {dev}: status bar restored")


# ----------------------------------------------------------------------------------- Android
def _demo(serial: str, *args: str) -> None:
    _run([ADB, "-s", serial, "shell", "am", "broadcast",
          "-a", "com.android.systemui.demo", *args])


def pin_android(serial: str = DEFAULT_ANDROID_SERIAL) -> None:
    # SystemUI demo mode must be allowed once per device before broadcasts are honoured.
    _run([ADB, "-s", serial, "shell", "settings", "put", "global", "sysui_demo_allowed", "1"])
    _demo(serial, "-e", "command", "enter")
    _demo(serial, "-e", "command", "clock", "-e", "hhmm", CLOCK_ANDROID)
    _demo(serial, "-e", "command", "battery", "-e", "level", "100", "-e", "plugged", "false")
    _demo(serial, "-e", "command", "network", "-e", "wifi", "show", "-e", "level", "4")
    _demo(serial, "-e", "command", "network", "-e", "mobile", "show",
          "-e", "datatype", "none", "-e", "level", "4")
    _demo(serial, "-e", "command", "notifications", "-e", "visible", "false")
    # Animations are a separate nondeterminism source from the status bar: a frame caught mid-transition
    # differs from the same frame caught after it, with no port change involved.
    for k in ("window_animation_scale", "transition_animation_scale", "animator_duration_scale"):
        _run([ADB, "-s", serial, "shell", "settings", "put", "global", k, "0"])
    print(f"  Android {serial}: demo mode pinned (clock {CLOCK_ANDROID}, full bars, "
          f"100% unplugged, notifications hidden, animations off)")


def clear_android(serial: str = DEFAULT_ANDROID_SERIAL) -> None:
    _demo(serial, "-e", "command", "exit")
    for k in ("window_animation_scale", "transition_animation_scale", "animator_duration_scale"):
        _run([ADB, "-s", serial, "shell", "settings", "put", "global", k, "1"])
    print(f"  Android {serial}: demo mode exited, animations restored")


# ------------------------------------------------------------------- SYSTEM-WIDE app theme
# Setting the OS theme, as opposed to handing the app an env var. Since the port derives its theme from
# AppInfo.RequestedTheme (Application.cs:61), and MauiReference now leaves UserAppTheme Unspecified unless
# MAUI_THEME is set, THIS is what makes a light-vs-dark board pass mean anything: both columns read the
# same system source instead of each being pinned by a per-column environment variable.
#
# Every setter returns the PREVIOUS value so the caller can restore it in a finally, and every setter is
# paired with a read-back — a theme that silently failed to apply produces a full board of wrong-theme
# frames that look perfectly healthy to every other check.


def _ios_appearance(udid: str | None, value: str | None = None) -> str:
    dev = _ios_udid(udid)
    if value is None:
        return _run(["xcrun", "simctl", "ui", dev, "appearance"]).stdout.strip()
    _run(["xcrun", "simctl", "ui", dev, "appearance", value])
    got = _run(["xcrun", "simctl", "ui", dev, "appearance"]).stdout.strip()
    if got != value:
        raise SystemExit(f"iOS {dev}: appearance is {got!r} after asking for {value!r}")
    print(f"  iOS {dev}: system appearance -> {value}")
    return got


def set_ios_theme(theme: str, udid: str | None = None) -> str:
    """Set the simulator's SYSTEM appearance; returns the previous value for restore."""
    previous = _ios_appearance(udid)
    _ios_appearance(udid, theme)
    return previous


def set_android_theme(theme: str, serial: str = DEFAULT_ANDROID_SERIAL) -> str:
    """`cmd uimode night yes|no` — the Configuration.uiMode both frameworks read."""
    previous = "dark" if "yes" in _run(
        [ADB, "-s", serial, "shell", "cmd", "uimode", "night"]).stdout.lower() else "light"
    _run([ADB, "-s", serial, "shell", "cmd", "uimode", "night", "yes" if theme == "dark" else "no"])
    got = _run([ADB, "-s", serial, "shell", "cmd", "uimode", "night"]).stdout.lower()
    if ("yes" in got) != (theme == "dark"):
        raise SystemExit(f"Android {serial}: uimode night reads {got.strip()!r} after asking for {theme!r}")
    print(f"  Android {serial}: system night mode -> {theme}")
    return previous


def _ssh(host: str, user: str, command: str) -> str:
    r = _run(["ssh", "-o", "BatchMode=yes", "-o", "ConnectTimeout=10", f"{user}@{host}", command])
    return (r.stdout or r.stderr).strip()


def _macos_dark_mode(host: str, user: str) -> bool:
    """The GUI session's ACTUAL appearance, via System Events."""
    out = _ssh(host, user,
               'osascript -e \'tell application "System Events" to tell appearance preferences '
               'to return dark mode\'')
    return out.strip().lower() == "true"


def set_macos_theme(theme: str, host: str, user: str) -> str:
    """Set the macOS VM's system appearance — drives BOTH the Catalyst and the AppKit columns.

    DRIVEN THROUGH SYSTEM EVENTS, NOT `defaults`. The obvious implementation —
    `defaults write -g AppleInterfaceStyle Dark` / `defaults delete -g AppleInterfaceStyle` — reads and
    writes a preference domain the logged-in GUI session does not honour over SSH. MEASURED on this VM:
    `defaults read -g AppleInterfaceStyle` reported the key ABSENT (i.e. light) at the same moment
    System Events reported `dark mode = true`, and a `defaults delete` left the session dark. A setter
    built on `defaults` therefore "succeeds", passes its own read-back, and captures a whole board in the
    WRONG THEME — the failure is invisible to every check except looking at a frame.

    System Events talks to the real appearance preference, so both the read and the write reflect what a
    launched app will actually see. Apps pick the appearance up at launch, which is all the harness needs
    (one process per page).
    """
    previous = "dark" if _macos_dark_mode(host, user) else "light"
    _ssh(host, user,
         f'osascript -e \'tell application "System Events" to tell appearance preferences '
         f'to set dark mode to {"true" if theme == "dark" else "false"}\'')
    time.sleep(1.5)  # the appearance change is asynchronous; a launch racing it gets the old theme
    got = "dark" if _macos_dark_mode(host, user) else "light"
    if got != theme:
        raise SystemExit(f"macOS {host}: System Events reports {got!r} after asking for {theme!r}")
    print(f"  macOS {host}: system appearance -> {theme} (was {previous})")
    return previous


WINDOWS_THEME_KEY = r"HKCU:\Software\Microsoft\Windows\CurrentVersion\Themes\Personalize"


def set_windows_theme(theme: str, host: str, user: str) -> str:
    """AppsUseLightTheme on the Windows VM (1 = light, 0 = dark). Read by WinUI at app start."""
    def ps(script: str) -> str:
        return _ssh(host, user, f'powershell -NoProfile -Command "{script}"')

    raw = ps(f"(Get-ItemProperty -Path '{WINDOWS_THEME_KEY}').AppsUseLightTheme")
    previous = "light" if raw.strip() != "0" else "dark"
    ps(f"Set-ItemProperty -Path '{WINDOWS_THEME_KEY}' -Name AppsUseLightTheme "
       f"-Value {0 if theme == 'dark' else 1}")
    got = ps(f"(Get-ItemProperty -Path '{WINDOWS_THEME_KEY}').AppsUseLightTheme").strip()
    if (got == "0") != (theme == "dark"):
        raise SystemExit(f"Windows {host}: AppsUseLightTheme reads {got!r} after asking for {theme!r}")
    print(f"  Windows {host}: system app theme -> {theme}")
    return previous


# --------------------------------------------------------------------------------------- CLI
def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("--ios", action="store_true")
    ap.add_argument("--android", action="store_true")
    ap.add_argument("--udid", default=None)
    ap.add_argument("--serial", default=DEFAULT_ANDROID_SERIAL)
    ap.add_argument("--clear", action="store_true", help="restore instead of pin")
    ap.add_argument("--macos", action="store_true")
    ap.add_argument("--windows", action="store_true")
    ap.add_argument("--host", default=None, help="VM host for --macos/--windows")
    ap.add_argument("--user", default=None, help="VM user for --macos/--windows")
    ap.add_argument("--theme", choices=("light", "dark"),
                    help="set the SYSTEM-WIDE theme instead of pinning chrome; prints the previous value")
    a = ap.parse_args()
    if not (a.ios or a.android or a.macos or a.windows):
        ap.error("pass --ios / --android / --macos / --windows")

    if a.theme:  # system-wide theme mode
        if a.ios:
            print(f"previous={set_ios_theme(a.theme, a.udid)}")
        if a.android:
            print(f"previous={set_android_theme(a.theme, a.serial)}")
        if a.macos or a.windows:
            if not (a.host and a.user):
                ap.error("--macos/--windows need --host and --user")
            setter = set_macos_theme if a.macos else set_windows_theme
            print(f"previous={setter(a.theme, a.host, a.user)}")
        return 0

    if a.ios:
        (clear_ios if a.clear else pin_ios)(a.udid)
    if a.android:
        (clear_android if a.clear else pin_android)(a.serial)
    if a.macos or a.windows:
        ap.error("--macos/--windows only support --theme (there is no chrome to pin on a windowed capture)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
