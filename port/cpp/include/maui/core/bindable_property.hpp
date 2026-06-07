#pragma once
// maui::core::bindable_property<T>  <=  Microsoft.Maui.Controls.BindableProperty
//
// The (shared, static) descriptor for a bindable property: name, default value, and the optional
// typed callbacks. Ported from src/Controls/src/Core/BindableProperty.cs — but, unlike the C#
// original (which is type-erased over System.Object), this is a *typed* template: the value type T
// is known at compile time, so default value, callbacks and equality are all typed (no std::any, no
// boxing, no RTTI). The per-instance value storage lives in property<T> (property.hpp); this is just
// the immutable description shared by every instance of the owning type.
//
// Identity-keyed and non-copyable — declare one as a `static const bindable_property<T>` (the C#
// `XxxProperty` field) and hand it to each property<T> member.
//
// M1 scope: name/default/read-only + the five callbacks. Binding-specific bits (BindingMode,
// TryConvert, attached/read-only keys, dependencies) are deferred to M5.

#include <functional>
#include <string_view>
#include <utility>

namespace maui::core
{
    class bindable_object;

    template <class T> class bindable_property
    {
    public:
        // The C# Create() optional parameters, strongly typed (no System.Object).
        struct options
        {
            std::function<void(bindable_object&, const T&, const T&)> property_changed;
            std::function<void(bindable_object&, const T&, const T&)> property_changing;
            std::function<T(bindable_object&, const T&)> coerce_value;
            std::function<bool(bindable_object&, const T&)> validate_value;
            std::function<T(const bindable_object&)> default_value_creator;
            bool is_read_only = false;
        };

        // `name` is borrowed, not owned — it must outlive the property. In practice it is always a
        // string literal (the `static const bindable_property` pattern, like C# nameof), so this holds
        // trivially. Do not construct a property from a temporary/dynamically-built string.
        explicit bindable_property(std::string_view name, T default_value = T{}, options opts = {})
            : name_(name), default_value_(std::move(default_value)), options_(std::move(opts))
        {
        }

        bindable_property(const bindable_property&) = delete;
        bindable_property(bindable_property&&) = delete;
        bindable_property& operator=(const bindable_property&) = delete;
        bindable_property& operator=(bindable_property&&) = delete;
        ~bindable_property() = default;

        [[nodiscard]] std::string_view name() const
        {
            return name_;
        }
        [[nodiscard]] const T& default_value() const
        {
            return default_value_;
        }
        [[nodiscard]] bool is_read_only() const
        {
            return options_.is_read_only;
        }
        [[nodiscard]] bool has_default_value_creator() const
        {
            return static_cast<bool>(options_.default_value_creator);
        }
        [[nodiscard]] T create_default(const bindable_object& owner) const
        {
            return options_.default_value_creator(owner);
        }
        [[nodiscard]] const options& callbacks() const
        {
            return options_;
        }

    private:
        std::string_view name_; // borrowed; the referent (a string literal) outlives the property
        T default_value_;
        options options_;
    };
} // namespace maui::core
