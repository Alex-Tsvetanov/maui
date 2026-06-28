#pragma once
// maui::core::observable<T> — a self-contained bindable property for view-models (PUBLIC_API_DESIGN.md §3-A).
//
// Replaces the static-descriptor-function + `property<T> member{*this, descriptor()}` boilerplate with ONE
// member: it bundles the name, the bindable_property<T> descriptor, and the per-instance property<T>. The
// inner property self-registers its name on the owner exactly as before, so stringly `set_binding(...)` and
// the typed `bind(...)` both still resolve it. VIEW-MODEL ONLY — controls keep their private property<T>
// because their setters carry side effects this would bypass.
//
// Non-movable: the descriptor borrows a string_view into name_, and property<T> stores a back-pointer to its
// owner; both require a stable address. Declare it as a data member of a bindable_object.

#include <string>
#include <string_view>
#include <utility>

#include "maui/core/bindable_object.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/event.hpp"
#include "maui/core/property.hpp"

namespace maui::core
{
    template <class T> class observable
    {
    public:
        using options = typename bindable_property<T>::options;

        observable(bindable_object& owner, std::string name, T default_value = T{}, options opts = {})
            : name_(std::move(name)), descriptor_(name_, std::move(default_value), std::move(opts)),
              property_(owner, descriptor_)
        {
        }
        observable(const observable&) = delete;
        observable(observable&&) = delete;
        observable& operator=(const observable&) = delete;
        observable& operator=(observable&&) = delete;
        ~observable() = default;

        [[nodiscard]] const T& get() const
        {
            return property_.get();
        }
        void set(T value)
        {
            property_.set(std::move(value));
        }

        [[nodiscard]] std::string_view name() const
        {
            return name_;
        }
        // Fired (old, new) when the value changes — subscribe for one-way data flow without a binding object.
        [[nodiscard]] event<T, T>& changed()
        {
            return property_.changed;
        }

        // The underlying typed slot — the bridge the typed binding layer binds against (bind(target, source)).
        [[nodiscard]] property<T>& as_property()
        {
            return property_;
        }
        [[nodiscard]] const property<T>& as_property() const
        {
            return property_;
        }

    private:
        std::string name_;                // owns the name; declared FIRST (string_view below borrows it)
        bindable_property<T> descriptor_; // declared SECOND
        property<T> property_;            // self-registers name_ on owner; declared THIRD (unregisters first)
    };
} // namespace maui::core
