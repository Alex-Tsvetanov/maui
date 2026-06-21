# CLAUDE.md — Operating manual for porting .NET MAUI to another language

> Read `PROJECT.md` (same folder) first — it defines **what** you are building and the layer/build
> order. This file defines **how** you work. The target language is **not yet chosen**; obey the
> `LANGUAGE-SPECIFIC: TBD` rule at the bottom until it is.

## Prime directive

You are porting .NET MAUI to a target language **T**. **Derive behavior from the source and tests —
never invent it.** When you don't know how something behaves, you read the original C# implementation
and its tests *before* writing a line. A plausible guess that diverges from MAUI's real behavior is a
bug, not a port.

## The three questions and where each is answered

For any type or member you port, you will need three things. Get each from its authoritative source —
do not substitute one for another:

1. **What is the contract?** (names, signatures, properties, platforms)
   → `vault/API Reference/<Namespace>/<Type>.md` (and its member notes), cross-checked against the
   ground truth `src/**/PublicAPI/<tfm>/PublicAPI.Shipped.txt`.
2. **How must it behave?** (defaults, edge cases, events, ordering, errors)
   → the **tests** in `src/**/tests/**` are the oracle. Find the test(s) covering the type and treat
   them as the executable spec. If behavior is untested, fall to the source.
3. **How is it implemented?** (algorithms, native wiring)
   → the original C# under `src/<area>/src/...` — the cross-platform `Foo.cs` plus the per-platform
   partials `Foo.{Android,iOS,Windows,Tizen,Standard}.cs`.

Supporting maps: `vault/Conceptual/` for architecture/intent; `vault/Home.md` and
`graphify-out/GRAPH_REPORT.md` for the big picture and dependency hot-spots.

## Workflow: bottom-up, one layer at a time

Follow the layer order in `PROJECT.md §3`. **Do not start a layer until the layer below it passes its
tests.** Within a layer, port in small, verifiable slices (one control / one primitive at a time).

### The per-component porting loop (TDD)

For each component (e.g. `Button`, `Color`, `Grid`):

1. **Contract** — open the vault API note(s) for the type; list its public members and platform
   availability. Confirm against the `PublicAPI.Shipped.txt` entry (authoritative).
2. **Behavior** — locate the C# tests for it (`grep -ril <TypeName> src/**/tests`). Read them. These
   define the required behavior.
3. **Reference** — read the C# source: the cross-platform file and the platform partial for your
   **first target platform** only (ignore the others for now).
4. **Port the tests first** — translate the relevant unit tests into T's test framework. They should
   compile and **fail** (red).
5. **Implement** — write the T code to make them pass (green). Mirror MAUI's structure (see the handler
   recipe below); keep the cross-platform vs platform split.
6. **Verify** — run the ported tests; for UI-bearing slices, also do the runtime check (M2/M4 in
   `PROJECT.md`). Only then move on.
7. **Record** — update the progress manifest (see below).

### The handler porting recipe (the core repeated unit)

Porting a control means porting its handler slice. Using `Button` as the template:

1. Port the **virtual-view interface** `IButton` (from `src/Core/src/Core/IButton.cs`) — properties
   only, no UI.
2. Port the **PropertyMapper / CommandMapper** infrastructure once (from the `ViewHandler` base and
   `src/Core/src/Handlers/Button/ButtonHandler.cs`): a mapping of property-name → update function.
3. Port the **handler** `ButtonHandler`: its mapper table (`"Text" -> MapText`, …) and `CreatePlatformView`.
4. Port the **platform partial** for the first target only (`ButtonHandler.Android.cs` etc.): create the
   native control and implement each `MapXxx(handler, view)` to push values to it.
5. Port the **control** `Button` (`src/Controls/src/Core/Button/Button.cs` + `Button.Mapper.cs`): the
   bindable, developer-facing class implementing `IButton`.
6. Wire it in **Hosting** so the control resolves to the handler.

