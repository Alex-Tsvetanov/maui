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
//
// Setter.TargetName retargets the assignment to a NAMED element rather than the trigger/style's own
// target. C#'s XAML resolves the name via the namescope; the reflection-free, code-first port has no
// namescope, so the retarget is given DIRECTLY as a bindable_object pointer (setter::of_for(target, …)).
// When a retarget is set, apply/unapply ignore the passed target and route to the retarget instead — the
// faithful behavioral equivalent (a named element is just a pointer once resolved). The retarget is
// NON-owning (the element tree owns it) and must outlive any apply/unapply.

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
            return setter{descriptor.name(), std::any{std::move(value)}, nullptr};
        }

        // Create a retargeting setter (Setter.TargetName): apply/unapply route to `target` regardless of the
        // object passed to apply()/unapply(). NON-owning — `target` must outlive the setter's use.
        template <class T>
        [[nodiscard]] static setter of_for(maui::core::bindable_object& target,
                                           const maui::core::bindable_property<T>& descriptor, T value)
        {
            return setter{descriptor.name(), std::any{std::move(value)}, &target};
        }

        void apply(maui::core::bindable_object& target, maui::core::setter_specificity specificity) const
        {
            effective_target(target).apply_setter(property_name_, value_, specificity);
        }
        void unapply(maui::core::bindable_object& target, maui::core::setter_specificity specificity) const
        {
            effective_target(target).clear_setter(property_name_, specificity);
        }

        [[nodiscard]] std::string_view property_name() const
        {
            return property_name_;
        }

    private:
        setter(std::string_view property_name, std::any value, maui::core::bindable_object* retarget)
            : property_name_(property_name), value_(std::move(value)), retarget_(retarget)
        {
        }

        // The object the setter actually writes to: the retarget (TargetName) when set, else the passed one.
        [[nodiscard]] maui::core::bindable_object& effective_target(maui::core::bindable_object& target) const
        {
            return retarget_ != nullptr ? *retarget_ : target;
        }

        std::string_view property_name_;                  // the descriptor name (a string literal — stable/borrowed)
        std::any value_;                                  // the boxed value (storage stays typed in property<T>)
        maui::core::bindable_object* retarget_ = nullptr; // Setter.TargetName retarget (NON-owning); null = self
    };
} // namespace maui::controls
