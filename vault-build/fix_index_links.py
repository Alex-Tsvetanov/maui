"""Fix ambiguous [[index]] links in conceptual docs -> the section's landing note
(the parent folder's renamed note). Falls back to [[_Conceptual]]."""
import re
from pathlib import Path

CONC = Path('vault/Conceptual')
notes = {p.stem for p in (Path('vault')).rglob('*.md')}
fixed = 0
for p in CONC.rglob('*.md'):
    folder = p.parent.name
    landing = folder if folder in notes else '_Conceptual'
    s = p.read_text(encoding='utf-8-sig')
    def repl(m):
        global fixed
        alias = m.group(1)
        fixed += 1
        if alias:
            return f'[[{landing}{alias}]]'
        return f'[[{landing}|index]]'
    ns = re.sub(r'\[\[index(\|[^\]]+)?\]\]', repl, s)
    if ns != s:
        p.write_text(ns, encoding='utf-8')
print('index links fixed:', fixed)
