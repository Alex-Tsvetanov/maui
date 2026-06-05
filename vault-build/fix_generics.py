"""Wikilink targets must match note basenames, which were filesystem-sanitized
(< > -> { }, and other illegal chars -> _). Apply the same sanitization to the
TARGET portion of every wikilink (keep the alias for display)."""
import re
from pathlib import Path

VAULT = Path('vault')
INVALID = re.compile(r'[:"/\\?*]')
def sanitize_target(t):
    t = t.replace('<', '{').replace('>', '}')
    t = INVALID.sub('_', t)
    return t

# match [[ target (#anchor)? ( \| or | alias)? ]]
LINK = re.compile(r'(\[\[)([^\]\|#]+)((?:#[^\]\|]+)?)((?:\\?\|[^\]]+)?\]\])')

def repl(m):
    open_, target, anchor, rest = m.group(1), m.group(2), m.group(3), m.group(4)
    return open_ + sanitize_target(target) + anchor + rest

fixed_files = 0
fixed_links = 0
for p in VAULT.rglob('*.md'):
    s = p.read_text(encoding='utf-8-sig')
    def count_repl(m):
        global fixed_links
        new = repl(m)
        if new != m.group(0):
            fixed_links += 1
        return new
    ns = LINK.sub(count_repl, s)
    if ns != s:
        p.write_text(ns, encoding='utf-8')
        fixed_files += 1
print('files fixed:', fixed_files, '| links fixed:', fixed_links)