Every subsequent control repeats steps 1, 3–6 (the mapper infra from step 2 is reused).

## Rules and guardrails

- **Fidelity over creativity.** Match MAUI's public surface and behavior. Idiomatic adaptation to T is
  expected *in form* (naming, language features) but **not in coverage or semantics**.
- **No invented behavior.** If a member's behavior isn't in the vault summary, the tests, or the source,
  go read the source. Don't ship a guess. If genuinely unspecified, leave a `// TODO: verify against
  src/<path>` marker rather than fabricating.
- **Preserve the architecture.** Keep virtual-view ⇄ handler ⇄ native separated. Do not collapse the
  handler seam into a direct renderer — that seam is the entire point of MAUI.
- **Mirror the partial-class split** as T's equivalent (cross-platform core + per-platform
  implementation files/modules), so adding platforms later is additive.
- **One platform first.** Pick the simplest viable target platform (decided in the language profile),
  get the full vertical slice working, *then* add platforms behind the same handlers.
- **Skip the out-of-scope namespaces** (`PROJECT.md §3`): Compatibility, Internals, legacy renderers,
  BlazorWebView, Maps — unless explicitly requested.
- **Tests are the contract.** When source and your intuition disagree, the test wins. When there's no
  test, write one that captures the source's behavior, then port.

## Verification & "done" per component

A component is done only when: (a) its public surface matches the `PublicAPI.Shipped.txt` coverage for
that type, (b) its ported tests pass, and (c) for UI components, it renders/reacts in the host app on
the first platform. Partial ports must be marked as such in the manifest, never silently.

## Parity comparison policy (visual fidelity rulings)

How the iOS parity loop judges the C++ port against real .NET MAUI. The comparison runs through
`port/cpp/tools/parity/` (Google Gemini by default, Claude vision as the quota fallback); each page's
differences are split into **port_diffs** (fix) vs **maui_quirks** (MAUI-side, discuss). User rulings
(2026-06-21) — DO NOT re-litigate these without the user:

1. **Microsoft's MAUI render IS the ground truth for all page CONTENT.** The port matches MAUI's actual
   rendered demos, in both code (syntax) and visuals: colors, control sizes, internal spacing, text,
   corner radius, fonts. Any *content* difference vs MAUI — **including any color difference** — is a
   **port bug to fix**, never excused as a "MAUI imperfection." MAUI's color/appearance is correct by
   definition.
2. **The only MAUI imperfection the port need not copy is the harness WRAPPER.** MAUI wraps each demo
   page in an inset card (large whole-screen padding/margins) and crops the page top/bottom. The port
   deliberately does **not** replicate this: it uses *modest* page padding for UX (some spacing — not
   content jammed to the screen edges — but **much less than MAUI's**), and may show more of the page.
   Do NOT flag the outer-inset magnitude, the resulting uniform global shift, or the top/bottom crop as
   port bugs. The inset is a uniform outer margin only — it never changes a control's size, color, or
   internal spacing; if those differ, that is a port_diff. (Port TODO: give the port modest page padding
   rather than edge-to-edge content.)
3. **New MAUI imperfections → flag, don't act.** When a sweep surfaces a MAUI-side quirk not covered
   above, record it in `docs/comparison/PARITY_REVIEW.md` (maui_quirks) and **pause for a user ruling** —
   neither auto-ignore nor auto-fix. Append the approved ruling to this list.

Workflow: run the sweep **review-only** (writes `PARITY_REVIEW.md`, board untouched) → user verifies and
rules on quirks → only then `--commit-board` adopts verdicts and the port_diffs become fix candidates.

## Progress tracking

Maintain a `port/STATUS.md` (create it on first run) with one row per component:
`| Component | Layer | Contract✓ | Tests ported | Impl | Platform(s) | Notes |`. Update it at step 7 of
the loop. This is the single source of truth for what's complete vs. stubbed.

## When to stop and ask the user

Use a question (don't guess) when:
- The **target language or first platform** is still unspecified and you're about to make a binding
  decision that depends on it.
- A MAUI behavior depends on a .NET-specific facility (e.g. `BindableProperty` value precedence,
  `SynchronizationContext`, reflection in XAML) that has **multiple** reasonable mappings in T.
- Scope is ambiguous (which controls/namespaces for this milestone).

## Conventions for working in this repo

- The original source is **read-only reference** — do not modify `src/`, `vault/`, or `graphify-out/`.
  Write the port into its own tree (decided with the language profile; e.g. `port/<lang>/`).
- Prefer `grep`/Glob to locate tests and source by type name over guessing paths.
- Keep commits/changes scoped to one component slice; verify before moving on.

---

## LANGUAGE-SPECIFIC — C++23

The language is **C++23**; the full profile is [`cpp/PROFILE.md`](cpp/PROFILE.md). **Read it before
emitting any code.** Operating rules that override/extend the generic workflow above:

- **API style is `snake_case`** (`maui::controls::button`, `view.text.set(...)`, `btn.clicked.connect(...)`).
  In every public header, comment the originating C# FQN so the vault/tests stay cross-referenceable
  (`// maui::controls::button  <=  Microsoft.Maui.Controls.Button`).
