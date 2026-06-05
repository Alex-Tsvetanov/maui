"""Fix wikilink pipe escapes: `\\|` is only valid inside Markdown table cells.
On non-table lines, convert `\\|` back to plain `|` so Obsidian resolves the alias.
Table rows (lines starting with `|`) keep the escape (required)."""
from pathlib import Path

VAULT = Path('vault')
fixed_files = 0
fixed_lines = 0
for p in VAULT.rglob('*.md'):
    lines = p.read_text(encoding='utf-8-sig').split('\n')
    changed = False
    out = []
    for ln in lines:
        if ln.lstrip().startswith('|'):
            out.append(ln)  # table row: keep \| escape
        else:
            if '\\|' in ln:
                out.append(ln.replace('\\|', '|'))
                fixed_lines += 1
                changed = True
            else:
                out.append(ln)
    if changed:
        p.write_text('\n'.join(out), encoding='utf-8')
        fixed_files += 1
print('files fixed:', fixed_files, '| lines fixed:', fixed_lines)
