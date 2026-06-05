import re
from pathlib import Path
from collections import Counter

VAULT = Path('vault')
notes = {}
for p in VAULT.rglob('*.md'):
    notes[p.stem] = p
# also assets for embeds
assets = set()
for p in VAULT.rglob('*'):
    if p.is_file() and p.suffix.lower() in ('.png', '.jpg', '.jpeg', '.gif', '.svg', '.webp', '.pdf'):
        assets.add(p.name)
        assets.add(p.stem)

WIKILINK = re.compile(r'(?<!\!)\[\[([^\]#]+?)(?:#[^\]]+?)?(?:\\?\|[^\]]+?)?\]\]')
EMBED = re.compile(r'\!\[\[([^\]\|#]+)')

def link_target(raw):
    # strip alias (after \| or |) and anchor
    t = re.split(r'\\?\|', raw, 1)[0]
    t = t.split('#', 1)[0]
    return t.strip()

total = 0
broken = 0
broken_samples = Counter()
files = 0
for p in VAULT.rglob('*.md'):
    files += 1
    s = p.read_text(encoding='utf-8-sig')
    for m in WIKILINK.finditer(s):
        target = link_target(m.group(1))
        total += 1
        if target not in notes:
            broken += 1
            broken_samples[target] += 1

print(f'files scanned: {files}')
print(f'wikilinks: {total}')
print(f'resolving: {total - broken} ({100*(total-broken)/max(total,1):.1f}%)')
print(f'broken: {broken}')
print('top unresolved targets:')
for t, c in broken_samples.most_common(25):
    print(f'  {c:5d}  {t}')
