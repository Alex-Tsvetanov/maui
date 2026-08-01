#!/usr/bin/env python3
"""WS-E android capture — fresh MauiReference (and cpp/xaml) baseline via adb intent extras.

Android `am start` does NOT propagate process env vars, so the page/theme are passed as INTENT EXTRAS
which each app reads on ANDROID (Platform.CurrentActivity.Intent.GetStringExtra). The two app families
read DIFFERENT keys, and BOTH are now sent on every launch:
    MauiReference  -> MAUI_THEME=Light|Dark   (App.xaml.cs:40, sets UserAppTheme)
    cpp / xaml     -> MAUI_APPEARANCE=light|dark (MauiHostActivity.java:70 -> set_platform_app_theme)

BOTH THEMES ARE SUPPORTED (--theme light|dark). This used to be light-only, described as "matching the
legacy convention" — but it was never a convention, it was a gap on two counts. (1) The output filename
hard-coded "_light", and (2) the launch hard-coded MAUI_THEME=Light and sent NO appearance extra at all,
so the cpp/xaml columns were never actually told a theme and simply fell through to their light default.
Nothing downstream was refusing dark: build_comparison_json.py's THEMES loop is platform-independent and
has always looked for android <key>_dark.png; the slot read null only because no such file was written.

--system-night additionally flips the DEVICE into night mode for the pass and restores it afterwards.
The intent extra themes the MAUI/port UI; it does NOT theme Android's own chrome (status bar, nav bar,
system dialogs). Setting night mode makes that chrome match, which is what a real dark-mode user sees
and what every other platform's dark capture already shows. A second dark-only emulator also works —
set MAUI_ANDROID_SERIAL — but is not required, since the night setting is restorable.

Per-page determinism mirrors capture_all_csharp_android.sh: force-stop + wait the process gone, clear
logcat, `am start -W` (blocks to first frame), poll the Displayed/Resumed barrier, dismiss any ANR
dialog, short settle, then `screencap -p` to STDOUT (Python writes the bytes to the repo — dodges the
macOS-TCC "simctl/adb can't write ~/Documents" trap the iOS flow hit; the shell redirect would also
work but stdout-capture is cleaner). Does NOT build/install.

  python3 capture_android_clean.py --app maui --only clipping,web_view
"""
import argparse
import os
import subprocess
import sys
import time

SERIAL = os.environ.get("MAUI_ANDROID_SERIAL", "emulator-5554")
ADB = os.environ.get("MAUI_ADB", "adb")
HERE = os.path.dirname(os.path.abspath(__file__))
CPP = os.path.abspath(os.path.join(HERE, "..", ".."))
PORT = os.path.abspath(os.path.join(CPP, ".."))
PAGES = os.path.join(PORT, "maui-reference", "pages")
REF_CAP = os.path.join(PORT, "maui-reference", "captures", "android")
COMP_CAP = os.path.join(CPP, "docs", "comparison", "captures", "android")

# `out` is now theme-parameterised. It used to hard-code "_light", which is the whole reason this
# board has never had an Android dark column: build_comparison_json.py ALREADY iterates both themes for
# android (its THEMES loop is platform-independent), so the dark slot read null purely because no
# <key>_dark.png file had ever been written — not because anything refused to score it.
APPS = {
    "maui": {"pkg": "dev.mauicpp.mauireference",
             "out": lambda k, th: os.path.join(REF_CAP, f"{k}_{th}.png")},
    "cpp": {"pkg": "com.maui_cpp.gallery",
            "out": lambda k, th: os.path.join(COMP_CAP, "cpp", f"{k}_{th}.png")},
    "xaml": {"pkg": "com.maui_cpp.gallery_xaml",
             "out": lambda k, th: os.path.join(COMP_CAP, "xaml", f"{k}_{th}.png")},
}


def adb(*args, **kw):
    return subprocess.run([ADB, "-s", SERIAL, *args], **kw)


def all_keys():
    return sorted(f[:-5] for f in os.listdir(PAGES)
                  if f.endswith(".xaml") and not f.startswith("gap_"))


def resolve_component(pkg):
    out = adb("shell", "cmd", "package", "resolve-activity", "-c",
              "android.intent.category.LAUNCHER", pkg, capture_output=True, text=True).stdout
    for line in out.splitlines():
        line = line.strip()
        if line.startswith("name="):
            return f"{pkg}/{line[5:].strip()}"
    raise SystemExit(f"could not resolve launcher activity for {pkg} (installed?)")


def wait_gone(pkg):
    for _ in range(40):
        pid = adb("shell", "pidof", pkg, capture_output=True, text=True).stdout.strip()
        if not pid:
            return
        time.sleep(0.25)


