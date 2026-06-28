# Public API design — the idiomatic-C++ `maui::ui` layer

> **What this is.** The design for the framework's idiomatic-C++23 **consumer-facing** API — an additive
> `maui::ui` layer over the existing `maui::controls/core/graphics` and the virtual-view⇄handler⇄native
> seam. It answers the context brief [`PUBLIC_API_REDESIGN.md`](PUBLIC_API_REDESIGN.md) with a before/after
> for each of that brief's §3 targets (A–J), the resolved control-instance **ownership model**, the design
> decisions, where **XAML** fits in the future ([§6](#6-xaml--the-future-layer-how-it-fits)), and a phased,
> non-regressing migration. Design-only; no framework code ships with this document.
>
> **Status.** Design approved (handle semantics = **hybrid: move-only `view_ref<T>` + copyable
> `weak_ref<T>` + explicit `.share()`**, 2026-06-26). Implementation is the four phases in [§5](#5-migration);
> begin with the P4 de-risk prototype (it most directly removes the authoring verbosity this redesign
> targets). This doc consolidates and supersedes the earlier v1/v2 drafts.

---

## 1. Summary & principles

The property/event/binding **core is already good** (`property<T>` is a real member object, the typed
`bind()` ladder exists, events use RAII tokens). The friction is in the **consumer-facing skin** — the
brief's §3 list:

- **(A)(B)** view-model boilerplate: a static `bindable_property<T>` descriptor *plus* a `property<T>`
  member, per property.
- **(C)** an imperative tree built from a non-owning `layout::add(i_view&)` — which forces the consumer
  to declare **every widget as a value member in reverse-destruction order** (the "window outlives page
  outlives label" dance). *This verbosity + ordering is the pain that most motivates the redesign.*
- **(D)(E)** stringly-typed bindings (`set_binding_context` + `set_binding("text","message")`).
- **(F)** events via `connect_scoped` with an `(old,new)` twin.
- **(G)** `std::make_shared<...>` at every paint/shape/image-source call site.
- **(H)** leaked `i_*` (`application::create_window()` returns `maui::core::i_window*`).
- **(I)** deep namespaces, no facade. **(J)** the handler/registration seam adjacent to the surface.

Principles, in priority order:

1. **Additive, never in-place.** Every win is a new header forwarding to an already-verified public
   method. Old code compiles byte-identically. (Two opt-in, *touches-internals* exceptions: the binding
   *friend hook* ([§3-E](#e-stringly-bindings--typed-uibind)), and `event<>&` accessors for `.clicked()`.)
2. **Behavioral fidelity is sacred.** Surface syntax only. Writes route through the control's real
   `set_*`, so side effects (e.g. `entry::set_text` truncation) always run. Event chainers preserve the
   real event arity (the `value_changed(old,new)` catch — [§2.5](#25-the-ownership-model-the-resolved-decision)).
3. **The seam stays.** virtual-view⇄handler⇄native is untouched; the handle never enters the tree — the
   builder parents the real `controls::button` via the unchanged `layout::add(i_view&)`.
4. **Honor §8 — and enforce it in the type system where we can.** A **move-only** handle makes §8's named
   cycle footgun a *compile error*, not a lint. The builder *supplies* the §8 forward edge the examples
   currently fake with member order.
5. **snake_case, zero-overhead.** Aliases name the same entities; factories inline one `make_shared`;
   chainers return the handle by rvalue-ref; the refcount is **non-atomic intrusive** (licensed by §8's
   UI-thread-only rule), guarded by a debug thread-id assert.

**The shape, in one screen** (counter — note the single owning root, `weak_ref` member, `std::move` into
the tree, compile-safe self-capture):

```cpp
#include "maui/ui.hpp"
namespace ui = maui::ui;

class my_app : public ui::app {                                    // (H) no i_* named
public:
    my_app() {
        auto label = ui::label("Count: 0");
        count_label_ = label.weak();                               // observer minted BEFORE the move
        root_ = ui::vstack(
            std::move(label),                                      // (C) tree adopts the owner
            ui::button("Increment").on_click([this]{               // (F) [this], not [button] — no cycle
                if (auto l = count_label_.lock())                  //     held mutation via weak_ref
                    l->set_text("Count: " + std::to_string(++count_));
            })
        ).spacing(12).padding(16);
        window_.set_content(root_->page_content());
        window_.set_title("Counter");
    }
    maui::controls::window& main_window() override { return window_; }
private:
    int count_ = 0;
    maui::controls::window window_;
    ui::view_ref<maui::controls::content_page> root_;              // ONE move-only owner of the tree
    ui::weak_ref<maui::controls::label> count_label_;              // observer (copyable, non-owning)
};
```

Compare today's counter (`examples/counter/main.cpp:55-66`): 5+ value members in reverse-destruction
order, each widget a member. The builder collapses that to **one owning root** plus a `weak_ref` for the
one widget mutated later — directly removing both "ordered members" and "every widget is a member."

---

## 2. The `maui::ui` surface

A new `namespace maui::ui` + a single curated umbrella `maui/ui.hpp`:

| Group | Exposes | Notes |
|---|---|---|
| **Handles** | `ui::view_ref<T>` (move-only owner), `ui::weak_ref<T>` (copyable observer) | the ownership core — see §2.5 |
| **Type aliases** | `ui::color`, `ui::thickness`, `ui::rect`, `ui::size`, `ui::font`, `ui::binding_mode`, `ui::scoped_connection`, `namespace colors = maui::graphics::colors` | zero-cost `using` |
| **Control aliases** | both **short** (`ui::vstack`, `ui::hstack`, `ui::grid`, `ui::button`, `ui::label`, `ui::entry`) **and full** (`ui::vertical_stack_layout`, `ui::solid_paint`, …) | maintainer ruling: both |
| **App base** | `ui::app` | hides `create_window()→i_window*` |
| **Resource factories** | `ui::solid(color)`, `ui::rounded_rect(r)`, `ui::linear_gradient(...)` | value-in, `shared_ptr` stored internally |
| **Reactive (VM)** | `maui::core::observable<T>` | one-member bindable property |
| **Events** | `ui::on(evt, λ)` | arity-flexible signal sugar |
| **Bindings** | `ui::bind(ctrl, &set_x[, &x, &evt])` `.to/.to_two_way`, `bindable_ref<T>` | typed, routes through `set_*` |
| **Builder** | `ui::vstack(...)`, `ui::hstack(...)`, `ui::grid(...)` → `view_ref<T>` | owning declarative tree |

**Public/detail boundary = the facade contents.** Anything not aliased into `maui::ui` (the handler
registry, `MAUI_REGISTER_HANDLER`, `setter_specificity`, raw `bindable_property`, type tags, most `i_*`)
stays internal by omission. `i_drawable`/`i_canvas` remain public on purpose (the subclass-to-paint
extension contract).

### 2.5 The ownership model (the resolved decision)

The consumer manipulates controls through **four cooperating roles**:

| Role | Tool | Semantics |
|---|---|---|
| **A — construct** | `ui::vstack(...)` builder | returns a **move-only** `view_ref<T>` owning the subtree |
| **B — hold** | `view_ref<T>` | **move-only** owning handle; one owner at a time |
| **C — store / capture** | `weak_ref<T>` | **copyable**, non-owning; `.lock()` is the dangling-safe accessor |
| **D — mutate** | `observable<T>` + `ui::bind` | reactive; mutate *data*, so a held handle is rarely reached for |

**The handle, concretely:**

```cpp
namespace maui::ui {

template <class Impl>
class view_ref {                                   // MOVE-ONLY owning handle (builder node + held handle)
    std::shared_ptr<Impl> impl_;                   // the heap control — stable address (§8)
    std::vector<std::shared_ptr<element>> retained_;   // children it owns: the §8 parent→child forward edge
public:
    view_ref(view_ref&&) noexcept = default;
    view_ref& operator=(view_ref&&) noexcept = default;
    view_ref(const view_ref&) = delete;            // ← THE DECISION: not copyable
    view_ref& operator=(const view_ref&) = delete;

    [[nodiscard]] view_ref       share() const;    // explicit SECOND OWNER (copies impl_ + retained_) — opt-in
    [[nodiscard]] weak_ref<Impl> weak()  const;    // non-owning observer

    Impl* operator->() const { return impl_.get(); }   // escape hatch for long-tail properties
    Impl& impl()        const { return *impl_; }

    // curated fluent chainers — rvalue-qualified so they thread through the builder move; forward to the
    // REAL set_* (side effects preserved). ~12 common props; the long tail uses operator->.
    view_ref&& text(std::string s) &&  requires requires(Impl& v){ v.set_text(s); }
        { impl_->set_text(std::move(s)); return std::move(*this); }
    view_ref&  text(std::string s) &   requires requires(Impl& v){ v.set_text(s); }
        { impl_->set_text(std::move(s)); return *this; }
    // ... padding / spacing / background / on_click / on_value_changed ...
};

template <class Impl>
class weak_ref {                                   // COPYABLE non-owning observer (over std::weak_ptr)
    std::weak_ptr<Impl> w_;
public:
    weak_ref() = default;
    [[nodiscard]] std::optional<view_ref<Impl>> lock() const;   // transient owner to dot into; empty if dead
    [[nodiscard]] bool alive() const { return !w_.expired(); }
};

}
```

**Why move-only (rationale, condensed):**

- **It makes §8's cycle footgun a compile error.** `button.on_click([button]{ ... })` — capturing an
  owning handle inside the control's own handler — is `call to deleted copy constructor`, not a silent,
  permanent, ASan-clean leak. For a **non-GC framework with a non-atomic refcount and no cycle collector**,
  type-system enforcement of §8 is categorically better than a lint.
- **The ergonomic cost is bounded and mostly imaginary.** A usage census found ~**322/392** event-connect
  sites capture `[this]`, and **zero** capture a control by copy — so copyable's hold+mutate "advantage"
  is largely a win over a problem that isn't there, and move-only's `.lock()` tax lands on ~111–130 real
  held-mutation sites reached through `this`, not a handler rewrite.
- **Prior art agrees.** No non-GC native-C++ retained toolkit (Qt, JUCE, GTK-C) hands the developer a
  copyable *owning* widget handle; the copyable-GC-feel systems (gtk-rs, Slint, Flutter) are backed by a
  GC or a borrow checker and *still* ship "use `Weak` in callbacks."

**Why the `.share()` opt-in (hybrid, not pure move-only):** a real minority of pages legitimately co-own
controls (cross-mutating sibling widgets, reparent/helper-retain sites). `.share()` mints a deliberate,
**greppable, type-visible** second owner for exactly those — a `weak_ref<T>` member announces "observer,"
a `view_ref<T>` member announces "owner." Reference-semantic ownership lives here, *where you ask for it*,
instead of being the unsafe default.

**Cycle-safety rules (state these verbatim in user docs):**
- `[h]` (capture an owning `view_ref`) → **compile error**. Good.
- `[w = h.weak()]` → the **blessed** self-capture / stored-reference form.
- `.share()` → for co-ownership across **independent lifetimes** (audited clusters). **Never** `.share()`
  to capture into a closure the control itself owns — that is the one remaining, *visible*, greppable way
  to re-create a cycle. Use `weak()` there.

**Refcount:** non-atomic intrusive, licensed by §8's UI-thread-only mutation rule (steady-state count is 1
under move-only). A debug-build thread-id assert catches an accidental cross-thread handle op.

**Ownership diagram (in words):**

```
app member (the ONE root: ui::view_ref<content_page>, move-only)
  └─owns─> shared_ptr<content_page>                 ── stable heap addr (never moves)
             └─retained_ owns─> shared_ptr<vstack>
                                  ├─retained_ owns─> shared_ptr<label>   ← observed by a weak_ref member
                                  └─retained_ owns─> shared_ptr<button>
                                                       └─ on_click token parked IN impl (RAII)
framework internals (UNCHANGED): layout.children_ raw i_view* · element.parent_ raw · view→handler shared_ptr · handler→native unique_ptr/ARC
weak_ref<T> : weak_ptr<Impl> (never owning; .lock() = dangling check)   VM : shared_ptr<bindable_object> (separate graph)
```

Last owner drops → `~controls::button` runs → its `event<>` members + parked tokens tear down → handler
drops → native released. **Deterministic, RAII, cycle-free by construction.**

---

## 3. Before/after per §3 target

Tagged **additive** (new headers only) or **touches-internals**. Current signatures cited `file:line`.

### (A)+(B) VM property boilerplate → `core::observable<T>` — *additive*

```cpp
// BEFORE (examples/data_binding/main.cpp:38-46)
class greeting_view_model : public maui::core::bindable_object {
    static const maui::core::bindable_property<std::string>& message_property() {        // (A)
        static const maui::core::bindable_property<std::string> descriptor{"message"};
        return descriptor;
    }
    maui::core::property<std::string> message{*this, message_property()};                 // (B)
};
// AFTER
class greeting_view_model : public maui::core::bindable_object {
public:
    maui::core::observable<std::string> message{*this, "message", "World"};
};
```

`observable<T>` bundles `{std::string name_; bindable_property<T> descriptor_; property<T> property_;}` by
value (member order load-bearing: name→descriptor→property). The inner `property<T>` self-registers
`"message"` exactly as `property.hpp:37-69`, so `set_binding` and typed `bind` both still resolve.
Non-movable. **VM-only** — never controls (side-effecting setters). Risk: per-instance descriptor (tiny);
one name-parity test.

### (C) imperative tree + member-ordering footgun → owning move-only builder + chaining — *additive*

```cpp
// BEFORE (examples/counter/main.cpp:55-66 — N reverse-ordered value members + non-owning add())
maui::controls::vertical_stack_layout root_;   // declared so it outlives its children
maui::controls::label count_label_;            // ...AFTER root_ (reverse-destruction order)
maui::controls::button increment_button_;
root_.set_spacing(12.0);
root_.add(count_label_);
root_.add(increment_button_);
// AFTER (one move-only owner of the whole tree; ordering footgun gone)
auto label = ui::label("Count: 0");
count_label_ = label.weak();                       // observer minted BEFORE moving (order load-bearing)
root_ = ui::vstack(
    std::move(label),                              // named handles are MOVED in
    ui::button("Increment").on_click([this]{ /* ... */ })   // rvalue temporaries need no std::move
).spacing(12);
```

`ui::vstack(...)` heap-allocates each control (`make_shared`, stable address), calls the **unchanged**
non-owning `layout::add(i_view&)` for the framework edge **and** retains each child's `shared_ptr` in
`retained_` (the §8 owning forward edge). The root `view_ref` owns the subtree; teardown is children-first
by `vector` destruction. Because the handle is **move-only**, a named child handle is `std::move`'d in
(moved-from local is then empty — declare a `weak_ref` for later access **before** the move). Inline
`rvalue` children need no `std::move`. Chainers are rvalue-qualified one-liners forwarding to the real
`set_*` (`entry`'s truncating setter still runs). **Risk:** the *mint-after-move dead-handle* footgun —
calling `.weak()` after the move observes an emptied handle; document "mint before the move," lint later.

### (D) `set_binding_context(shared_ptr)` → typed source named directly — *additive*

```cpp
// BEFORE
greeting_.set_binding_context(view_model_);
greeting_.set_binding("text", "message", maui::core::binding_mode::one_way);
// AFTER (no ambient context for the typed path)
auto h = ui::bind(greeting_, &ui::label::set_text).to(view_model_->message);
```

`set_binding_context` stays public/unchanged for the stringly/XAML path. **Identical across all handle
semantics** — `bind` holds its own RAII `binding_handle` and never reaches for a consumer owning handle
(order `bind_` teardown before the subtree owner).

### (E) stringly bindings → typed `ui::bind` — *additive default; friend hook touches-internals (opt-in)*

```cpp
// BEFORE — a misspelled name is a silent dead binding
greeting_.set_binding("text", "message", maui::core::binding_mode::one_way);
// AFTER — one-way VM→control (the write goes through set_text)
auto h1 = ui::bind(greeting_, &ui::label::set_text).to(vm_->message);
// AFTER — two-way entry⇄VM
auto h2 = ui::bind(input_, &ui::entry::set_text, &ui::entry::text, &input_.text_changed)
             .to_two_way(vm_->message);
```

`ui::bind` synthesizes a `bindable_ref<T>{ get: via getter, set: **via set_***, changed: control event }`
and reuses the existing ladder (`core/binding.hpp:81-151`). The source is typed → a wrong name is a
compile error. The `<TGet,TSet>` adapter handles asymmetric `entry::text()→string_view`/`set_text(string)`
with **no** control-header change. Keep stringly `set_binding` for dynamic/markup.

**Opt-in friend hook (*touches-internals*):** per *side-effect-free* property, a control author may
`friend struct maui::ui::detail::bind_access;` and expose the backing `property<T>&` for direct
`bind(property<T>&, property<T>&)`. Author-vetted (`from_binding` writes bypass `set_*`); added
control-by-control.

### (F) `connect_scoped` + `(old,new)` twin → `ui::on` + node-parked `.on_*` — *additive (+ `.clicked()` accessor: touches-internals)*

```cpp
// BEFORE
maui::core::scoped_connection text_token_;
text_token_ = maui::core::connect_scoped(input_.text_changed,
    [this](const std::string& /*old*/, const std::string& nv){ vm_->message.set(nv); });
// AFTER — node-parked, token lives in the impl, [this] capture (no cycle)
ui::button("Increment").on_click([this]{ ++count_; });
ui::on(input_.text_changed, [this](const std::string& nv){ vm_->message.set(nv); });   // arity-adapts
```

Keep `event<Args...>` + `connection_token` + RAII `scoped_connection`. `.on_*(fn)` parks the
`scoped_connection` **inside the impl** (the framework's own subscription is never a cycle); the only
remaining cycle source is a user owning-capture, which move-only blocks at compile time. `ui::on` is
`requires`-dispatched: accepts `(old,new)`, `(new)`, or `()`.

**Two fidelity rules:** (1) **do not drop the `(old,new)` event *type*** — `value_changed` is
`event<double,double>` (`slider.hpp`/`stepper.hpp`); a `.on_value_changed([](double v){...})` chainer must
be documented as narrowing, and a two-arg form offered. (2) `b.clicked.connect(...)` →
`b.clicked().connect(...)` via a small `event<>&` accessor on the handle (preserves the connect idiom +
~180 tests).

### (G) `make_shared` resources → value factories — *additive*

```cpp
// BEFORE
btn.set_background(std::make_shared<maui::graphics::solid_paint>(maui::graphics::colors::aqua));
view.set_clip(std::make_shared<maui::graphics::rounded_rectangle>(8.0));
// AFTER
btn.background(ui::solid(ui::colors::aqua));   // chainer, or set_background(ui::solid(...)) on the impl
view.set_clip(ui::rounded_rect(8));
```

Factories wrap `make_shared` internally; `shared_ptr` stays the stored type for shared/polymorphic/
identity-bearing resources (`i_drawable`, `i_image_source`, `formatted_string`). Value-wrap only
immutable copy-cheap leaves.

### (H) `i_*` leaks → `ui::app` base — *additive*

```cpp
// BEFORE
class my_app : public maui::controls::application {
    maui::core::i_window* create_window() override { return &window_; }   // i_window leaks
};
// AFTER
class my_app : public maui::ui::app {
    maui::controls::window& main_window() override { return window_; }     // no i_* named
};
```

```cpp
namespace maui::ui {
class app : public maui::controls::application {
public:
    maui::core::i_window* create_window() final { return &main_window(); }
    virtual maui::controls::window& main_window() = 0;
};
}
```

The app typically holds the tree as a `view_ref<content_page> root_` member. `add(i_view&)` never appears
in consumer code. **Both app-authoring shapes kept indefinitely** — `ui::app` is additive; raw
`create_window()` overrides keep working. Zero `i_*` in the happy path.

### (I) deep namespaces → `maui/ui.hpp` umbrella (short + full) — *additive*

```cpp
#include "maui/ui.hpp"
namespace ui = maui::ui;
ui::vstack(...)                          // short alias
ui::vertical_stack_layout also_ok_;      // full name also exported
auto p = ui::solid(ui::colors::aqua);
```

Curated `using`-aliases (zero runtime cost). Both terse and full names per the maintainer ruling; plus
`ui::view_ref`, `ui::weak_ref`.

### (J) handler/registration seam → stays hidden — *additive (by omission)*

`MAUI_REGISTER_HANDLER` / `add_maui_controls_handlers` / `handler()` / type tags are **not** aliased into
`maui::ui`. `ui::app` boots via the unchanged `use_shared_maui_app` / `use_maui_app<T>()`. The seam is
untouched.

---

## 4. The design decisions

| # | Decision | Ruling |
|---|---|---|
| 1 | **Construction** | Owning **move-only** builder (`ui::vstack(...)`) primary; rvalue-qualified node chaining; imperative `set_*`+`add` remain underneath. |
| 2 | **Properties** | `core::observable<T>` for consumer **VMs only**; controls keep private `property<T>`. |
| 3 | **Bindings** | Typed `ui::bind(...).to/.to_two_way` via `bindable_ref<T>` (route through `set_*`); **plus** opt-in per-control friend hook for side-effect-free properties; stringly `set_binding` retained. |
| 4 | **Events** | Keep `event<Args...>`+tokens+`scoped_connection`; add arity-flexible `ui::on`, node-parked `.on_*`, `.clicked()` accessor; **keep** the `(old,new)` twin at the control level. |
| 5 | **Resources** | Value factories at call sites; `shared_ptr` stays the stored type. |
| 6 | **Facade** | `namespace maui::ui` + `maui/ui.hpp`, exporting **both** short aliases and full names + the handle types. |
| 7 | **`i_*` exposure** | Zero in the happy path (`ui::app` absorbs the only forced one); `i_drawable`/`i_canvas` stay public on purpose. |
| 8 | **Migration** | Additive layer; no in-place signature changes (see §5). |
| 9 | **Handle copy-semantics** | **Hybrid: `view_ref<T>` move-only · `weak_ref<T>` copyable observer · explicit `.share()`.** Non-atomic intrusive refcount + debug thread assert. |

---

## 5. Migration

**Additive `maui::ui` layer. Reject in-place** (it would force simultaneous edits across 178 gallery pages
+ 7 examples + ~180 tests for zero behavioral gain, risking the §8 model and the iOS parity pipeline).

| Phase | Ships | Touches internals? | Effort |
|---|---|---|---|
| **P1 — Facade + resources** | `maui/ui.hpp` aliases (short **and** full); `ui::app` base; value factories. | No | **S** |
| **P2 — Reactive VM + events** | `core::observable<T>`; arity-flexible `ui::on`. Convert `data_binding` + `counter` as showcases. | No (`.clicked()` accessor: small) | **M** |
| **P3 — Typed bindings** | `bindable_ref<T>`, `ui::bind(...).to/.to_two_way`; `tests/ui/` mirroring the `bind()` precedence + re-entrancy + entry-truncation-under-two-way suite. Opt-in friend hook per-control. | Default no; friend hook per property | **M–L** |
| **P4 — Owning handle + builder** | `ui::view_ref<T>` (move-only) + `ui::weak_ref<T>` + `.share()`; `ui::vstack/hstack/grid(...)` owning-adopt; rvalue chainers + node-parked `.on_*`; non-atomic refcount + debug thread assert. Convert remaining examples. | No | **L** |

**Nothing changes existing signatures by default.** The 7 examples become living showcases; the 178
gallery pages migrate **opportunistically** — a half-migrated gallery renders identically (same `add`), so
parity captures are unaffected. Both app-authoring shapes coexist. Each phase is headless-first;
`tools/dev.sh` inner loop, `tools/gate.sh --fast` before any push.

**De-risk P4 first** (smallest headless proof, one throwaway TU, zero changes to `controls/`/`core/`/seam):
rebuild `counter` on `view_ref`+`weak_ref`+`vstack`, asserting (1) builder mount resolves `button`'s
handler by `type_tag`; (2) `[b]` self-capture is a **compile error** and `[w = b.weak()]` does not leak;
(3) two-way `bind` past `max_length` still truncates; (4) curated chainers + `operator->` escape compile
clean at clang-tidy-0; (5) the `content_page` `shared_ptr<element>` hand-off doesn't double-own.

---

## 6. XAML — the future layer (how it fits)

**The verbosity is already solved without XAML.** The "every widget is a member, in reverse-destruction
order" pain that motivates this redesign is removed by the `maui::ui` builder (§3-C) in **C++23, today**.
You do **not** need XAML to author non-verbosely. XAML's distinct value is different and additive:

- **1-to-1 reuse of existing MAUI `.xaml` markup** (no re-authoring the corpus), and
- **differential conformance testing** — the same `.xaml` renders in MAUI and in the port, and the trees
  must match. This aligns with the prime directive ("derive from MAUI, never invent") and the existing
  iOS parity pipeline.

**Realization — build-time codegen, not in-TU reflection.** XAML is **layer 6**. A **host-side codegen
tool** parses `.xaml` + a type-converter table and **emits snake_case C++ that calls the `maui::ui`
builder / `observable<T>` / `ui::bind`** — exactly what `PROFILE.md` §6 already prescribes ("generate C++
from XAML at build time… never assume runtime type discovery"). This gives the compile-time guarantees
("missing property = compile error," zero runtime markup parsing) with **no runtime reflection and no
C++26 in the application**, so it compiles on the headless **and** the Objective-C++ `apple`/`ios`
backends. The codegen tool itself may use anything internally (it never ships in the binary).

**Explicitly set aside: the in-TU `#embed` + `constexpr` + P2996-reflection design** (a separate
community proposal). It is elegant but incompatible with this port: it (a) reverses the no-reflection
doctrine (`PROFILE.md` §6), and (b) requires C++26 + a GCC-16-only toolchain — which cannot compile the
Objective-C++ `.mm` `apple`/`ios` backends (AppleClang is required for UIKit/AppKit interop), i.e. it
would abandon the primary native targets. "Compile-time XAML" is achievable via codegen **without** that
toolchain bet; the reflection-in-every-TU flavor is the only part that breaks the backend.

**Why the builder comes first.** XAML codegen's *emit target* **is** the `maui::ui` builder, and its
binding/VM/ownership model independently converges with this doc's (compiled member-pointer bindings ≈
typed `ui::bind`; observable property members ≈ `observable<T>`; `unique_ptr<Page>` + activate-borrows +
teardown-order + no-`std::any` ≈ §8 + the move-only handle). So building the builder first is the
prerequisite either way; doing XAML first would mean emitting into the raw `controls::` API and redoing
it. When we reach layer 6, adopting a computed read-only `ro_property` (a lambda-backed `observable<T>`
extension making `TwoWay`-to-computed ill-formed) is worth picking up from that design.

**Status:** out of scope for the current implementation (P1–P4). Captured here so the builder is designed
as a clean codegen target. A separate layer-6 brief will detail the dialect scope (markup extensions,
attached properties, type converters, `xmlns` resolution) when sequenced.

---

## 7. Open risks & follow-ups

- **The `.share()` discipline tripwire (flip condition).** The hybrid's edge over *pure* move-only is the
  rule "default `weak()`; `.share()` only for an audited cluster, justified in review." If authors
  cargo-cult `.share()` to silence the copy-deleted error, the hybrid degrades to copyable-with-extra-steps.
  Mitigate with a clang-tidy check that flags a `.share()` result captured into a closure owned by the
  same control. **If the discipline proves unattainable, drop `.share()` and ship pure move-only.**
- **Mint-after-move dead handle.** `.weak()` must be taken *before* `std::move`-ing a named handle into the
  tree. Document + lint.
- **`value_changed` arity in chainers.** Preserve both `(old,new)` args (offer a 2-arg overload) or
  document the narrowing — do not smuggle an arity change into "ergonomic" chainers.
- **Non-atomic refcount cross-thread risk.** §8 licenses it; an accidental cross-thread op is UB the debug
  thread-id assert only *sometimes* catches. Deliberate risk acceptance.
- **Curated chainer cut line.** Which ~12 properties get fluent chainers vs the `operator->` long tail?
  Needs a usage census over the 178 pages.
- **Templated `content_view`/`content_page` `shared_ptr<element>` seam.** Verify the
  `view_ref → shared_ptr<element>` hand-off at `content_presenter` doesn't double-own (P4 prototype).
- **Reactive path barely exists today** (~96% imperative). The redesign *wants* role D to grow (it
  dissolves the hold+mutate question), but cannot lean on it to excuse anything today.
- **Residual window↔content-root teardown ordering (ASan-found).** The builder removes the *children*
  member-ordering footgun, but one cross-edge survives: a `window` non-owningly references its content
  (`set_content` + the window's menu/toolbar trackers subscribe into the page tree). So the content root
  must **outlive** the window — i.e. **declare the `view_ref<content_page> root_` member BEFORE the
  `window` member** (the window then destructs first, while the page is alive). Getting this wrong is a
  heap-use-after-free at teardown (caught by ASan, invisible to non-sanitized runs). Mitigation now: the
  rule + a comment in every example. Follow-up: a `ui::app` host helper that owns the window and the
  content root together in the correct order, so the consumer can't get it wrong.

---

*Answers [`PUBLIC_API_REDESIGN.md`](PUBLIC_API_REDESIGN.md). Authoritative behavior remains the C# `src/`
+ tests (`port/CLAUDE.md`); the C++23 profile is `PROFILE.md` (§6 no-reflection, §7 property system, §8
ownership, §9 snake_case, §11 locked decisions).*
