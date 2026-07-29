#!/usr/bin/env python3
"""Windows board convergence metric.

`pages <= 1.0%` is bimodal at the threshold: a 0.2pp wobble flips pages either way and hides
sub-percent movement in both directions. Mean/median over ALL pages shows whether the port is
actually converging. Usage: board.py [comparison.json] ; --json for machine output.
"""
import json, pathlib, re, statistics, sys

DEFAULT = pathlib.Path(__file__).resolve().parents[3] / 'docs' / 'comparison' / 'comparison.json'
path = next((a for a in sys.argv[1:] if not a.startswith('-')), DEFAULT)
NUM = re.compile(r'Light: SSIM ([\d.]+), ([\d.]+)% .* Dark: SSIM ([\d.]+), ([\d.]+)%')

# Pages whose CONTENT changes between runs, so their score measures the harness rather than the
# port. Measured 2026-07-29 by running two identical full passes against the same binary: these two
# swung -49.22pp and -1.86pp with no code change, while every other page stayed within +-0.26pp.
# Averaging them in makes the board mean move for reasons no code change can influence.
VOLATILE = {"context_flyout", "hybrid_web_view"}

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
