"""Extract XML doc <summary>/<remarks>/<returns>/<param> from MAUI source .cs.

v2: handles multi-line declarations (member name on one line, `{`/`(` on the
next) and resolves <inheritdoc/>. Keys are aligned to vault note aliases:
  type   -> "Namespace.Nested.Type"            (generics stripped)
  member -> "Namespace.Nested.Type.Member"     (generics stripped)

Output: vault-build/xmldocs.json
"""
import json, re, glob, html
from pathlib import Path

SUMMARY_RE = re.compile(r'<summary>(.*?)</summary>', re.S | re.I)
REMARKS_RE = re.compile(r'<remarks>(.*?)</remarks>', re.S | re.I)
RETURNS_RE = re.compile(r'<returns>(.*?)</returns>', re.S | re.I)
PARAM_RE = re.compile(r'<param name="([^"]+)">(.*?)</param>', re.S | re.I)
INHERIT_RE = re.compile(r'<inheritdoc\s*(?:cref="([^"]+)")?\s*/?>', re.I)

NS_FILE = re.compile(r'^\s*namespace\s+([A-Za-z0-9_.]+)\s*;')
NS_BLOCK = re.compile(r'^\s*namespace\s+([A-Za-z0-9_.]+)\s*\{?\s*$')
TYPE_DECL = re.compile(
    r'\b(?:public|internal|protected|private|sealed|abstract|static|partial|readonly|ref|unsafe|new)\s+'
    r'(?:(?:sealed|abstract|static|partial|readonly|ref|unsafe|new)\s+)*'
    r'(class|struct|interface|enum|record)\s+([A-Za-z_][A-Za-z0-9_]*)')

TERM = re.compile(r'[{;(=]|=>')
KEYWORDS = {'class', 'struct', 'interface', 'enum', 'record', 'get', 'set', 'return',
            'if', 'for', 'foreach', 'while', 'switch', 'using', 'namespace', 'new',
            'where', 'add', 'remove', 'value', 'void', 'public', 'private', 'protected',
            'internal', 'static', 'readonly', 'const', 'event', 'this', 'base'}


def cref_name(ref):
    ref = re.sub(r'^[A-Za-z]:', '', ref)
    ref = ref.split('(')[0]
    return ref.rsplit('.', 1)[-1]


def clean(t):
    if not t:
        return ''
    t = re.sub(r'<see\s+cref="([^"]+)"\s*/?>', lambda m: '`' + cref_name(m.group(1)) + '`', t)
    t = re.sub(r'<seealso\s+cref="([^"]+)"\s*/?>', lambda m: '`' + cref_name(m.group(1)) + '`', t)
    t = re.sub(r'<see\s+langword="([^"]+)"\s*/?>', r'`\1`', t)
    t = re.sub(r'<paramref\s+name="([^"]+)"\s*/?>', r'`\1`', t)
    t = re.sub(r'<typeparamref\s+name="([^"]+)"\s*/?>', r'`\1`', t)
    t = re.sub(r'</?(para|c|code|list|item|description|term|b|i)>', ' ', t, flags=re.I)
    t = re.sub(r'<[^>]+>', '', t)
    t = re.sub(r'^\s*///?\s?', '', t, flags=re.M)
    t = html.unescape(t)
    t = re.sub(r'\s+', ' ', t).strip()
    return t


def member_name_from_decl(buf, type_kind):
    """Extract the declared member's simple name from an accumulated decl buffer."""
    # cut at first terminator
    m = TERM.search(buf)
    head = buf[:m.start()] if m else buf
    term = buf[m.start():m.start() + 2] if m else ''
    head = head.strip()
    if not head:
        return None
    # enum member: bare identifier (no modifiers/type), inside an enum
    if type_kind == 'enum':
        mm = re.match(r'^([A-Za-z_][A-Za-z0-9_]*)', head)
        return mm.group(1) if mm else None
    # method/ctor/operator: name is identifier right before '('
    if buf[m.start():m.start() + 1] == '(' if m else False:
        before = head
        # operator
        opm = re.search(r'\boperator\s+(\S+)\s*$', before)
        if opm:
            return 'operator ' + opm.group(1)
        # strip generic args on the method name
        before = re.sub(r'<[^>]*>\s*$', '', before)
        mm = re.search(r'([A-Za-z_][A-Za-z0-9_]*)\s*$', before)
        return mm.group(1) if mm else None
    # property / field / event / expression-bodied: last identifier in head
    # remove a trailing generic
    head2 = re.sub(r'<[^>]*>\s*$', '', head)
    ids = re.findall(r'[A-Za-z_][A-Za-z0-9_]*', head2)
    if not ids:
        return None
    name = ids[-1]
    if name in KEYWORDS:
        # e.g. "public event EventHandler Foo" last id Foo ok; but "public int Value" -> Value
        # if last is a keyword, take the previous non-keyword
        for cand in reversed(ids):
            if cand not in KEYWORDS:
                return cand
        return None
    return name


types_out = {}
members_out = {}
inherit_members = {}   # key -> cref (or None for bare)
inherit_types = {}

