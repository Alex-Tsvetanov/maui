#pragma once
// maui::core::bindable_object  <=  Microsoft.Maui.Controls.BindableObject
//
// Base of every element. In the C# original this also owns a per-object dictionary of boxed
// (System.Object) property values; here that central, type-erased store is gone — each property's
// value lives, strongly typed, in its own property<T> member (property.hpp / PROFILE.md §7). So this
// base only provides the change-notification surface (INotifyPropertyChanged / INotifyPropertyChanging,
// keyed by name) that the property<T> members drive. The value precedence itself lives in property<T>.
//
// property<T> is a friend so it can route notifications through the overridable on_property_changed
// hook (which subclasses like visual_element will extend at M5) rather than raising the events
// directly.

#include <any>
#include <memory>
#include <optional>
#include <string_view>
#include <type_traits> // --- runtime bindings (W1-02): is_base_of_v in set_binding_context ---
#include <unordered_map>
#include <utility>

#include "maui/core/binding_mode.hpp" // --- runtime bindings (W1-02): property_handle metadata ---
#include "maui/core/event.hpp"
#include "maui/core/move_only_function.hpp"
#include "maui/core/setter_specificity.hpp"
#include "maui/core/type_tag.hpp"

namespace maui::core
{
    // Derives enable_shared_from_this so a property mutation can PIN its owner for the duration of
    // the change notification (property.hpp). C# is safe here for free: a managed `this` on the stack
    // is a GC root, so no PropertyChanged handler can make the CLR reclaim the object while any frame
    // of OnPropertyChanged is still running (BindableObject.cs:637-644 touches `this` after the raise,
    // and Element.cs:709-724 does the same in its override). The port has no such root — dropping the
    // last shared_ptr inside a handler runs operator delete immediately — so it re-creates the root
    // explicitly. NOTE: because a class may have only ONE unambiguous enable_shared_from_this base
    // (two make libc++ populate NEITHER, silently), no bindable_object subclass may add its own.
    class bindable_object : public std::enable_shared_from_this<bindable_object>
    {
    public:
        bindable_object() = default;
        bindable_object(const bindable_object&) = delete;
        bindable_object(bindable_object&&) = delete;
        bindable_object& operator=(const bindable_object&) = delete;
        bindable_object& operator=(bindable_object&&) = delete;
        virtual ~bindable_object() = default;

        // INotifyPropertyChanged / INotifyPropertyChanging, keyed by property name.
        event<std::string_view> property_changed;
        event<std::string_view> property_changing;

        // Style/setter seam (M5b): set/clear a property by its bindable_property descriptor name at a given
        // specificity. The typed property<T> member self-registers a handle (below) on construction, so a
        // style/trigger/VSM setter can reach a typed property without a central type-erased value store.
        // The value crossing the boundary is boxed in std::any (storage stays typed in property<T>); both
        // are a no-op when `name` is not a registered property.
        void apply_setter(std::string_view name, const std::any& value, setter_specificity specificity);
        void clear_setter(std::string_view name, setter_specificity specificity);

        // ---- BindingContext (typed, inherited) — BindableObject.BindingContext + SetInheritedBindingContext ----
        // The data context an element binds against. Held type-erased (a shared_ptr<void> + a type_tag, so
        // the typed getter is checked — no RTTI, no unchecked cast). An explicit set marks this object so an
        // inherited context can no longer override it; otherwise a parent's context flows DOWN the element
        // tree (controls::element propagates it to logical children from on_binding_context_changed). The
        // value is shared (shared_ptr) because one view-model is typically the context for a whole subtree.
        // A type-erased binding context: a shared_ptr<void> + the type_tag of what it points at (nullopt
        // when there is no context). Passed from a parent to its children to inherit the context.
        struct binding_context_box
        {
            std::shared_ptr<void> value;
            std::optional<type_tag> type;
            // --- runtime bindings (W1-02): the context viewed as a binding SOURCE ---
            // `boxed` carries the shared_ptr<X> as a std::any (the engine's self-path leaf value; an
            // empty any = no context), and `object` the context as a walkable bindable_object node
            // (null when X doesn't derive bindable_object). Both are captured by set_binding_context
            // (the only place the static type X is known) and ride along through inheritance.
            std::any boxed;
            std::shared_ptr<bindable_object> object;
            // --- end runtime bindings (W1-02) ---
        };

