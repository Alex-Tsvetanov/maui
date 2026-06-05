"""Deterministically generate the API Reference portion of the Obsidian vault
from api_model.json + xmldocs.json.

Emits:
  vault/API Reference/_API Reference.md            (root API MOC)
  vault/API Reference/<Namespace>/_<Namespace>.md  (namespace MOC)
  vault/API Reference/<Namespace>/<Type>.md        (type note)
  vault/API Reference/<Namespace>/<Type>/<Member>.md (member note, overloads collapsed)
"""
import json, re, os
from pathlib import Path
from collections import defaultdict

VAULT = Path('vault')
APIDIR = VAULT / 'API Reference'

model = json.loads(Path('vault-build/api_model.json').read_text(encoding='utf-8-sig'))
xml = json.loads(Path('vault-build/xmldocs.json').read_text(encoding='utf-8-sig'))
xtypes = xml['types']; xmembers = xml['members']

TFM_FRIENDLY = {
    'net': 'All platforms (.NET)', 'net-android': 'Android', 'net-ios': 'iOS',
    'net-maccatalyst': 'Mac Catalyst', 'net-macos': 'macOS', 'net-tizen': 'Tizen',
    'net-windows': 'Windows', 'netstandard': '.NET Standard', 'netstandard2.0': '.NET Standard 2.0',
}
PLATFORM_ORDER = ['net', 'net-android', 'net-ios', 'net-maccatalyst', 'net-macos', 'net-windows', 'net-tizen', 'netstandard', 'netstandard2.0']

INVALID = re.compile(r'[<>:"/\\|?*]')
def fsname(s):
    s = s.replace('<', '{').replace('>', '}')
    s = INVALID.sub('_', s)
    s = s.strip().strip('.')
    return s[:180]

def strip_generic(fqn):
    return re.sub(r'<[^>]*>', '', fqn)

def gen_display(fqn):
    # turn Foo<T> human readable, keep generic
    return fqn.rsplit('.', 1)[-1]

types = model['types']
namespaces = model['namespaces']

# ---- group members per type by (kind, name), collapsing overloads ----
for fqn, t in types.items():
    grouped = defaultdict(lambda: {'kind': None, 'name': None, 'signatures': [], 'tfms': set()})
    for m in t['members']:
        key = (m['kind'], m['name'])
        g = grouped[key]
        g['kind'] = m['kind']; g['name'] = m['name']
        g['signatures'].append({'signature': m['signature'], 'returns': m.get('returns'), 'raw': m.get('raw')})
        g['tfms'].update(m.get('tfms', []))
    t['grouped'] = []
    for (kind, name), g in grouped.items():
        g['tfms'] = sorted(g['tfms'])
        t['grouped'].append(g)

# ---- compute globally unique basenames ----
# type basenames
type_title = {}
basename_count = defaultdict(int)
# first pass desired
desired = {}
for fqn, t in types.items():
    desired[fqn] = t['name']  # simple, may include <T>
    basename_count[fsname(t['name']).lower()] += 1
for fqn, t in types.items():
    base = t['name']
    if basename_count[fsname(base).lower()] > 1:
        nslast = t['namespace'].rsplit('.', 1)[-1] if t['namespace'] else 'global'
        base = f"{t['name']} ({nslast})"
        # if still colliding, use full namespace
    type_title[fqn] = base

# ensure type titles unique
seen = defaultdict(int)
for fqn in list(type_title):
    key = fsname(type_title[fqn]).lower()
    seen[key] += 1
    if seen[key] > 1:
        ns = types[fqn]['namespace']
        type_title[fqn] = f"{types[fqn]['name']} ({ns})"

