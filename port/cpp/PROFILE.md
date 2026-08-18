# PROFILE — C++26 target

> The language profile for the MAUI port. Read after `../PROJECT.md` and `../CLAUDE.md`.
> **Language:** C++26 · **API style:** idiomatic `snake_case` · **First platform:** Apple (develop on
> macOS, then iOS/Mac Catalyst) after a headless bring-up of M0–M5.

---

## 1. Toolchain & standard

- **Standard:** **C++26** is the target — `#embed` carries every compile-time-XAML page (408 TUs) and
  the `{Binding}` auto-wire path is written against static reflection (P2996 `std::meta`), both C++26
  features. Features we lean on from C++23 and earlier: `std::expected`, `std::optional`,
  `std::variant`, `std::span`, ranges, `<format>`/`std::print`, deducing `this`, `if consteval`,
  `std::move_only_function`, coroutines.
  - *Built with `-std=c++23` / `/std:c++23` today*, not `c++26`: clang accepts `#embed` under c++23 as
    an extension (silenced with `-Wno-c23-extensions`), and no shipping toolchain has `std::meta`, so
    the reflection path stays feature-gated (`maui/xaml/feature.hpp`). Raising the pin is a build
    change that has to clear `gate.sh` on all four backends — MSVC has no `#embed` at all, which is
    why the Windows/Android lanes compile a bytes-mode twin of each XAML TU instead.
  - *Toolchain gap:* the libc++ shipping with AppleClang 21 (libc++ 210106) does **not** expose
    `std::move_only_function` (not in `<functional>`, no `__cpp_lib_move_only_function`, absent even
    under `-fexperimental-library`). The port provides `maui::core::move_only_function` as a drop-in
    until libc++ ships it; swap back then.
- **Compilers (min):** Clang 18+ (incl. AppleClang for Obj-C++ `.mm`), GCC 14+, MSVC 19.40+ (VS 2022 17.10+).
- **Modules:** prefer **headers** for now (cross-compiler module tooling is still uneven). Revisit named
  modules once the core stabilizes. Do not block bring-up on modules.
- **Warnings:** `-Wall -Wextra -Wpedantic` (treat warnings as errors in CI once green).

## 2. Build / test / package tooling

| Concern | Choice | Notes |
|---|---|---|
| Build system | **CMake ≥ 3.28** + **Ninja** + **ccache**, with `CMakePresets.json` | one preset per backend (`headless`, `apple`, `windows`, `android`, `linux`); ccache auto-wired in `CMakeLists.txt` (the lanes share no objects, so it is what keeps the multi-preset gate affordable). Inner loop `tools/dev.sh`, full gate `tools/gate.sh` — see `STATUS.md` "Build & test". |
| Test framework | **GoogleTest + GMock** (via `ctest`) | port the C# unit tests (NUnit/xUnit) into GoogleTest `TEST`/`TEST_P` cases; GMock covers the handler / native-view seam at M1+. (Catch2 was the original default; superseded.) |
| Benchmarks | **Google Benchmark** | migrate the C# BenchmarkDotNet benchmarks (`src/**/tests/*.Benchmarks`). |
| Dependency mgmt | **vcpkg** (manifest mode, `vcpkg.json`) | keep deps minimal: `gtest`, `benchmark`; `fmt` only if `<format>` gaps appear. |
| Formatting/lint | clang-format + clang-tidy | a `.clang-format` mirroring this profile's style. |
| Headless CI | build the `headless` preset, run `ctest` | M0–M5 are fully testable with **no device**. |

## 3. Port repository layout (`port/cpp/`)

