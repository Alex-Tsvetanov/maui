# Public API redesign — context brief

> **Purpose.** This doc is the *standalone* starting context for a focused session that redesigns the
> framework's **public (consumer-facing) API** to be idiomatic C++ — not a 1-to-1 transliteration of
> C# .NET MAUI. Read this first; it tells you what the framework is, what the public surface looks like
> today, exactly which "C#-isms" read wrong to a C++-only developer, what you must NOT break, and the
> design decisions to make. You do **not** need the parity/UI history to do this work.

## 1. What the framework is (one paragraph)

A C++23 port of .NET MAUI living entirely under `port/cpp/`. Cross-platform UI: a virtual-view ⇄
handler ⇄ native-view architecture with three backends compiled per CMake preset — **headless**
(unit-testable, no device), **apple** (AppKit), **ios** (UIKit). The UI is feature-complete and now
renders near-pixel-identical to real MAUI on iOS. Authoritative docs: `port/PROJECT.md` (layers/
milestones), `port/cpp/PROFILE.md` (the C++23 language profile — naming, ownership doctrine, idiom map),
`port/CLAUDE.md` (operating manual). The libraries are `maui_graphics / core / controls / layouts /
xaml / hosting / essentials`; a consumer links `maui::hosting` via `find_package(maui CONFIG REQUIRED)`.

## 2. The public API a consumer actually touches today

Canonical, idiomatic-as-currently-designed usage lives in **`port/cpp/examples/`** (hello_world,
counter, layouts, data_binding, collection_view, custom_drawing, + the heavy `gallery`). The entry
point (`port/cpp/include/maui/maui_main.hpp`) is already clean and C++-shaped:

```cpp
#include "maui/maui_main.hpp"
class my_app : public maui::controls::application { /* owns window→page→content */
    maui::core::i_window* create_window() override { return &window_; }
};
maui::hosting::maui_app_builder use_shared_maui_app(maui::hosting::maui_app_builder b) {
    b.use_maui_app<my_app>();
    return b;
}
```

But once you build UI, the C# heritage shows. Real excerpt from `examples/data_binding/main.cpp`:

```cpp
class greeting_view_model : public maui::core::bindable_object {
    static const maui::core::bindable_property<std::string>& message_property() {       // (A)
        static const maui::core::bindable_property<std::string> descriptor{"message"};
        return descriptor;
    }
    maui::core::property<std::string> message{*this, message_property()};                // (B)
};
// ...
prompt_.set_text("Type a name…");                                                        // (C)
greeting_.set_binding_context(view_model_);                                              // (D)
greeting_.set_binding("text", "message", maui::core::binding_mode::one_way);             // (E)
text_token_ = maui::core::connect_scoped(input_.text_changed,                            // (F)
    [this](const std::string& old_value, const std::string& new_value) { ... });
root_.set_padding(maui::core::thickness{16.0});  root_.set_spacing(12.0);                // (C)
```

## 3. The problem — C#-isms that read wrong to a C++-only developer

These are the redesign targets. Each is a faithful C# translation that a C++ dev finds confusing or
verbose:

- **(A) `bindable_property<T>` descriptors** — static-member functions returning a named static
  descriptor (`{"message"}`). This is C#'s `BindableProperty.Create(...)` ceremony. A C++ dev expects a
  plain member or a typed property object, not a registry keyed by string name.
- **(B) `property<T> member{*this, descriptor()}`** — the C# bindable-property member wrapper;
  boilerplate-heavy to declare a single field.
- **(C) setter-per-property (`set_text` / `set_padding` / `set_spacing` / `set_width_request` …)** —
  mirrors C# property setters. No fluent/builder chaining, no aggregate/designated initialization, no
  construction-from-config. Building a tree is a long sequence of imperative `set_*` calls.
- **(D) `set_binding_context(shared_ptr)`** — C# `BindingContext`. Couples user code to `shared_ptr`
  view-models and an implicit ambient context.
- **(E) `set_binding("text", "message", mode)`** — **stringly-typed** binding by property name. No
  compile-time checking; this is XAML-binding ergonomics in C++ where we could bind with typed
  member/lambda accessors.
- **(F) events as `connect`/`connect_scoped(evt, λ)` with `(old, new)` args** — C#
  `EventHandler<ValueChangedEventArgs>` shape. Works, but the token/scoped-connection model + the
  twin-arg signature may not be the most natural C++ signal idiom.
- **`shared_ptr<...>` in user code** — paints, shapes, image sources, templates, view-models are all
  `std::make_shared<...>` + `std::static_pointer_cast<...>` at call sites (see any page). Heavy for a
  consumer; a value/handle/RAII-friendly surface may be possible.
- **`i_*` interfaces leaking** (`i_window`, `i_button`, `i_view`) — the virtual-view contracts surface
  in consumer signatures (e.g. `create_window()` returns `i_window*`). Internally load-bearing; question
  is how much should be public.
