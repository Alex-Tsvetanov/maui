# STATUS — MAUI → C++23 port progress

> One row per component. Update at step 7 of the per-component loop (`CLAUDE.md`). Never mark a
> partial port as done silently — use the Notes column.
> Legend: ✅ done · 🚧 in progress · ⬜ not started · — n/a

## Build & test (headless) — run from `port/cpp/`

```bash
export VCPKG_ROOT="$HOME/vcpkg"          # registry clone; brew's vcpkg binary alone lacks the toolchain file
cmake --preset headless && cmake --build --preset headless
ctest --preset headless                  # all ported tests (graphics + core + controls: 183 cases, green)
./build/headless/maui_graphics_benchmarks --benchmark_min_time=0.02s   # Google Benchmark (not a ctest test)
```

**Resume:** continue to the next milestone below, following `CLAUDE.md`. **M1 (Core) is COMPLETE; M2
(button) is IN PROGRESS — the headless vertical slice (M2a) is done; the macOS AppKit backend (M2b) is next.**
The `PROFILE.md §11` decisions are **locked** (view owns handler; `property<T>` member object;
per-type `concept`-vs-`i_*` rule; headers not modules). M1 build order — all done: `event`,
`dispatcher`, `setter_specificity`(+list), `bindable_property<T>` / `bindable_object` / `property<T>`
(fully typed, no `std::any`), primitives (`thickness`/`font`/enums), contracts (`i_element` /
`i_transform` / `i_view` / `i_text_style` / `i_text`), and the handler seam (`type_tag`,
`property_mapper`/`command_mapper`, `i_element_handler`/`i_view_handler`, CRTP `view_handler`,
`handler_registry`/`service_registry`). **M2a (done):** the Controls layer's first slice — `i_button` /
`i_padding` / `i_button_stroke` / `i_text_button` contracts, the cross-platform `button_handler`
(+ headless platform partial), a minimal CRTP `view<ViewInterface>` base, and the `button` control —
proving the seam both directions on headless (Text virtual→native via the mapper; native tap→`clicked`
via `send_*`). **M2 API decision (locked):** virtual-view interfaces stay bare-noun getters and concrete
controls expose **method accessors** (`button.text()`/`set_text()`), each backed by a private
`property<T>` value engine (a property change → `view::on_property_changed` → `handler->update_value`).
**Next: M2b** — the macOS AppKit (Obj-C++ `.mm`) `button_handler` over a real `NSButton` + target-action,
a minimal sample app proving a tap, and the PROFILE §6 `MAUI_REGISTER_HANDLER` self-registration macro.

## Tooling — format, lint, sanitizers (run from `port/cpp/`)

- **clang-format** (v22, `port/cpp/.clang-format`, Microsoft base / 120 col / Allman): applied
  automatically after every Write/Edit by the `.claude/settings.json` PostToolUse hook
  (`port/cpp/tools/format-hook.sh`, scoped to C/C++ under `port/cpp/`). Manual: `clang-format -i <file>`.
- **clang-tidy** (`port/cpp/.clang-tidy`, a curated ruleset; uses the llvm keg at
  `/opt/homebrew/opt/llvm/bin`). Off by default; lint the library sources with the `tidy` preset —
  advisory, findings don't fail the build: `cmake --preset tidy && cmake --build --preset tidy`.
  **The M0 sources (`color`, `path_builder`, `path_f`) are clang-tidy-clean — 0 findings.** When
  The M1 core sources **and headers** (events/dispatcher/bindable/primitives/contracts + the handler
  seam) are likewise clang-tidy-clean — verified with a full-header pass (`--header-filter`). When
  invoking the keg's clang-tidy directly it needs the AppleClang sysroot, e.g.
  `clang-tidy --extra-arg=-isysroot --extra-arg="$(xcrun --show-sdk-path)" -p build/headless <file>`.
