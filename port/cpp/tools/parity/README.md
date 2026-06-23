# iOS parity comparison — Gemini-default, Claude-fallback

The **default image-comparison engine** for the iOS pixel-parity loop. It judges each demo
page (real **.NET MAUI** vs the **C++ port**, light-vs-light and dark-vs-dark) with **Google
Gemini** vision, and falls back to **Claude's own vision** only when Gemini's quota is hit.

Why Gemini by default: it's an *independent* judge (doesn't anchor on the existing
`parity_status.json` verdicts) and is cheap/fast enough to re-sweep all 172 pages. Claude
vision remains the fallback so the loop never stalls when the API quota runs out.

## Files

| File | Role |
| --- | --- |
| `gemini_compare.py` | Core: compares ONE page (4 screenshots) → one JSON verdict. Exit **75** on quota. |
| `run_parity.py` | Batch driver: sweeps pages, writes the review artifacts + fallback set. Non-destructive by default. |
| `gen_review_md.py` | Renders `docs/comparison/PARITY_REVIEW.md` from `parity_review.json` (human-verification doc). |

Verdict shape — `light`/`dark`/`*_note` stay drop-in for `parity_status.json`; the two **buckets**
separate MAUI's own imperfections from genuine port bugs:

```json
{"key":"button","light":"diff","dark":"diff","light_note":"...","dark_note":"...",
 "maui_quirks":["MAUI insets whole page in a card; port is edge-to-edge"],
 "port_diffs":["CornerRadius button has square corners","pink Button text is letter-spaced"]}
```
`light`/`dark` ∈ `match | minor | diff | cpp_blank | cs_blank`, judged from **port_diffs only**.

- **`port_diffs`** — genuine C++ issues to fix; these drive the verdict.
- **`maui_quirks`** — MAUI-side imperfections (harness card inset, top/bottom crop, chrome, MAUI color
  quirks) that are *subject to a human ruling* and do NOT drive the verdict. The whole-screen inset is
  treated as a uniform outer margin only — a control that is a different **size** or **spacing** is a
  port_diff, never a quirk.

## Outputs (non-destructive by default)

`run_parity.py` writes, without touching the tracked board:
- `docs/comparison/parity_review.json` — full verdicts + buckets.
- `docs/comparison/PARITY_REVIEW.md` — human-verification doc mirroring `README.md`, with a
  **"MAUI imperfection categories — RULE ON EACH"** agenda + per-page port_diffs / maui_quirks.
- `docs/comparison/parity_fallback.json` — the set Gemini couldn't judge (quota/missing/error).

Only `--commit-board` merges `light`/`dark` verdicts into `parity_status.json` (and `--gen-readme`
regenerates the tracker `README.md`). Rule on the quirk categories first, record the rulings in
`port/CLAUDE.md` / `port/PROJECT.md`, THEN fix the port and/or commit the board.

## Models & quota (min-maxed)

The free-tier daily limits make model choice critical — `gemini-flash-latest` resolves to
**Gemini 3.5 Flash, only 20 requests/day**, which cannot cover a 172-page sweep. Usable
vision+text-out models and their limits:

| Model | RPM | RPD | Role |
| --- | --: | --: | --- |
| `gemini-3.1-flash-lite` | 15 | **500** | **workhorse** — clears a full sweep in one day |
| `gemini-3-flash` / `gemini-2.5-flash` / `gemini-3.5-flash` | 5 | 20 each | premium — better bucketing, scarce |
| `gemini-2.5-flash-lite` | 10 | 20 | extra lite quota |

`run_parity.py` drives a **quota-aware cascade** (`--models`, default best-first):
`gemini-3.5-flash → gemini-3-flash → gemini-2.5-flash → gemini-3.1-flash-lite → gemini-2.5-flash-lite`.
It spends the scarce premium (20-RPD) models first — best verdicts on the early pages — then rotates
to flash-lite for the bulk when each 429s, and only falls back to Claude when **all** are exhausted
(~580 Gemini RPD total). Each page records the model that judged it; full-flash verdicts are more
reliable than flash-lite (flash-lite occasionally mis-buckets the crop quirk as a port_diff — weight
your review accordingly). Pacing is automatic from each model's RPM (no manual `--delay` needed).
Force a single model with `--model gemini-3.1-flash-lite` (consistent verdicts, still 500/day).

