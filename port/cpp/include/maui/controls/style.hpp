#pragma once
// maui::controls::style  <=  Microsoft.Maui.Controls.Style
//
// A reusable bundle of property setters (with a target type) that can be applied to a bindable_object at
// a given specificity — the code-first equivalent of a XAML <Style>. apply() pushes each setter's value
// through bindable_object::apply_setter (so it lands in the typed property<T> at the style specificity);
// unapply() clears those values. A based_on style is applied first at the lowered (base-style)
// specificity, so the derived style's own setters win — mirroring Style.ApplyCore / SetterSpecificity
// .AsBaseStyle. Ported from Style.cs (the IStyle.Apply/UnApply core).
//
// Scope (M5b): target_type + setters + a directly-assigned based_on. Implicit styles, ResourceDictionary
// lookup, style classes, BasedOn-by-resource-key, DynamicResource, and Behaviors/Triggers attached to a
// Style are deferred (STATUS.md). The target_type is carried (for the future implicit-style match) but
// not yet enforced against the target — applying is by typed descriptor, already type-correct at the
// call site (setter::of).

#include <memory>
#include <utility>
#include <vector>

#include "maui/controls/setter.hpp"
#include "maui/core/bindable_object.hpp"
#include "maui/core/setter_specificity.hpp"
#include "maui/core/type_tag.hpp"

namespace maui::controls
{
    class style
    {
    public:
        explicit style(maui::core::type_tag target_type) : target_type_(target_type)
        {
        }

        // Convenience factory: style::of<button>() captures the target type tag.
        template <class TTarget> [[nodiscard]] static style of()
        {
            return style{maui::core::type_tag::of<TTarget>()};
        }

        [[nodiscard]] maui::core::type_tag target_type() const
        {
            return target_type_;
        }

        // Add a setter to the bundle (fluent — returns *this so calls can chain).
        style& add(setter value)
        {
            setters_.push_back(std::move(value));
            return *this;
        }

        // The style this one inherits from; its setters apply at a lowered (base-style) specificity so the
        // derived style's own setters take precedence.
        void set_based_on(std::shared_ptr<style> value)
        {
            based_on_ = std::move(value);
        }
        [[nodiscard]] const std::shared_ptr<style>& based_on() const
        {
            return based_on_;
        }

        // Apply / un-apply every setter at `specificity` (defaulting to the explicit/local-style level).
        // based_on (if any) is applied first at the lowered base-style specificity (so derived setters win)
        // and un-applied last, mirroring Style.ApplyCore/UnApplyCore.
        void apply(maui::core::bindable_object& target,
                   maui::core::setter_specificity specificity = maui::core::setter_specificity::style_local) const;
        void unapply(maui::core::bindable_object& target,
                     maui::core::setter_specificity specificity = maui::core::setter_specificity::style_local) const;

    private:
        maui::core::type_tag target_type_;
        std::vector<setter> setters_;
        std::shared_ptr<style> based_on_;
    };
} // namespace maui::controls
