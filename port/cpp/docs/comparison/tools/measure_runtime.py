#!/usr/bin/env python3
"""Time to first frame (TTFF), measured the way PREDICTIONS.md defines it.

THE DEFINITION IS THE WHOLE POINT
---------------------------------
From the study's validity requirements, agreed before measuring:

    Startup defined framework-agnostically as launch -> first captured frame that is not the
    launch/blank screen, using the capture path the parity board already relies on. Window-exists is
    NOT used as the signal: a window can precede first content paint by a long way on the managed
    side and barely at all on the native side, which would systematically flatter one column.

So this polls SCREENSHOTS and classifies each one, rather than asking the OS when a window appeared.
A frame counts as first-content when it is neither

  * FLAT — a uniform launch/blank screen (std < FLAT_STD over a downscale), nor
  * the .NET purple SPLASH — detected by colour DOMINANCE via capture_guard, because counting
    distinct colours does NOT find it (the iOS splash has 217 distinct colours in a 240x110
    downscale and sails through a "fewer than N colours" test),

and differs from the pre-launch baseline frame. The baseline check is what stops a stale window from
the previous rep being scored as an instant start.

WHAT IS MEASURED, AND WHAT IS NOT
---------------------------------
  ios       simctl terminate -> launch -> poll `simctl io screenshot`      MEASURED
  android   am force-stop    -> am start -> poll `adb exec-out screencap`  MEASURED
  maccatalyst / appkit / windows                                           NOT MEASURED

The VM lanes are deliberately left unmeasured rather than measured badly. Their only capture path is
SSH -> screencapture -> scp, which costs roughly 1-3 s per sample. TTFF for these apps is itself on
the order of 1 s, so the instrument's resolution is the same size as the signal: the number would be
mostly transport latency and would flatter whichever column happens to sit on the faster side of a
round trip. `measure_size.py` records the Windows lanes as `remote_only` for the same class of reason.
A guest-side timer (launch and poll inside the VM, report only the delta) would fix this; until that
exists these lanes report `measured: false` with the reason attached.

REPORTING
---------
Distributions, never a bare mean — startup is a tail phenomenon. Each record carries n, min, median,
p95, max, the full sample list, AND `poll_resolution_s` (the measured median interval between
consecutive polls). A TTFF quoted without its polling resolution implies precision the instrument does
not have.

Cold vs warm is recorded per record: cold force-stops first, warm relaunches an already-resident app.

Usage:
    measure_runtime.py --metric ttff --all-platforms
    measure_runtime.py --metric ttff --platform ios --reps 15 --page label
"""
from __future__ import annotations

import argparse
import json
import os
import statistics
import subprocess
import sys
import tempfile
import time
from pathlib import Path

HERE = Path(__file__).resolve().parent
COMPARISON = HERE.parent                 # …/port/cpp/docs/comparison
CPP = COMPARISON.parents[1]              # …/port/cpp   (parent is docs/, not cpp/)
REPO = COMPARISON.parents[3]             # …/maui       (same arithmetic measure_size.py uses)
OUT = COMPARISON / "measurements.json"

sys.path.insert(0, str(CPP / "tools" / "parity"))
from capture_guard import splash_verdict  # noqa: E402

FLAT_STD = 1.0          # a uniform launch/blank screen has essentially no variance
SETTLE_CAP_S = 30.0     # give up on a rep after this long and record it as a miss
IOS_UDID = os.environ.get("MAUI_SIM_UDID", "C4926671-2FA7-428E-B4A4-480692EE742B")
ANDROID_SERIAL = os.environ.get("MAUI_ANDROID_SERIAL", "emulator-5554")
ADB = os.environ.get("MAUI_ADB", "adb")

IOS_APPS = {
    "maui_xaml": ("dev.mauicpp.mauireference", "MAUI_COMPARE_PAGE"),
    "cpp": ("dev.maui-cpp.ios-gallery", "MAUI_SAMPLE_PAGE"),
    "cpp_xaml": ("dev.maui-cpp.ios-gallery-xaml", "MAUI_SAMPLE_PAGE"),
}
ANDROID_APPS = {
    "maui_xaml": ("dev.mauicpp.mauireference", "MAUI_COMPARE_PAGE"),
    "cpp": ("dev.mauicpp.apphost", "MAUI_SAMPLE_PAGE"),
    "cpp_xaml": ("dev.mauicpp.apphost.xaml", "MAUI_SAMPLE_PAGE"),
}
UNMEASURED_LANES = {
    "maccatalyst": "capture path is SSH -> screencapture -> scp (~1-3 s per sample); TTFF is ~1 s, so "
                   "instrument resolution matches the signal. Needs a guest-side timer.",
    "appkit": "same as maccatalyst — VM capture round trip dominates the measurement.",
    "windows": "app builds and runs on the guest; host-side polling measures transport, not startup.",
}


