# PROJECT — Recreate .NET MAUI in another language

> **Status:** language-agnostic charter. The target language is **not yet chosen**.
> Sections marked `LANGUAGE-SPECIFIC: TBD` will be filled in once the language is decided.
> This file defines **what** is being built. `CLAUDE.md` (same folder) defines **how** an agent should build it.

---

## 1. Mission

Reimplement the .NET MAUI UI framework in a target language **T**, preserving its public API
shape and runtime behavior, by deriving the design from the original C# source and its tests
in this repository — **not** by guessing.

The deliverable is a framework in language T that lets a developer write **one** UI definition and
run it natively on multiple platforms, mirroring MAUI's architecture and behavior.

## 2. What MAUI actually is (the one insight that matters)

MAUI is **not** a renderer. It is an **abstraction that maps one cross-platform UI description onto
several native UI toolkits** (Android Views, iOS UIKit, Windows WinUI, …). The whole framework is
organized around one pattern — the **Handler** — repeated for every control:

```
  Cross-platform "virtual view"        Handler (the mapper)            Native platform view
  IButton  (a contract, no UI)  <-->   ButtonHandler  <-->  Android AppCompatButton / iOS UIButton / WinUI Button
  Button   (developer-facing API)      PropertyMapper + CommandMapper
```

- A **virtual view** is a plain interface/object with properties (e.g. `IButton.Text`). It has **no** rendering.
- A **handler** owns a **PropertyMapper** (`"Text" -> MapText`) and a **CommandMapper** (imperative ops).
  When a property changes, the mapper invokes the platform-specific `MapText(handler, view)` which pushes
  the value to the native control. This is the seam that makes one API drive N toolkits.
- **Controls** (`Button`) are the friendly, bindable, XAML-able classes that implement the virtual-view
  interfaces and add their own mapper entries.

Reference example (read these first — they are the Rosetta Stone of the whole port):
- Contract:        `src/Core/src/Core/IButton.cs`  (+ `IView.cs`, `IElement.cs`, `IText.cs`)
- Handler:         `src/Core/src/Handlers/Button/ButtonHandler.cs`  (the PropertyMapper/CommandMapper)
- Handler iface:   `src/Core/src/Handlers/Button/IButtonHandler.cs`
- Platform glue:   `src/Core/src/Handlers/Button/ButtonHandler.{Android,iOS,Windows,Tizen,Standard}.cs`
- Control:         `src/Controls/src/Core/Button/Button.cs` + `Button.Mapper.cs`

**Internalize the Button slice end-to-end before porting anything.** Every other control is the same shape.

## 3. Architecture & dependency order (build bottom-up)

| # | Layer | C# namespace(s) | Source root | Depends on | Notes |
|---|-------|-----------------|-------------|-----------|-------|
| 0 | **Graphics** | `Microsoft.Maui.Graphics` | `src/Graphics/src/Graphics` | nothing | Primitives: `Color`, `PointF/SizeF/RectF`, `ICanvas`, `IDrawable`, `Paint`, geometry. Pure, portable, **start here**. |
| 1 | **Core abstractions** | `Microsoft.Maui` | `src/Core/src/Core` | Graphics | The virtual-view interfaces (`IView`, `IElement`, `ILayout`, `IButton`, …) + handler contracts. The contract layer. |
| 2 | **Handlers** | `Microsoft.Maui.Handlers` | `src/Core/src/Handlers` | Core | The mappers. One handler per control + per-platform partials. |
| 3 | **Platform** | `Microsoft.Maui.Platform` | `src/Core/src/Platform/<OS>` | Handlers + native SDK | Native glue. **Platform-bound, not language-bound** — the hardest, least portable part. |
| 4 | **Layouts** | `Microsoft.Maui.Layouts` | `src/Core/src/Layouts` | Core | Measure/arrange algorithms (Grid, Stack, Flex, Absolute). Algorithmic — port carefully against tests. |
| 5 | **Controls** | `Microsoft.Maui.Controls` | `src/Controls/src/Core` | Core, Handlers, Layouts | Developer API: pages, views, `BindableObject`/`BindableProperty`, binding engine, `Style`, `Shell`, gestures. The bulk of the surface. |
| 6 | **XAML** | `Microsoft.Maui.Controls.Xaml` | `src/Controls/src/Xaml` | Controls | Markup → object graph. Optional early; can be deferred behind the code-first API. |
| 7 | **Essentials** | `Microsoft.Maui.ApplicationModel`, `.Devices*`, `.Storage`, `.Media`, `.Networking`, `.Authentication` | `src/Essentials/src` | Core | Device/platform APIs. Independent of the UI stack — can be ported in parallel. |
| 8 | **Hosting** | `Microsoft.Maui.Hosting` | `src/Core/src` (Hosting) | all | App builder + DI registration that wires handlers to controls. Needed to run anything. |

**Explicitly out of scope** (do not port unless asked): `Microsoft.Maui.Controls.Compatibility*`
(Xamarin.Forms back-compat), `*.Internals`, `Controls.Handlers.Compatibility` (legacy renderers),
`BlazorWebView`, and the `*.Maps` add-ons. These are legacy or niche and are not core MAUI.