```
port/cpp/
  PROFILE.md            (this file)
  CMakeLists.txt        (root; selects backend by preset/option MAUI_BACKEND)
  CMakePresets.json
  vcpkg.json
  include/maui/...       public headers (the API surface, snake_case)
  src/
    graphics/            layer 0
    core/                layer 1  (virtual-view interfaces + property/handler infra)
    handlers/            layer 2  (mappers; core + per-backend)
    layouts/             layer 4
    controls/            layer 5
    xaml/                layer 6  (deferred)
    essentials/          layer 7
    hosting/             layer 8
    platform/
      headless/          the Standard backend (in-memory; used by tests)
      apple/             .mm Obj-C++ (UIKit/AppKit)
      windows/           C++/WinRT (WinUI 3)
      android/           JNI/NDK
      linux/             GTK4 (community extension; beyond MAUI scope)
  tests/                 GoogleTest ports of src/**/tests
  benchmarks/            Google Benchmark ports of src/**/tests/*.Benchmarks
  samples/               tiny host apps for the runtime proof (M2/M4)
```

Mirror MAUI's **cross-platform vs platform split** as: a core `.cpp` per component + a per-backend
`.cpp`/`.mm` under `src/platform/<backend>/`, selected by the build. This is the C++ analog of C#
`partial class Foo` + `Foo.Android.cs`.

**File organization (one primary type per header + matching `.cpp`).** Declarations go in the header,
definitions in the `.cpp` — `color.hpp`/`color.cpp` is the model. Keep only what must be inline
(templates, `constexpr`, trivial friend operators) in the header. **No** "re-export" shim headers that
merely `#include` a bigger one, and **no** dumping several unrelated types into one header. A tightly
coupled *cluster* may share a header where splitting would hurt cohesion (e.g. `event` + its
`disconnectable` base + `scoped_connection`). Interdependent value families (point/size, rect) achieve
one-type-per-header via **forward declarations** in each header, with the cross-type conversions and
mixed operators defined in the `.cpp`s. Internal-only helpers live under `src/<layer>/detail/`, never in
the public `include/` tree. This keeps incremental rebuilds fast (editing one `.cpp` recompiles only it).

## 4. Backend strategy — all platforms

| Backend | Native toolkit | Interop mechanism | Source ext | Dispatcher / UI thread | Priority | Key gotchas |
|---|---|---|---|---|---|---|
| **headless / Standard** | none (in-memory views) | pure C++ | `.cpp` | inline / test scheduler | **1st (M0–M5)** | mirrors MAUI `*.Standard.cs`; lets every non-visual layer be unit-tested. The reference for "what a backend must implement". |
| **macOS** | **AppKit** (`NSView`/`NSButton`…) | **Objective-C++** (`.mm`): include C++ and ObjC in one TU, ARC for ObjC objects | `.mm` | `dispatch_get_main_queue()` / `NSRunLoop` | **2nd — primary dev target** | ARC vs C++ lifetime: hold ObjC views in `__strong` ivars wrapped by a C++ owner; bridge with `__bridge`. |
| **iOS + Mac Catalyst** | **UIKit** (`UIView`/`UIButton`…) | Objective-C++ (`.mm`); shares ~90% with macOS via a thin AppKit/UIKit shim | `.mm` | `DispatchQueue.main` (GCD) | **3rd** | device deploy + simulator; Mac Catalyst is UIKit-on-mac, reuse iOS backend. |
| **Windows** | **WinUI 3** (`Microsoft.UI.Xaml.Controls`) | **C++/WinRT** (header-only projection) | `.cpp` | `DispatcherQueue` | 4th | projection ceremony (`winrt::` types, `IInspectable`, activation, async `IAsyncAction`); needs Windows App SDK. |
| **Android** | **Android Views** (`android.view.View`, `AppCompatButton`) | **JNI/NDK** (call Java/Kotlin via `JNIEnv`) | `.cpp` | post to main `Looper`/`Handler` | 5th — **do last** | JNI marshalling, local/global refs, thread attach/detach, Java object lifetime vs C++; the least portable layer. |
| **Linux** (extension) | **GTK4** (`GtkWidget`/`GtkButton`) | gtkmm or raw GObject C API | `.cpp` | `g_idle_add` / GLib main loop | optional — **beyond MAUI scope** | MAUI has no Linux target; this is a net-new backend. Same handler seam applies, so it's additive. Treat as a stretch goal once 2+ official backends work. |

