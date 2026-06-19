export const meta = {
  name: 'ios-parity-assess',
  description: 'Assess iOS theme-matched parity (light↔light, dark↔dark) per page; emit per-page light/dark verdicts',
  phases: [{ title: 'Assess parity' }],
}

const CMP = '/Users/Alex.Tsvetanov/Documents/GitHub/maui/port/cpp/docs/comparison'
const keys = typeof args === 'string' ? JSON.parse(args) : args

const SCHEMA = {
  type: 'object', additionalProperties: false,
  properties: {
    key: { type: 'string' },
    light: { type: 'string', enum: ['match', 'minor', 'diff', 'cpp_blank', 'cs_blank'] },
    dark: { type: 'string', enum: ['match', 'minor', 'diff', 'cpp_blank', 'cs_blank'] },
    light_note: { type: 'string', description: 'one line: what differs in the light pair (or "identical")' },
    dark_note: { type: 'string', description: 'one line: what differs in the dark pair (or "identical")' },
  },
  required: ['key', 'light', 'dark', 'light_note', 'dark_note'],
}

log(`Assessing iOS parity for ${keys.length} pages (light↔light + dark↔dark)`)

const results = await pipeline(
  keys,
  k => agent(
`Assess **iOS pixel/layout parity** between **real .NET MAUI** and the **C++ port** for the page "${k}", theme-for-theme. All four are iPhone 17 simulator screenshots; both stacks render native-default controls + the system font in the SAME appearance, so this is an apples-to-apples comparison — judge TRUE parity, not theme/style noise.

Read all four:
- MAUI light: \`${CMP}/csharp_ios_light/${k}.png\`     C++ light: \`${CMP}/cpp_ios_light/${k}.png\`
- MAUI dark:  \`${CMP}/csharp_ios_dark/${k}.png\`      C++ dark:  \`${CMP}/cpp_ios_dark/${k}.png\`

Judge the LIGHT pair (MAUI light vs C++ light) and the DARK pair (MAUI dark vs C++ dark) SEPARATELY. For each, pick:
- **match** — pixel-identical or trivially so (same controls, positions, sizes, colors, text; only sub-pixel/font-hinting differences).
- **minor** — small differences (slightly different spacing/padding, font weight, a few px of position/size, a shade off).
- **diff** — notable: a control missing/extra/mis-ordered, wrong size, wrong color, wrong layout, overlap, different text content, or different control chrome (e.g. one draws a button background the other doesn't).
- **cpp_blank** — the C++ side is blank/empty while MAUI shows content. **cs_blank** — MAUI blank while C++ shows content.

Describe the specific difference in one line each (or "identical"). Be precise about WHAT differs (control, color, position) so it can be fixed.

Return: key=\`${k}\`, light, dark, light_note, dark_note.`,
    { label: `assess:${k}`, phase: 'Assess parity', schema: SCHEMA }
  )
)

const ok = results.filter(Boolean)
const tally = (field) => ok.reduce((m, r) => ((m[r[field]] = (m[r[field]] || 0) + 1), m), {})
return {
  total: ok.length,
  light_tally: tally('light'),
  dark_tally: tally('dark'),
  verdicts: ok.map(r => ({ key: r.key, light: r.light, dark: r.dark, light_note: r.light_note, dark_note: r.dark_note })),
}
