"""Parse all PublicAPI.Shipped.txt files into a unified API model.

Output: vault-build/api_model.json
  {
    "namespaces": { "Microsoft.Maui.Controls": { "types": ["...FQN..."] } },
    "types": {
       "<typeFQN>": {
          "name": "Button", "namespace": "Microsoft.Maui.Controls",
          "kind": "class|interface|enum|struct|delegate|unknown",
          "tfms": ["net","net-android",...],
          "assemblies": ["Controls.Core"],
          "members": [ {"raw":..., "name":..., "kind":..., "signature":..., "returns":..., "tfms":[...]} ]
       }
    }
  }
"""
import glob, json, os, re
from pathlib import Path
from collections import defaultdict

ROOT = Path('.')
MODIFIERS = {'~', 'abstract', 'const', 'override', 'sealed', 'static', 'virtual',
             'readonly', 'extern', 'async', 'partial', 'new', 'unsafe', 'volatile',
             'ref', 'implicit', 'explicit', 'sealed', 'required'}

def tfm_from_path(p):
    parts = p.replace('\\', '/').split('/')
    # find the PublicAPI segment, tfm is the next dir
    if 'PublicAPI' in parts:
        i = parts.index('PublicAPI')
        if i + 1 < len(parts):
            return parts[i + 1]
    # flat files (WindowsForms/Wpf)
    return 'net'

def assembly_from_path(p):
    p = p.replace('\\', '/')
    # heuristic: take the segment under src/.../src or meaningful name
    m = re.search(r'src/([^/]+(?:/[^/]+)*?)/(?:src/)?(?:PublicAPI|PublicAPI\.Shipped)', p)
    # fallback: use the path piece before /PublicAPI
    idx = p.find('/PublicAPI')
    if idx == -1:
        idx = p.find('/PublicAPI.Shipped')
    head = p[:idx]
    return head.split('/src/')[0].split('/')[-1] if '/src/' in head else head.split('/')[-1]

def strip_modifiers(s):
    # remove leading ~ then leading modifier words
    s = s.lstrip()
    if s.startswith('~'):
        s = s[1:].lstrip()
    while True:
        tok = s.split(' ', 1)[0]
        base = tok.lstrip('~')
        if base in MODIFIERS:
            s = s.split(' ', 1)[1] if ' ' in s else ''
            s = s.lstrip()
        else:
            break
    return s

def parse_line(line):
    """Return (kind, lhs, rettype) where kind in {'type','member'}."""
    line = line.rstrip('\n')
    if not line.strip() or line.strip().startswith('#'):
        return None
    s = strip_modifiers(line.strip())
    if not s:
        return None
    rettype = None
    if ' -> ' in s:
        lhs, rettype = s.split(' -> ', 1)
        lhs = lhs.strip(); rettype = rettype.strip()
        return ('member', lhs, rettype)
    else:
        return ('type', s.strip(), None)

# Pass 1: collect type FQNs and tfm/assembly availability
files = sorted(set(glob.glob('src/**/PublicAPI.Shipped.txt', recursive=True)))
type_tfms = defaultdict(set)
type_assemblies = defaultdict(set)
member_lines = []  # (typeguess pending) store (lhs, rettype, tfm, assembly, rawmods)
all_type_fqns = set()

raw_records = []  # (kind, lhs, rettype, tfm, assembly, raw)
for f in files:
    tfm = tfm_from_path(f)
    asm = assembly_from_path(f)
    for raw in open(f, encoding='utf-8-sig'):
        parsed = parse_line(raw)
        if not parsed:
            continue
        kind, lhs, rettype = parsed
        raw_records.append((kind, lhs, rettype, tfm, asm, raw.strip()))
        if kind == 'type':
            all_type_fqns.add(lhs)
            type_tfms[lhs].add(tfm)
            type_assemblies[lhs].add(asm)

# Build sorted list of type fqns for longest-prefix matching
type_list = sorted(all_type_fqns, key=len, reverse=True)
type_set = set(all_type_fqns)

def declaring_type_of(qualifier):
    # qualifier: member qualified name w/o params, e.g. Microsoft.Maui.Controls.Button.Text
    # find longest type fqn that is a prefix followed by '.'
    best = None
    # quick: iteratively strip last segment
    parts = qualifier
    # try progressively shorter prefixes by removing trailing .segment
    cur = qualifier
    while '.' in cur:
        cur = cur.rsplit('.', 1)[0]
        if cur in type_set:
            return cur
    return None

def member_qualifier(lhs):
    """Strip params/value/get-set to get the qualified member name."""
    q = lhs
    # remove params
    pidx = q.find('(')
    if pidx != -1:
        q = q[:pidx]
    # remove ' = value'
    if ' = ' in q:
        q = q.split(' = ', 1)[0]
    # remove trailing .get/.set
    if q.endswith('.get') or q.endswith('.set'):
        q = q.rsplit('.', 1)[0]
    return q.strip()

