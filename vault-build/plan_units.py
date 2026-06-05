"""Build balanced Phase-B work units from the remaining 'Summary pending' notes.

Each unit -> vault-build/units/unit_NN.json:
  {"unit": N, "namespaces": [...], "count": K,
   "items": [{"alias","kind","signature","declaring_type","namespace","note_path"}]}
"""
import json, re, math
from pathlib import Path
from collections import defaultdict

API = Path('vault/API Reference')
UNITS = Path('vault-build/units')
UNITS.mkdir(parents=True, exist_ok=True)
for old in UNITS.glob('unit_*.json'):
    old.unlink()

def fm(s, key):
    m = re.search(rf'^{key}:\s*"?([^"\n]+)"?\s*$', s, re.M)
    return m.group(1).strip() if m else ''

ALIAS_RE = re.compile(r'^aliases:\s*\n\s*-\s*"?([^"\n]+)"?', re.M)
SIG_RE = re.compile(r'## Signatures?\s*\n\s*```csharp\s*\n(.*?)\n```', re.S)

by_ns = defaultdict(list)
for note in API.rglob('*.md'):
    s = note.read_text(encoding='utf-8-sig')
    if 'Summary pending' not in s:
        continue
    am = ALIAS_RE.search(s)
    if not am:
        continue
    alias = am.group(1).strip()
    ns = fm(s, 'namespace')
    kind = fm(s, 'member_kind') or fm(s, 'kind') or 'type'
    decl = fm(s, 'declaring_type')
    sigm = SIG_RE.search(s)
    sig = ' / '.join(l.strip() for l in sigm.group(1).split('\n') if l.strip())[:300] if sigm else ''
    if not ns:
        # type notes: namespace is in frontmatter 'namespace' OR derive from alias
        ns = alias.rsplit('.', 1)[0]
    by_ns[ns].append({
        'alias': alias, 'kind': kind, 'signature': sig,
        'declaring_type': decl, 'namespace': ns,
        'note_path': str(note).replace('\\', '/'),
    })

# Build balanced units. Split large namespaces into ~400-item chunks; bin-pack rest.
TARGET = 430
chunks = []  # (label, items)
for ns, items in sorted(by_ns.items(), key=lambda kv: -len(kv[1])):
    items.sort(key=lambda d: d['alias'])
    if len(items) > TARGET:
        n = math.ceil(len(items) / TARGET)
        size = math.ceil(len(items) / n)
        for i in range(0, len(items), size):
            chunks.append((f'{ns} [{i // size + 1}]', items[i:i + size]))
    else:
        chunks.append((ns, items))

# greedy bin-pack small chunks together up to TARGET
units = []
chunks.sort(key=lambda c: -len(c[1]))
for label, items in chunks:
    if len(items) >= TARGET * 0.6:
        units.append([(label, items)])
    else:
        placed = False
        for u in units:
            tot = sum(len(it) for _, it in u)
            if tot + len(items) <= TARGET:
                u.append((label, items)); placed = True; break
        if not placed:
            units.append([(label, items)])

manifest = []
for n, u in enumerate(units, 1):
    items = [it for _, grp in u for it in grp]
    nss = sorted({lbl for lbl, _ in u})
    data = {'unit': n, 'namespaces': nss, 'count': len(items), 'items': items}
    (UNITS / f'unit_{n:02d}.json').write_text(json.dumps(data, ensure_ascii=False, indent=1), encoding='utf-8')
    manifest.append({'unit': n, 'namespaces': nss, 'count': len(items)})

Path('vault-build/units_manifest.json').write_text(json.dumps(manifest, ensure_ascii=False, indent=1), encoding='utf-8')
print(f'units: {len(units)} | total pending items: {sum(m["count"] for m in manifest)}')
for m in manifest:
    print(f"  unit {m['unit']:02d}: {m['count']:4d}  {', '.join(m['namespaces'])[:80]}")
