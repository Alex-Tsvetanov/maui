"""Finalize the vault: rename index notes, add API<->Conceptual backlinks,
write Home.md and Obsidian Base files."""
import json, re
from pathlib import Path

VAULT = Path('vault')
CONC = VAULT / 'Conceptual'
API = VAULT / 'API Reference'

# 1) Rename Conceptual index.md -> <folder>.md so wikilinks resolve unambiguously
renamed = 0
for idx in list(CONC.rglob('index.md')):
    folder = idx.parent
    target = folder / (folder.name + '.md')
    if target.exists():
        target = folder / (folder.name + ' (overview).md')
    if not target.exists():
        idx.rename(target)
        renamed += 1

# 2) API -> Conceptual backlinks.
# Build conceptual index: normalized name -> note basename (for controls/features).
def norm(s):
    return re.sub(r'[^a-z0-9]', '', s.lower())

conc_index = {}
for md in CONC.rglob('*.md'):
    rel = md.relative_to(CONC).as_posix()
    area = rel.split('/')[0]
    if area in ('user-interface', 'fundamentals', 'platform-integration', 'data-cloud', 'xaml'):
        conc_index.setdefault(norm(md.stem), (md.stem, rel))

model = json.loads(Path('vault-build/api_model.json').read_text(encoding='utf-8-sig'))
xref = json.loads(Path('vault-build/xref_map.json').read_text(encoding='utf-8-sig'))
# reverse: type fqn (generic-stripped) -> note title
backlinked = 0
for fqn, t in model['types'].items():
    key = norm(t['name'].split('<')[0])
    if key in conc_index and len(key) > 3:
        stem, rel = conc_index[key]
        title = xref.get(re.sub(r'<[^>]*>', '', fqn))
        if not title:
            continue
        # find the note file
        nsdir = API / re.sub(r'[<>:"/\\|?*]', '_', t['namespace'])
        note = nsdir / (re.sub(r'[<>:"/\\|?*]', '_', title) + '.md')
        if note.exists():
            txt = note.read_text(encoding='utf-8-sig')
            if '## Guide' not in txt and '## See also' in txt:
                txt = txt.replace('## See also', f'## Guide\n\n- 📖 Conceptual: [[{stem}]]\n\n## See also', 1)
                note.write_text(txt, encoding='utf-8')
                backlinked += 1

# 3) Home.md
ns_count = len(model['namespaces'])
type_count = len(model['types'])
home = f'''---
title: "Home"
tags:
  - moc
  - home
---

# .NET MAUI Knowledge Vault

> [!abstract] A complete offline mirror of the .NET MAUI framework
> Conceptual documentation (from the official docs) + the full public API reference,
> cross-linked. Generated from the `dotnet/maui` source and `dotnet/docs-maui`.

## Start here

- [[_Conceptual|📚 Conceptual Documentation]] — guides mirroring [learn.microsoft.com/dotnet/maui](https://learn.microsoft.com/en-us/dotnet/maui/?view=net-maui-10.0)
- [[_API Reference|🧩 API Reference]] — {type_count} public types across {ns_count} namespaces

## Browse

- [[API Reference.base|🗂️ API Reference (database view)]]
- [[Controls.base|🎛️ Controls (database view)]]

## Key namespaces

- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls]] — pages, layouts, views, cells
- [[_Microsoft.Maui|Microsoft.Maui]] — core abstractions (IView, IElement, handlers contracts)
- [[_Microsoft.Maui.Graphics|Microsoft.Maui.Graphics]] — drawing, colors, geometry
- [[_Microsoft.Maui.ApplicationModel|Microsoft.Maui.ApplicationModel]] — app/device APIs (Essentials)
- [[_Microsoft.Maui.Controls.Shapes|Microsoft.Maui.Controls.Shapes]] — vector shapes & paths

## How this vault is organized

> [!info] Structure
> - **Conceptual/** — narrative guides, mirroring the online docs tree.
> - **API Reference/`<Namespace>`/** — one note per public type, each linking to its member notes.
> - **API Reference/`<Namespace>`/`<Type>`/** — one note per member (methods, properties, events, fields), overloads grouped.
'''
(VAULT / 'Home.md').write_text(home, encoding='utf-8')

# 4) Obsidian Bases
api_base = '''filters:
  and:
    - 'file.hasTag("api")'
properties:
  note.kind:
    displayName: "Kind"
  note.namespace:
    displayName: "Namespace"
  note.member_kind:
    displayName: "Member"
views:
  - type: table
    name: "All types"
    filters:
      and:
        - 'file.hasTag("kind/class") || file.hasTag("kind/interface") || file.hasTag("kind/struct") || file.hasTag("kind/enum")'
    order:
      - file.name
      - namespace
      - kind
    groupBy:
      property: namespace
      direction: ASC
  - type: table
    name: "Members"
    filters:
      and:
        - 'note.member_kind'
    order:
      - file.name
      - declaring_type
      - member_kind
'''
(VAULT / 'API Reference.base').write_text(api_base, encoding='utf-8')

controls_base = '''filters:
  and:
    - 'file.hasTag("api")'
    - 'note.namespace == "Microsoft.Maui.Controls"'
views:
  - type: cards
    name: "Controls"
    filters:
      and:
        - 'file.hasTag("kind/class")'
    order:
      - file.name
      - kind
  - type: table
    name: "All Controls types"
    order:
      - file.name
      - kind
'''
(VAULT / 'Controls.base').write_text(controls_base, encoding='utf-8')

print('index notes renamed:', renamed)
print('API->Conceptual backlinks added:', backlinked)
print('Home.md + 2 bases written')