# member basenames: "<TypeTitleSimple>.<member>" ; ensure unique globally
member_title = {}  # (fqn, kind, name) -> basename
mseen = defaultdict(int)
for fqn, t in types.items():
    tsimple = type_title[fqn]
    for g in t['grouped']:
        base = f"{tsimple}.{g['name']}"
        key = fsname(base).lower()
        mseen[key] += 1
        if mseen[key] > 1:
            base = f"{tsimple}.{g['name']} ({g['kind']})"
            key2 = fsname(base).lower()
            mseen[key2] += 1
            if mseen[key2] > 1:
                base = f"{tsimple}.{g['name']} ({g['kind']} {mseen[key2]})"
        member_title[(fqn, g['kind'], g['name'])] = base

def platforms_table(tfms):
    if not tfms:
        return ''
    rows = []
    tset = set(tfms)
    for p in PLATFORM_ORDER:
        if p in tset:
            rows.append(f"| {TFM_FRIENDLY.get(p, p)} | ✅ |")
    if 'net' in tset:
        # 'net' means cross-platform base; note it
        pass
    if not rows:
        return ''
    return "| Platform | Available |\n|---|---|\n" + "\n".join(rows) + "\n"

def yaml_list(items):
    return "\n".join(f"  - {i}" for i in items)

KIND_SECTION = [
    ('constructor', 'Constructors'),
    ('property', 'Properties'),
    ('method', 'Methods'),
    ('event', 'Events'),
    ('field', 'Fields'),
    ('const', 'Constants'),
    ('operator', 'Operators'),
]

LEAD_MOD = re.compile(r'^~?(abstract|const|override|sealed|static|virtual|readonly|extern|async|partial|new|unsafe|volatile|ref|implicit|explicit|required)\b')
def pretty_member_sigs(decl_fqn, kind, name, sigs):
    """Return a list of readable C#-ish signature lines."""
    declg = strip_generic(decl_fqn)
    out = []
    if kind == 'property':
        rettype = None; has_get = False; has_set = False; mods = ''
        for s in sigs:
            raw = (s.get('raw') or s.get('signature') or '').strip()
            lead = ''
            mm = LEAD_MOD.match(raw)
            if raw.startswith('~'):
                raw2 = raw[1:].lstrip()
            else:
                raw2 = raw
            if raw2.endswith('.get') or '.get ->' in raw2 or raw2.split('->')[0].strip().endswith('.get'):
                has_get = True
                if s.get('returns'):
                    rettype = s['returns']
            if raw2.split('->')[0].strip().endswith('.set'):
                has_set = True
            for kw in ('static', 'abstract', 'virtual', 'override', 'sealed'):
                if re.search(r'\b' + kw + r'\b', raw):
                    mods = kw + ' '
        rettype = rettype or 'object'
        acc = '{ get; set; }' if (has_get and has_set) else ('{ get; }' if has_get else '{ set; }')
        return [f'{mods}{rettype} {name} {acc}']
    for s in sigs:
        raw = (s.get('raw') or s.get('signature') or '').strip()
        if raw.startswith('~'):
            raw = raw[1:].lstrip()
        lhs, _, ret = raw.partition(' -> ')
        # strip declaring type prefix to start at member name
        prefix = declg + '.'
        idx = lhs.find(prefix)
        if idx != -1:
            lhs = lhs[:idx] + lhs[idx + len(prefix):]
        if ret:
            if kind in ('method', 'constructor', 'operator'):
                out.append(f'{ret} {lhs}'.strip())
            else:  # field/const/event
                out.append(f'{ret} {lhs}'.strip())
        else:
            out.append(lhs.strip())
    return out

def xml_type(fqn):
    k = strip_generic(fqn)
    return xtypes.get(k) or xtypes.get(k.replace('+', '.'))

def xml_member(fqn, name):
    k = strip_generic(fqn) + '.' + name
    return xmembers.get(k)

count_types = 0
count_members = 0

