#!/usr/bin/env python3
# Regenerate README.md — the theme-matched iOS pixel-parity tracker.
#
# Compares the C++ port vs real .NET MAUI on iOS, theme-for-theme (light-vs-light, dark-vs-dark), from the
# 4-way capture dirs (csharp_ios_{light,dark} / cpp_ios_{light,dark}). Per-page parity status is tracked in
# parity_status.json ({key: {"light": <s>, "dark": <s>}}, s in match|minor|diff|blank|pending) and rendered
# as a glanceable grid + progress counter. Re-run after editing parity_status.json or re-capturing:
#   python3 docs/comparison/gen_parity_readme.py
import json
import os

HERE = os.path.dirname(os.path.abspath(__file__))
STATUS_PATH = os.path.join(HERE, "parity_status.json")
KEYS = sorted(f[:-4] for f in os.listdir(os.path.join(HERE, "csharp_ios_light")) if f.endswith(".png"))

# Curated display titles (the 6 keys whose C# page name differs from the C++ gallery page); others title-cased.
TITLE = {
    "controls_stack": "Control stack",
    "alignment": "Layout alignment (Start/Center/End/Fill)",
    "shapes": "Shapes",
    "border": "Border",
    "collectionview": "CollectionView",
    "gradient": "Gradient brushes",
}
EMOJI = {"match": "🟢", "minor": "🟡", "diff": "🔴", "blank": "⬛", "pending": "⬜"}
LABEL = {"match": "match", "minor": "minor", "diff": "diff", "blank": "blank", "pending": "pending"}


def title(k):
    return TITLE.get(k, k.replace("_", " ").title())


def load_status():
    data = json.load(open(STATUS_PATH)) if os.path.exists(STATUS_PATH) else {}
    changed = False
    for k in KEYS:
        if k not in data:
            data[k] = {"light": "pending", "dark": "pending"}
            changed = True
    if changed:
        json.dump(data, open(STATUS_PATH, "w"), indent=2, sort_keys=True)
    return data


def combined(st):
    pair = (st.get("light", "pending"), st.get("dark", "pending"))
    if "diff" in pair or "blank" in pair:
        return "diff" if "diff" in pair else "blank"
    if "minor" in pair:
        return "minor"
    if pair == ("match", "match"):
        return "match"
    return "pending"


def main():
    status = load_status()
    counts = {s: 0 for s in EMOJI}
    for k in KEYS:
        counts[combined(status[k])] += 1
    total = len(KEYS)
    o = []
    o.append("# C++ port vs .NET MAUI — iOS pixel-parity tracker")
    o.append("")
    o.append("Theme-matched iOS comparison: each page rendered by **real .NET MAUI** vs the **C++ port**, on the "
             "same iPhone 17 simulator, compared **light-vs-light** and **dark-vs-dark**. Both stacks render "
             "native-default controls + the system font (the C# app's `dotnet new maui` default `Styles.xaml` + "
             "OpenSans are stripped; appearance forced via `MAUI_THEME` / `MAUI_APPEARANCE`). Goal: pixel-perfect "
             "parity, fixed example-by-example.")
    o.append("")
    o.append(f"**Progress: {counts['match']} / {total} 🟢 matched** "
             f"· {counts['minor']} 🟡 minor · {counts['diff'] + counts['blank']} 🔴 diff · {counts['pending']} ⬜ pending")
    o.append("")
    o.append("Status legend: 🟢 pixel-match (both themes) · 🟡 minor diff · 🔴 notable diff to fix · "
             "⬜ not yet reviewed. Per-theme verdicts in `parity_status.json`.")
    o.append("")
    o.append("> macOS / Mac Catalyst 4-way comparison is **Phase 2** (pending: aligning the gallery window size "
             "to the C# window). The earlier 2-way macOS grid + notes live in [PARITY_FINDINGS.md](PARITY_FINDINGS.md).")
    o.append("")
    o.append("| Page | Status | .NET MAUI (light) | C++ (light) | .NET MAUI (dark) | C++ (dark) |")
    o.append("| --- | :---: | --- | --- | --- | --- |")
    for k in KEYS:
        st = status[k]
        c = combined(st)
        badge = f"{EMOJI[c]}<br>L:{LABEL[st.get('light','pending')]}<br>D:{LABEL[st.get('dark','pending')]}"
        o.append(
            f"| {title(k)} | {badge} "
            f"| ![](csharp_ios_light/{k}.png) | ![](cpp_ios_light/{k}.png) "
            f"| ![](csharp_ios_dark/{k}.png) | ![](cpp_ios_dark/{k}.png) |"
        )
    o.append("")
    open(os.path.join(HERE, "README.md"), "w").write("\n".join(o))
    print(f"README: {total} pages — {counts['match']} matched, {counts['minor']} minor, "
          f"{counts['diff'] + counts['blank']} diff, {counts['pending']} pending")


if __name__ == "__main__":
    main()
