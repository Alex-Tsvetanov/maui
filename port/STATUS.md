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
**both** the headless backend (354 tests) and the **macOS AppKit backend** (real `NSButton`, 281 tests
incl. a native tap via `performClick:`). **M3 (layout) is COMPLETE. M4 (control set v1) is DONE
(headless + macOS): `label`, `entry`, `image` (aspect + file source + async uri/stream sources), the
stack + `grid` layout controls, `content_page` + a `navigation_page` push/pop stack, and the shared
ViewMapper — the full generic IView surface (Visibility/Opacity/IsEnabled/AutomationId + the render
transform + FlowDirection + Background/Shadow/Clip). **M5 (binding / styles / lifecycle) COMPLETE (cross-platform)**
— M5a data binding (typed accessor), M5b styles/setters/triggers/VSM, and M5c Application/Window/element
lifecycle + typed inherited BindingContext are all done; only the native NSWindow `window_handler` is a
deferred tail (the lifecycle itself is proven on both backends).**
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
scale/anchor/perspective/identity + apply_flow_direction) GTest cases. **Out of scope (deferred):**
Background (needs paint), Shadow (needs i_shadow), Clip (needs i_shape), Width/Height (layout-driven),
Semantics, InputTransparent; and C#'s `IsConnectingHandler()` default-skip.
**M4c — grid control (Unit B, done):** `grid : layout<i_grid_layout>` over the existing `grid_layout_manager`
— `row_definition`/`column_definition` (concrete `i_grid_*_definition`, default Star), RowDefinitions /
ColumnDefinitions, bindable row/column spacing, and the Grid.Row/Column/RowSpan/ColumnSpan attached
properties (per-child `cell_info` in a pointer-keyed map; C# validation: row/col ≥ 0, span ≥ 1). Reuses
`layout_handler` (a grid is an `i_layout`; `MAUI_REGISTER_HANDLER(grid, layout_handler)`), so it inherits the
native panel + the chained `view_mapper()`. The grid *manager* already existed (M3b).
**M4c — minimal content_page (Unit C, done):** `i_content_view` (`content()` + i_padding) + `content_page :
view<i_content_view>` (a single non-owning Content + bindable Title/Padding; measure/arrange port C#'s
LayoutExtensions.MeasureContent/ArrangeContent — content sized within padding) + a `content_page_handler`
hosting the content's native view as a subview of a plain `NSView` (headless mirrors the hosted content).
Navigation / toolbar / safe-area / dialogs / templates deferred.
**M4c — image file source (Unit D, done):** `i_image_source` / `i_file_image_source` + a concrete
`file_image_source` + an `image_source::from_file(path)` factory; `i_image` / `image` gained a bindable
`source` (`property<shared_ptr<i_image_source>>`); `image_handler::map_source` loads the file SYNCHRONOUSLY
(`[[NSImage alloc] initWithContentsOfFile:]`; headless mirrors the path + a loaded flag). Async loading +
cancellation, the IImageSourceService / service-provider seam, uri / stream / font sources, and caching deferred.
**M4c — coordinator integration (done):** the four units merged onto cpp-port-kit; **Unit A's apple
transform / flow_direction wiring was retrofitted onto all six platform structs** (button / label / entry /
image / layout / content_page each override `update_transform` / `update_flow_direction` to call the
apple_view_ops helpers; grid reuses layout_handler). Full gate green: **326 headless / 264 apple** GTest,
clang-tidy 0 findings (including the Obj-C++ `.mm` files), ASan/UBSan + TSan clean.
**M4d — ViewMapper visuals: Background + Shadow + Clip (Unit V, done):** stood up the visual-layer value
types that `i_view` only forward-declared, and mapped them. New `maui::graphics::paint` + `solid_paint`
(one color), `maui::core::i_shadow` + `shadow` (radius / opacity / paint / offset), `maui::graphics::i_shape`
+ `rectangle` / `round_rectangle` / `ellipse` (`path_for_bounds` built from the existing `path_f`
append_* helpers). `view_platform_base` gained non-owning background / shadow / clip mirrors +
`update_background` / `update_shadow` / `update_clip`; `view_mapper()` added the three keys; `view<>` makes
`background()` / `shadow()` / `clip()` bindable (`property<shared_ptr<>>` + non-template descriptors). Shared
AppKit helpers in `src/platform/apple/apple_visual_ops.hpp`: `apply_background` (solid → `layer.backgroundColor`),
`apply_shadow` (`layer.shadow{Color,Opacity,Radius/2,Offset}`), `apply_clip` (`shape->path_for_bounds` → a
`path_f`→`CGPath` walk → a `CAShapeLayer` mask). Self-contained + headless-complete; the apple per-control
overrides are the coordinator's retrofit. 8 headless + 8 apple GTest cases. Gradients / per-corner clip /
Width/Height/Semantics/InputTransparent deferred.
**M4d — async image sources (Unit I, done):** the image-source subsystem grew uri + stream sources, an
async loader with cancellation, an in-memory cache, and the service-registry seam. New contracts
`i_uri_image_source` (uri + caching_enabled) / `i_stream_image_source` (a bytes provider —
`image_bytes get_bytes(token)`, simplifying C#'s `Task<Stream>`); concrete `uri_image_source` /
`stream_image_source` + `image_source::from_uri` / `from_stream` factories. The service seam:
`i_image_source_service` (async `load(source, token, on_result)`), an `image_source_service_registry`
(mirrors handler_registry — `register_service<Source, Service>()` keyed by the source interface, `resolve`
via a dynamic_cast probe walk) with a process-wide default + `register_default_image_source_services`, an
`image_source_result` (the native image handle + RAII disposer + a headless kind/detail mirror), and per-
source `file/uri/stream_image_source_service` (cross-platform decl + `{headless,apple}` partials sharing one
`decode_image_bytes` primitive). The `cancellation_token` is a `shared_ptr<atomic<bool>>` flag
(is_cancelled/cancel). The `image_source_loader` (ports ImageSourcePartLoader + ImageSourceServiceResultManager):
`begin_load()` disposes the prior result + cancels the prior token + mints a fresh one; `update_source()`
resolves the service / handles the uri cached fast-path (`std::map<uri,bytes>`, no expiry), loads, marshals
the apply onto the dispatcher (headless `manual_dispatcher` pumped in tests; apple inline when no dispatcher),
does the **source-identity recheck** (`!token.is_cancelled() && source == current_source_`), then
`complete_load`. `image_handler` now OWNS a loader (`source_loader()`): `map_source` keeps the SYNC file fast-
path and routes uri/stream through the loader async; the cross-platform routing lives once in
image_handler.cpp, the three per-backend primitives (`load_file_source_sync` / `apply_loaded_result` /
`clear_source_native`) touch the native NSImageView / headless mirror. `image_platform` gained a
`source_kind` mirror; still derives `view_platform_base`, mapper still chains `view_mapper()`. THREADING: the
loader's mutable state is touched only on the dispatcher (UI) thread; this cut's services load synchronously
(no worker), so the only cross-thread element is the atomic cancellation flag → TSan-clean. UAF-safe teardown
via a loader liveness weak_ptr the queued apply checks; ABA-safe because token cancellation gates the apply
independent of pointer identity. CMake: +4 cross-platform core `.cpp` (loader / registry / services-glue /
uri_bytes) + a per-backend `image_source_services.{cpp,mm}` (apple `.mm` `-fobjc-arc`). 6 new headless +
3 new apple GTest cases (extend image_tests.cpp / image_apple_tests.mm — no test-list churn). DEFERRED: disk
caching + CacheValidity expiry, font image sources, the full DI service-provider, resolution-dependent reload,
a production HTTP stack (apple uri does `file://` + a synchronous `http(s)` NSData fetch; headless never hits
the network — stream sources + `file://` only). **326 → 331 headless / 264 → 267 apple**, clang-tidy 0 findings
(incl. the `.mm` files + the gated tests, no suppressions).
**M4d — navigation pages (Unit N, done):** `navigation_page` — a push/pop stack of content_pages hosting
the current page's native view in a container (swapped on push/pop, no animation). `content_page` gained
the **page lifecycle** (Page.cs SendAppearing/SendDisappearing + `appearing`/`disappearing` events,
idempotent via a `has_appeared_` flag). New contracts: `navigation_request` (the new stack + animated) +
`i_stack_navigation` (request_navigation / navigation_finished, IStackNavigation.cs). `navigation_page :
view<i_view> + i_stack_navigation` owns a NON-owning `vector<content_page*>` with push/pop/pop_to_root +
current_page/root_page/navigation_stack; push/pop/pop_to_root mutate the stack then fire Disappearing(prev)
+ Appearing(new) — BOTH before notifying the handler — per NavigationPage.cs's MauiNavigationImpl ORDER
(C#'s async semaphore/TaskCompletionSource/overlap queue collapsed to a synchronous single transition).
The drive is a `"request_navigation"` COMMAND (payload = a `navigation_request`); `navigation_page_handler`
hosts a plain NSView container and swaps the current page's `native_view()` subview (headless mirrors the
hosted page). `navigation_page_platform` derives `view_platform_base`; the handler chains `view_mapper()`.
`navigation_page::set_handler` re-issues the request after wiring (C# OnHandlerChangedCore) so a freshly-
attached container hosts the current page; the navigation_page also fires the ROOT's initial
send_appearing() itself (standing in for the absent window lifecycle). **Deferred:** the navigation bar /
back button / title bar, push/pop animations, the modal stack, InsertPageBefore/RemovePage, and Shell.
15 headless + 5 apple GTest cases; self-registers (MAUI_REGISTER_HANDLER). *characterization.
**M4d — coordinator integration (done):** the three units merged onto cpp-port-kit; **Unit V's apple
Background / Shadow / Clip wiring was retrofitted onto all six platform structs** (button / label / entry /
image / layout / content_page each override `update_background` / `update_shadow` / `update_clip` → the
apple_visual_ops helpers; the clip mask is sized to the NSView bounds; grid reuses layout_handler). Full
gate green: **354 headless / 281 apple** GTest, clang-tidy 0 (incl. the `.mm` files + the gated tests via
the `tidy` preset), ASan/UBSan + TSan clean (Unit I's async loader runs single-threaded on the dispatcher,
so TSan stays clean). **M4 (control set v1) is COMPLETE.**
**Still next: M6** (second platform — iOS, behind the same handlers) or the deferred tails — the native
NSWindow `window_handler` (M5c), plus the M4 deferrals (per-corner clip, navigation
chrome, image disk-cache + a production HTTP stack). **Gradient paints (M4 backlog) are now done** (headless +
AppKit `CAGradientLayer`). M5a (binding) + M5b (styles/setters/triggers/VSM) +
M5c (Application/Window/lifecycle + typed inherited BindingContext) are all done and gated on both backends.

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
  - `cmake --preset asan-ubsan && cmake --build --preset asan-ubsan && ctest --preset asan-ubsan` — ASan+UBSan (default checked build; **354/354 green**)
  - `cmake --preset tsan && cmake --build --preset tsan && ctest --preset tsan` — ThreadSanitizer (**354/354 green**)
  - `msan` preset is for **Linux/Clang CI only** — `-fsanitize=memory` is unsupported on AppleClang/macOS.

## Milestones (see `PROJECT.md §5`)

| Milestone | Description | Status |
|---|---|---|
| M0 | Graphics primitives compile + pass ported tests | ✅ |
| M1 | Core contracts + property/handler infra + dispatcher, unit-tested | ✅ |
| M2 | `button` end-to-end (headless → macOS), tap works in sample app | ✅ |
| M3 | Layout measure/arrange (`stack_layout`, `grid`) pass layout tests | ✅ |
| M4 | Control set v1 (label, entry, image, layouts, content page) on macOS | ✅ |
| M5 | `bindable_object`/`bindable_property`, binding, style, lifecycle | ✅ (M5a binding, M5b styles/triggers/VSM, M5c Application/Window/lifecycle + inherited BindingContext; native NSWindow host deferred) |
| M6 | Second platform (iOS) behind the same handlers | ⬜ |
| M7 | XAML and/or Essentials (as prioritized) | ⬜ |

## Deferred backlog (revisit later — each was a documented "first cut", never a silent gap)

Consolidated from the per-component Notes above. Pick these off opportunistically or alongside the
milestone that needs them; none blocks M5.

**ViewMapper / visuals (M4b–M4d):**
- ~~Gradient paints~~ **DONE (backlog):** `gradient_stop` / `gradient_paint` (abstract) / `linear_gradient_paint` / `radial_gradient_paint` ported; AppKit `apply_background` now installs a `CAGradientLayer` (axial/radial) for gradient paints. The `ImagePaint`/`PatternPaint` kinds remain unported (out of scope).
- Per-corner clip radii — `round_rectangle` is a single uniform corner radius today; add the 4-corner `corner_radius`.
- Layout-driven ViewMapper props — Width/Height/MinimumWidth/Height/MaximumWidth/Height (these flow from the layout pass, not a direct push).
- `Semantics` (accessibility), `InputTransparent`, `ToolTip`, `ContextFlyout`, native `ZIndex` ordering; C#'s `IsConnectingHandler()` default-skip optimization.

**Image (M4c–M4d):**
- Disk cache + `CacheValidity` expiry (only an in-memory uri cache today); a production HTTP stack (currently a synchronous `NSURLSession`/`NSData` fetch); the full DI service-provider (we use a flat typed registry).
- `font_image_source`; resolution-dependent reload (@2x/@3x + screen-DPI); `IsAnimationPlaying` / `IsOpaque` / `IsLoading`.

**Navigation (M4d):**
- Navigation bar / back button / title bar; push/pop animations; the modal stack (`PushModalAsync`); `InsertPageBefore` / `RemovePage`; Shell; the window-hierarchy guard in `SendAppearing` (needs the Window/Application lifecycle — an M5 item).

**Text controls (M2–M4):**
- AppKit `character_spacing` (needs an attributed string / `NSKernAttributeName`), `vertical_text_alignment` (custom field editor), `placeholder_color` (attributed placeholder) — headless mirrors them all.
- `button` padding on AppKit (custom `NSButtonCell` / content insets); `entry` ReturnType / ClearButtonVisibility / cursor+selection / keyboard / prediction.

**Styles / triggers / VSM (M5b):**
- Implicit styles + `ResourceDictionary` lookup + style classes + `BasedOn`-by-resource-key + `DynamicResource` (only a directly-assigned `based_on` style object is supported, applied at the lowered `as_base_style` specificity); `Style.ApplyToDerivedTypes` / `CanCascade` / `Behaviors` / triggers-in-a-Style.
- The VSM #18103/#34363 nuance — implicit-style VSM **downgraded** below a manual value + the system-state `WithFullVsmPriority` promotion (and the constant it needs) — only manifests when `VisualStateGroups` is assigned via an implicit style; a directly-driven `visual_state_manager` applies every state at the one `visual_state_setter` specificity (above manual). Also deferred: the attached-property `VisualStateGroupsProperty` storage on `view<>`, `StateTriggers`, the `is_enabled → Disabled/Normal` auto-drive, Clone + duplicate-name validation.
- Triggers: `Setter.TargetName` (re-targeting another element), `EnterActions`/`ExitActions`, `MultiTrigger`, per-trigger index ordering (all triggers share `setter_specificity::trigger`), and the BindingContext-sourced `data_trigger` (lands with BindingContext in M5c). `property_trigger<T>` watches a concrete `property<T>` directly (the typed substitute for `GetValue(Property)`).

**Application / Window / lifecycle (M5c):**
- The native **`window_handler` over `NSWindow`** (the apple host: create an NSWindow, set its contentView to the root page's `native_view()`, map `NSWindowDidBecomeMain`/`WillClose` → the window lifecycle) + a headless mirror — the cross-platform Window/Application lifecycle is done and proven on both backends, but it is not yet hosted in a real native window.
- `Page.SendAppearing`'s internal "only appear once attached to a window" guard: the **window now DRIVES** `content_page::send_appearing()` on activate (and `send_disappearing()` on deactivate), which achieves the windowed-appearing end result; the navigation_page retains its self-drive for the no-window case (the M4d deviation), so a hard guard inside `send_appearing()` stays deferred (it would regress the nav self-drive).
- Multi-window orchestration, `Resumed`/`Stopped`/`Backgrounding`/`Created` ordering subtleties, modal windows, persisted state, window geometry/chrome (X/Y/Width/Height/TitleBar), themes; the `i_window`/`i_application` Core contracts (the port's `window`/`application` are concrete-only so far); a runnable `maui_app_sample` NSWindow. BindingContext string/relative paths + `bind_to_context` convenience remain deferred to M7 (typed inheritance is done).

**Layout (M3):** `ClipsToBounds` / `ISafeAreaView` / `ICrossPlatformLayout` on `i_layout`; `z_index` ordering inside the managers.

**Graphics (M0):** `System.Numerics` interop — `Vector2`/`Vector4`/`Matrix3x2` casts + `Transform`/`TransformBy` on point/rect/`path_f` (needs a maui linalg type).

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
| ViewMapper transforms + FlowDirection (`transform_spec`, `map_transform`/`map_flow_direction`; `view<>` transform + flow_direction bindable; `apple_view_ops.hpp`) | core/handlers + controls + platform/apple | ✅ | ✅* | ✅ | headless + macOS | M4c Unit A — widens `view_mapper()` with the render transform + FlowDirection. `view_platform_base` gained a POD `transform_spec` (ten ITransform scalars, identity defaults) + `transform`/`flow_direction` mirrors + `update_transform`/`update_flow_direction`. Ten transform keys all route to ONE `map_transform` (reads all ten off i_view → pushes the whole spec, so any change rebuilds the full transform, per TransformationExtensions) + `flow_direction`→`map_flow_direction`. The ten `view<>` transform getters + `flow_direction()` now bindable (NON-template descriptor free-fns in `view.cpp`) with public setters; names match the mapper keys. Shared AppKit helpers `apply_transform`/`apply_flow_direction` in the new `src/platform/apple/apple_view_ops.hpp` (faithful TransformationExtensions CATransform3D build incl. m34 perspective; NSView.userInterfaceLayoutDirection). **Retrofitted (M4c integration) onto all six platform structs** (button/label/entry/image/layout/content_page override `update_transform`/`update_flow_direction` → the helpers; grid reuses layout_handler), so transforms/flow_direction reach the native NSView. CMake +`-framework QuartzCore` (apple `maui_core`). Background/Shadow/Clip/Width/Height/Semantics/InputTransparent deferred (need new value types or are layout-driven). 5 headless + 6 apple GTest cases. *characterization |
| `grid` control (`row_definition`/`column_definition`, Grid.Row/Column/Span attached props) | controls | ✅ | ✅* | ✅ | headless + macOS | M4c Unit B — `grid : layout<i_grid_layout>` over the existing `grid_layout_manager`. Concrete row/column definitions (default Star) + RowDefinitions/ColumnDefinitions; bindable row/column spacing; the Grid.Row/Column/RowSpan/ColumnSpan attached props stored per-child in a pointer-keyed `cell_info` map (C# validation: row/col ≥ 0, span ≥ 1). Reuses `layout_handler` (a grid is an i_layout) → inherits the native NSView panel + chained `view_mapper()`. 22 headless + 4 apple GTest cases; self-registers. *characterization |
| `content_page` (`i_content_view`, `content_page`, `content_page_handler`) | controls + core/handlers | ✅ | ✅* | ✅ | headless + macOS | M4c Unit C — a minimal page hosting a single Content. `i_content_view` (content + i_padding); `content_page : view<i_content_view>` (non-owning Content + bindable Title/Padding; measure/arrange port C# MeasureContent/ArrangeContent — content sized within padding). `content_page_handler` hosts the content's native view as a subview of a plain NSView (headless mirror). Chains `view_mapper()`; derives `view_platform_base`. Navigation/toolbar/safe-area/dialogs/templates deferred. 11 headless + 4 apple GTest cases; self-registers. *characterization |
| image file source (`i_image_source`/`i_file_image_source`, `file_image_source`, `image.source`) | core + controls + handlers | ✅ | ✅* | ✅ | headless + macOS | M4c Unit D — `image` gained a real bindable `source` (`property<shared_ptr<i_image_source>>`). `i_image_source`/`i_file_image_source` contracts + concrete `file_image_source` + `image_source::from_file(path)`. `image_handler::map_source` loads the file **synchronously** (`[[NSImage alloc] initWithContentsOfFile:]`; headless mirrors path + loaded flag; empty/null/failed → clears). Async loading + cancellation, the IImageSourceService/service-provider seam, uri/stream/font sources + caching deferred. 6 headless + 4 apple GTest cases. *characterization |
| async image sources (`i_uri_image_source`/`i_stream_image_source`, `uri/stream_image_source`, `i_image_source_service`/`image_source_service_registry`, `image_source_loader`, `cancellation_token`) | core + controls + handlers | ✅ | ✅* | ✅ | headless + macOS | M4d Unit I — uri + stream sources, an async loader with cancellation, an in-memory uri cache, and the service-registry seam. `i_uri_image_source` (uri + caching_enabled) / `i_stream_image_source` (a bytes provider `image_bytes get_bytes(token)`, simplifying C#'s `Task<Stream>`); concrete `uri_image_source`/`stream_image_source` + `from_uri`/`from_stream`. `i_image_source_service` (async `load(source, token, on_result)`); `image_source_service_registry` (mirrors handler_registry; `register_service<Source,Service>()` keyed by source interface, `resolve` via dynamic_cast probe) + a default registry; `image_source_result` (native handle + RAII disposer + headless kind/detail mirror); per-source `file/uri/stream_image_source_service` (cross-platform decl + `{headless,apple}` partials sharing one `decode_image_bytes`). `image_source_loader` (ports ImageSourcePartLoader + ImageSourceServiceResultManager): `begin_load` cancels the prior token + disposes the prior result; `update_source` resolves the service / uri cached fast-path (`map<uri,bytes>`, no expiry), marshals the apply onto the dispatcher (headless `manual_dispatcher`; apple inline), does the source-identity recheck (`!token.is_cancelled() && source==current_source_`), `complete_load`. `image_handler` OWNS the loader (`source_loader()`); `map_source` keeps the SYNC file fast-path + routes uri/stream async (routing cross-platform; 3 per-backend primitives touch NSImageView / mirror). Still chains `view_mapper()`; `image_platform` derives `view_platform_base` (+ `source_kind` mirror). THREADING: loader state touched only on the dispatcher thread; services load synchronously (no worker) so the sole cross-thread element is the atomic cancellation flag → TSan-clean; UAF-safe teardown via a loader liveness weak_ptr; ABA-safe via token cancellation. Disk caching/CacheValidity, font sources, the full DI provider, resolution reload, and a production HTTP stack deferred (apple uri = `file://` + sync `http(s)` NSData; headless = stream + `file://`, no network). 6 headless + 3 apple GTest cases (extend the existing image test files). *characterization |
| page lifecycle + `navigation_page` (`navigation_request`/`i_stack_navigation`, `navigation_page`, `navigation_page_handler`; `content_page` Appearing/Disappearing) | core + controls + core/handlers | ✅ | ✅* | ✅ | headless + macOS | M4d Unit N — a push/pop page stack. `content_page` gained the page lifecycle (Page.cs SendAppearing/SendDisappearing + `appearing`/`disappearing` events, idempotent via `has_appeared_`). `navigation_request` (new stack + animated) + `i_stack_navigation` (request_navigation/navigation_finished, IStackNavigation.cs). `navigation_page : view<i_view> + i_stack_navigation` owns a NON-owning `vector<content_page*>` (caller owns the pages) with push/pop/pop_to_root + current_page/root_page/navigation_stack; push/pop/pop_to_root mutate the stack then fire Disappearing(prev)+Appearing(new) BOTH before notifying the handler, per NavigationPage.cs MauiNavigationImpl ORDER (C#'s async semaphore/TaskCompletionSource/overlap queue collapsed to a synchronous single transition). Drive is a `"request_navigation"` COMMAND (payload = `navigation_request`); `navigation_page_handler` hosts a plain NSView container and swaps the current page's `native_view()` subview (headless mirrors the hosted page); `navigation_page_platform` derives `view_platform_base`, handler chains `view_mapper()`. `set_handler` re-issues the request after wiring (C# OnHandlerChangedCore) so the attached container hosts the current page; the navigation_page fires the ROOT's initial send_appearing() itself (absent window lifecycle). Nav bar/back button/title bar, push/pop animation, modal stack, InsertPageBefore/RemovePage, Shell deferred. 15 headless + 5 apple GTest cases; self-registers. *characterization |
| ViewMapper visuals: Background/Shadow/Clip (`paint`/`solid_paint`, `i_shadow`/`shadow`, `i_shape`/`rectangle`/`round_rectangle`/`ellipse`; `apple_visual_ops.hpp`) | graphics + core + controls + platform/apple | ✅ | ✅* | ✅ | headless + macOS | M4d Unit V — stands up the visual-layer value types `i_view` only forward-declared. `paint` + `solid_paint` (one color); `i_shadow` + `shadow` (radius/opacity/paint/offset; defaults 10/1/black/0); `i_shape` + `rectangle`/`round_rectangle`/`ellipse` (`path_for_bounds` via the existing `path_f` append_* helpers). `view_platform_base` gained background/shadow/clip mirrors + `update_background`/`update_shadow`/`update_clip`; `view_mapper()` keys background/shadow/clip; `view<>` makes them bindable (`property<shared_ptr<>>` + non-template descriptors). AppKit helpers `apply_background` (layer.backgroundColor) / `apply_shadow` (layer.shadow*) / `apply_clip` (path_f→CGPath→CAShapeLayer mask) in `apple_visual_ops.hpp`. **Retrofitted (M4d integration) onto all six platform structs** (button/label/entry/image/layout/content_page override `update_*`→the helpers; clip sized to NSView bounds; grid reuses layout_handler) so the visuals reach the native layer. Gradients/per-corner clip/Width/Height/Semantics deferred. 8 headless + 9 apple GTest cases. *characterization |
| gradient paints (`gradient_stop`, `gradient_paint` abstract, `linear_gradient_paint`, `radial_gradient_paint`; AppKit `CAGradientLayer`) | graphics + platform/apple | ✅ | ✅* | ✅ | headless + macOS | M4 backlog — the gradient brush kinds the M4d Background cut deferred. `gradient_stop` (offset/color value type, ordered by offset = PaintGradientStop.CompareTo); `gradient_paint : paint` (abstract) holds the stops + ports `GetColorAt` linear interpolation, StartColor/EndColor + StartColorIndex/EndColorIndex, GetSortedStops, IsTransparent (any stop alpha < 1), SetGradientStops/AddOffset/RemoveOffset/BlendStartAndEndColors; default stops white→white (empty restores it). `linear_gradient_paint` (start/end `point`, default (0,0)/(1,1)); `radial_gradient_paint` (center `point` + radius, default (0.5,0.5)/0.5). The GradientPaint-copy ctors leave start/end (or center/radius) at the C# struct default (Point.Zero/0), not the parameterless-ctor values — faithful to C#. `background_color()` returns the start/end blend (port decision: C#'s GradientPaint doesn't override BackgroundColor; this satisfies the abstract paint contract with a representative color). AppKit: `apply_background` (apple_visual_ops.hpp) now dynamic_casts to `linear`/`radial_gradient_paint` and installs a tagged `CAGradientLayer` sublayer (axial/radial `type`, `startPoint`/`endPoint` (radial endPoint = PaintExtensions GetRadialGradientPaintEndPoint, cornerRadius = radius), `colors[]`/`locations[]` from the sorted stops — ports GetCAGradientLayerColors incl. the transparent-stop neighbor-borrow + GetCAGradientLayerLocations incl. the even-spread fallback), sized to the backing-layer bounds; switching to solid/null removes it. The six platform structs are UNCHANGED (their `update_background` already routes through `apply_background`). `ImagePaint`/`PatternPaint` out of scope. 36 headless (gradient_paint_tests.cpp) + 8 apple (view_mapper_apple_tests.mm) GTest cases. *characterization (no C# unit-test oracle for these types). |

| layout foundation (`dimension`, `i_container`/`i_layout`/`i_stack_layout`, `i_layout_manager`) | core/layouts | ✅ | — | ✅ | headless + macOS | `ILayout : IView + IContainer + IPadding` (M3 subset; ClipsToBounds / ISafeAreaView / ICrossPlatformLayout deferred to M4). `dimension` = Unset(NaN)/Minimum(0)/Maximum(inf) + is_explicit_set |
| stack layout managers (`layout_manager`/`stack_layout_manager` + vertical/horizontal) | layouts | ✅ | ✅* | ✅ | headless + macOS | New `maui_layouts` lib — pure cross-platform measure/arrange (ResolveConstraints, MeasureSpacing, stacking). 28 GTest cases via mock view/stack (spacing / padding / min-max / collapsed-vs-hidden / fill / child-constraint). The layout *controls* + native panel are M4. *ported from C# Stack*LayoutManagerTests (the oracle) |
| Grid (`grid_length`/`grid_unit_type`, `i_grid_*` contracts, `grid_layout_manager`) | core + layouts | ✅ | ✅* | ✅ | headless + macOS | The full row/column algorithm: Absolute/Auto/Star sizing, row+column spans, spacing, padding, min/max, two-pass measure + arrange-time star expansion (pimpl grid_structure cached measure→arrange, like C# GridStructure). 19 GTest cases (representative subset of the large GridLayoutManagerTests). *ported from the C# oracle |
| layout controls (`i_layout_handler`, `layout<>` base, `vertical_stack_layout`/`horizontal_stack_layout`, `layout_handler`) | controls + core/handlers | ✅ | ✅* | ✅ | headless + macOS | M4b Unit 1 — wrap the M3 stack managers in controls + a native host panel. `layout<LayoutInterface>` owns the non-owning child list (i_container) + bindable padding, lazily builds its manager via `create_layout_manager()`, and overrides measure/arrange to delegate to the manager (a layout computes its own geometry); arrange also sizes the panel. `layout_handler` (i_layout_handler add/remove/clear/insert/update/update_z_index) hosts a plain NSView panel (headless: child-count mirror). `view_handler` derives `i_view_handler` *virtually* (layout_handler = view_handler + i_layout_handler); `i_view_handler::native_view()` exposes the hosted native view. Retrofitted onto `view_mapper()` (is_enabled keeps the base mirror — NSView has no enabled state). maui_layouts linked into maui_controls. Grid *control* deferred. 14 headless + 4 apple GTest cases; self-registers. *characterization |
| `entry` control (`i_text_input`/`i_entry`, `entry_handler`) | controls + core/handlers | ✅ | ✅* | ✅ | headless + macOS | M4b Unit 3 — first inbound-text + first editable native control. Bindable text/placeholder/is_password/is_read_only/max_length (UTF-8-byte truncation in set_text) + i_text_style appearance + alignment; `completed` + `text_changed(old,new)` events. Editable NSTextField (NSSecureTextField cell swap for password, preserving font/color) + an NSTextFieldDelegate trampoline (controlTextDidChange→send_text_changed, controlTextDidEndEditing→send_completed). Chains `view_mapper()`. AppKit defers character_spacing/vertical alignment/placeholder_color (headless mirrors them). 13 headless + 7 apple GTest cases; self-registers. *characterization |
| `image` control (`aspect`/`i_image`, `image_handler`; aspect only) | controls + core/handlers | ✅ | ✅* | ✅ | headless + macOS | M4b Unit 3 (minimal) — maps ONLY the scaling mode: `aspect` (aspect_fit/aspect_fill/fill/center) → NSImageView.imageScaling (+ centered). No image bytes loaded — the async image *source* subsystem (IImageSource/loaders/caching) is deferred. Chains `view_mapper()`. 5 headless + 2 apple GTest cases; self-registers. *characterization |

| data binding (`binding_mode`, `bind()` + `binding_handle`; `bindable_property` default_binding_mode) | core | ✅ | ✅* | ✅ | headless | M5a — **typed accessor bindings** (reflection-free, the PROFILE §6 / code-first substitute for C#'s string-path + INotifyPropertyChanged). `bind(property<T>& target, property<U>& source, mode, convert, convert_back)` observes the source via the existing `property<T>.changed` and pushes at `setter_specificity::from_binding` (so a manual set still wins + clears restore the bound value); modes one_way/two_way (re-entrancy-guarded, no feedback loop)/one_time/one_way_to_source; `default_mode` resolves to the target's `default_binding_mode` (+ two_way→one_way_to_source on a read-only target); RAII `binding_handle` tears the subscriptions down. 8 GTest cases (every mode + converter + the precedence interaction + teardown). **BindingContext + tree inheritance folded into M5c** (shares the element-tree propagation). String paths / registered-accessor table / StringFormat / FallbackValue / MultiBinding / compiled bindings deferred to M7. *characterization |

| styles / setters / triggers / VSM (`setter`, `style`, `property_trigger<T>` + `trigger_handle`, `visual_state`/`visual_state_group`/`visual_state_manager`) | controls | ✅ | ✅* | ✅ | headless | M5b — apply/un-apply bundles of property values through the **existing** value-precedence ladder (`property<T>::set/clear(value, specificity)`). Foundation: each `property<T>` **self-registers** an erased `{apply(any, specificity), clear(specificity)}` handle with its `bindable_object` (keyed by descriptor name) so `setter::of(descriptor, value)` reaches a typed slot via `apply_setter`/`clear_setter` — value boxed at the boundary, **storage stays typed**. `style` = target type-tag + `vector<setter>` (+ a directly-assigned `based_on` applied at `setter_specificity::as_base_style`); `view<>` gains `set_style`/`style()` (apply at `style_local`, un-apply the previous). `property_trigger<T>` watches a `property<T>` via `.changed`, applies setters at `setter_specificity::trigger` (above a manual value) while it equals the target, RAII `trigger_handle` tears down + un-applies. `visual_state_manager::go_to_state` swaps the outgoing/incoming state setters at `visual_state_setter`. 15 GTest cases (style precedence ladder + replace + based_on; trigger apply/override/restore/teardown; VSM swap/idempotent/unknown/manual-interaction). Ported `SetterSpecificity.AsBaseStyle`. Implicit styles / ResourceDictionary / DynamicResource / the VSM implicit-style downgrade + system-state promotion / EventTrigger / MultiTrigger / StateTriggers deferred (see backlog). *characterization |

| Application / Window / element lifecycle + inherited BindingContext (`controls::element`, `window`, `application`; `bindable_object` BindingContext) | controls | ✅ | ✅* | ✅ | headless | M5c — a non-template `controls::element` re-roots `view<>` (mirroring C#'s BindableObject → Element → VisualElement), carrying the shared tree lifecycle: `for_each_logical_child` (containers — `content_page`/`layout<>`/`navigation_page` — override it), **typed inherited BindingContext** (held on `bindable_object` as `shared_ptr<void>` + a `type_tag` so `binding_context<X>()` is checked; an explicit set blocks inheritance; `on_binding_context_changed` propagates down), and the **Window back-ref + Loaded/Unloaded** (`set_containing_window` flows the window down the subtree, firing Loaded top-down / Unloaded bottom-up). `window` hosts one root page + the IWindow lifecycle (`send_created`/`activated`/`deactivated`/`destroying`); activating it Loads the page subtree + drives `content_page` Appearing (the windowed-appearing the M4d deviation lacked). `application` owns the windows + the one-time `started` hook (`open_window` starts+creates+activates). 13 GTest cases (BindingContext inherit/override/through-layout/typed-null; window activate/deactivate Loaded-Unloaded + subtree propagation; application open/close/start + context inheritance app→window→page). Native NSWindow `window_handler`, modal/multi-window, and string-path bindings deferred (see backlog). *characterization |

_(extend this table as components are added)_