# ---- emit type + member notes ----
for fqn, t in types.items():
    ns = t['namespace']
    nsdir = APIDIR / fsname(ns)
    nsdir.mkdir(parents=True, exist_ok=True)
    ttitle = type_title[fqn]
    tfile = nsdir / (fsname(ttitle) + '.md')

    xd = xml_type(fqn) or {}
    kind = xd.get('kind') or t['kind']
    summary = xd.get('summary', '')
    remarks = xd.get('remarks', '')

    fm = ['---',
          f'title: "{ttitle}"',
          'tags:',
          '  - api',
          f'  - kind/{kind}',
          f'  - ns/{fsname(ns).replace(".", "-")}',
          'aliases:',
          f'  - "{fqn}"',
          f'namespace: "{ns}"',
          f'kind: {kind}',
          'platforms:',
          yaml_list([TFM_FRIENDLY.get(p, p) for p in PLATFORM_ORDER if p in set(t['tfms'])]) or '  - All platforms (.NET)',
          f'assemblies:',
          yaml_list(t['assemblies']) or '  - Microsoft.Maui',
          '---', '']
    body = [f'# {ttitle}', '',
            f'> [!abstract] {kind.capitalize()} in `{ns}`',
            f'> Full name: `{fqn}`', '']
    if summary:
        body += [summary, '']
    else:
        body += ['> [!todo] Summary pending', '> No XML documentation summary was found in source for this type.', '']

    pt = platforms_table(t['tfms'])
    if pt:
        body += ['## Platforms', '', pt, '']

    # member sections with links
    bykind = defaultdict(list)
    for g in t['grouped']:
        bykind[g['kind']].append(g)
    for kkey, ktitle in KIND_SECTION:
        items = sorted(bykind.get(kkey, []), key=lambda g: g['name'])
        if not items:
            continue
        body += [f'## {ktitle}', '', '| Name | Summary |', '|---|---|']
        for g in items:
            mbase = member_title[(fqn, g['kind'], g['name'])]
            xm = xml_member(fqn, g['name']) or {}
            msum = (xm.get('summary', '') or '').replace('|', '\\|')
            if len(msum) > 160:
                msum = msum[:157] + '…'
            body.append(f"| [[{fsname(mbase)}\\|{g['name']}]] | {msum} |")
        body.append('')

    if remarks:
        body += ['## Remarks', '', remarks, '']

    body += ['## See also', '',
             f"- [[_{fsname(ns)}|{ns} namespace]]",
             f"- [Online API docs](https://learn.microsoft.com/dotnet/api/{strip_generic(fqn).lower()})", '']
    tfile.write_text('\n'.join(fm + body), encoding='utf-8')
    count_types += 1

    # member notes
    if t['grouped']:
        mdir = nsdir / fsname(ttitle)
        mdir.mkdir(parents=True, exist_ok=True)
        for g in t['grouped']:
            mbase = member_title[(fqn, g['kind'], g['name'])]
            mfile = mdir / (fsname(mbase) + '.md')
            xm = xml_member(fqn, g['name']) or {}
            mfm = ['---',
                   f'title: "{mbase}"',
                   'tags:',
                   '  - api',
                   f'  - member/{g["kind"]}',
                   f'  - ns/{fsname(ns).replace(".", "-")}',
                   'aliases:',
                   f'  - "{fqn}.{g["name"]}"',
                   f'declaring_type: "{ttitle}"',
                   f'member_kind: {g["kind"]}',
                   '---', '']
            mbody = [f'# {ttitle}.{g["name"]}', '',
                     f'> [!abstract] {g["kind"].capitalize()} of [[{fsname(ttitle)}|{ttitle}]]',
                     f'> Namespace: `{ns}`', '']
            if xm.get('summary'):
                mbody += [xm['summary'], '']
            else:
                mbody += ['> [!todo] Summary pending', '> No XML summary found in source.', '']
            # signatures
            sigs = g['signatures']
            pretty = pretty_member_sigs(fqn, g['kind'], g['name'], sigs)
            mbody += ['## Signature' + ('s' if len(pretty) > 1 else ''), '', '```csharp']
            mbody += pretty
            mbody += ['```', '']
            if xm.get('params'):
                mbody += ['## Parameters', '', '| Parameter | Description |', '|---|---|']
                for pn, pv in xm['params'].items():
                    mbody.append(f"| `{pn}` | {pv.replace('|', '\\|')} |")
                mbody.append('')
            if xm.get('returns'):
                mbody += ['## Returns', '', xm['returns'], '']
            if xm.get('remarks'):
                mbody += ['## Remarks', '', xm['remarks'], '']
            mbody += ['## See also', '', f'- Declaring type: [[{fsname(ttitle)}|{ttitle}]]',
                      f"- [[_{fsname(ns)}|{ns} namespace]]", '']
            mfile.write_text('\n'.join(mfm + mbody), encoding='utf-8')
            count_members += 1

