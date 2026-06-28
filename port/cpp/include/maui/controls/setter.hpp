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
// target. There are two ways to supply that target:
//   - of_for(target, …): the retarget is given DIRECTLY as an already-resolved bindable_object pointer —
//     the reflection-free shortcut for callers that already hold the element (NON-owning; it must outlive
//     any apply/unapply).
//   - of_named(…, name): the C#-faithful path — the TargetName STRING is stored and resolved at apply()
//     time against the apply target's element namescope (Setter.FindTargetByName: Element.FindByName then
//     the parent-chain namescope walk), exactly like C#'s XAML Setter. An unresolved name throws
//     xaml_parse_exception, mirroring C#'s `?? throw new XamlParseException(...)`.
// When either retarget channel is set, apply/unapply ignore the passed target and route to the resolved
// element instead — a named element is just a pointer once resolved.

#include <any>
#include <string>
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
            return setter{descriptor.name(), std::any{std::move(value)}, nullptr, {}};
        }

        // Create a retargeting setter (Setter.TargetName) from an ALREADY-resolved element: apply/unapply
        // route to `target` regardless of the object passed to apply()/unapply(). NON-owning — `target`
        // must outlive the setter's use.
        template <class T>
        [[nodiscard]] static setter of_for(maui::core::bindable_object& target,
                                           const maui::core::bindable_property<T>& descriptor, T value)
        {
            return setter{descriptor.name(), std::any{std::move(value)}, &target, {}};
        }

        // Create a retargeting setter from a TargetName STRING (Setter.TargetName as authored): the name is
        // resolved at apply() time against the apply target's element namescope (see resolve_target). The
        // common, C#-faithful channel — use this when the named element isn't yet known at setter-creation.
        template <class T>
        [[nodiscard]] static setter of_named(const maui::core::bindable_property<T>& descriptor, T value,
                                             std::string target_name)
        {
            return setter{descriptor.name(), std::any{std::move(value)}, nullptr, std::move(target_name)};
        }

        // The TYPE-ERASED loader builder: the XAML loader resolves a <Setter Property="…" Value="…"/> to an
        // already-converted, already-boxed value plus the backing descriptor name (the apply_setter routing
        // key), so it cannot use the typed of() factories above. `bindable_name` is the descriptor name from
        // xaml_property_registry::property_entry.bindable_name — a string_view into a static descriptor name
        // (register_bindable_property stores descriptor.name()), so the borrow stays valid for the same
        // reason of()'s does. The value's held type must match the property's T (the loader converts it via
        // the property's value-type converter before calling this). Mirrors C#'s Setter created by the XAML
        // SetterValueProvider, whose Property/Value are resolved at ProvideValue (apply) time.
        [[nodiscard]] static setter of_erased(std::string_view bindable_name, std::any value)
        {
            return setter{bindable_name, std::move(value), nullptr, {}};
        }

        // The TargetName-in-style variant of of_erased: the assignment retargets to the named element,
        // resolved at apply() time against the apply target's namescope (Setter.TargetName).
        [[nodiscard]] static setter of_named_erased(std::string_view bindable_name, std::any value,
                                                    std::string target_name)
        {
            return setter{bindable_name, std::move(value), nullptr, std::move(target_name)};
        }

        void apply(maui::core::bindable_object& target, maui::core::setter_specificity specificity) const
        {
            resolve_target(target).apply_setter(property_name_, value_, specificity);
        }
        void unapply(maui::core::bindable_object& target, maui::core::setter_specificity specificity) const
        {
            resolve_target(target).clear_setter(property_name_, specificity);
        }

        [[nodiscard]] std::string_view property_name() const
        {
            return property_name_;
        }

    private:
        setter(std::string_view property_name, std::any value, maui::core::bindable_object* retarget,
               std::string target_name)
            : property_name_(property_name), value_(std::move(value)), retarget_(retarget),
              target_name_(std::move(target_name))
        {
        }

        // The object the setter actually writes to (Setter.Apply's `targetObject`): the pre-resolved retarget
        // when set, else the TargetName-resolved element when a name is stored, else the passed target. The
        // name-resolution path is out-of-line (setter.cpp) so this header doesn't pull in element.hpp.
        [[nodiscard]] maui::core::bindable_object& resolve_target(maui::core::bindable_object& target) const
        {
            if (retarget_ != nullptr)
            {
                return *retarget_;
            }
            if (!target_name_.empty())
            {
                return resolve_named_target(target);
            }
            return target;
        }

        // Setter.FindTargetByName over the apply target (must be an element with a namescope). Throws
        // xaml_parse_exception when the name does not resolve, like C#'s `?? throw new XamlParseException`.
        [[nodiscard]] maui::core::bindable_object& resolve_named_target(maui::core::bindable_object& target) const;

        std::string_view property_name_;                  // the descriptor name (a string literal — stable/borrowed)
        std::any value_;                                  // the boxed value (storage stays typed in property<T>)
        maui::core::bindable_object* retarget_ = nullptr; // pre-resolved TargetName retarget (NON-owning); null = self
        std::string target_name_;                         // Setter.TargetName (resolved at apply); empty = none
    };
} // namespace maui::controls
