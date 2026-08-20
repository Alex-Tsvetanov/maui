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
   → the ground truth `src/**/PublicAPI/<tfm>/PublicAPI.Shipped.txt` (the exact shipped public surface),
   read alongside the type's C# source for member semantics.
2. **How must it behave?** (defaults, edge cases, events, ordering, errors)
   → the **tests** in `src/**/tests/**` are the oracle. Find the test(s) covering the type and treat
   them as the executable spec. If behavior is untested, fall to the source.
3. **How is it implemented?** (algorithms, native wiring)
   → the original C# under `src/<area>/src/...` — the cross-platform `Foo.cs` plus the per-platform
   partials `Foo.{Android,iOS,Windows,Tizen,Standard}.cs`.

(Architecture/intent formerly in `vault/Conceptual/` and the `graphify-out/` dependency graph: those
trees were removed — read the C# source directly, or learn.microsoft.com for conceptual background;
restore from git history if you truly need them.)

## Workflow: bottom-up, one layer at a time

Follow the layer order in `PROJECT.md §3`. **Do not start a layer until the layer below it passes its
tests.** Within a layer, port in small, verifiable slices (one control / one primitive at a time).

### The per-component porting loop (TDD)

For each component (e.g. `Button`, `Color`, `Grid`):

1. **Contract** — read the type's `PublicAPI.Shipped.txt` entry (authoritative) + its C# source; list
   its public members and platform availability.
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
- **No invented behavior.** If a member's behavior isn't in the tests, go read the source.
  Don't ship a guess. If genuinely unspecified, leave a `// TODO: verify against
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