- **Backend order:** build the **headless** backend first (M0–M5, fully unit-testable, no device), then
  the **macOS** Obj-C++ backend (M2 proves the seam), then iOS. Mirror MAUI's partial-class split as a
  core `.cpp` + a per-backend `.cpp`/`.mm` under `src/platform/<backend>/`.
- **Ownership is explicit (no GC).** Follow the doctrine in `PROFILE.md §8` from layer 1: `shared_ptr`
  element tree, `weak_ptr` for parent/binding back-refs, pimpl-owned native views, token-based event
  teardown in destructors. Do not defer ownership decisions.
- **No reflection.** Use explicit registration (`PROFILE.md §6`): `register_handler<…>()`, static
  property registrars, a type-keyed service registry. **Defer XAML (layer 6)** behind the code-first API.
- **Tooling commands** — fast inner loop: `tools/dev.sh [regex]` (incremental Ninja rebuild + `ctest -R`
  on one preset, seconds). Full pre-push gate (all backends + sanitizers + clang-tidy): `tools/gate.sh`
  (`--fast` subset / `--clean` from-scratch). Both use `ccache` and `ctest -j`; **do not run all presets
  for routine iteration**, and never wipe a `build/<preset>` dir to "be safe" (Ninja is incremental).
  Raw form: `cmake --preset headless && cmake --build --preset headless && ctest --preset headless -j`.
  See `STATUS.md` "Build & test" for the performance model. Port C# tests (`src/**/tests`) into GoogleTest
  `TEST`/`TEST_P` cases — they remain the behavioral oracle.
- **Property system & handler infra:** implement per the sketches in `PROFILE.md §7` (bindable_property
  with value precedence) and `§5` (CRTP `view_handler` + non-generic `i_view_handler`). Verify property
  precedence against the ported `src/Controls/tests/Core.UnitTests` BindableProperty/Binding tests.

**Open decisions to confirm with the user as you reach them** (defaults in `PROFILE.md §11`):
view-owns-handler vs handler-owns-view; `property<T>` ergonomics; `concept` vs `i_*` per contract;
modules vs headers. Also confirm the **dev machine / first real platform** if it changes from macOS.

Begin at **M0 (Graphics)** per `PROJECT.md §5`, on the headless backend.

### Note for the macOS session
This kit (and the `vault/` it references) is committed so it can be pulled on the dev machine.
`graphify-out/graph.json` (the full node-level graph, >100 MB) is **not** pushed — use the committed
`graphify-out/GRAPH_REPORT.md` for the dependency overview, or regenerate the graph locally if needed.
The original C# `src/` and its `tests/` are in the repo and are the implementation + behavioral sources.
