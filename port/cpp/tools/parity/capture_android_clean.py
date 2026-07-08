#!/usr/bin/env python3
"""WS-E android capture — fresh MauiReference (and cpp/xaml) baseline via adb intent extras.

Android `am start` does NOT propagate process env vars, so the page/theme are passed as INTENT EXTRAS
(--es MAUI_COMPARE_PAGE <key> --es MAUI_THEME Light) which MauiReference's App.xaml.cs reads on ANDROID
(Platform.CurrentActivity.Intent.GetStringExtra). Android parity is LIGHT-ONLY (the board's android
dark slot is null), matching the legacy convention.

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

APPS = {
    "maui": {"pkg": "dev.mauicpp.mauireference",
             "out": lambda k: os.path.join(REF_CAP, f"{k}_light.png")},
    "cpp": {"pkg": "com.maui_cpp.gallery",
            "out": lambda k: os.path.join(COMP_CAP, "cpp", f"{k}_light.png")},
    "xaml": {"pkg": "com.maui_cpp.gallery_xaml",
             "out": lambda k: os.path.join(COMP_CAP, "xaml", f"{k}_light.png")},
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
    args = ap.parse_args()

    spec = APPS[args.app]
    pkg = spec["pkg"]
    component = resolve_component(pkg)
    keys = [k.strip() for k in args.only.split(",") if k.strip()] or all_keys()
    page_env = "MAUI_COMPARE_PAGE" if args.app == "maui" else "MAUI_SAMPLE_PAGE"

    # warm-up (absorb cold-start/JIT)
    adb("shell", "am", "start", "-W", "-n", component, "--es", page_env, keys[0],
        stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    time.sleep(2.0)
    adb("shell", "am", "force-stop", pkg, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)

    n = 0
    for key in keys:
        adb("shell", "am", "force-stop", pkg, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        wait_gone(pkg)
        adb("logcat", "-c", stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        adb("shell", "am", "start", "-W", "-n", component,
            "--es", page_env, key, "--es", "MAUI_THEME", "Light",
            stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        wait_ready(pkg)
        adb("shell", "am", "broadcast", "-a", "android.intent.action.CLOSE_SYSTEM_DIALOGS",
            stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        time.sleep(args.settle)
        out = spec["out"](key)
        os.makedirs(os.path.dirname(out), exist_ok=True)
        png = adb("exec-out", "screencap", "-p", capture_output=True).stdout
        if not png or len(png) < 1000:
            print(f"  WARN: empty screencap: {key}")
            continue
        with open(out, "wb") as fh:
            fh.write(png)
        n += 1
        print(f"[{n}] {args.app} {key} -> {os.path.relpath(out, PORT)} ({len(png)}B)")
    print(f"ANDROID_CAPTURE_DONE ({n} shots)")


if __name__ == "__main__":
    sys.exit(main())