**Why Apple first:** Objective-C++ gives the cleanest C++↔native interop (one `.mm` translation unit
sees both worlds, no marshalling/projection layer), iOS+Mac Catalyst reuse most of the macOS backend,
and it matches the dev machine. Build the **headless** backend first (proves M0–M5 with no device),
then **macOS** (proves the handler seam end-to-end, M2), then **iOS/Mac Catalyst**.

## 5. Idiom map — C# (.NET MAUI) → C++26

| C# construct | C++26 mapping | Notes |
|---|---|---|
| `partial class Foo` + `Foo.Android.cs` | core `foo.cpp` + `src/platform/<backend>/foo.cpp\|.mm`, joined at link time; native handle via **pimpl** | the cross-platform/platform seam. |
| `BindableProperty` + value precedence | `maui::core::bindable_property<T>` registered on a `bindable_object` with a **precedence stack** (default < style < binding < manual) | see §7. The single hardest core piece. |
| property `public string Text { get; set; }` | a `property<T>` **member object**: `btn.text.get()`, `btn.text.set("Hi")`, `btn.text.changed.connect(...)` | no `get_/set_` proliferation; backed by a `bindable_property`. |
| `event EventHandler X` / `+=` | `maui::core::event<sender*, args>` member; `x.connect(fn) -> token`, `x.disconnect(token)`, `x.raise(...)` | hand-rolled signal using `std::move_only_function` + token list. |
| `INotifyPropertyChanged` | base with a `property_changed` event keyed by property name (`std::string_view`) | drives bindings. |
| generic base `ViewHandler<TVirtual, TPlatform>` | **CRTP** `view_handler<Derived, Virtual, Platform>` + a non-generic `i_view_handler` interface | avoids vtable-through-templates; keeps a polymorphic handle. |
| `IServiceProvider` / MS.Ext.DI | a small `service_registry` (type-keyed) + a **handler registry** mapping virtual-view tag → handler factory | **no reflection** — registration is explicit (see §6). |
| `async Task<T>` / `await` | C++26 coroutines: `maui::core::task<T>` + an executor; UI marshalling via the **dispatcher** | `co_await dispatcher.run_on_ui(...)`. |
| `SynchronizationContext` / `Dispatcher` | `maui::core::dispatcher` posting to the backend's main loop (GCD / DispatcherQueue / Looper) | every backend supplies one. |
| extension methods | free functions in the type's namespace (ADL) | e.g. `maui::controls::set_padding(view, ...)`. |
| nullable `T?` (ref) | `T*` / `std::shared_ptr<T>` (null = absent); nullable value `int?` → `std::optional<int>` | ownership per §8. |
| `string` (UTF-16) | **`std::string` (UTF-8)** internally; convert at the native boundary | document the boundary; AppKit/UIKit want `NSString` (UTF-16) — convert in the `.mm` shim. |
| `IDisposable` / `DisconnectHandler()` | **RAII**: destructors + an explicit `disconnect()` in the handler; native view released in handler dtor | deterministic teardown replaces GC finalization. |
| reflection (XAML loader, DI scan) | **code generation or explicit registration tables** — C++26 has static reflection (P2996), but no shipping toolchain does | defer XAML; hand-register handlers/types (see §6). |
| `enum` / `[Flags]` | `enum class name { ... }` (snake_case members); flags via `enum class` + bitwise operator overloads | |

## 6. The no-reflection consequence (read this)

MAUI uses reflection for handler discovery, DI, and XAML instantiation. C++26 standardises static
reflection (P2996), but **no toolchain we build on ships it** — `__cpp_reflection` is undefined on
Apple clang 21, so `MAUI_HAS_XAML_REFLECTION` is off everywhere (`maui/xaml/feature.hpp`). Until that
changes, treat reflection as absent and replace it with **explicit registration**:

