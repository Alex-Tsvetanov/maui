#pragma once
// maui::controls::setter  <=  Microsoft.Maui.Controls.Setter
//
// One property assignment inside a style / trigger / visual state: "set property P to value V". Created
// TYPED via setter::of(descriptor, value) — which boxes the typed value in a std::any at this boundary —
// and then applied to any bindable_object at a given specificity by routing through
// bindable_object::apply_setter / clear_setter (the property<T> self-registration unboxes it back to the
// typed slot, so STORAGE stays typed). Ported from Setter.cs.
//
// A style holds a heterogeneous vector<setter> (different value types), so setter erases T after capture —
// it is a plain (copyable) value type, not a template.

#include <any>
#include <string_view>
#include <utility>

#include "maui/core/bindable_object.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/setter_specificity.hpp"

namespace maui::controls
{
    class setter
    {
    public:
        // Create a setter assigning `descriptor` = `value`. The value type T is captured (boxed) here so the
        // matching property<T> handle can unbox it on apply.
        template <class T> [[nodiscard]] static setter of(const maui::core::bindable_property<T>& descriptor, T value)
        {
            return setter{descriptor.name(), std::any{std::move(value)}};
        }

        void apply(maui::core::bindable_object& target, maui::core::setter_specificity specificity) const
        {
            target.apply_setter(property_name_, value_, specificity);
        }
        void unapply(maui::core::bindable_object& target, maui::core::setter_specificity specificity) const
        {
            target.clear_setter(property_name_, specificity);
        }

        [[nodiscard]] std::string_view property_name() const
        {
            return property_name_;
        }

    private:
        setter(std::string_view property_name, std::any value) : property_name_(property_name), value_(std::move(value))
        {
        }

        std::string_view property_name_; // the descriptor name (a string literal — stable/borrowed)
        std::any value_;                 // the boxed value (storage stays typed in property<T>)
    };
} // namespace maui::controls
