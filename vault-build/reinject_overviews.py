"""Re-inject saved namespace overviews into freshly regenerated MOCs,
after the '- [Online namespace docs]' line and before the first type table."""
import json, re
from pathlib import Path

API = Path('vault/API Reference')
overviews = json.loads(Path('vault-build/overviews.json').read_text(encoding='utf-8-sig'))
ONLINE = re.compile(r'^- \[Online namespace docs\]')
done = 0
for ns, block in overviews.items():
    moc = API / ns / ('_' + ns + '.md')
    if not moc.exists():
        continue
    lines = moc.read_text(encoding='utf-8-sig').split('\n')
    if any(l.strip() == '## Overview' for l in lines):
        continue  # already has one
    ins = None
    for i, ln in enumerate(lines):
        if ONLINE.match(ln):
            ins = i + 1
            break
    if ins is None:
        continue
    new = lines[:ins] + ['', block, ''] + lines[ins:]
    moc.write_text('\n'.join(new), encoding='utf-8')
    done += 1
print('overviews re-injected:', done)
