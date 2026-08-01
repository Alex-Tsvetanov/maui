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


# --------------------------------------------------------------------------------------- CLI
def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("--ios", action="store_true")
    ap.add_argument("--android", action="store_true")
    ap.add_argument("--udid", default=None)
    ap.add_argument("--serial", default=DEFAULT_ANDROID_SERIAL)
    ap.add_argument("--clear", action="store_true", help="restore instead of pin")
    a = ap.parse_args()
    if not (a.ios or a.android):
        ap.error("pass --ios and/or --android")
    if a.ios:
        (clear_ios if a.clear else pin_ios)(a.udid)
    if a.android:
        (clear_android if a.clear else pin_android)(a.serial)
    return 0


if __name__ == "__main__":
    sys.exit(main())
