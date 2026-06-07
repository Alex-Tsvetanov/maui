#pragma once
// maui::core::bindable_object  <=  Microsoft.Maui.Controls.BindableObject
//
// Base of every element: a type-erased property value store with value precedence. Ported from
// src/Controls/src/Core/BindableObject.cs (the value path: SetValue/GetValue/ClearValue/
// SetValueActual). Each property gets a setter_specificity_list of boxed values; the highest
// specificity wins. Mirrors the C# semantics: handler values are special-cased away when any other
// value is set; a value set below the current specificity is kept (silently) for later un-apply;
// change notification fires only when the effective value actually changes, in the order
// property_changing-delegate -> property_changing event -> (store) -> property_changed event ->
// property_changed-delegate.
//
// M1 scope: value precedence + change notification + the bindable_property callbacks + re-entrancy
// (delayed setters). Binding/BindingContext/styles/dynamic-resources are deferred to M5.

#include <any>
#include <queue>
#include <string_view>
#include <unordered_map>
#include <utility>

#include "maui/core/event.hpp"
#include "maui/core/setter_specificity.hpp"
#include "maui/core/setter_specificity_list.hpp"

namespace maui::core
{
    class bindable_property;

    class bindable_object
    {
    public:
        bindable_object() = default;
        bindable_object(const bindable_object &) = delete;
        bindable_object(bindable_object &&) = delete;
        bindable_object &operator=(const bindable_object &) = delete;
        bindable_object &operator=(bindable_object &&) = delete;
        virtual ~bindable_object() = default;

        // ---- boxed value access (the C# object-based API) ----
        [[nodiscard]] std::any get_value(const bindable_property &property) const;
        void set_value(const bindable_property &property, std::any value); // at ManualValueSetter
        void set_value(const bindable_property &property, std::any value, setter_specificity specificity);
        void clear_value(const bindable_property &property); // at ManualValueSetter
        void clear_value(const bindable_property &property, setter_specificity specificity);

        // ---- typed convenience (what property<T> builds on) ----
        template <class T> [[nodiscard]] T get_value(const bindable_property &property) const
        {
            return std::any_cast<T>(get_value(property));
        }
        template <class T> void set_value(const bindable_property &property, T value)
        {
            set_value(property, std::any(std::move(value)));
        }

        // INotifyPropertyChanged / INotifyPropertyChanging, keyed by property name.
        event<std::string_view> property_changed;
        event<std::string_view> property_changing;

    protected:
        virtual void on_property_changed(std::string_view name);
        virtual void on_property_changing(std::string_view name);

    private:
        struct set_request
        {
            const bindable_property *property;
            std::any value;
            setter_specificity specificity;
        };
        struct context
        {
            const bindable_property *property = nullptr;
            setter_specificity_list<std::any> values;
            bool is_being_set = false;
            std::queue<set_request> delayed;
        };

        // mutable: a default-value creator lazily materializes its context from const get_value.
        context &get_or_create_context(const bindable_property &property) const;
        [[nodiscard]] context *get_context(const bindable_property &property) const;
        void set_value_core(const bindable_property &property, std::any value, setter_specificity specificity);
        void set_value_actual(const bindable_property &property, context &ctx, std::any value,
                              setter_specificity specificity, bool silent);
        void clear_value_core(const bindable_property &property, setter_specificity specificity);

        mutable std::unordered_map<const bindable_property *, context> properties_;
    };
} // namespace maui::core
