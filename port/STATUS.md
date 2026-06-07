# STATUS — MAUI → C++23 port progress

> One row per component. Update at step 7 of the per-component loop (`CLAUDE.md`). Never mark a
> partial port as done silently — use the Notes column.
> Legend: ✅ done · 🚧 in progress · ⬜ not started · — n/a

## Build & test (headless) — run from `port/cpp/`

```bash
export VCPKG_ROOT="$HOME/vcpkg"          # registry clone; brew's vcpkg binary alone lacks the toolchain file
cmake --preset headless && cmake --build --preset headless
ctest --preset headless                  # all ported tests (graphics + core + controls: 247 cases, green)
./build/headless/maui_graphics_benchmarks --benchmark_min_time=0.02s   # Google Benchmark (not a ctest test)
```

**macOS / AppKit backend** (real NSViews; Obj-C++ `.mm` + ARC):

```bash
cmake --preset apple && cmake --build --preset apple && ctest --preset apple   # 230 cases incl. real NSButton tap
./build/apple/maui_button_sample                                                # sample window (Ctrl-C / close to quit)
```

**Resume:** continue to the next ⬜ milestone below, following `CLAUDE.md`. **M1 (Core) and M2 (button,
the Rosetta Stone) are COMPLETE** — the virtual-view ⇄ handler ⇄ native seam is proven end-to-end on
**both** the headless backend (282 tests) and the **macOS AppKit backend** (real `NSButton`, 246 tests
incl. a native tap via `performClick:`). **M3 (layout) is COMPLETE. M4 (control set v1) — `label`, the
shared ViewMapper, `entry`, minimal `image`, and the stack layout controls are DONE (headless + macOS);
next: `grid` control + `page`.**
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
**M3b (done): Grid** — `grid_length` / `grid_unit_type`, the row/column definition contracts, `i_grid_layout`
(definitions + spacing + Row/Column/Span attached-property queries), and `grid_layout_manager`: Absolute /
Auto / Star sizing, row+column spans, spacing, padding, min/max, two-pass measure, and arrange-time star
expansion (pimpl grid_structure cached between measure/arrange, exactly as C#'s GridStructure). 19 GTest
cases ported from GridLayoutManagerTests (a representative subset of a very large suite).
**M4a (done): `label`** — the second concrete control, proving the handler recipe generalizes
(display-only: properties flow virtual→native, no events). New contracts `text_alignment` /
`text_decorations` / `i_text_alignment` / `i_label`; the `label` control + `label_handler` (single mapper
keyed on i_label — no chaining) with a headless mirror and a real macOS NSTextField (label style). The
Apple `.mm`s now share `apple_conversions.hpp` (maui color/font → NSColor/NSFont). Also disabled clang-tidy
`portability-template-virtual-member-function` — it flagged the deliberate inline virtual overrides on the
`view<ViewInterface>` template base (latent since M2a; the vtable odr-uses them all on our toolchain).
**M4b — shared ViewMapper (Unit 2, done):** the generic-IView property mapper every handler chains.
New `view_platform_base` (a non-template base for the platform-view structs: headless mirrors
`hidden`/`alpha`/`enabled`/`automation_id` + virtual `update_visibility`/`update_opacity`/
`update_is_enabled`/`update_automation_id`, backend-overridable) and `view_mapper()` — a
`property_mapper<i_view, i_view_handler>` with keys `"visibility"`/`"opacity"`/`"is_enabled"`/
`"automation_id"`. `i_view_handler` gained `view_platform_base* platform_base()`; the CRTP `view_handler`
implements it NON-BREAKINGLY (`if constexpr (is_base_of<view_platform_base, Platform>)` → ptr else null,
so handlers whose platform view doesn't derive the base still compile and the maps no-op). The four
`view<ViewInterface>` properties are now BINDABLE (private `property<T>` bound to NON-template free-function
descriptors `is_enabled_property()`/`opacity_property()`/`visibility_property()`/`automation_id_property()`
in the new `src/controls/view.cpp` — one descriptor per property across all instantiations; opacity clamps
to [0,1] like VisualElement). `button_platform`/`label_platform` now derive `view_platform_base` and the
handlers chain `view_mapper()` (button: `set_chained({&text_mapper(), &view_mapper()})` so generic-IView
keys run first; label: the `property_mapper(view_mapper(), {...})` ctor). AppKit `.mm`s override the four
`update_*` on the real `NSButton`/`NSTextField` (`hidden`/`alphaValue`/`setEnabled:`/
`accessibilityIdentifier`). 9 headless + 2 apple GTest cases (`view_mapper_tests.cpp` /
`view_mapper_apple_tests.mm`). **Deferred (note for coordinator):** the wider ViewMapper set —
Width/Height/Background/transforms/Clip/Shadow/FlowDirection/Semantics/InputTransparent — and C#'s
`IsConnectingHandler()` default-skip optimization. The `MAUI_PLATFORM_APPLE` compile definition (PUBLIC,
apple build only) guards the platform structs' `update_*` override declarations so the headless build keeps
the base mirrors (one backend per build → no ODR mismatch). **Coordinator: to retrofit Units 1 & 3's new
handlers, chain `maui::core::view_mapper()` into each handler's `mapper()` (single-sub-mapper handlers use
the `property_mapper(view_mapper(), {entries})` ctor like `label_handler`; multi-sub-mapper handlers use
`set_chained({&other_mapper(), &view_mapper()})` like `button_handler`), and make each platform-view struct
derive `maui::core::view_platform_base`.**
**M4b — layout controls (Unit 1, done):** `i_layout_handler` (the add / remove / clear / insert / update /
update_z_index child seam), a templated `layout<LayoutInterface>` control base (owns the non-owning child
list + i_container / i_padding, lazily builds its M3 `i_layout_manager` from `create_layout_manager()`, and
overrides measure/arrange to delegate to the manager — a layout computes its own geometry — while arrange
also sizes the host panel), and `vertical_stack_layout` / `horizontal_stack_layout` (+ bindable Spacing).
The `layout_handler` hosts a native container (a plain `NSView` panel on macOS; a headless child-count
mirror) and syncs subviews via the command seam. `view_handler` now derives `i_view_handler` *virtually*
(so `layout_handler` is both a `view_handler` and an `i_layout_handler` with a single `i_view_handler`
subobject), and `i_view_handler` gained `native_view()` (the real native view a panel hosts). `maui_layouts`
is linked into `maui_controls`. The grid *control* is deferred (the grid *manager* already exists).
**M4b — entry + minimal image (Unit 3, done):** `entry` — the first inbound-text + first editable native
control: `i_text_input` / `i_entry`, bindable text / placeholder / is_password / is_read_only / max_length
+ appearance / alignment, `completed` + `text_changed(old, new)` events, over an editable `NSTextField`
(an `NSSecureTextField` cell swap for password) with an `NSTextFieldDelegate` trampoline
(controlTextDidChange → send_text_changed, controlTextDidEndEditing → send_completed). `image` (minimal) —
an `aspect` enum + `i_image` (aspect only) mapped to `NSImageView.imageScaling`. The full async image
*source* subsystem (loaders / caching) is deferred.
**M4b — coordinator integration (done):** the three units merged onto cpp-port-kit; Units 1 & 3's platform
structs retrofitted onto `view_platform_base` + chained `view_mapper()` (layout overrides 3 `update_*` — a
plain NSView panel has no enabled state, so is_enabled keeps the base mirror; entry / image, being
NSControls, override all 4). Full gate green: **282 headless / 246 apple** GTest, clang-tidy 0 findings
(including the Obj-C++ `.mm` files), ASan/UBSan + TSan clean.
**M4c — ViewMapper widening: transforms + FlowDirection (Unit A, done; headless-complete):** the shared
`view_mapper()` grew the render transform and FlowDirection. `view_platform_base` gained a POD
`transform_spec` (the ten ITransform scalars — translation_x/y, scale, scale_x/y, rotation, rotation_x/y,
anchor_x/y; identity defaults) + a `transform` mirror and a `flow_direction` mirror, plus backend-
overridable `update_transform(const transform_spec&)` / `update_flow_direction(flow_direction)` (headless
bodies store the mirrors). `view_mapper()` added ten transform keys — all routing to ONE shared
`map_transform` that reads all ten scalars off the i_view and pushes the whole `transform_spec` (so any
single change rebuilds the entire transform, matching TransformationExtensions.UpdateTransformation) —
plus `"flow_direction"` → `map_flow_direction`. The `view<>` transform getters + `flow_direction()` are
now BINDABLE (private `property<double>` / `property<flow_direction>` over NEW non-template descriptor
free-fns in `view.cpp`: translation/rotation default 0, scale 1, anchor 0.5, flow_direction MatchParent)
with public setters (`set_translation_x` … `set_flow_direction`); property names match the mapper keys
exactly. Shared AppKit helpers in the new header `src/platform/apple/apple_view_ops.hpp`:
`maui::platform::apple::apply_transform(void* native, const maui::core::transform_spec&)` (faithful port
of TransformationExtensions — wantsLayer, anchorPoint, the CATransform3D build with the m34 perspective
for out-of-plane rotation; the anchor-offset term reads the NSView layer bounds, the AppKit analog of
view.Frame) and `apply_flow_direction(void* native, maui::core::flow_direction)`
(NSView.userInterfaceLayoutDirection; MatchParent → the app-wide default). These are provided for the
coordinator's per-control retrofit — NOT wired into any control here. CMake gained `-framework QuartzCore`
on the apple `maui_core` link (CATransform3D C symbols; Cocoa doesn't pull them). 5 new headless
(`view_mapper_tests.cpp`: identity defaults, single-setter-rebuilds-whole-spec, flow_direction, initial-
values-on-attach, label-generalization) + 5 new apple (`view_mapper_apple_tests.mm`: apply_transform
scale/anchor/perspective/identity + apply_flow_direction) GTest cases. Full gate green: **287 headless /
251 apple**, clang-tidy 0 findings on the changed `.cpp`/headers (headless + apple DBs). **Out of scope
(deferred):** Background (needs paint), Shadow (needs i_shadow), Clip (needs i_shape), Width/Height
(layout-driven), Semantics, InputTransparent; and C#'s `IsConnectingHandler()` default-skip. **Coordinator:
to retrofit each control, override `update_transform`/`update_flow_direction` on its platform struct to
call `apply_transform(native, value)` / `apply_flow_direction(native, value)` from apple_view_ops.hpp
(no view_mapper change needed — chaining `view_mapper()` is already done for every control).**
**Still next: M4** — the `grid` control (wrapping the existing grid manager) and `page`.

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
  - `cmake --preset asan-ubsan && cmake --build --preset asan-ubsan && ctest --preset asan-ubsan` — ASan+UBSan (default checked build; **282/282 green**)
  - `cmake --preset tsan && cmake --build --preset tsan && ctest --preset tsan` — ThreadSanitizer (**282/282 green**)
  - `msan` preset is for **Linux/Clang CI only** — `-fsanitize=memory` is unsupported on AppleClang/macOS.

## Milestones (see `PROJECT.md §5`)

| Milestone | Description | Status |
|---|---|---|
| M0 | Graphics primitives compile + pass ported tests | ✅ |
| M1 | Core contracts + property/handler infra + dispatcher, unit-tested | ✅ |
| M2 | `button` end-to-end (headless → macOS), tap works in sample app | ✅ |
| M3 | Layout measure/arrange (`stack_layout`, `grid`) pass layout tests | ✅ |
| M4 | Control set v1 (label, entry, image, layouts, page) on macOS | 🚧 |
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
| `label` control (`i_label`/`i_text_alignment`/`text_alignment`/`text_decorations`, `label_handler`) | controls + core/handlers | ✅ | ✅* | ✅ | headless + macOS | Second control (display-only) — proves the recipe generalizes. Bindable text/text_color/font/char_spacing/padding/alignments/decorations/line_height; mapper keyed on i_label, now chained onto `view_mapper()` (M4b). Headless mirror + real macOS NSTextField (text/textColor/font/alignment). Shared `apple_conversions.hpp` (color/font→AppKit). 5 headless + 2 apple GTest cases; self-registers. *characterization |
| shared ViewMapper (`view_platform_base`, `view_mapper`; `view<>` IsEnabled/Opacity/Visibility/AutomationId bindable) | core/handlers + controls | ✅ | ✅* | ✅ | headless + macOS | M4b Unit 2 — the generic-IView property mapper every handler chains. `view_platform_base` (non-template platform-view base: headless mirrors hidden/alpha/enabled/automation_id + backend-overridable update_*); `view_mapper()` = `property_mapper<i_view, i_view_handler>` keyed visibility/opacity/is_enabled/automation_id. `i_view_handler::platform_base()` + CRTP `view_handler` non-breaking `if constexpr` impl. The four `view<>` props bindable via NON-template descriptor free-fns in `src/controls/view.cpp` (opacity clamped [0,1]). button/label platform structs derive the base + chain `view_mapper()`; AppKit pushes to NSButton/NSTextField (hidden/alphaValue/setEnabled:/accessibilityIdentifier). Wider ViewMapper set (Width/Height/Background/transforms/Clip/Shadow/FlowDirection) + IsConnectingHandler skip deferred. 9 headless + 2 apple GTest cases. *characterization |
| ViewMapper transforms + FlowDirection (`transform_spec`, `map_transform`/`map_flow_direction`; `view<>` transform + flow_direction bindable; `apple_view_ops.hpp`) | core/handlers + controls + platform/apple | ✅ | ✅* | ✅ | headless + macOS | M4c Unit A — widens `view_mapper()` with the render transform + FlowDirection. `view_platform_base` gained a POD `transform_spec` (ten ITransform scalars, identity defaults) + `transform`/`flow_direction` mirrors + `update_transform`/`update_flow_direction`. Ten transform keys all route to ONE `map_transform` (reads all ten off i_view → pushes the whole spec, so any change rebuilds the full transform, per TransformationExtensions) + `flow_direction`→`map_flow_direction`. The ten `view<>` transform getters + `flow_direction()` now bindable (NON-template descriptor free-fns in `view.cpp`) with public setters; names match the mapper keys. Shared AppKit helpers `apply_transform`/`apply_flow_direction` in the new `src/platform/apple/apple_view_ops.hpp` (faithful TransformationExtensions CATransform3D build incl. m34 perspective; NSView.userInterfaceLayoutDirection) — provided for the coordinator's per-control retrofit, NOT wired into any control. CMake +`-framework QuartzCore` (apple `maui_core`). Background/Shadow/Clip/Width/Height/Semantics/InputTransparent deferred (need new value types or are layout-driven). 5 headless + 5 apple GTest cases. *characterization |

| layout foundation (`dimension`, `i_container`/`i_layout`/`i_stack_layout`, `i_layout_manager`) | core/layouts | ✅ | — | ✅ | headless + macOS | `ILayout : IView + IContainer + IPadding` (M3 subset; ClipsToBounds / ISafeAreaView / ICrossPlatformLayout deferred to M4). `dimension` = Unset(NaN)/Minimum(0)/Maximum(inf) + is_explicit_set |
| stack layout managers (`layout_manager`/`stack_layout_manager` + vertical/horizontal) | layouts | ✅ | ✅* | ✅ | headless + macOS | New `maui_layouts` lib — pure cross-platform measure/arrange (ResolveConstraints, MeasureSpacing, stacking). 28 GTest cases via mock view/stack (spacing / padding / min-max / collapsed-vs-hidden / fill / child-constraint). The layout *controls* + native panel are M4. *ported from C# Stack*LayoutManagerTests (the oracle) |
| Grid (`grid_length`/`grid_unit_type`, `i_grid_*` contracts, `grid_layout_manager`) | core + layouts | ✅ | ✅* | ✅ | headless + macOS | The full row/column algorithm: Absolute/Auto/Star sizing, row+column spans, spacing, padding, min/max, two-pass measure + arrange-time star expansion (pimpl grid_structure cached measure→arrange, like C# GridStructure). 19 GTest cases (representative subset of the large GridLayoutManagerTests). *ported from the C# oracle |
| layout controls (`i_layout_handler`, `layout<>` base, `vertical_stack_layout`/`horizontal_stack_layout`, `layout_handler`) | controls + core/handlers | ✅ | ✅* | ✅ | headless + macOS | M4b Unit 1 — wrap the M3 stack managers in controls + a native host panel. `layout<LayoutInterface>` owns the non-owning child list (i_container) + bindable padding, lazily builds its manager via `create_layout_manager()`, and overrides measure/arrange to delegate to the manager (a layout computes its own geometry); arrange also sizes the panel. `layout_handler` (i_layout_handler add/remove/clear/insert/update/update_z_index) hosts a plain NSView panel (headless: child-count mirror). `view_handler` derives `i_view_handler` *virtually* (layout_handler = view_handler + i_layout_handler); `i_view_handler::native_view()` exposes the hosted native view. Retrofitted onto `view_mapper()` (is_enabled keeps the base mirror — NSView has no enabled state). maui_layouts linked into maui_controls. Grid *control* deferred. 14 headless + 4 apple GTest cases; self-registers. *characterization |
| `entry` control (`i_text_input`/`i_entry`, `entry_handler`) | controls + core/handlers | ✅ | ✅* | ✅ | headless + macOS | M4b Unit 3 — first inbound-text + first editable native control. Bindable text/placeholder/is_password/is_read_only/max_length (UTF-8-byte truncation in set_text) + i_text_style appearance + alignment; `completed` + `text_changed(old,new)` events. Editable NSTextField (NSSecureTextField cell swap for password, preserving font/color) + an NSTextFieldDelegate trampoline (controlTextDidChange→send_text_changed, controlTextDidEndEditing→send_completed). Chains `view_mapper()`. AppKit defers character_spacing/vertical alignment/placeholder_color (headless mirrors them). 13 headless + 7 apple GTest cases; self-registers. *characterization |
| `image` control (`aspect`/`i_image`, `image_handler`; aspect only) | controls + core/handlers | ✅ | ✅* | ✅ | headless + macOS | M4b Unit 3 (minimal) — maps ONLY the scaling mode: `aspect` (aspect_fit/aspect_fill/fill/center) → NSImageView.imageScaling (+ centered). No image bytes loaded — the async image *source* subsystem (IImageSource/loaders/caching) is deferred. Chains `view_mapper()`. 5 headless + 2 apple GTest cases; self-registers. *characterization |

_(extend this table as components are added)_