def sh(*cmd, **kw) -> subprocess.CompletedProcess:
    return subprocess.run(cmd, capture_output=True, text=True, **kw)


def _thumb(path: str):
    import numpy as np
    from PIL import Image
    return np.asarray(Image.open(path).convert("RGB").resize((160, 300))).astype(int)


def _diff_frac(a, b) -> float:
    import numpy as np
    return float((np.abs(a - b).sum(axis=2) > 25).mean())


def frame_state(path: str, baseline, previous) -> tuple[str, object]:
    """('flat'|'splash'|'unchanged'|'transition'|'content'|'unreadable', thumbnail).

    WHY THIS IS NOT A BYTE COMPARE. The first version tested `raw_png_bytes == baseline_bytes` and
    reported an Android cold start of 0.29 s — the same size as the poll interval, and implausible.
    A frame-by-frame probe showed why: the true sequence is baseline -> .NET splash (0.6-1.4 s) ->
    a mid-fade transition (1.7 s) -> stable content (2.0 s). Byte equality is defeated by a single
    changed pixel, so the *home screen* — clock ticking, wallpaper alive — read as "different from
    baseline", and its std easily clears the flatness bar. The measurement was firing on the launcher.

    So: compare by PIXEL FRACTION, and require the frame to be STABLE. A transition frame differs
    from both its neighbours; real content holds still. Stability costs one extra poll and is
    disclosed in `poll_resolution_s`.
    """
    try:
        a = _thumb(path)
    except Exception:
        return "unreadable", None
    if splash_verdict(path)[0]:
        return "splash", a
    if float(a[20:280].std()) < FLAT_STD:
        return "flat", a
    if baseline is not None and _diff_frac(a, baseline) < 0.02:
        return "unchanged", a          # still whatever was on screen before launch
    if previous is None or _diff_frac(a, previous) > 0.005:
        return "transition", a         # still moving — a fade or animation, not first content
    return "content", a


# ------------------------------------------------------------------ per-platform adapters
def ios_stop(bundle):    sh("xcrun", "simctl", "terminate", IOS_UDID, bundle)
def ios_shot(dest):      return sh("xcrun", "simctl", "io", IOS_UDID, "screenshot", "--type=png", dest).returncode == 0


def ios_launch(bundle, page_env, page):
    env = dict(os.environ, **{f"SIMCTL_CHILD_{page_env}": page})
    return sh("xcrun", "simctl", "launch", IOS_UDID, bundle, env=env).returncode == 0


def android_component(pkg):
    out = sh(ADB, "-s", ANDROID_SERIAL, "shell", "cmd", "package", "resolve-activity",
             "-c", "android.intent.category.LAUNCHER", pkg).stdout
    for line in out.splitlines():
        s = line.strip()
        if s.startswith("name="):
            return f"{pkg}/{s.split('=', 1)[1].strip()}"
    return None


def android_stop(pkg):
    # force-stop kills the PROCESS but leaves its last frame composited on screen, so the pre-launch
    # baseline would BE the rendered page and the relaunched app would read as "unchanged" forever
    # (measured: every rep missed at 30 s). HOME returns to the launcher, giving a baseline that the
    # app's first content frame genuinely differs from.
    sh(ADB, "-s", ANDROID_SERIAL, "shell", "am", "force-stop", pkg)
    sh(ADB, "-s", ANDROID_SERIAL, "shell", "input", "keyevent", "KEYCODE_HOME")


def android_shot(dest):
    r = subprocess.run([ADB, "-s", ANDROID_SERIAL, "exec-out", "screencap", "-p"], capture_output=True)
    if r.returncode != 0 or not r.stdout:
        return False
    Path(dest).write_bytes(r.stdout)
    return True


def android_launch(component, page_env, page):
    return sh(ADB, "-s", ANDROID_SERIAL, "shell", "am", "start", "-n", component,
              "--es", page_env, page).returncode == 0


# ------------------------------------------------------------------ the measurement
def one_rep(launch, shoot, stop, cold: bool, tmp: str) -> tuple[float | None, list[float], str]:
    """(ttff_seconds | None, poll_intervals, note). Time is measured from just before launch returns."""
    if cold:
        stop()
        time.sleep(1.0)
    # Baseline: whatever is on screen BEFORE launch. A rep that never changes it is a miss, not a 0.
    baseline = None
    if shoot(tmp):
        try:
            baseline = _thumb(tmp)
        except Exception:
            baseline = None

    t0 = time.monotonic()
    if not launch():
        return None, [], "launch failed"
    polls, last, previous, state = [], t0, None, "none"
    while True:
        if not shoot(tmp):
            continue
        now = time.monotonic()
        polls.append(now - last)
        last = now
        state, thumb = frame_state(tmp, baseline, previous)
        previous = thumb
        if state == "content":
            # `now` is when the STABLE frame was captured; the content was already on screen at the
            # previous poll. Report the earlier bound — the later one would charge the app for the
            # stability check. The gap between them is poll_resolution_s, reported alongside.
            return max(0.0, now - t0 - polls[-1]), polls, ""
        if now - t0 > SETTLE_CAP_S:
            return None, polls, f"no content frame within {SETTLE_CAP_S:.0f}s (last state: {state})"


