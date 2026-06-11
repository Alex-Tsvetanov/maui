// maui::controls::style — apply/unapply the setter bundle at the style specificity (style.hpp).
#include "maui/controls/style.hpp"

#include <cstddef>
#include <memory>
#include <vector>

#include "maui/core/bindable_object.hpp"
#include "maui/core/setter_specificity.hpp"
#include "maui/core/type_tag.hpp"

namespace maui::controls
{
    bool style::can_be_applied_to(const std::vector<maui::core::type_tag>& target_chain) const
    {
        // Style.CanBeAppliedTo: the exact target type always matches; base types only with
        // ApplyToDerivedTypes. The chain is declared most-derived first (chain[0] = the control's own
        // type — C#'s `targetType`; the rest is the port of the Type.BaseType walk).
        if (target_chain.empty())
        {
            return true; // no declared target type — match anything (the pre-W1-15 class-style behavior)
        }
        if (target_type_ == target_chain.front())
        {
            return true;
        }
        if (!apply_to_derived_types_)
        {
            return false;
        }
        for (std::size_t i = 1; i < target_chain.size(); ++i)
        {
            if (target_type_ == target_chain[i])
            {
                return true;
            }
        }
        return false;
    }

    std::shared_ptr<style> style::effective_based_on(const resource_resolver& resolve) const
    {
        // Style.Apply resolves `BasedOn ?? GetBasedOnResource(bindable)`: a direct based_on wins, else the
        // base_resource_key is looked up in the target's resource chain (only possible with a resolver).
        if (based_on_)
        {
            return based_on_;
        }
        if (resolve && !base_resource_key_.empty())
        {
            return resolve(base_resource_key_);
        }
        return nullptr;
    }

    void style::apply(maui::core::bindable_object& target, maui::core::setter_specificity specificity,
                      const resource_resolver& resolve) const
    {
        // BasedOn first, at a lowered specificity, so this style's own setters override the inherited ones.
        if (const std::shared_ptr<style> base = effective_based_on(resolve))
        {
            base->apply(target, specificity.as_base_style(), resolve);
        }
        for (const auto& value : setters_)
        {
            value.apply(target, specificity);
        }
    }

    void style::unapply(maui::core::bindable_object& target, maui::core::setter_specificity specificity,
                        const resource_resolver& resolve) const
    {
        for (const auto& value : setters_)
        {
            value.unapply(target, specificity);
        }
        if (const std::shared_ptr<style> base = effective_based_on(resolve))
        {
            base->unapply(target, specificity.as_base_style(), resolve);
        }
    }
} // namespace maui::controls
