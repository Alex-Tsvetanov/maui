"""Fill `> [!todo] Summary pending` placeholders in vault notes, in place.

Lookup sources, merged in order (later wins only if earlier missing):
  1. vault-build/xmldocs.json  (Phase A: real source summaries)
  2. any extra {alias_key: summary|obj} JSON maps passed as argv (Phase B)

Each note carries its identity in frontmatter `aliases` (FQN or FQN.member).
Keys are matched generic-stripped.
"""
import json, re, sys
from pathlib import Path

API = Path('vault/API Reference')

def strip_generic(s):
    return re.sub(r'<[^>]*>', '', s)

xml = json.loads(Path('vault-build/xmldocs.json').read_text(encoding='utf-8-sig'))
# unified map: key -> {summary, remarks?, returns?, params?}
M = {}
for k, v in xml['types'].items():
    M[strip_generic(k)] = v
for k, v in xml['members'].items():
    M.setdefault(strip_generic(k), v)

# extra maps (Phase B): values may be a string or an object
for arg in sys.argv[1:]:
    try:
        extra = json.loads(Path(arg).read_text(encoding='utf-8-sig'))
    except Exception as e:
        print(f'  skip {arg}: {e!r}'); continue
    for k, v in extra.items():
        kk = strip_generic(k)
        if kk in M and M[kk].get('summary'):
            continue
        if isinstance(v, str):
            M[kk] = {'summary': v.strip()}
        elif isinstance(v, dict) and v.get('summary'):
            M[kk] = v

ALIAS_RE = re.compile(r'^aliases:\s*\n\s*-\s*"?([^"\n]+)"?', re.M)
PLACEHOLDER = re.compile(r'> \[!todo\] Summary pending\n> [^\n]*\n')
SEEALSO = '## See also'

def esc(t):
    return t.strip()

filled = 0
scanned = 0
for note in API.rglob('*.md'):
    s = note.read_text(encoding='utf-8-sig')
    if 'Summary pending' not in s:
        continue
    scanned += 1
    am = ALIAS_RE.search(s)
    if not am:
        continue
    key = strip_generic(am.group(1).strip())
    info = M.get(key)
    if not info or not info.get('summary'):
        continue
    summary = esc(info['summary'])
    # replace placeholder callout with the summary paragraph
    ns, nsub = PLACEHOLDER.subn(lambda m: summary + '\n', s, count=1)
    if nsub == 0:
        continue
    # optionally insert Remarks / Returns / Parameters before "See also" if absent
    inserts = []
    if info.get('remarks') and '## Remarks' not in ns:
        inserts.append(f'## Remarks\n\n{esc(info["remarks"])}\n')
    if info.get('returns') and '## Returns' not in ns:
        inserts.append(f'## Returns\n\n{esc(info["returns"])}\n')
    if info.get('params') and '## Parameters' not in ns:
        rows = '\n'.join(f"| `{p}` | {v.replace('|', '\\|')} |" for p, v in info['params'].items())
        inserts.append('## Parameters\n\n| Parameter | Description |\n|---|---|\n' + rows + '\n')
    if inserts and SEEALSO in ns:
        block = '\n'.join(inserts) + '\n'
        ns = ns.replace(SEEALSO, block + SEEALSO, 1)
    note.write_text(ns, encoding='utf-8')
    filled += 1

print(f'pending notes scanned: {scanned}')
print(f'notes filled: {filled}')
print(f'lookup keys available: {len(M)}')
