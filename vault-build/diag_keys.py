import json, re
from pathlib import Path
from collections import Counter

API = Path('vault/API Reference')
def strip_generic(s): return re.sub(r'<[^>]*>', '', s)

xml = json.loads(Path('vault-build/xmldocs.json').read_text(encoding='utf-8-sig'))
M = set(strip_generic(k) for k in xml['types']) | set(strip_generic(k) for k in xml['members'])

ALIAS_RE = re.compile(r'^aliases:\s*\n\s*-\s*"?([^"\n]+)"?', re.M)
hit = 0; miss = 0
miss_samples = []
miss_by_ns = Counter()
for note in API.rglob('*.md'):
    s = note.read_text(encoding='utf-8-sig')
    if 'Summary pending' not in s:
        continue
    am = ALIAS_RE.search(s)
    if not am:
        continue
    key = strip_generic(am.group(1).strip())
    if key in M:
        hit += 1
    else:
        miss += 1
        if len(miss_samples) < 30:
            miss_samples.append(key)
        ns = '.'.join(key.split('.')[:3])
        miss_by_ns[ns] += 1

print(f'pending-with-key-in-xmldocs (should have been filled): {hit}')
print(f'pending-with-NO-key (genuinely undocumented OR key mismatch): {miss}')
print('sample misses:')
for s in miss_samples:
    print('  ', s)
print('miss by top namespace prefix:')
for ns, c in miss_by_ns.most_common(12):
    print(f'  {c:5d}  {ns}')