def member_kind(lhs, rettype, raw):
    if '(' in lhs:
        # method or ctor or operator
        name_before = lhs[:lhs.find('(')]
        simple = name_before.rsplit('.', 1)[-1]
        decl = declaring_type_of(member_qualifier(lhs))
        if decl is not None:
            tsimple = decl.rsplit('.', 1)[-1].split('<')[0]
            if simple.split('<')[0] == tsimple:
                return 'constructor'
        if 'operator' in name_before:
            return 'operator'
        return 'method'
    if lhs.endswith('.get') or lhs.endswith('.set'):
        return 'property'
    if ' = ' in lhs:
        # const or enum member
        if 'const' in raw:
            return 'const'
        return 'field'
    # field or event: events usually have delegate return type; hard to tell
    if rettype and ('EventHandler' in rettype or rettype.endswith('Handler') or 'event' in raw):
        return 'event'
    return 'field'

types = {}
def ensure_type(fqn):
    if fqn not in types:
        ns = fqn.rsplit('.', 1)[0] if '.' in fqn else ''
        # handle nested types: namespace is the part up to first type segment; approximate using declaring
        name = fqn.rsplit('.', 1)[-1]
        types[fqn] = {
            'fqn': fqn, 'name': name, 'namespace': ns,
            'kind': 'unknown', 'tfms': sorted(type_tfms.get(fqn, [])),
            'assemblies': sorted(type_assemblies.get(fqn, [])),
            'members_map': {},
        }
    return types[fqn]

for fqn in all_type_fqns:
    ensure_type(fqn)

# Assign members
member_avail = defaultdict(set)  # (declfqn, raw_normalized) -> tfms
member_info = {}
for kind, lhs, rettype, tfm, asm, raw in raw_records:
    if kind != 'type':
        q = member_qualifier(lhs)
        decl = declaring_type_of(q)
        if decl is None:
            continue
        mk = member_kind(lhs, rettype, raw)
        simple = q[len(decl) + 1:] if q.startswith(decl + '.') else q.rsplit('.', 1)[-1]
        # signature: reconstruct human form
        sig = lhs
        # for property, collapse get/set later; key by simple name + param sig
        key = (decl, mk, simple, lhs.split('->')[0])
        member_avail[key].add(tfm)
        if key not in member_info:
            member_info[key] = {
                'declaring': decl, 'kind': mk, 'name': simple,
                'signature': sig, 'returns': rettype, 'raw': raw,
            }

# attach members to types, merging property get/set
for key, info in member_info.items():
    decl = info['declaring']
    t = types.get(decl)
    if t is None:
        t = ensure_type(decl)
    info['tfms'] = sorted(member_avail[key])
    t['members_map'].setdefault(info['kind'], []).append(info)

# Infer type kind from members / heuristics
for fqn, t in types.items():
    mk = t['members_map']
    if 'const' in mk and len(mk) == 1:
        pass
    # interface: name starts with I + uppercase
    nm = t['name'].split('<')[0]
    if re.match(r'^I[A-Z]', nm):
        t['kind'] = 'interface'
    # We cannot perfectly know enum/struct/class from PublicAPI alone; default class
    if t['kind'] == 'unknown':
        t['kind'] = 'class'

# Build namespaces (top-level real namespaces: namespace of a type, but nested types have type-as-namespace)
namespaces = defaultdict(set)
def real_namespace(fqn):
    # namespace = longest prefix that is NOT itself a known type
    parts = fqn.split('<')[0].split('.')
    # remove the type's own simple name
    for i in range(len(parts) - 1, 0, -1):
        cand = '.'.join(parts[:i])
        if cand not in type_set:
            return cand
    return '.'.join(parts[:-1])

for fqn, t in types.items():
    ns = real_namespace(fqn)
    t['namespace'] = ns
    namespaces[ns].add(fqn)

# finalize
out_types = {}
for fqn, t in types.items():
    members = []
    for mklist in t['members_map'].values():
        members.extend(mklist)
    out_types[fqn] = {
        'fqn': fqn, 'name': t['name'], 'namespace': t['namespace'],
        'kind': t['kind'], 'tfms': t['tfms'], 'assemblies': t['assemblies'],
        'members': members,
    }

model = {
    'namespaces': {ns: sorted(v) for ns, v in namespaces.items()},
    'types': out_types,
    'stats': {
        'namespaces': len(namespaces),
        'types': len(out_types),
        'members': sum(len(t['members']) for t in out_types.values()),
    },
}
Path('vault-build').mkdir(exist_ok=True)
Path('vault-build/api_model.json').write_text(json.dumps(model, ensure_ascii=False), encoding='utf-8')
print('namespaces:', model['stats']['namespaces'])
print('types:', model['stats']['types'])
print('members:', model['stats']['members'])
print('sample namespaces:', sorted(namespaces, key=lambda n: -len(namespaces[n]))[:15])
