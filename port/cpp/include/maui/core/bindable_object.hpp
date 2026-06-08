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
#include <unordered_map>
#include <utility>

#include "maui/core/event.hpp"
#include "maui/core/move_only_function.hpp"
#include "maui/core/setter_specificity.hpp"
#include "maui/core/type_tag.hpp"

namespace maui::core
{
    class bindable_object
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
        };

        template <class X> void set_binding_context(std::shared_ptr<X> value)
        {
            set_binding_context_raw(
                binding_context_box{std::static_pointer_cast<void>(std::move(value)), type_tag::of<X>()},
                /*is_explicit=*/true);
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
    };
} // namespace maui::core