- **Sanitizers** (`Sanitizers.md`) — target-level via the `maui_sanitizers` interface lib, one lane each:
  - `cmake --preset asan-ubsan && cmake --build --preset asan-ubsan && ctest --preset asan-ubsan` — ASan+UBSan (default checked build; **183/183 green**)
  - `cmake --preset tsan && cmake --build --preset tsan && ctest --preset tsan` — ThreadSanitizer (**183/183 green**)
  - `msan` preset is for **Linux/Clang CI only** — `-fsanitize=memory` is unsupported on AppleClang/macOS.

## Milestones (see `PROJECT.md §5`)

| Milestone | Description | Status |
|---|---|---|
| M0 | Graphics primitives compile + pass ported tests | ✅ |
| M1 | Core contracts + property/handler infra + dispatcher, unit-tested | ✅ |
| M2 | `button` end-to-end (headless → macOS), tap works in sample app | 🚧 |
| M3 | Layout measure/arrange (`stack_layout`, `grid`) pass layout tests | ⬜ |
| M4 | Control set v1 (label, entry, image, layouts, page) on macOS | ⬜ |
| M5 | `bindable_object`/`bindable_property`, binding, style, lifecycle | ⬜ |
| M6 | Second platform (iOS) behind the same handlers | ⬜ |
| M7 | XAML and/or Essentials (as prioritized) | ⬜ |

## Components