> ### ⚠ STANDING DOCTRINE — NO EXEMPTIONS (2026-08-17)
>
> User directive, verbatim in intent: *"your task is to fix all these inconsistencies when comparing
> against MAUI so that the C++ port acts as MAUI. Any previously established rule that says otherwise
> should be invalidated right now."*
>
> **Microsoft's MAUI render IS the ground truth for all page CONTENT — colours, control sizes, internal
> spacing, text, corner radius, fonts.** Where MAUI renders X and the port renders Y, the port is wrong
> and Y is a bug to fix — including when MAUI's X looks like a platform limitation, an init-timing
> artifact, or a bug. Any content difference, **including any colour difference**, is a port bug, never
> a "MAUI imperfection". "MAUI does something odd here" is a specification, not an excuse.
>
> (This paragraph IS former ruling 1, folded in — the two said the same thing. Former ruling 3
> "flag, don't act" is RETIRED: it existed to route newly-found quirks to a user ruling, and there are
> no exemptions left to grant. Record MAUI-side oddities in `docs/comparison/PARITY_REVIEW.md` and keep
> going. Citations to rulings 1 and 3 elsewhere resolve here.)
>
> This revoked five quirk-exemption rulings (2, 7, 8, 9, 10) and ruling 12's exemption clause; their
> text was DELETED on 2026-08-20 rather than left as revoked-but-readable, because a live exemption and
> a dead one look identical at a glance. The surviving numbers are deliberately NOT renumbered — commit
> messages, PARITY_REVIEW.md and port SOURCE FILES cite them by number.
>
> The one thing that is still exempt is CAPTURE CHROME, and it is not a rendering exemption: a system
> status bar whose clock/battery differ between two shots differs because of the TIME, not the port
> (pixel_score's `crop_top`).

How the iOS parity loop judges the C++ port against real .NET MAUI. The comparison runs through
`port/cpp/tools/parity/` (Google Gemini by default, Claude vision as the quota fallback); each page's
differences are split into **port_diffs** (fix) vs **maui_quirks** (MAUI-side, discuss). User rulings
(2026-06-21) — read them THROUGH the 2026-08-17 override above, which revokes every exemption:

4. **Match MAUI's NATIVE-default control rendering, and do NOT adopt `Styles.xaml`.** The reference app
   (`port/maui-reference/app/App.xaml`) deliberately does not merge `Styles.xaml` — it renders
   native-default controls plus the system font — so the demo's default Button/Label/Entry styles are
   never applied and the port must not add them. It must, however, reproduce the NATIVE control
   defaults: an unset `Padding` leaves the native default (mirroring MAUI's `MapPadding`) rather than
   zeroing it. Zeroing `UIButton`'s content insets is what once crammed `clipping`'s digit row.

Workflow: run the sweep **review-only** (writes `PARITY_REVIEW.md`, board untouched) → user verifies and
rules on quirks → only then `--commit-board` adopts verdicts and the port_diffs become fix candidates.

5. **The four required comparisons (2026-07-05 ruling)** — every review model (Sonnet, Gemini, and the
   pixel-perfect/SSIM score) must judge FOUR image pairs per page per theme, not just one:
   - Comparison 1: MAUI light vs C++ light
   - Comparison 2: MAUI light vs C++ & XAML light
   - Comparison 3: MAUI dark vs C++ dark
   - Comparison 4: MAUI dark vs C++ & XAML dark

   i.e. MAUI is the ground truth compared against **both** the `cpp` (code-first builder) and `xaml`
   (compile-time-XAML) framework columns, independently, in both themes — not just `cpp` vs `maui`.
   Record each comparison's own verdict; do not average/collapse them into a single cpp-only score. The
   two columns routinely disagree and that disagreement is the finding: when the loader was corrected to
   match XamlC, `pixel_xaml` went green while `pixel` went red, which is what localised the defect to the
   code-first builder.

6. **The MAUI ground truth is `port/maui-reference/` (2026-07-05 restructure, XAML-first).** The old
   out-of-repo C#-only `~/maui-compare` app is superseded by the in-repo `port/maui-reference/app`
   (MauiReference), whose pages are the CANONICAL SHARED XAML files in `port/maui-reference/pages/` —
   the exact same `.xaml` bytes the port's `gallery_xaml` app `#embed`s (one file, two frameworks).
   The board's own MAUI column is captured STRAIGHT INTO
   `port/cpp/docs/comparison/captures/<platform>/maui/` by `recapture.py` — one writer per destination,
   on every lane. `port/maui-reference/captures/` is `port/tools/e2e/e2e.py`'s separate ground-truth
   root for the VERIFICATION_LOOP workflow; the two capture paths are decoupled, not chained. The
   deterministic 10-step verification loop any agent can resume is codified in
   **`port/maui-reference/docs/VERIFICATION_LOOP.md`** (authoring rules: `docs/AUTHORING.md`); the
   single tool driving it is **`port/tools/e2e/e2e.py`** (see its README). Ruling 4's
   `~/maui-compare/App.xaml` citation maps to `port/maui-reference/app/App.xaml`, which preserves the
   same no-Styles.xaml native-default rendering.

11. **When `src/` is AMBIGUOUS or self-contradictory, the shipped RENDER decides (2026-07-16, premise
    corrected 2026-08-20).** Originally written as "`src/` is a stale snapshot, the render is newer".
    THAT PREMISE IS FALSE and was retired by measurement: the worked example is CheckBox, where
    `CheckBoxHandler.iOS.cs` says `MinimumSize => 44f` while `MauiCheckBox.cs` says
    `DefaultSize = 18.0f`, and BOTH lines are present verbatim in our `src/`, in tag 10.0.71 (what the
    board renders) and in tag 10.0.90 (current upstream). No version sync can resolve it, because there
    is no version skew — two files in the SAME source disagree and the render follows one of them.
    So: `src/` remains the oracle for MECHANISM, and where reading it yields two answers, the shipped
    render is the tiebreak. Mark such a choice a DOCUMENTED DEVIATION in code, citing both lines and
    this ruling. The port dropped the 44 floor (`check_box_handler.mm`), fixing `entry` (+20px → 0.00)
    and `border_playground` (→ 0.37).

12. **ANYTHING MAUI CAN EXPRESS, THE PORT MUST BE ABLE TO EXPRESS — in both dialects (2026-08-20).**
    The `port/maui-reference` pages are shared XAML: the same bytes MAUI compiles and the port `#embed`s.
    When a page cannot be written faithfully because the port's loader or builder lacks a feature, the
    gap is a PORT GAP and the fix is to IMPLEMENT the missing piece — not to simplify the twin down to
    what the port already supports, and not to exempt the resulting diff. Degrading the twin hides the
    gap in the one artifact that is supposed to reveal it, and makes MAUI's own column render something
    original MAUI never would.
    Worked example: `header_footer_template` used an `<x:Array>` of plain strings because the loader had
    no string→ImageSource binding, so every row showed `cover1.jpg`. It was fixed by giving both sides a
    real per-row model bound through `{Binding Image}` — i.e. by closing the gap, not by exempting it.


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

- The original C# `src/` is **read-only reference** — do not modify it. Write the port into its own
  tree (decided with the language profile; e.g. `port/<lang>/`).
- Prefer `grep`/Glob to locate tests and source by type name over guessing paths.
- Keep commits/changes scoped to one component slice; verify before moving on.

---

## LANGUAGE-SPECIFIC — C++26

The language is **C++26**; the full profile is [`cpp/PROFILE.md`](cpp/PROFILE.md). **Read it before
emitting any code.** Operating rules that override/extend the generic workflow above:

- **API style is `snake_case`** (`maui::controls::button`, `view.text.set(...)`, `btn.clicked.connect(...)`).
  In every public header, comment the originating C# FQN so the source/tests stay cross-referenceable
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

### Note for the dev machine
The original C# `src/` and its `tests/` are in the repo and are the implementation + behavioral sources
(plus `src/**/PublicAPI/*.Shipped.txt` for the exact public surface). The former `vault/` API/conceptual
docs and `graphify-out/` dependency graph were removed to keep the tree port-scoped — restore either from
git history if a future porting slice needs them.
