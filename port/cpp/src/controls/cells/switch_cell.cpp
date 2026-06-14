// maui::controls::switch_cell — the shared bindable-property descriptors (the On descriptor raises
// OnChanged). See switch_cell.hpp; ported from src/Controls/src/Core/Cells/SwitchCell.cs.

#include "maui/controls/cells/switch_cell.hpp"

#include <string>

#include "maui/core/bindable_property.hpp"
#include "maui/core/binding_mode.hpp"
#include "maui/graphics/color.hpp"

namespace maui::controls
{
    const maui::core::bindable_property<bool>& switch_cell::on_property()
    {
        // SwitchCell.OnProperty: default false, TwoWay; a change raises OnChanged(new value).
        static const maui::core::bindable_property<bool> descriptor{
            "on",
            false,
            {.property_changed =
                 [](maui::core::bindable_object& bindable, const bool& /*old_value*/, const bool& new_value) {
                     dynamic_cast<switch_cell&>(bindable).on_changed.raise(new_value);
                 },
             .default_binding_mode = maui::core::binding_mode::two_way}};
        return descriptor;
    }

    const maui::core::bindable_property<std::string>& switch_cell::text_property()
    {
        static const maui::core::bindable_property<std::string> descriptor{"text"};
        return descriptor;
    }

    const maui::core::bindable_property<maui::graphics::color>& switch_cell::on_color_property()
    {
        static const maui::core::bindable_property<maui::graphics::color> descriptor{"on_color"};
        return descriptor;
    }
} // namespace maui::controls
