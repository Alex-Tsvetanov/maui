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

#include <string_view>

#include "maui/core/event.hpp"

namespace maui::core
{
    class bindable_object
    {
    public:
        bindable_object() = default;
        bindable_object(const bindable_object &) = delete;
        bindable_object(bindable_object &&) = delete;
        bindable_object &operator=(const bindable_object &) = delete;
        bindable_object &operator=(bindable_object &&) = delete;
        virtual ~bindable_object() = default;

        // INotifyPropertyChanged / INotifyPropertyChanging, keyed by property name.
        event<std::string_view> property_changed;
        event<std::string_view> property_changing;

    protected:
        virtual void on_property_changed(std::string_view name);
        virtual void on_property_changing(std::string_view name);

        template <class U> friend class property;
    };
} // namespace maui::core
