#!/usr/bin/env python3
"""Generate the Android section of docs/comparison/README.md from docs/comparison/android/<col>/*.png.

The C++ MAUI port's gallery pages are rendered by the Android app host (a real Activity/APK built by
tools/parity/build_android_apphost.sh, no gradle) on the maui-test emulator and captured via `adb screencap`
into docs/comparison/android/cpp/<key>.png. Per the cross-platform goal every iOS example gets an Android
render here. This emits one Markdown row per page (the union of page_keys.txt + whatever was captured), so
the macOS/iOS/Android boards stay row-for-row aligned. Output to stdout; the caller appends it to README.md
between the <!-- ANDROID:BEGIN --> / <!-- ANDROID:END --> markers (idempotent regeneration).

Future: MAUI (MauiCompare-android) and C++&XAML (gallery_xaml app host) Android columns — this generator
already lays out 3 cells per row and shows whichever exist.

Usage: python3 tools/parity/gen_android_readme_section.py > /tmp/android_section.md
"""
import os
import glob

ROOT = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", ".."))
COMP = os.path.join(ROOT, "docs", "comparison")
AND = os.path.join(COMP, "android")
KEYS_FILE = os.path.join(ROOT, "tools", "parity", "page_keys.txt")
H = 360
COLUMNS = (("maui", "MAUI"), ("cpp", "C++"), ("xaml", "C++&amp;XAML"))


def page_keys():
    keys = []
    if os.path.exists(KEYS_FILE):
        with open(KEYS_FILE, encoding="utf-8") as f:
            keys = [line.strip() for line in f if line.strip()]
    # Include any captured key not in page_keys.txt, so nothing is silently dropped.
    captured = {os.path.splitext(os.path.basename(p))[0]
                for col, _ in COLUMNS for p in glob.glob(os.path.join(AND, col, "*.png"))}
    for k in sorted(captured):
        if k not in keys:
            keys.append(k)
    return keys


def title(key):
    return key.replace("_", " ").title()


def cell(key):
    imgs = []
    for col, label in COLUMNS:
        p = os.path.join(AND, col, f"{key}.png")
        if os.path.exists(p):
            imgs.append(f'<td align="center">{label}<br><img src="android/{col}/{key}.png" height="{H}"></td>')
    if not imgs:
        return "—"
    return "<table><tr>" + "".join(imgs) + "</tr></table>"


def main():
    keys = page_keys()
    n_cpp = len(glob.glob(os.path.join(AND, "cpp", "*.png")))
    out = []
    out.append("## Android — C++ MAUI port on the emulator")
    out.append("")
    out.append(
        "The C++ port's gallery pages rendered by the **Android app host** (a real Activity/APK built by "
        "`tools/parity/build_android_apphost.sh` — aapt2/d8/apksigner, no gradle) on the `maui-test` emulator, "
        "captured via `adb screencap`. Per the cross-platform goal, every iOS example has an Android render "
        "here. Pages built on controls whose **Android handler is not yet implemented** (CollectionView, "
        "Picker, Date/TimePicker, Border/Shapes, …) render blank or partial — the honest current state of the "
        "Android backend (the layout/container handlers + ~12 widget handlers are done; see "
        "[../MACOS_ANDROID_RESUME.md](../MACOS_ANDROID_RESUME.md)). The MAUI and C++&amp;XAML Android columns "
        "are future work (need MauiCompare-android + the gallery_xaml app host); the table already reserves "
        "their cells."
    )
    out.append("")
    out.append(f"**Coverage:** {n_cpp} / 172 pages captured (C++ column).")
    out.append("")
    out.append("| # | Example | Android — MAUI ┃ C++ ┃ C++&amp;XAML |")
    out.append("| --- | --- | --- |")
    for i, k in enumerate(keys, 1):
        out.append(f"| {i} | **{title(k)}** | {cell(k)} |")
    out.append("")
    print("\n".join(out))


if __name__ == "__main__":
    main()
