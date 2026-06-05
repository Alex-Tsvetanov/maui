export const meta = {
  name: 'maui-fill-summaries',
  description: 'Phase B: 22 agents write doc summaries for undocumented MAUI API members into per-unit JSON maps',
  phases: [{ title: 'Summaries', detail: 'one agent per unit writes vault-build/summaries/unit_NN.json' }],
}

const RESULT = {
  type: 'object',
  additionalProperties: false,
  properties: {
    unit: { type: 'number' },
    ok: { type: 'boolean' },
    count: { type: 'number' },
    note: { type: 'string' },
  },
  required: ['unit', 'ok', 'count', 'note'],
}

const UNITS = 22

function prompt(n) {
  const nn = String(n).padStart(2, '0')
  return `You are writing accurate API doc summaries for an Obsidian vault that mirrors the .NET MAUI documentation. Goal: fill members/types that the MAUI source did NOT document with \`///\` comments, by writing them yourself from the signatures + your knowledge of MAUI (reading source only when needed).

INPUT: Read \`D:\\GitHub\\maui\\vault-build\\units\\unit_${nn}.json\`. It has an \`items\` array; each item = {alias, kind, signature, declaring_type, namespace, note_path}. \`alias\` is the fully-qualified key (e.g. \`Microsoft.Maui.Controls.Button.Text\`).

TASK: For EVERY item, write a concise summary.
- Style: .NET XML-doc tone, 1-2 sentences. Properties: "Gets or sets ...". Methods: "<verb>s ...". Events: "Occurs when ...". Handler/renderer \`Map*\`/compatibility types: "Maps/renders the cross-platform <X> to the native platform control." Interfaces: "Defines ...". Delegates: "Represents the method that ...". Enums/fields/constructors: describe precisely.
- Ground every summary in the \`signature\`, \`alias\` name, and \`declaring_type\`. You MAY read MAUI source under \`D:\\GitHub\\maui\\src\\\` (grep for the declaring type) when a member's purpose is unclear — but for obvious accessors/handlers/mappers the signature + name is enough. NEVER invent specific default values, exact parameter semantics, or platform details you cannot see — write a conservative structural description instead.
- One line each, no markdown headers, no wikilinks.

OUTPUT: Write a single JSON object \`{ "<alias>": "<summary>", ... }\` covering EVERY item's alias to \`D:\\GitHub\\maui\\vault-build\\summaries\\unit_${nn}.json\` (UTF-8, valid JSON).

VALIDATE: re-read the file, confirm it parses and the number of keys equals the number of items (every alias present, no empty values). Do NOT edit any vault notes.

Return the structured result: unit=${n}, ok (true if file written and validated), count (number of keys written), and a one-line note.`
}

const results = await parallel(
  Array.from({ length: UNITS }, (_, i) => i + 1).map((n) => () =>
    agent(prompt(n), { label: `unit:${String(n).padStart(2, '0')}`, phase: 'Summaries', schema: RESULT })
  )
)

const ok = results.filter(Boolean).filter((r) => r.ok)
const totalWritten = ok.reduce((s, r) => s + (r.count || 0), 0)
log(`Done: ${ok.length}/${UNITS} units, ${totalWritten} summaries written`)
return { units_ok: ok.length, total_units: UNITS, summaries_written: totalWritten, results: results.filter(Boolean) }
