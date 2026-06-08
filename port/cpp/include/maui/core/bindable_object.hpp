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
#include <string_view>
#include <unordered_map>

#include "maui/core/event.hpp"
#include "maui/core/move_only_function.hpp"
#include "maui/core/setter_specificity.hpp"

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

    protected:
        virtual void on_property_changed(std::string_view name);
        virtual void on_property_changing(std::string_view name);

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

        std::unordered_map<std::string_view, property_handle> properties_;

        template <class U> friend class property;
    };
} // namespace maui::core