- **Handler registry:** `register_handler<button, button_handler>()` in hosting startup; resolve by a
  stable per-type tag (`maui::core::type_tag` — a `constexpr` id, not RTTI-dependent).
  - *Decided (2026-06): explicit primitive now, opt-in self-registration later.* `register_handler`
    is the predictable, tree-shakeable primitive (#22). A `MAUI_REGISTER_HANDLER(view, handler)` macro
    (a static registrar that calls `register_handler`) may be offered at M2 for "non-explicit"
    convenience — but **self-registration requires the handlers to live in a CMake OBJECT library**
    (or `--whole-archive`), otherwise the static library linker drops the unreferenced TU and the
    registrar never runs. Reflection-style auto-discovery is impossible in C++; this is the closest.
- **Property registration:** each `bindable_property` is declared statically on its owner type at load
  time (a `static` registrar), not discovered.
- **XAML:** deferred (layer 6). When tackled, parse markup to a builder that calls the same explicit
  factories — or generate C++ from XAML at build time. Never assume runtime type discovery.

## 7. Property system (`maui::core`) — implemented, **no type erasure in the core**

C#'s `BindableObject` stores every value as `System.Object` in one per-object dictionary (it has no
choice — reflection-driven XAML/styles need a uniform store, and GC makes boxing cheap). A C++ rewrite
should **not** copy that: a central store forces type erasure (`std::any` ⇒ no compile-time safety,
heap-per-value, RTTI). Instead the value lives, strongly typed, in each property's own member. (Decided
with the maintainer; see §11.)

```cpp
// Packed-uint64 precedence key; integer order IS the precedence. constexpr value type.
class setter_specificity { /* default < binding < dynamic-resource < manual < trigger < vsm < handler */ };
template <class T> class setter_specificity_list { /* sorted typed store, highest specificity wins */ };

// Shared, static, *typed* descriptor (the C# `XxxProperty` field). No std::any.
template <class T> class bindable_property {
  bindable_property(std::string name, T default_value = {}, options /*typed callbacks*/ = {});
  // options: property_changed/changing, coerce_value, validate_value, default_value_creator (all typed)
};

// Per-instance member: owns the typed value store + the value precedence + a typed change event.
template <class T> class property {
  property(bindable_object& owner, const bindable_property<T>& descriptor);
  const T& get() const;                              // zero-copy; falls back to the descriptor default
  void set(T value, setter_specificity = manual);    // precedence + notification, all typed
  void clear(setter_specificity = manual);
  event<T, T> changed;                               // (old, new), delivered by const ref
};

class bindable_object {                              // base of every element — notification surface only
  event<std::string_view> property_changed, property_changing;   // INotifyPropertyChanged, keyed by name
protected: virtual void on_property_changed(std::string_view);   // property<T> routes through this (friend)
};
```

Storage is lazy (an unset property is just the shared descriptor default, no per-instance allocation).
Value precedence matches MAUI exactly (default < style/dynamic-resource < binding < manual < trigger <
vsm; handler values overridden by any other set; below-current-specificity sets kept silently for
un-apply), verified against `src/Controls/tests/Core.UnitTests` (`SetterSpecificityListTests` +
BindableProperty/value-path tests). **Type erasure is allowed only at boundaries that inherently need
it** — XAML/style set-by-name (parsing a string → `T`, M5/M6), via a per-property adapter — never in the
hot path.

## 8. Memory & ownership doctrine (C++-specific, mandatory)

MAUI is garbage-collected; C++ is not. Adopt one explicit model and keep it everywhere:

- **Element tree:** nodes are `std::shared_ptr<element>`. A parent holds `shared_ptr` to children;
  child holds **`weak_ptr` to parent** (`Parent`) to break cycles. `BindingContext` back-references are
  `weak_ptr`.
- **Virtual view ⇄ handler:** the handler holds a **non-owning** `weak_ptr`/raw pointer to its virtual
  view; the virtual view holds a `shared_ptr<i_view_handler>` (or owns it) — pick one direction and be
  consistent (recommended: view owns handler).
- **Native view:** owned by the handler via **pimpl** (`unique_ptr` to a platform impl, or an ARC
  `__strong` ivar inside the `.mm`). Released in the handler destructor (`disconnect`).
