# STATUS — MAUI → C++23 port progress

> One row per component. Update at step 7 of the per-component loop (`CLAUDE.md`). Never mark a
> partial port as done silently — use the Notes column.
> Legend: ✅ done · 🚧 in progress · ⬜ not started · — n/a

## Build & test (headless) — run from `port/cpp/`

```bash
export VCPKG_ROOT="$HOME/vcpkg"          # registry clone; brew's vcpkg binary alone lacks the toolchain file
cmake --preset headless && cmake --build --preset headless
ctest --preset headless                  # all ported tests (graphics + core + controls: 214 cases, green)
./build/headless/maui_graphics_benchmarks --benchmark_min_time=0.02s   # Google Benchmark (not a ctest test)
```

**macOS / AppKit backend** (real NSViews; Obj-C++ `.mm` + ARC):

```bash
cmake --preset apple && cmake --build --preset apple && ctest --preset apple   # 207 cases incl. real NSButton tap
./build/apple/maui_button_sample                                                # sample window (Ctrl-C / close to quit)
```

**Resume:** continue to the next ⬜ milestone below, following `CLAUDE.md`. **M1 (Core) and M2 (button,
the Rosetta Stone) are COMPLETE** — the virtual-view ⇄ handler ⇄ native seam is proven end-to-end on
**both** the headless backend (214 tests) and the **macOS AppKit backend** (real `NSButton`, 207 tests
incl. a native tap via `performClick:`). **M3 (layout) IN PROGRESS: M3a (stack managers) done; M3b (Grid) next.**
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
**M2b (done):** the macOS AppKit backend — `src/platform/apple/button_handler.mm` (Obj-C++/ARC) drives a
real `NSButton` (Text→`title`; a target-action trampoline routes the click to `send_clicked()`), the
`button_platform` gained a backend-defined destructor (RAII for the retained NSButton), CMake grew an
`apple` preset (OBJCXX + `-fobjc-arc` + `-framework Cocoa`), the seam is verified by `button_apple_tests.mm`
(real `NSButton performClick:` → `clicked`), and `maui_button_sample` is a runnable macOS window hosting
the control. **M2 backfill (done):** opt-in self-registration — `default_handler_registry()`,
`handler_registrar<view,handler>`, and the `MAUI_REGISTER_HANDLER` sugar (button self-registers in
button.cpp); and the **full button surface** — text_color / font / character_spacing / padding / stroke
are now bindable + mapped (headless mirrors every property; AppKit maps font / text_color (contentTintColor)
/ stroke (layer border), with character_spacing + padding documented as AppKit TODOs needing an attributed
title / custom cell). **Deferred to M3/M4:** the shared **ViewMapper** for the generic IView properties
(needs a common platform-view base + the visual-element property set) and image source.
**M3a (done):** the layout foundation — `dimension`, `i_container` / `i_layout` / `i_stack_layout`,
`i_layout_manager` contracts — and a new `maui_layouts` library: `layout_manager` (ResolveConstraints)
+ `stack_layout_manager` (MeasureSpacing) bases + `vertical_stack_layout_manager` /
`horizontal_stack_layout_manager`. Pure cross-platform measure/arrange (no native); 28 GTest cases
ported from C#'s Stack*LayoutManagerTests (spacing / padding / min-max / collapsed-vs-hidden / fill).
**Next: M3b — Grid** (`grid_length` Auto/Star/Absolute, row/column definitions + spans, the grid layout
manager + GridLayoutManagerTests). The layout *controls* + native container panel land at M4.

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
  - `cmake --preset asan-ubsan && cmake --build --preset asan-ubsan && ctest --preset asan-ubsan` — ASan+UBSan (default checked build; **214/214 green**)
  - `cmake --preset tsan && cmake --build --preset tsan && ctest --preset tsan` — ThreadSanitizer (**214/214 green**)
  - `msan` preset is for **Linux/Clang CI only** — `-fsanitize=memory` is unsupported on AppleClang/macOS.

## Milestones (see `PROJECT.md §5`)

| Milestone | Description | Status |
|---|---|---|
| M0 | Graphics primitives compile + pass ported tests | ✅ |
| M1 | Core contracts + property/handler infra + dispatcher, unit-tested | ✅ |
| M2 | `button` end-to-end (headless → macOS), tap works in sample app | ✅ |
| M3 | Layout measure/arrange (`stack_layout`, `grid`) pass layout tests | 🚧 |
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
| `button` control | controls | ✅ | ✅* | ✅ | headless + macOS | The Rosetta Stone's virtual view. **Full own surface bindable + mapped**: Text + the i_text_style appearance (text_color/font/character_spacing) + Padding + the i_button_stroke border (stroke_color/thickness/corner_radius); clicked/pressed/released events + optional command; IsEnabled gating (ButtonElement order: command→event; release always clears IsPressed). Self-registers its handler (MAUI_REGISTER_HANDLER). 14 headless GTest cases (seam both directions + every property). *characterization |
| `button_handler` AppKit backend (`.mm`) + `maui_button_sample` | platform/apple | ✅ | ✅* | ✅ | macOS | Real `NSButton` via Obj-C++/ARC: Text→`title`; font→`NSFont`; text_color→`contentTintColor`; stroke→layer border; target-action trampoline → `send_clicked`; RAII NSButton release. (character_spacing/padding are documented AppKit TODOs.) 7 GTest cases drive a real NSButton (`performClick:` → `clicked`; appearance; disabled suppressed; disconnect; registry-resolved). Sample app = a live NSWindow round-tripping a tap. Translated from ButtonHandler.iOS.cs (no AppKit oracle in mainline MAUI; macOS there is Mac Catalyst/UIKit). *characterization |
| handler self-registration (`default_handler_registry`/`handler_registrar`/`MAUI_REGISTER_HANDLER`) | core | ✅ | ✅* | ✅ | headless + macOS | Opt-in §6 self-registration over the explicit primitive; macro-free registrar + macro sugar; noexcept registrar (load-time). OBJECT-library tree-shaking caveat documented. clang-tidy `^MAUI_` allow-listed. *characterization |

| layout foundation (`dimension`, `i_container`/`i_layout`/`i_stack_layout`, `i_layout_manager`) | core/layouts | ✅ | — | ✅ | headless + macOS | `ILayout : IView + IContainer + IPadding` (M3 subset; ClipsToBounds / ISafeAreaView / ICrossPlatformLayout deferred to M4). `dimension` = Unset(NaN)/Minimum(0)/Maximum(inf) + is_explicit_set |
| stack layout managers (`layout_manager`/`stack_layout_manager` + vertical/horizontal) | layouts | ✅ | ✅* | ✅ | headless + macOS | New `maui_layouts` lib — pure cross-platform measure/arrange (ResolveConstraints, MeasureSpacing, stacking). 28 GTest cases via mock view/stack (spacing / padding / min-max / collapsed-vs-hidden / fill / child-constraint). The layout *controls* + native panel are M4. *ported from C# Stack*LayoutManagerTests (the oracle) |

_(extend this table as components are added)_