## API key (NOT in the repo)

The key lives **outside the repo** so it can never be committed:

```
~/.config/maui-parity/gemini_api_key      # mode 600
```

Resolution order: `$GEMINI_API_KEY` → that file. Model: `$GEMINI_MODEL`
(default `gemini-flash-latest`). To rotate, overwrite that file (or export the env var).

## Usage

```bash
# one page (prints a JSON verdict + buckets; exits 75 on quota, 2 on missing images)
python3 port/cpp/tools/parity/gemini_compare.py button

# review-only full sweep -> PARITY_REVIEW.md (board untouched). Cascade + auto-pacing by default.
python3 port/cpp/tools/parity/run_parity.py --all

# single consistent model (500/day), or a custom cascade
python3 port/cpp/tools/parity/run_parity.py --all --model gemini-3.1-flash-lite
python3 port/cpp/tools/parity/run_parity.py --all --models "gemini-2.5-flash:5,gemini-3.1-flash-lite:15"

# specific pages (review-only)
python3 port/cpp/tools/parity/run_parity.py button grid horizontal_stack

# AFTER you've ruled on the quirks & verified the review: adopt verdicts into the tracked board
python3 port/cpp/tools/parity/run_parity.py --all --commit-board --gen-readme --delay 5
```

## The loop protocol (default → fallback)

1. **Default (review-only):** run `run_parity.py --all --delay 5`. Gemini judges each page into
   port_diffs vs maui_quirks; results go to `PARITY_REVIEW.md` / `parity_review.json`. The tracked
   board is NOT modified.
2. **Rule on MAUI imperfections:** the human reviews `PARITY_REVIEW.md`, rules on each quirk category
   (ignore / match / case-by-case); rulings are recorded in `port/CLAUDE.md` / `port/PROJECT.md` so the
   loop stops re-litigating them. New quirk categories surfaced by the sweep get asked, not assumed.
3. **On quota:** the moment Gemini returns `RESOURCE_EXHAUSTED` / HTTP 429, the sweep stops and every
   not-yet-judged page (plus any missing/errored pages) is written to
   `docs/comparison/parity_fallback.json` under `"fallback"`.
4. **Fallback:** the loop agent judges that `fallback` set with **its own vision** — the
   `docs/comparison/parity_assess_wf.js` workflow (Claude subagents) — then continues.
5. **Adopt / fix:** only after the review is verified, `--commit-board` adopts the verdicts and the
   port_diffs become fix candidates.

`run_parity.py` prints a machine-readable summary as its **last stdout line**:

```json
{"judged":N,"errored":[...],"missing":[...],"quota_hit":true|false,"fallback":[...]}
```

## Calibration (honest, not lenient)

Both stacks are captured at the **same resolution** (iPhone 17, 1206×2622), so control sizes,
spacing, and counts are directly comparable — a size/spacing difference is a **real** diff, not
"scale noise." The prompt discounts only genuine capture artifacts: the MAUI harness's gray
container card / nav bar / status bar, and transient animation phase. It is deliberately **not**
biased toward "match" — on `button` it correctly reports a `diff` (thinner C++ buttons, pink-button
letter-spacing) that the older hand-classified board called a match.

## Capturing the baselines

- **C++ port** → `capture_all_cpp.py` drives the installed `dev.maui-cpp.ios-gallery`.
- **MAUI baseline** → `capture_all_csharp.py` drives the installed maui-compare app
  (`com.companyname.mauicompare`, `MAUI_COMPARE_PAGE` / `MAUI_THEME`).

⚠️ **The maui-compare app MUST declare `UILaunchScreen` in `Platforms/iOS/Info.plist`.** Without it iOS
runs the app in legacy compatibility mode — letterboxed and laid out at ~320pt logical, upscaled ~1.25×
to the 402pt device — so every MAUI control renders ~1.25× larger than the native-rendering C++ gallery
(402/320 = 1.256). This silently scaled the *entire* old baseline and produced a whole class of bogus
"MAUI is larger / C++ smaller / fonts bigger" diffs (the port was correct). Both stacks must render
**native** for the "same resolution, directly comparable" claim above to hold. A clean
`dotnet build -t:Rebuild` is required for the plist change to take effect (an incremental build won't
reprocess it).