        template <class X> void set_binding_context(std::shared_ptr<X> value)
        {
            binding_context_box box;
            box.value = std::static_pointer_cast<void>(value);
            box.type = type_tag::of<X>();
            // --- runtime bindings (W1-02): capture the engine-facing source forms ---
            if (value)
            {
                box.boxed = value;
            }
            if constexpr (std::is_base_of_v<bindable_object, X>)
            {
                box.object = value;
            }
            // --- end runtime bindings (W1-02) ---
            set_binding_context_raw(std::move(box), /*is_explicit=*/true);
        }
        // The context as X, or nullptr if unset or stored as a different type (the type_tag guards the cast).
        template <class X> [[nodiscard]] std::shared_ptr<X> binding_context() const
        {
            if (!binding_context_.value || binding_context_.type != type_tag::of<X>())
            {
                return nullptr;
            }
            return std::static_pointer_cast<X>(binding_context_.value);
        }
        [[nodiscard]] bool has_binding_context() const
        {
            return static_cast<bool>(binding_context_.value);
        }
        [[nodiscard]] const binding_context_box& raw_binding_context() const
        {
            return binding_context_;
        }
        // Set an INHERITED context (from a parent). A no-op if this object has an explicitly-set context —
        // mirrors SetInheritedBindingContext's "don't override a locally-set value" guard.
        void set_inherited_binding_context(const binding_context_box& context)
        {
            set_binding_context_raw(context, /*is_explicit=*/false);
        }

        // Fired when the effective binding context changes (BindableObject.BindingContextChanged).
        event<> binding_context_changed;

    protected:
        virtual void on_property_changed(std::string_view name);
        virtual void on_property_changing(std::string_view name);
        // BindableObject.OnBindingContextChanged — the base raises binding_context_changed; controls::element
        // overrides it to ALSO propagate the new context down to its logical children.
        virtual void on_binding_context_changed();

    private:
        // A typed-erased view onto one property<T>: apply un-boxes (std::any_cast<T>) + set()s at the given
        // specificity; clear removes that specificity. Created by property<T>'s constructor.
        struct property_handle
        {
            move_only_function<void(const std::any&, setter_specificity)> apply;
            move_only_function<void(setter_specificity)> clear;
            // --- runtime bindings (W1-02): the name->getter/converting-setter channel one typed slot
            // exposes to the string-path binding engine (each populated by property<T>'s constructor).
            move_only_function<std::any()> get;                                      // current value, boxed
            move_only_function<std::shared_ptr<bindable_object>()> get_object;       // value as a walkable node
            move_only_function<bool(const std::any&, setter_specificity)> try_apply; // converting set
            move_only_function<std::any()> get_default;                              // descriptor default, boxed
            move_only_function<bool()> is_set;            // property<T>::is_set() — explicitly set at any specificity?
            move_only_function<void()> demote_to_binding; // silent SetBinding demote (BindableObject.SetBinding)
            type_tag type = type_tag::of<void>();         // type_tag::of<T> (the converter targetType analog)
            binding_mode default_binding_mode = binding_mode::one_way;
            bool is_read_only = false;
            // --- end runtime bindings (W1-02) ---
        };
        // property<T> (a friend) registers/unregisters its handle on construction/destruction.
        void register_property(std::string_view name, property_handle handle);
        void unregister_property(std::string_view name);