def wait_ready(pkg):
    for _ in range(60):
        lc = adb("logcat", "-d", capture_output=True, text=True).stdout
        if f"Displayed {pkg}/" in lc:
            return
        acts = adb("shell", "dumpsys", "activity", "activities", capture_output=True, text=True).stdout
        if "ResumedActivity" in acts and f"{pkg}/" in acts:
            return
        time.sleep(0.25)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--app", required=True, choices=list(APPS))
    ap.add_argument("--only", default="")
    ap.add_argument("--settle", type=float, default=1.8)
    # Default stays "light" so every existing invocation behaves exactly as before.
    ap.add_argument("--theme", default="light", choices=("light", "dark"),
                    help="app theme for this pass; also drives the output filename suffix")
    # The apps take their theme as an intent extra (see the launch below), which themes the MAUI/port UI
    # but NOT Android's own chrome — status bar, nav bar, system dialogs. --system-night additionally
    # flips the DEVICE into night mode so that chrome matches, which is what a real dark-mode user sees
    # and what the other platforms' dark captures already show. It is a device-global setting, so it is
    # restored on exit; run the light and dark passes separately rather than interleaving them.
    ap.add_argument("--system-night", action="store_true",
                    help="also set the emulator UI mode to night for this pass, and restore it after")
    args = ap.parse_args()

    spec = APPS[args.app]
    restore_night = None
    if args.system_night:
        cur = adb("shell", "cmd", "uimode", "night", capture_output=True, text=True).stdout.strip()
        restore_night = "yes" if "yes" in cur.lower() else "no"
        want = "yes" if args.theme == "dark" else "no"
        adb("shell", "cmd", "uimode", "night", want,
            stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        print(f"  uimode night -> {want} (was {restore_night}; will restore)")
        time.sleep(1.5)  # let the system settle before the first launch
    pkg = spec["pkg"]
    component = resolve_component(pkg)
    keys = [k.strip() for k in args.only.split(",") if k.strip()] or all_keys()
    page_env = "MAUI_COMPARE_PAGE" if args.app == "maui" else "MAUI_SAMPLE_PAGE"

    # warm-up (absorb cold-start/JIT)
    adb("shell", "am", "start", "-W", "-n", component, "--es", page_env, keys[0],
        "--es", "MAUI_THEME", "Dark" if args.theme == "dark" else "Light",
        "--es", "MAUI_APPEARANCE", args.theme,
        stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    time.sleep(2.0)
    adb("shell", "am", "force-stop", pkg, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)

    n = 0
    for key in keys:
        adb("shell", "am", "force-stop", pkg, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        wait_gone(pkg)
        adb("logcat", "-c", stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        # BOTH theme extras are sent, every time. The two app families read DIFFERENT keys and each
        # ignores the other's: MauiReference reads MAUI_THEME (App.xaml.cs:40 -> UserAppTheme), while the
        # C++ apphost reads MAUI_APPEARANCE (MauiHostActivity.java:70 -> set_platform_app_theme). The old
        # code sent only MAUI_THEME=Light, so the cpp/xaml columns received NO appearance at all and fell
        # through to their light default — they were never actually being told a theme.
        adb("shell", "am", "start", "-W", "-n", component,
            "--es", page_env, key,
            "--es", "MAUI_THEME", "Dark" if args.theme == "dark" else "Light",
            "--es", "MAUI_APPEARANCE", args.theme,
            stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        wait_ready(pkg)
        adb("shell", "am", "broadcast", "-a", "android.intent.action.CLOSE_SYSTEM_DIALOGS",
            stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        time.sleep(args.settle)
        out = spec["out"](key, args.theme)
        os.makedirs(os.path.dirname(out), exist_ok=True)
        png = adb("exec-out", "screencap", "-p", capture_output=True).stdout
        if not png or len(png) < 1000:
            print(f"  WARN: empty screencap: {key}")
            continue
        with open(out, "wb") as fh:
            fh.write(png)
        n += 1
        print(f"[{n}] {args.app} {key} -> {os.path.relpath(out, PORT)} ({len(png)}B)")
    if restore_night is not None:
        # Restore the DEVICE-GLOBAL night setting we changed. Leaving an emulator in night mode would
        # silently darken the NEXT light pass and read as a port regression.
        adb("shell", "cmd", "uimode", "night", restore_night,
            stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        print(f"  uimode night restored -> {restore_night}")
    print(f"ANDROID_CAPTURE_DONE ({n} shots, theme={args.theme})")


if __name__ == "__main__":
    sys.exit(main())