def measure_platform(platform: str, reps: int, page: str, warm: bool) -> dict:
    if platform in UNMEASURED_LANES:
        return {col: {"measured": False, "reason": UNMEASURED_LANES[platform]}
                for col in ("maui_xaml", "cpp", "cpp_xaml")}
    apps = IOS_APPS if platform == "ios" else ANDROID_APPS
    out: dict = {}
    for col, (ident, page_env) in apps.items():
        if platform == "ios":
            bundle = ident
            launch, shoot, stop = (lambda b=bundle: ios_launch(b, page_env, page)), ios_shot, (lambda b=bundle: ios_stop(b))
            installed = sh("xcrun", "simctl", "get_app_container", IOS_UDID, bundle).returncode == 0
        else:
            comp = android_component(ident)
            installed = comp is not None
            launch, shoot, stop = (lambda c=comp: android_launch(c, page_env, page)), android_shot, (lambda p=ident: android_stop(p))
        if not installed:
            out[col] = {"measured": False, "reason": f"app not installed ({ident})"}
            print(f"  {platform}/{col}: NOT INSTALLED — skipped")
            continue

        samples, all_polls, misses = [], [], []
        with tempfile.TemporaryDirectory() as td:
            tmp = str(Path(td) / "f.png")
            for i in range(reps):
                ttff, polls, note = one_rep(launch, shoot, stop, cold=not warm, tmp=tmp)
                all_polls += polls
                if ttff is None:
                    misses.append(note or "miss")
                else:
                    samples.append(round(ttff, 3))
                print(f"  {platform}/{col} rep {i+1}/{reps}: "
                      + (f"{ttff:.3f}s" if ttff is not None else f"MISS ({note})"))
        rec = {
            "measured": bool(samples),
            "start_kind": "warm" if warm else "cold",
            "page": page,
            "n": len(samples),
            "misses": len(misses),
            "samples_s": samples,
            "poll_resolution_s": round(statistics.median(all_polls), 3) if all_polls else None,
            "note": "TTFF = launch -> first captured frame that is neither flat nor the .NET splash "
                    "and differs from the pre-launch baseline; NOT window-exists. Resolution is the "
                    "median poll interval and bounds the precision of every figure here.",
        }
        if samples:
            rec.update({
                "min_s": min(samples),
                "median_s": round(statistics.median(samples), 3),
                "p95_s": round(sorted(samples)[max(0, int(len(samples) * 0.95) - 1)], 3),
                "max_s": max(samples),
            })
        if misses:
            rec["miss_reasons"] = sorted(set(misses))
        out[col] = rec
    return out


def main(argv=None) -> int:
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--metric", default="ttff", choices=["ttff"])
    ap.add_argument("--platform", action="append",
                    help="ios | android | maccatalyst | appkit | windows (repeatable)")
    ap.add_argument("--all-platforms", action="store_true")
    ap.add_argument("--reps", type=int, default=10)
    ap.add_argument("--page", default="label", help="page key to launch (default: label)")
    ap.add_argument("--warm", action="store_true", help="warm start (default: cold — force-stop first)")
    a = ap.parse_args(argv)

    plats = (["ios", "android", "maccatalyst", "appkit", "windows"] if a.all_platforms
             else (a.platform or ["ios"]))
    data = json.loads(OUT.read_text()) if OUT.exists() else {}
    section = data.setdefault("ttff", {})
    for p in plats:
        print(f"[{p}] measuring TTFF ({'warm' if a.warm else 'cold'}, {a.reps} reps, page={a.page})")
        section[p] = measure_platform(p, a.reps, a.page, a.warm)
    data["ttff_meta"] = {
        "definition": "launch -> first captured frame that is not the launch/blank screen "
                      "(PREDICTIONS.md validity requirements); window-exists deliberately NOT used",
        "reps": a.reps, "page": a.page, "start_kind": "warm" if a.warm else "cold",
    }
    OUT.write_text(json.dumps(data, indent=1))

    print()
    for p, cols in section.items():
        for col, r in cols.items():
            if not r.get("measured"):
                print(f"  {p:12}{col:11} unmeasured — {r.get('reason','')[:70]}")
            else:
                print(f"  {p:12}{col:11} n={r['n']:2} median {r['median_s']:.3f}s  p95 {r['p95_s']:.3f}s  "
                      f"(res {r['poll_resolution_s']}s, {r['misses']} miss)")
    print(f"\nwrote {OUT}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