# ---- namespace MOCs ----
for ns, tlist in namespaces.items():
    nsdir = APIDIR / fsname(ns)
    nsdir.mkdir(parents=True, exist_ok=True)
    moc = nsdir / ('_' + fsname(ns) + '.md')
    bykind = defaultdict(list)
    for fqn in tlist:
        t = types[fqn]
        xd = xml_type(fqn) or {}
        k = xd.get('kind') or t['kind']
        bykind[k].append((type_title[fqn], xd.get('summary', '')))
    fm = ['---', f'title: "{ns}"', 'tags:', '  - api', '  - namespace',
          f'  - ns/{fsname(ns).replace(".", "-")}', '---', '']
    body = [f'# {ns}', '', f'> [!info] Namespace', f'> `{ns}` — {len(tlist)} public types.', '',
            f'- [Online namespace docs](https://learn.microsoft.com/dotnet/api/{ns.lower()})', '']
    for k in ['class', 'interface', 'struct', 'enum', 'delegate', 'unknown']:
        items = sorted(bykind.get(k, []))
        if not items:
            continue
        body += [f'## {k.capitalize()}es' if k in ('class',) else f'## {k.capitalize()}s', '',
                 '| Type | Summary |', '|---|---|']
        for title, summ in items:
            summ = (summ or '').replace('|', '\\|')
            if len(summ) > 160:
                summ = summ[:157] + '…'
            body.append(f"| [[{fsname(title)}\\|{title}]] | {summ} |")
        body.append('')
    body += ['## See also', '', '- [[_API Reference]]', '']
    moc.write_text('\n'.join(fm + body), encoding='utf-8')

# ---- root API MOC ----
APIDIR.mkdir(parents=True, exist_ok=True)
rootfm = ['---', 'title: "API Reference"', 'tags:', '  - api', '  - moc', '---', '']
rootbody = ['# .NET MAUI API Reference', '',
            f'> [!abstract] Generated from the repository public API surface',
            f'> {len(types)} public types · {count_members} member pages · {len(namespaces)} namespaces', '',
            '## Namespaces', '', '| Namespace | Types |', '|---|---|']
for ns in sorted(namespaces, key=lambda n: n):
    rootbody.append(f"| [[_{fsname(ns)}\\|{ns}]] | {len(namespaces[ns])} |")
rootbody += ['', '## See also', '', '- [[Home]]', '']
(APIDIR / '_API Reference.md').write_text('\n'.join(rootfm + rootbody), encoding='utf-8')

# ---- xref map for conceptual cross-linking ----
xref_map = {}
for fqn in types:
    xref_map[strip_generic(fqn)] = fsname(type_title[fqn])
for (fqn, kind, name), base in member_title.items():
    xref_map[strip_generic(fqn) + '.' + name] = fsname(base)
Path('vault-build/xref_map.json').write_text(json.dumps(xref_map, ensure_ascii=False), encoding='utf-8')

print(f'type notes: {count_types}')
print(f'member notes: {count_members}')
print(f'namespaces: {len(namespaces)}')
print(f'xref entries: {len(xref_map)}')
