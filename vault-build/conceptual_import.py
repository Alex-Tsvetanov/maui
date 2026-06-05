"""Import dotnet/docs-maui conceptual docs into the Obsidian vault.

- Mirrors docs/ tree under vault/Conceptual/ (preserves URL-like structure).
- Merges existing DocFX frontmatter into Obsidian frontmatter (+ tags, source link).
- Rewrites doc->doc .md links to [[wikilinks]]; leaves image/asset links relative.
- Converts [!INCLUDE] to embeds, :::image to standard image, strips zone/moniker markers.
- Copies media/asset files so relative links resolve.
- Builds a Conceptual MOC from TOC.yml.
"""
import re, shutil, yaml
from pathlib import Path

SRC = Path('docs-maui-src/docs')
DST = Path('vault/Conceptual')
DST.mkdir(parents=True, exist_ok=True)

FM_RE = re.compile(r'^---\s*\n(.*?)\n---\s*\n', re.S)
MD_LINK = re.compile(r'\[([^\]]+)\]\((?!https?://)([^)]+?\.md)(#[^)]*)?\)')
INCLUDE_RE = re.compile(r'\[!INCLUDE\s*\[[^\]]*\]\(([^)]+?\.md)\)\]', re.I)
IMAGE_BLOCK = re.compile(r':::image\b[^:]*?source="([^"]+)"[^:]*?(?:alt-text="([^"]*)")?[^:]*?:::', re.I | re.S)
ZONE_MARK = re.compile(r'^\s*:::\s*(zone|moniker|zone-end|moniker-end|row|row-end|column|column-end)\b.*$', re.I | re.M)

def topic_area(rel):
    return rel.split('/')[0] if '/' in rel else 'general'

def transform(text, rel):
    m = FM_RE.match(text)
    meta = {}
    body = text
    if m:
        try:
            meta = yaml.safe_load(m.group(1)) or {}
        except Exception:
            meta = {}
        body = text[m.end():]
    title = (meta.get('title') or Path(rel).stem.replace('-', ' ').title())
    desc = meta.get('description', '')
    area = topic_area(rel)

    # includes -> embeds
    body = INCLUDE_RE.sub(lambda mm: f"![[{Path(mm.group(1)).stem}]]", body)
    # :::image -> markdown image (keep relative path so copied asset resolves)
    body = IMAGE_BLOCK.sub(lambda mm: f"![{(mm.group(2) or '').strip()}]({mm.group(1)})", body)
    # strip zone/moniker structural markers (keep inner content)
    body = ZONE_MARK.sub('', body)
    # doc->doc md links -> wikilinks by basename
    def _link(mm):
        text_, path, anchor = mm.group(1), mm.group(2), (mm.group(3) or '')
        base = Path(path).stem
        if base.lower() in ('index', 'toc'):
            # link to folder index; use folder name
            base = Path(path).parent.name or base
        if anchor:
            return f"[[{base}{anchor}|{text_}]]"
        return f"[[{base}|{text_}]]"
    body = MD_LINK.sub(_link, body)

    # build obsidian frontmatter
    fm = ['---', f'title: "{str(title).replace(chr(34), chr(39))}"']
    if desc:
        fm.append(f'description: "{str(desc).replace(chr(34), chr(39))[:300]}"')
    fm += ['tags:', '  - conceptual', f'  - area/{area}']
    if meta.get('ms.date'):
        fm.append(f'ms_date: "{meta["ms.date"]}"')
    # source URL on learn
    url_rel = rel[:-3] if rel.endswith('.md') else rel
    if url_rel.endswith('/index'):
        url_rel = url_rel[:-6]
    fm.append(f'source: "https://learn.microsoft.com/dotnet/maui/{url_rel}?view=net-maui-10.0"')
    fm += ['---', '']
    return '\n'.join(fm) + '\n' + body.lstrip('\n')

# Walk and import
md_count = 0
asset_count = 0
for p in SRC.rglob('*'):
    if p.is_dir():
        continue
    rel = p.relative_to(SRC).as_posix()
    out = DST / rel
    out.parent.mkdir(parents=True, exist_ok=True)
    if p.suffix.lower() == '.md':
        try:
            text = p.read_text(encoding='utf-8-sig')
        except Exception:
            continue
        out.write_text(transform(text, rel), encoding='utf-8')
        md_count += 1
    elif p.suffix.lower() in ('.png', '.jpg', '.jpeg', '.gif', '.svg', '.webp', '.bmp', '.pdf'):
        shutil.copy2(p, out)
        asset_count += 1
    # skip yml/json toc etc. (handled separately)

# Build Conceptual MOC from TOC.yml
def render_toc(node, depth=0):
    lines = []
    items = node if isinstance(node, list) else node.get('items', [])
    for it in items:
        name = it.get('name', '')
        href = it.get('href', '')
        sub = it.get('items')
        indent = '  ' * depth
        if href and href.endswith('.md'):
            base = Path(href).stem
            if base in ('index', 'toc'):
                base = Path(href).parent.name or base
            lines.append(f"{indent}- [[{base}|{name}]]")
        elif href and not href.startswith('http'):
            base = Path(href.rstrip('/')).name or name
            lines.append(f"{indent}- **{name}**")
        else:
            lines.append(f"{indent}- **{name}**")
        if sub:
            lines += render_toc(sub, depth + 1)
    return lines

toc_path = SRC / 'TOC.yml'
moc_lines = ['---', 'title: "Conceptual Documentation"', 'tags:', '  - conceptual', '  - moc', '---', '',
             '# .NET MAUI Documentation', '',
             '> [!abstract] Conceptual guides', '> Mirrors the structure of the official .NET MAUI documentation.', '',
             'Source: [learn.microsoft.com/dotnet/maui](https://learn.microsoft.com/en-us/dotnet/maui/?view=net-maui-10.0)', '']
if toc_path.exists():
    try:
        toc = yaml.safe_load(toc_path.read_text(encoding='utf-8-sig'))
        moc_lines += render_toc(toc)
    except Exception as e:
        moc_lines.append(f'_(TOC parse failed: {e})_')
moc_lines += ['', '## See also', '', '- [[Home]]', '- [[_API Reference]]', '']
(DST / '_Conceptual.md').write_text('\n'.join(moc_lines), encoding='utf-8')

print('conceptual md imported:', md_count)
print('assets copied:', asset_count)