| Component | Layer | Contract✓ | Tests ported | Impl | Platform(s) | Notes |
|---|---|---|---|---|---|---|
| project scaffold (CMake/presets/vcpkg/GoogleTest+GMock+Google Benchmark) | infra | — | — | ✅ | headless | green smoke `ctest` (Step 1a done); AppleClang 21, cmake 4.3, ninja, vcpkg toolchain |
| `color` + `colors` | graphics | ✅ | ✅ | ✅ | headless | 36 GTest cases green (incl. exact HSL/HSV→hex, 147 named colors round-trip). System.Numerics (Vector4) interop omitted — TODO; byte/int ctors exposed as from_rgb/from_rgba factories |
| `point`/`point_f`/`size`/`size_f` | graphics | ✅ | ✅ | ✅ | headless | 124 parse cases + cross-cast/operator characterization green; double+float families; Vector2/Matrix3x2 interop omitted (TODO) |
| `rect`/`rect_f` | graphics | ✅ | ✅ | ✅ | headless | 66 parse cases + contains/union/intersect/inflate/edges/casts characterization; `Union`→`union_with` (C++ keyword) |
| `path_f` + `path_operation` | graphics | ✅ | ✅* | ✅ | headless | build/shapes/flatten/bounds/reverse/rotate/separate/arc + SVG arc (ArcFlattener, SVGArcTo). *characterization tests (no C# oracle). Matrix3x2 `Transform` omitted (TODO) |
| `path_builder` (SVG parser) | graphics | ✅ | ✅* | ✅ | headless | M/L/H/V/C/S/Q/T/A/Z abs+rel; *characterization tests. Faithful to PathBuilder.cs (incl. its dead implicit-after-M branch) |
| benchmarks (color, path) | graphics | — | — | ✅ | headless | BenchmarkDotNet → Google Benchmark; `maui_graphics_benchmarks` builds + runs |
| `event<>` + `scoped_connection` | core | ✅ | ✅* | ✅ | headless | 16 GTest cases; multicast with snapshot-on-raise (mirrors .NET `X?.Invoke`) + token/RAII teardown (the WeakEventManager role per §8). Uses port-provided `move_only_function` (libc++ lacks `std`'s). *characterization — infra, no C# oracle |
| `i_dispatcher` + `manual_dispatcher` | core | ✅ | ✅* | ✅ | headless | 11 GTest cases; mirrors `IDispatcher`/`IDispatcherTimer` (dispatch / dispatch_delayed / timer + is_dispatch_required). Headless impl is a deterministic **virtual-clock** pump (`run_pending`/`advance`) — no wall-clock/threads. *characterization (C# Standard impl just throws) |
| `setter_specificity` (+ `setter_specificity_list`) | core | ✅ | ✅* | ✅ | headless | packed-uint64 precedence key + per-property value store; 14 GTest cases incl. a port of `SetterSpecificityListTests`. *characterization |
| `bindable_property<T>` + `bindable_object` + `property<T>` | core | ✅ | ✅* | ✅ | headless | **fully-typed** value layer, **no `std::any`** (per §7): each `property<T>` member owns a `setter_specificity_list<T>` and the value precedence; `bindable_object` is just the notification base; `bindable_property<T>` is the typed shared descriptor. Change-notification (changing→changed, real-change-only), coerce/validate, lazy+cached default-creator, handler override, zero-copy `get()→const T&`, per-instance `.changed`. 15 cases derived from `Core.UnitTests`. *characterization |
| primitives: `visibility`/`flow_direction`/`layout_alignment`/`thickness`/`font` | core | ✅ | ✅* | ✅ | headless | Core value types/enums the view contracts depend on; 26 GTest cases. *characterization |
| `i_element`/`i_transform`/`i_view`/`i_text_style`/`i_text` | core | ✅ | ✅* | ✅ | headless | virtual-view contracts (abstract `i_*` classes, §11 per-type rule). Full `IView` surface; heavy sub-objects (paint/semantics/shadow/clip) + the typed view-handler accessor forward-declared/deferred to M3/M4/#22. 5 GTest cases (mock conformance). *characterization |
| handler seam: `i_element_handler`/`i_view_handler`, `property_mapper`/`command_mapper`, CRTP `view_handler`, `type_tag`, `handler_registry`/`service_registry`, `i_maui_context` | core | ✅ | ✅* | ✅ | headless | 17 GTest cases (mock handler over a fake platform view: connect creates+connects+maps, update_value/invoke, disconnect; mapper chaining/override; type_tag identity; registries). No-reflection **explicit registration** (PROFILE §6); platform view = `void*` and command args = `std::any` (boundary-confined erasure only). M1 simplifications noted in headers: no merged-mapper cache, no CanInvokeMappers gate, reciprocal `view.Handler=this` deferred to M2 hosting. *characterization |
| `i_button`/`i_padding`/`i_button_stroke`/`i_text_button` | core | ✅ | ✅* | ✅ | headless | Button virtual-view contracts (IButton : IView, IPadding, IButtonStroke; ITextButton adds IText). `IButton.Pressed/Released/Clicked()` → `send_pressed/send_released/send_clicked()` (renamed: C++ can't share a name between the method and the control's event). *characterization |
| `view<ViewInterface>` (minimal control base) | controls | ✅ | ✅* | ✅ | headless | bindable_object + i_view impl + handler ownership/wiring + measure/arrange seam. Templated on the control's view-interface to avoid the i_view diamond (no virtual inheritance). **M2 subset** — full VisualElement property set / real layout deferred to M3/M4. *characterization |
| `button_handler` (+ headless partial) | core/handlers | ✅ | ✅* | ✅ | headless | CRTP `view_handler<button_handler, i_button, button_platform>`; cross-platform mapper tables (Text live; text_color/font/stroke/padding deferred) + per-backend platform recipe (create/connect/disconnect/map_text). `button_platform` is a single cross-platform struct (native slot + headless mirror + event callbacks). *characterization |
| `button` control | controls | ✅ | ✅* | ✅ | headless→macOS | The Rosetta Stone's virtual view. Text (bindable, mapped), clicked/pressed/released events + optional command, IsEnabled gating (ButtonElement order: command→event; release always clears IsPressed). 12 GTest cases: control behavior + the headless seam end-to-end (both directions). macOS native verified in M2b. *characterization |

_(extend this table as components are added)_
