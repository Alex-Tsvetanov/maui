// maui::controls::text_cell — the shared bindable-property descriptors. See text_cell.hpp; ported from
// src/Controls/src/Core/Cells/TextCell.cs.

#include "maui/controls/cells/text_cell.hpp"

#include <string>

#include "maui/core/bindable_property.hpp"
#include "maui/graphics/color.hpp"

namespace maui::controls
{
    const maui::core::bindable_property<std::string>& text_cell::text_property()
    {
        static const maui::core::bindable_property<std::string> descriptor{"text"};
        return descriptor;
    }

    const maui::core::bindable_property<std::string>& text_cell::detail_property()
    {
        static const maui::core::bindable_property<std::string> descriptor{"detail"};
        return descriptor;
    }

    const maui::core::bindable_property<maui::graphics::color>& text_cell::text_color_property()
    {
        static const maui::core::bindable_property<maui::graphics::color> descriptor{"text_color"};
        return descriptor;
    }

    const maui::core::bindable_property<maui::graphics::color>& text_cell::detail_color_property()
    {
        static const maui::core::bindable_property<maui::graphics::color> descriptor{"detail_color"};
        return descriptor;
    }
} // namespace maui::controls
