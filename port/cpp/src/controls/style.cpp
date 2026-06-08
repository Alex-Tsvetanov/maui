// maui::controls::style — apply/unapply the setter bundle at the style specificity (style.hpp).
#include "maui/controls/style.hpp"

#include "maui/core/bindable_object.hpp"
#include "maui/core/setter_specificity.hpp"

namespace maui::controls
{
    void style::apply(maui::core::bindable_object& target, maui::core::setter_specificity specificity) const
    {
        // BasedOn first, at a lowered specificity, so this style's own setters override the inherited ones.
        if (based_on_)
        {
            based_on_->apply(target, specificity.as_base_style());
        }
        for (const auto& value : setters_)
        {
            value.apply(target, specificity);
        }
    }

    void style::unapply(maui::core::bindable_object& target, maui::core::setter_specificity specificity) const
    {
        for (const auto& value : setters_)
        {
            value.unapply(target, specificity);
        }
        if (based_on_)
        {
            based_on_->unapply(target, specificity.as_base_style());
        }
    }
} // namespace maui::controls
