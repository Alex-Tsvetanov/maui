"""Extract the agent-authored overview blocks from the enriched namespace MOCs
so they survive a regeneration. Saves vault-build/overviews.json {ns: block}."""
import json, re
from pathlib import Path

API = Path('vault/API Reference')
TYPE_HEAD = re.compile(r'^## (Classes|Interfaces|Structs|Enums|Delegates|Unknowns)\s*$')
overviews = {}
for moc in API.glob('*/_*.md'):
    ns = moc.parent.name
    lines = moc.read_text(encoding='utf-8-sig').split('\n')
    start = None
    end = None
    for i, ln in enumerate(lines):
        if start is None and ln.strip() == '## Overview':
            start = i
        elif start is not None and TYPE_HEAD.match(ln):
            end = i
            break
    if start is not None:
        block = '\n'.join(lines[start:end]).rstrip() if end else '\n'.join(lines[start:]).rstrip()
        overviews[ns] = block
Path('vault-build/overviews.json').write_text(json.dumps(overviews, ensure_ascii=False), encoding='utf-8')
print('overviews extracted:', len(overviews))