- **deep namespaces** — `maui::core::`, `maui::controls::`, `maui::graphics::` at every call site; no
  curated top-level facade / `using` surface for consumers.
- **handler/registration seam** — `MAUI_REGISTER_HANDLER`, `handler_type_tag`, `attach_handler` are
  internal but adjacent to the public surface; keep them out of the consumer's face.

## 4. Hard constraints — do NOT break these

1. **Behavioral fidelity to MAUI is sacred.** The redesign is about the *surface syntax/ergonomics*,
   not semantics. Every control must behave exactly as it does now (and as real MAUI does). Behavior is
   derived from the C# source/tests, never invented — see `port/CLAUDE.md`.
2. **The virtual-view ⇄ handler ⇄ native seam stays.** It is the whole point of the architecture
   (`PROFILE.md`). Don't collapse it. A nicer public API should layer *over* it.
3. **snake_case is the house style** (`PROFILE.md` — `maui::controls::button`, `btn.clicked.connect`).
   Keep it. The redesign is not a rename to CamelCase.
4. **Ownership doctrine (`PROFILE.md §8`)** — `shared_ptr` element tree, `weak_ptr` back-refs,
   pimpl-owned native views, token-based event teardown. A friendlier surface must still honor it (no
   leaks, no cycles, RAII teardown).
5. **Don't regress the test suite, the examples, the gallery, or iOS parity.** Baselines: headless
   `ctest` **3080** tests green; `tools/gate.sh --fast` PASS (headless + apple build + clang-tidy **0**
   findings, no suppressions); the 7 examples + 178-page gallery build and run; the iOS parity pipeline
   (`tools/parity`) still captures the gallery. Inner loop: `tools/dev.sh [regex]`. (Requires
   `VCPKG_ROOT` set; `ccache` installed.)
6. **Headless-first.** Prototype + unit-test any new API on the headless backend before the native ones.

## 5. In scope / out of scope

- **In scope:** the *consumer-facing* surface — how you declare a control + its properties, build a
  tree (fluent/builder/initializer vs imperative setters), express bindings (typed vs stringly), wire
  events (signal idiom), pass paints/shapes/templates (value vs `shared_ptr`), and the
  namespace/`using` facade. Likely additive: a new ergonomic layer + adapters over the existing controls
  (so old code keeps working) OR a measured migration of the public signatures.
- **Out of scope (for the design phase):** the handler/native backends, the layout engine, XAML, the
  property *engine* internals (`bindable_property` value-precedence is correct — the question is only
  how it's *exposed*). Don't touch `src/` (the C# reference), `vault/`, `graphify-out/` (read-only).

## 6. Design decisions to make (the agenda for the new session)

1. **Construction & tree-building:** keep imperative `set_*`? add fluent chaining (`.text(...).padding(...)`)?
   designated-initializer/config structs? a declarative builder (`vstack({ label("..."), button(...) })`)?
   How does it stay zero-overhead and ownership-correct?
2. **Properties:** can a control field be a plain typed property object (`property<T>` with no descriptor
   boilerplate) while preserving the binding engine? Hide `bindable_property` descriptors from consumers?
3. **Bindings:** offer **typed** binding (member-pointer / lambda accessor, compile-checked) alongside or
   instead of the stringly `set_binding("text","message")`? What about one-way/two-way ergonomics?
4. **Events/signals:** is `connect_scoped(evt, λ)` the right idiom, or a `signal<...>`/observer with
   `operator+=` / RAII connection? Argument shape (drop the C# `(old,new)` twin where a single value
   suffices)?
5. **Value vs `shared_ptr`:** can paints/shapes/brushes/image-sources be passed by value or via small
   handle types, with `shared_ptr` confined to the internal tree?
6. **Facade & namespaces:** a curated `maui::ui` (or top-level) surface + `using`s so consumers aren't
   typing `maui::core::`/`maui::graphics::` everywhere. What's public vs `detail`?
7. **`i_*` exposure:** which interfaces should a consumer ever name (e.g. `create_window` return type)?
8. **Migration strategy:** additive ergonomic layer (old API stays) vs in-place signature change (update
   the 178 gallery pages + 7 examples + tests). Recommend one, with effort/blast-radius.

## 7. Where to start

Read `PROFILE.md` (esp. the idiom map + §7 property system + §8 ownership) and §5 handler infra; skim
`include/maui/controls/{view,button,label,entry,application,window}.hpp` and
`include/maui/core/{bindable_object,bindable_property,property,event}.hpp` for the current public shapes;
then read all 7 `examples/*/main.cpp` as the live "best current usage" to critique. Produce a proposed
idiomatic API (with before/after snippets for each item in §3) and a migration recommendation before
writing code.
