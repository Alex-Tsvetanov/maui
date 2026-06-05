export const meta = {
  name: 'maui-fill-summaries-missing',
  description: 'Phase B retry: write doc summaries for the 7 units whose maps are missing',
  phases: [{ title: 'Summaries', detail: 'one agent per missing unit writes its JSON map' }],
}

const MISSING = [2, 13, 15, 16, 17, 18, 19]

function prompt(n) {
  const nn = String(n).padStart(2, '0')
  return `You are writing accurate API doc summaries for an Obsidian vault that mirrors the .NET MAUI documentation. Goal: fill members/types that the MAUI source did NOT document with \`///\` comments, by writing them yourself from the signatures + your knowledge of MAUI (reading source only when needed).

INPUT: Read \`D:\\GitHub\\maui\\vault-build\\units\\unit_${nn}.json\`. It has an \`items\` array; each item = {alias, kind, signature, declaring_type, namespace, note_path}. \`alias\` is the fully-qualified key (e.g. \`Microsoft.Maui.Controls.Button.Text\`).

TASK: For EVERY item, write a concise summary.
- Style: .NET XML-doc tone, 1-2 sentences. Properties: "Gets or sets ...". Methods: "<verb>s ...". Events: "Occurs when ...". Handler/renderer/compatibility types: "Maps/renders the cross-platform <X> to the native platform control." Interfaces: "Defines ...". Delegates: "Represents the method that ...". Enums/fields/constructors: describe precisely.
- Ground every summary in the \`signature\`, \`alias\` name, and \`declaring_type\`. You MAY read MAUI source under \`D:\\GitHub\\maui\\src\\\` when a member's purpose is unclear. NEVER invent specific default values, exact parameter semantics, or platform details you cannot see — write a conservative structural description instead.
- One line each, no markdown headers, no wikilinks.

OUTPUT: Write a single JSON object \`{ "<alias>": "<summary>", ... }\` covering EVERY item's alias to \`D:\\GitHub\\maui\\vault-build\\summaries\\unit_${nn}.json\` (UTF-8, valid JSON). This file IS the deliverable.

VALIDATE: re-read the file, confirm it parses and the number of keys equals the number of items. Do NOT edit any vault notes. End your final message with exactly: MAP: vault-build/summaries/unit_${nn}.json (<number_of_keys>)`
}

const results = await parallel(
  MISSING.map((n) => () => agent(prompt(n), { label: `unit:${String(n).padStart(2, '0')}`, phase: 'Summaries' }))
)
log(`Retry finished for ${MISSING.length} units`)
return { attempted: MISSING.length, results: results.filter(Boolean) }
