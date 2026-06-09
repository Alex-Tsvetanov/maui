// maui::controls::style — apply/unapply the setter bundle at the style specificity (style.hpp).
#include "maui/controls/style.hpp"

#include <memory>

#include "maui/core/bindable_object.hpp"
#include "maui/core/setter_specificity.hpp"

namespace maui::controls
{
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
