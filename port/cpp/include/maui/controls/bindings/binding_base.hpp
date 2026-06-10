#pragma once
// maui::controls::binding_base  <=  Microsoft.Maui.Controls.BindingBase
//
// The abstract base of every binding: mode selection, StringFormat, TargetNullValue / FallbackValue,
// and the internal apply/unapply seam element::set_binding and context changes drive. Also home to
// the two value sentinels (C# Binding.DoNothing / BindableProperty.UnsetValue) converters may return.
//
// Port notes:
//   - C#'s BindableObject.SetValueActual invokes binding.Apply(fromTarget: true) whenever a bound
//     target property changes (unless that change happened while a fromTarget apply was in flight —
//     the `_applying` guard). The port reproduces this with a target WATCH owned here: the base
//     apply() subscribes to the target's property_changed for the bound property and routes it
//     through the same guard, so two_way / one_way_to_source flows behave exactly like C#.
//   - try_format implements the string.Format subset MAUI bindings exercise: positional "{n}"
//     placeholders, "{{"/"}}" escapes, and an all-'0' zero-pad numeric spec ("{0:000}"). Any other
//     format spec renders the value plainly; malformed strings return nullopt (C#'s caught
//     FormatException — the unformatted value flows instead).
//   - Collection synchronization (Enable/DisableCollectionSynchronization) is .NET-threading
//     machinery and is not ported (the visual tree is single-threaded per PROFILE §8).
//
// Lifetime (§8): a binding is owned by its target element (element::set_binding). It holds the
// target raw + weak-token and never owns a source; the watch disconnects in unapply()/the destructor
// only while the target is alive.

#include <any>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <string_view>

#include "maui/core/bindable_object.hpp"
#include "maui/core/binding_mode.hpp"
#include "maui/core/event.hpp"
#include "maui/core/setter_specificity.hpp"
#include "maui/core/type_tag.hpp"

namespace maui::controls
{
    class element;

    // The "do not update the target" converter return (C# Binding.DoNothing / MultiBinding.DoNothing).
    struct do_nothing_value
    {
        bool operator==(const do_nothing_value&) const = default;
    };
    // The "no value" marker (C# BindableProperty.UnsetValue): a multi-converter returns it to fall
    // back to the binding's FallbackValue; per-element in convert_back it skips that source.
    struct unset_value
    {
        bool operator==(const unset_value&) const = default;
    };
    [[nodiscard]] inline bool is_do_nothing(const std::any& value)
    {
        return std::any_cast<do_nothing_value>(&value) != nullptr;
    }
    [[nodiscard]] inline bool is_unset_value(const std::any& value)
    {
        return std::any_cast<unset_value>(&value) != nullptr;
    }

    class binding_base
    {
    public:
        binding_base(const binding_base&) = delete;
        binding_base(binding_base&&) = delete;
        binding_base& operator=(const binding_base&) = delete;
        binding_base& operator=(binding_base&&) = delete;
        virtual ~binding_base();

        [[nodiscard]] maui::core::binding_mode mode() const
        {
            return mode_;
        }
        void set_mode(maui::core::binding_mode value);

        [[nodiscard]] const std::string& string_format() const
        {
            return string_format_;
        }
        void set_string_format(std::string value);

        // The value replacing a RESOLVED null source value (not used when resolution fails).
        [[nodiscard]] const std::any& target_null_value() const
        {
            return target_null_value_;
        }
        void set_target_null_value(std::any value);

        // The value used when the binding cannot produce one (path unresolved / no source).
        [[nodiscard]] const std::any& fallback_value() const
        {
            return fallback_value_;
        }
        void set_fallback_value(std::any value);

        [[nodiscard]] bool is_applied() const
        {
            return is_applied_;
        }

        [[nodiscard]] virtual std::shared_ptr<binding_base> clone() const = 0;

        // ---- the internal seam (C# internal members; element::set_binding + multi_binding drive it) ----

        // Apply to a target property against `context` (the target's binding context box).
        virtual void apply(const maui::core::bindable_object::binding_context_box& context,
                           maui::core::bindable_object& target, std::string_view target_property,
                           bool from_binding_context_changed, maui::core::setter_specificity specificity);
        // Re-apply on the stored source/target; from_target pushes target -> source (two_way / OWTS).
        // The base only marks the binding applied (C# BindingBase.Apply(fromTarget) => IsApplied = true).
        virtual void apply(bool from_target);
        virtual void unapply(bool from_binding_context_changed = false);

        // BindingBase.GetSourceValue: null -> TargetNullValue, then StringFormat. Subclasses run their
        // converter first. `target_type` is the target property's type_tag (the C# targetType).
        [[nodiscard]] virtual std::any get_source_value(std::any value, maui::core::type_tag target_type) const;
        // BindingBase.GetTargetValue: the identity (Binding overrides with converter.ConvertBack).
        [[nodiscard]] virtual std::any get_target_value(std::any value, maui::core::type_tag source_type) const;

        // MultiBinding routes its inner relative-source bindings at the REAL target element instead of
        // its proxy (C# RelativeSourceTargetOverride; held weakly via the element's liveness token).
        void set_relative_source_target_override(element* value);
        [[nodiscard]] element* relative_source_target_override() const;

        // BindingBase.TryFormat (see the header note for the supported subset).
        [[nodiscard]] static std::optional<std::string> try_format(std::string_view format,
                                                                   std::span<const std::any> args);

    protected:
        binding_base() = default;

        // Throws std::runtime_error once applied (C# "Cannot change a binding while it's applied").
        void throw_if_applied() const;

        // The target watch (see the header note): installed by the base apply(), removed by a full
        // unapply()/destruction. Re-entrant target changes during a from-target apply are swallowed,
        // mirroring BindableObject._applying.
        void watch_target(maui::core::bindable_object& target, std::string_view target_property);
        void unwatch_target();

    private:
        maui::core::binding_mode mode_ = maui::core::binding_mode::default_mode;
        std::string string_format_;
        std::any target_null_value_;
        std::any fallback_value_;
        bool is_applied_ = false;

        bool applying_from_target_ = false;
        maui::core::bindable_object* watched_target_ = nullptr;
        std::weak_ptr<void> watched_target_alive_;
        std::string watched_property_;
        maui::core::connection_token watch_token_ = 0;

        element* relative_override_ = nullptr;
        std::weak_ptr<void> relative_override_alive_;
    };
} // namespace maui::controls
