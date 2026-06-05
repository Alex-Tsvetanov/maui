import json, re
from pathlib import Path

model = json.loads(Path('vault-build/api_model.json').read_text(encoding='utf-8-sig'))
xml = json.loads(Path('vault-build/xmldocs.json').read_text(encoding='utf-8-sig'))
xref = json.loads(Path('vault-build/xref_map.json').read_text(encoding='utf-8-sig'))
xtypes = xml['types']

def strip_generic(fqn):
    return re.sub(r'<[^>]*>', '', fqn)

ns_types = model['namespaces']
# rank namespaces by type count, take top 30
ranked = sorted(ns_types.items(), key=lambda kv: -len(kv[1]))
top = [ns for ns, t in ranked if len(t) >= 3][:30]

payload = []
for ns in top:
    fqns = ns_types[ns]
    types = []
    for fqn in sorted(fqns, key=lambda f: model['types'][f]['name']):
        t = model['types'][fqn]
        title = xref.get(strip_generic(fqn), t['name'])
        summ = (xtypes.get(strip_generic(fqn), {}) or {}).get('summary', '')
        types.append({'title': title, 'name': t['name'], 'kind': t['kind'], 'summary': summ[:140]})
    # cap to 50 types in prompt to bound size; note if truncated
    truncated = len(types) > 50
    payload.append({
        'namespace': ns,
        'type_count': len(fqns),
        'types': types[:50],
        'truncated': truncated,
    })

Path('vault-build/ns_payload.json').write_text(json.dumps(payload, ensure_ascii=False), encoding='utf-8')
print('namespaces in payload:', len(payload))
print('payload bytes:', len(json.dumps(payload)))
for p in payload[:8]:
    print(f"  {p['namespace']} ({p['type_count']} types)")