- **Events/bindings:** subscriptions return **tokens**; store them and disconnect in destructors to
  avoid dangling. Never capture owning `shared_ptr` of the subscriber inside its own handler (cycle).
- **Threading:** mutate the visual tree only on the UI thread; cross-thread work returns via the
  `dispatcher`.

> This doctrine is the most important net-new design vs. the C# original. Get it right at layer 1 — it
> is far cheaper than retrofitting ownership later.

## 9. API naming transform (snake_case rules)

| C# | C++ | Example |
|---|---|---|
| namespace `Microsoft.Maui.X` | `maui::x` (drop vendor prefix, lowercase) | `Microsoft.Maui.Controls` → `maui::controls`; `Microsoft.Maui` → `maui::core` |
| type `PascalCase` | `snake_case` | `VisualElement` → `visual_element`, `ContentPage` → `content_page`, `Button` → `button` |
| method `DoThing()` | `do_thing()` | `InvalidateMeasure()` → `invalidate_measure()` |
| property `Text` | `property<T>` member `text` | `view.text.set(...)` |
| event `Clicked` | `event<...>` member `clicked` | `btn.clicked.connect(...)` |
| interface `IButton` | `i_button` (or a concept where it's a static contract) | prefer concepts for pure shape contracts; `i_`-classes where runtime polymorphism is needed |
| enum `LayoutAlignment.Center` | `enum class layout_alignment { center }` | members snake_case |
| generic `Foo<T>` | `foo<T>` template | |

Keep the **mapping to the C# name discoverable**: in each public header, document the originating C#
FQN in a comment (e.g. `// maui::controls::button  <=  Microsoft.Maui.Controls.Button`) so the vault and
tests stay cross-referenceable.

## 10. Where to start (concrete first steps on macOS)

1. Scaffold `port/cpp/` (CMake + presets + vcpkg + GoogleTest), `headless` preset building an empty lib + a
   green smoke test.
2. **M0 — Graphics:** port `maui::graphics` primitives (`color`, `point_f`, `size_f`, `rect_f`,
   geometry) from `src/Graphics/src/Graphics`; port `src/Graphics/tests` into GoogleTest. Green = M0.
3. **M1 — Core:** property system (§7) + event/dispatcher + the `i_view`/`i_element` contracts +
   `view_handler` base + handler registry (§6). Unit-test the property precedence against the ported
   BindableProperty tests.
4. **M2 — Button, headless then macOS:** follow the handler recipe in `../CLAUDE.md`. Headless backend
   first (testable), then the macOS `.mm` backend; prove a tap in a tiny `samples/` AppKit app.

## 11. Decisions locked at M1 (2026-06-06)

Confirmed with the maintainer before building the Core layer. These are now binding for the port:

- **View owns handler.** The virtual view holds `shared_ptr<i_view_handler>`; the handler holds a
  `weak_ptr<i_view>` back-reference. The native view lives in the handler (pimpl) and is released in
  the handler's destructor (`disconnect`). Mirrors MAUI's `IElement.Handler` and the §8 element tree;
  deterministic teardown when the view is dropped.
- **`property<T>` is a member object** — `btn.text.set(...)`, `btn.text.get()`, `btn.text.changed`.
  Owners are non-copyable, heap-only (already mandated by §8); each property is wired with a
  back-pointer to its owner in the owner's constructor. Backed by a `bindable_property` (§7).
- **Contracts: per-type rule** — *runtime polymorphism ⇒ `i_*` abstract class; compile-time-only shape
  ⇒ `concept`.* So `i_element` / `i_view` / `i_layout` / `i_view_handler` are abstract classes (the
  handler seam is runtime-polymorphic); `concept`s are used only to constrain genuine generics (e.g.
  the `view_handler<Derived, Virtual, Platform>` CRTP parameters). No `concept` mirrors of the classes.
- **Headers, not named modules** (revisit post-M5). Portable and low-risk; vcpkg ships headers and
  cross-compiler module tooling is still uneven. Do not block bring-up on modules.
