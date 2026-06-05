"""Rewrite DocFX xref references in conceptual docs to Obsidian wikilinks.

Handles:
  <xref:FQN>                         -> [[NoteTitle|LastSegment]]
  <xref:FQN?displayProperty=...>     -> [[NoteTitle|Type.Member]]
  [text](xref:FQN)                   -> [[NoteTitle|text]]
Unknown FQNs (System.*, etc.) become `LastSegment` inline code.
"""
import json, re
from pathlib import Path

xref = json.loads(Path('vault-build/xref_map.json').read_text(encoding='utf-8-sig'))
DST = Path('vault/Conceptual')

ANGLE = re.compile(r'<xref:([^>?]+)(\?[^>]*)?>')
MDXREF = re.compile(r'\[([^\]]+)\]\(xref:([^)?]+)(\?[^)]*)?\)')

def title_for(fqn):
    fqn = fqn.strip().strip('`')
    return xref.get(fqn)

def last_seg(fqn):
    return fqn.rstrip('`').split('?')[0].rsplit('.', 1)[-1]

def angle_sub(m):
    fqn = m.group(1).strip()
    disp = m.group(2) or ''
    t = title_for(fqn)
    label = fqn.rsplit('.', 1)[-1]
    if 'nameWithType' in disp:
        label = '.'.join(fqn.split('.')[-2:])
    if t:
        return f'[[{t}|{label}]]'
    return f'`{label}`'

def md_sub(m):
    text, fqn = m.group(1), m.group(2).strip()
    t = title_for(fqn)
    if t:
        return f'[[{t}|{text}]]'
    return text

changed = 0
for p in DST.rglob('*.md'):
    s = p.read_text(encoding='utf-8-sig')
    ns = MDXREF.sub(md_sub, s)
    ns = ANGLE.sub(angle_sub, ns)
    if ns != s:
        p.write_text(ns, encoding='utf-8')
        changed += 1
print('conceptual files updated:', changed)
