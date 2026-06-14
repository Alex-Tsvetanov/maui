// maui::controls::table_section_base — the shared Title / TextColor descriptors. See
// table_section_base.hpp; ported from src/Controls/src/Core/TableView/TableSectionBase.cs.

#include "maui/controls/table_section_base.hpp"

#include <string>

#include "maui/core/bindable_property.hpp"
#include "maui/graphics/color.hpp"

namespace maui::controls
{
    const maui::core::bindable_property<std::string>& table_section_base::title_property()
    {
        static const maui::core::bindable_property<std::string> descriptor{"title"};
        return descriptor;
    }

    const maui::core::bindable_property<maui::graphics::color>& table_section_base::text_color_property()
    {
        static const maui::core::bindable_property<maui::graphics::color> descriptor{"text_color"};
        return descriptor;
    }
} // namespace maui::controls
