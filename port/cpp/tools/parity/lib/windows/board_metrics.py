#!/usr/bin/env python3
"""Windows board convergence metric.

`pages <= 1.0%` is bimodal at the threshold: a 0.2pp wobble flips pages either way and hides
sub-percent movement in both directions. Mean/median over ALL pages shows whether the port is
actually converging. Usage: board.py [comparison.json] ; --json for machine output.
"""
import json, pathlib, re, statistics, sys

DEFAULT = pathlib.Path(__file__).resolve().parents[4] / 'docs' / 'comparison' / 'comparison.json'
path = next((a for a in sys.argv[1:] if not a.startswith('-')), DEFAULT)
NUM = re.compile(r'Light: SSIM ([\d.]+), ([\d.]+)% .* Dark: SSIM ([\d.]+), ([\d.]+)%')

# Pages whose CONTENT changes between runs, so their score measures the harness rather than the
# port. Measured 2026-07-29 by running two identical full passes against the same binary: these
# swung -49.22pp and -1.86pp with no code change, while every other page stayed within +-0.26pp.
# Averaging them in makes the board mean move for reasons no code change can influence.
#
# web_view ADDED 2026-07-30, and the evidence was sitting in that same noise-floor pass unexamined.
# Between 5fb379b62a and 6b9baecfaa -- the commit literally titled "measure the NOISE FLOOR -- two
# identical runs, no code change" -- web_view's DARK score went 4.32% -> 33.85% (+29.53pp) while its
# light score stayed bit-identical at 2.04%. I only inspected the two largest movers that day and
# never looked at the third, so it went into the board mean for a full campaign.
# MECHANISM: MauiWebView.LoadUrl is `async void` and awaits EnsureCoreWebView2Async() (which spawns
# msedgewebview2.exe) before assigning Source, so the screenshot races WebView2 init. MAUI's own
# 1008x240 content rect has been captured in THREE materially different states across 13 recaptures
# of the ground-truth column, with no MAUI-side change: all-white (WebView2 painted a blank surface),
# the real welcome.html content, and all-(32,32,32) (WebView2 never painted). The port cannot match a
# target that alternates.
# WHY THE ASYMMETRY: the rect is 241,920 px = 29.53% of the frame. In light MAUI's white 255 vs the
# port's 243 page background is a 12-level delta, UNDER pixel_score's DIFF_THRESHOLD of 25, so it
# scores 0.000%; in dark 255-vs-32 is 223 and every pixel counts. One rectangle, one threshold, a
# 200x apparent asymmetry -- the port has been getting a free pass in light on a rectangle it does
# not draw at all.
# The paragraph above ended "do not re-litigate without new run-to-run data". That data now exists,
# and it FALSIFIES the exclusion for both WebView pages, so they are de-listed (2026-08-01).
#
# What was actually wrong: the old claim was "a ported WebView would not fix it", tested by PAINTING
# THE RECT WHITE. That is not a ported WebView -- it is a stand-in for one, and it can only ever match
# whichever MAUI state happens to be white. A REAL Microsoft.UI.Xaml.Controls.WebView2 was ported
# (src/platform/windows/{web_view,hybrid_web_view}_handler.cpp) and both pages went green on both
# themes and both columns:
#     web_view         0.14%/29.68% -> 0.33%/0.00%
#     hybrid_web_view  2.09%/63.21% -> 0.21%/0.19%
# Two real handlers had simply never been written -- MAUI_WINDOWS_SWAPS listed neither unit, so the
# build kept the headless partials, which create no native view at all.
#
# The THREE-STATE alternation described above is real but is a SETTLE artifact, not an unmatched
# target: at --settle 5 all six web_view cells (maui/cpp/xaml x light/dark) render identical content
# (mean 253.20, 177 grey levels) and every port/theme pair scores green. hybrid_web_view is
# byte-stable at both settles (242.17, 239 levels) -- it makes no network round trip. MAUI was still
# observed flipping at the default 1.0s settle (a fresh maui/web_view_dark came back blank white,
# 255.00/1 level, ~5 minutes after a frozen capture with content), so raising the settle for
# WebView2-hosting pages is the durable fix; the scores above are at the STANDARD protocol settle.
#
# context_flyout STAYS excluded, and for a different reason than these two ever had: it hosts a
# WebView on https://bing.com (examples/gallery/pages/context_flyout_page.hpp), i.e. genuinely live
# external content that differs between two captures taken seconds apart. It is the user's explicit
# standing exemption.
VOLATILE = {"context_flyout"}

rows = {}
for p in json.load(open(path)):
    if p.get("name") in VOLATILE:
        continue
    w = p.get('platforms', {}).get('windows')
    if not w:
        continue
    for key, col in (('pixel', 'cpp'), ('pixel_xaml', 'xaml')):
        m = NUM.search((w.get(key) or {}).get('review') or '')
        if m:
            ls, ld, ds, dd = float(m[1]), float(m[2]), float(m[3]), float(m[4])
            rows.setdefault(p['name'], {})[col] = (ls, ld, ds, dd)

if not rows:
    sys.exit(f'no windows pixel scores in {path}')

out = {'pages': len(rows)}
for col in ('cpp', 'xaml'):
    for theme, di, si in (('light', 1, 0), ('dark', 3, 2)):
        # keep SSIM paired with its own page's diff -- sorting the two lists separately
        # silently compares unrelated pages.
        pairs = [(v[col][di], v[col][si]) for v in rows.values() if col in v]
        out[f'{col}_{theme}'] = {
            'mean_diff': round(statistics.fmean(d for d, _ in pairs), 3),
            'median_diff': round(statistics.median(d for d, _ in pairs), 3),
            'green': sum(1 for d, s in pairs if d <= 1.0 and s >= 0.98),
            'diff_le_1pct': sum(1 for d, _ in pairs if d <= 1.0),
            'ssim_ok': sum(1 for _, s in pairs if s >= 0.98),
        }

if '--json' in sys.argv:
    print(json.dumps(out, indent=2))
else:
    print(f"windows: {out['pages']} pages (excluding {len(VOLATILE)} volatile: {', '.join(sorted(VOLATILE))})")
    for k, v in out.items():
        if k == 'pages':
            continue
        print(f"  {k:12} mean {v['mean_diff']:6.2f}%  median {v['median_diff']:6.2f}%  "
              f"diff<=1%: {v['diff_le_1pct']:3}  ssim>=.98: {v['ssim_ok']:3}  BOTH: {v['green']:3}")
    worst = sorted(rows.items(), key=lambda kv: -kv[1].get('cpp', (0, 0))[1])[:12]
    print("\nworst cpp light:")
    for name, v in worst:
        print(f"  {v['cpp'][1]:6.2f}%  {name}")