## 4. Authoritative information sources (what to consult, and for what)

The four layers below are the complete spec. Each answers a different question:

| You need… | Source | Where |
|-----------|--------|-------|
| **API contract** (exact public surface, names, signatures, platforms) | the Roslyn surface files | the ground truth `src/**/PublicAPI/<tfm>/PublicAPI.Shipped.txt` |
| **Architecture & intent** (how a feature works, why) | the C# source (+ learn.microsoft.com for background) | `src/<area>/src/...`, read for structure/intent |
| **Behavioral spec** (the real contract: defaults, edge cases, events, ordering) | the **test suite** — treat as the oracle | `src/Controls/tests/Core.UnitTests`, `src/Controls/tests/Xaml.UnitTests`, `src/Core/tests/**`, `src/Graphics/tests/**`, and device tests |
| **Implementation reference** (algorithms, platform wiring) | the original C# source | `src/<area>/src/...` (cross-platform `.cs` + `.{Android,iOS,Windows,...}.cs` partials) |

> **The three layers of truth (all in `src/`):** `PublicAPI.Shipped.txt` gives the API *shape*; the
> **tests** tell you *how it must behave*; the **source** tells you *how it's done*. A faithful port
> needs all three. (The former `vault/` docs + `graphify-out/` graph were removed — restore from git
> history if a slice ever needs the conceptual/dependency views.)

## 5. Milestones (definition of progress)

1. **M0 — Graphics primitives** compile and pass ported Graphics tests (`Color`, `RectF`, geometry math).
2. **M1 — Core contracts** — the virtual-view interfaces + a minimal handler base (`PropertyMapper`,
   `CommandMapper`, `ViewHandler` base) exist and are unit-tested.
3. **M2 — First control, one platform** — `Button` end-to-end (contract → handler → one native platform)
   renders and responds to a tap in a tiny host app. **This proves the whole architecture.**
4. **M3 — Layout** — measure/arrange for `StackLayout` + `Grid` pass the ported layout tests.
5. **M4 — Control set v1** — Label, Entry, Image, Layouts, Page/ContentPage on the first platform.
6. **M5 — Controls semantics** — `BindableObject`/`BindableProperty`, data binding, `Style`, lifecycle.
7. **M6 — Second platform** — add a second native backend behind the same handlers (validates the seam).
8. **M7 — XAML and/or Essentials** — as prioritized.

## 6. Success criteria

- **API parity:** the public surface matches `PublicAPI.Shipped.txt` for each ported namespace
  (names mapped to idiomatic T per the language profile, but 1:1 in coverage).
- **Behavioral parity:** the ported unit tests (from `src/**/tests`) pass — these *are* the spec.
- **Runtime proof:** a sample app shows a `Button`, `Label`, and a `StackLayout`/`Grid` rendering and
  reacting on at least one real target platform (M2 + M4).
- **Architectural fidelity:** the virtual-view ⇄ handler ⇄ native split is preserved (not collapsed
  into a monolithic renderer).

## 7. Known hard parts (budget for these)

- **Native interop per platform** (layer 3) — the bulk of the effort, and bound to each OS's SDK, not
  to T. Porting the language does not remove this work.
- **Layout measure/arrange** (layer 4) — subtle algorithms; lean hard on the layout tests.
- **BindableProperty + binding engine** (layer 5) — property system with change notification, coercion,
  value precedence (default < style < binding < manual). Read source + tests closely.
- **XAML** (layer 6) — a parser/loader; defer until the code-first API is solid.

---

## LANGUAGE-SPECIFIC — C++26

The target language is **C++26**, with an **idiomatic `snake_case`** public API. Full details live in
[`cpp/PROFILE.md`](cpp/PROFILE.md). Key commitments that affect this charter:

- **Backends (all platforms documented):** `headless`/Standard → **macOS (AppKit, Obj-C++)** →
  iOS + Mac Catalyst (UIKit) → Windows (WinUI 3 / C++/WinRT) → Android (JNI/NDK), plus **Linux (GTK4)**
  as an *extension beyond MAUI's scope*. See `cpp/PROFILE.md §4`.
- **First platform:** the **headless** backend (covers M0–M5 with no device), then **macOS** as the
  primary dev target (Apple interop is the cleanest), then iOS.
- **Two net-new C++ concerns** the C# original doesn't have, addressed in the profile:
  1. **Ownership/lifetime doctrine** (no GC) — `shared_ptr` element tree, `weak_ptr` to break parent/
     binding cycles, pimpl-owned native views (`§8`).
  2. **No reflection** — handler discovery, DI, and XAML become **explicit registration / codegen**
     (`§6`); XAML (layer 6) is deferred behind the code-first API.
- **Tooling:** CMake + Ninja, GoogleTest + GMock via `ctest`, Google Benchmark, vcpkg. The C# unit
  tests in `src/**/tests` are ported into GoogleTest and remain the behavioral oracle.