files = glob.glob('src/**/*.cs', recursive=True)
processed = 0
for fp in files:
    if any(seg in fp for seg in ('\\obj\\', '/obj/', '\\bin\\', '/bin/')):
        continue
    try:
        text = open(fp, encoding='utf-8-sig').read()
    except Exception:
        continue
    lines = text.split('\n')
    processed += 1
    namespace = ''
    type_stack = []  # list of (name, kind, depth_at_open)
    depth = 0
    doc_buf = []
    i = 0
    n = len(lines)
    while i < n:
        line = lines[i]
        stripped = line.strip()
        m = NS_FILE.match(line)
        if m:
            namespace = m.group(1); i += 1; continue
        m = NS_BLOCK.match(line)
        if m and ' class ' not in line and ' interface ' not in line:
            namespace = m.group(1)
        if stripped.startswith('///'):
            doc_buf.append(line); i += 1; continue
        if stripped.startswith('['):  # attribute(s) - keep doc buffer
            depth += line.count('{') - line.count('}'); i += 1; continue
        mt = TYPE_DECL.search(line)
        if mt:
            kind = mt.group(1); tname = mt.group(2)
            ctx = '.'.join(t[0] for t in type_stack)
            fqtype = '.'.join(filter(None, [namespace, ctx, tname]))
            if doc_buf:
                blob = '\n'.join(doc_buf)
                s = SUMMARY_RE.search(blob); r = REMARKS_RE.search(blob)
                inh = INHERIT_RE.search(blob)
                if s or r:
                    types_out[fqtype] = {'summary': clean(s.group(1) if s else ''),
                                         'remarks': clean(r.group(1) if r else ''), 'kind': kind}
                elif inh:
                    inherit_types[fqtype] = inh.group(1)
            type_stack.append((tname, kind, depth))
            depth += line.count('{') - line.count('}')
            doc_buf = []; i += 1; continue
        if doc_buf and stripped and not stripped.startswith('//'):
            # accumulate declaration across up to 8 lines until a terminator
            decl_lines = []
            j = i
            while j < n and j < i + 8:
                decl_lines.append(lines[j])
                if TERM.search(lines[j]):
                    break
                j += 1
            decl = ' '.join(s.strip() for s in decl_lines)
            cur_kind = type_stack[-1][1] if type_stack else ''
            name = member_name_from_decl(decl, cur_kind)
            ctx = '.'.join(t[0] for t in type_stack)
            if name and name not in KEYWORDS:
                name_norm = re.sub(r'<[^>]*>', '', name)
                fqmem = '.'.join(filter(None, [namespace, ctx, name_norm]))
                blob = '\n'.join(doc_buf)
                s = SUMMARY_RE.search(blob); r = REMARKS_RE.search(blob)
                rr = RETURNS_RE.search(blob); inh = INHERIT_RE.search(blob)
                params = {p: clean(v) for p, v in PARAM_RE.findall(blob)}
                if (s or r) and fqmem not in members_out:
                    members_out[fqmem] = {'summary': clean(s.group(1) if s else ''),
                                          'remarks': clean(r.group(1) if r else ''),
                                          'returns': clean(rr.group(1) if rr else ''),
                                          'params': params}
                elif inh and fqmem not in members_out and fqmem not in inherit_members:
                    inherit_members[fqmem] = inh.group(1)
            # advance past consumed lines, updating depth
            for k in range(i, j + 1):
                if k < n:
                    depth += lines[k].count('{') - lines[k].count('}')
                    while type_stack and depth <= type_stack[-1][2]:
                        type_stack.pop()
            doc_buf = []
            i = j + 1
            continue
        # plain line
        if stripped and not stripped.startswith('//'):
            doc_buf = []
        depth += line.count('{') - line.count('}')
        while type_stack and depth <= type_stack[-1][2]:
            type_stack.pop()
        i += 1

# ---- resolve <inheritdoc/> ----
# index documented members by simple name
by_simple = {}
for key, v in members_out.items():
    simple = key.rsplit('.', 1)[-1]
    by_simple.setdefault(simple, []).append(v)

resolved = 0
def lookup_cref(ref):
    ref = re.sub(r'^[A-Za-z]:', '', ref).split('(')[0]
    if ref in members_out:
        return members_out[ref]
    if ref in types_out:
        return types_out[ref]
    simple = ref.rsplit('.', 1)[-1]
    cands = by_simple.get(simple)
    if cands:
        return cands[0]
    return None

for key, cref in inherit_members.items():
    src = None
    if cref:
        src = lookup_cref(cref)
    else:
        simple = key.rsplit('.', 1)[-1]
        cands = by_simple.get(simple)
        if cands:
            src = cands[0]
    if src and src.get('summary'):
        members_out[key] = {'summary': src['summary'], 'remarks': src.get('remarks', ''),
                            'returns': src.get('returns', ''), 'params': src.get('params', {})}
        resolved += 1

for key, cref in inherit_types.items():
    if cref:
        src = lookup_cref(cref)
        if src and src.get('summary') and key not in types_out:
            types_out[key] = {'summary': src['summary'], 'remarks': src.get('remarks', ''), 'kind': 'class'}
            resolved += 1

out = {'types': types_out, 'members': members_out,
       'stats': {'files': processed, 'type_docs': len(types_out),
                 'member_docs': len(members_out), 'inheritdoc_resolved': resolved}}
Path('vault-build/xmldocs.json').write_text(json.dumps(out, ensure_ascii=False), encoding='utf-8')
print('files:', processed, '| type docs:', len(types_out),
      '| member docs:', len(members_out), '| inheritdoc resolved:', resolved)