        // The shared apply path for an explicit (is_explicit) or inherited binding-context set: an inherited
        // set is ignored once a value was set explicitly; an unchanged value (same shared_ptr) short-circuits;
        // otherwise the value is stored and on_binding_context_changed() fires (driving propagation).
        void set_binding_context_raw(binding_context_box context, bool is_explicit);

        std::unordered_map<std::string_view, property_handle> properties_;
        binding_context_box binding_context_;
        bool binding_context_explicit_ = false;

        template <class U> friend class property;

        // --- runtime bindings (W1-02) -------------------------------------------------------------
        // The name->GETTER channel (the mirror of apply_setter) plus the converting setter the
        // string-path binding engine drives. The boxed representation is std::any (boxed_value.hpp):
        // an EMPTY any is a null value; nullopt from the optional-returning getters means "no property
        // registered under `name`" (C#'s reflection-miss -> "property not found" binding failure).
        // "binding_context" is a recognized name (C#'s BindableObject.BindingContextProperty): it
        // reads the context box's source forms, so paths like "binding_context.title" resolve.
    public:
        [[nodiscard]] bool has_property(std::string_view name) const;
        // BindableObject.IsSet(BindableProperty): whether the named property has been explicitly set at
        // any specificity (vs. still reading the shared descriptor default). The faithful stand-in for
        // C#'s `Color? == null` null-check where the port models a nullable value type (e.g. Color) as a
        // non-nullable value whose DEFAULT is a legitimate value — so a handler cannot tell "unset" from
        // "explicitly set to the default-constructed value" (default-constructed color IS opaque black)
        // by comparing the value. false for an unregistered property (and never materializes a default).
        [[nodiscard]] bool is_property_set(std::string_view name) const;
        // The property's current value, boxed (materializes a default-value creator, like C# GetValue).
        [[nodiscard]] std::optional<std::any> try_get_value(std::string_view name) const;
        // The value as a walkable bindable_object node (property<shared_ptr<U>> where U derives
        // bindable_object), or null (value null, not an object, or no such property).
        [[nodiscard]] std::shared_ptr<bindable_object> try_get_object(std::string_view name) const;
        // Converting name->setter (BindingExpressionHelper.TryConvert + SetValueCore): unboxes through
        // the boxed_value lattice, then set()s at `specificity`. false = unknown property or value not
        // convertible (the binding-failure path — apply_setter stays the exact-typed style channel).
        bool try_set_value(std::string_view name, const std::any& value, setter_specificity specificity);
        // Descriptor metadata by name (GetRealizedMode / read-only checks / the default-value fallback
        // a failed resolution applies). type is type_tag::of<T> — the converter targetType analog.
        [[nodiscard]] std::optional<binding_mode> property_default_binding_mode(std::string_view name) const;
        [[nodiscard]] std::optional<bool> property_is_read_only(std::string_view name) const;
        [[nodiscard]] std::optional<std::any> property_default_value(std::string_view name) const;
        [[nodiscard]] std::optional<type_tag> property_type(std::string_view name) const;
        // BindableObject.SetBinding's silent demote: a value sitting above FromBinding is moved down to
        // FromBinding (no change notification — the value itself is unchanged) so an incoming binding's
        // first apply replaces it instead of being outranked.
        void demote_value_to_binding(std::string_view name);
        // Set the binding context from a pre-built box (the engine's typed-erased entry — a context
        // BINDING and multi_binding's proxy use it; equivalent to an explicit set_binding_context).
        void set_binding_context_box(binding_context_box box)
        {
            set_binding_context_raw(std::move(box), /*is_explicit=*/true);
        }
        // Weak liveness token (PROFILE §8): lets a binding expression check that a non-shared_ptr-owned
        // source node (e.g. an ancestor element) is still alive before touching it or its events.
        [[nodiscard]] std::weak_ptr<void> weak_token() const
        {
            return liveness_;
        }

    private:
        std::shared_ptr<void> liveness_ = std::make_shared<char>(0);
        // --- end runtime bindings (W1-02) ---------------------------------------------------------
    };
} // namespace maui::core
