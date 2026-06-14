// maui::controls::entry_cell — the shared bindable-property descriptors. See entry_cell.hpp; ported
// from src/Controls/src/Core/Cells/EntryCell.cs (+ TextAlignmentElement for the alignment descriptors).

#include "maui/controls/cells/entry_cell.hpp"

#include <string>

#include "maui/core/bindable_property.hpp"
#include "maui/core/binding_mode.hpp"
#include "maui/core/text_alignment.hpp"
#include "maui/graphics/color.hpp"

namespace maui::controls
{
    const maui::core::bindable_property<std::string>& entry_cell::text_property()
    {
        // EntryCell.TextProperty: TwoWay.
        static const maui::core::bindable_property<std::string> descriptor{
            "text", std::string{}, {.default_binding_mode = maui::core::binding_mode::two_way}};
        return descriptor;
    }

    const maui::core::bindable_property<std::string>& entry_cell::label_property()
    {
        static const maui::core::bindable_property<std::string> descriptor{"label"};
        return descriptor;
    }

    const maui::core::bindable_property<std::string>& entry_cell::placeholder_property()
    {
        static const maui::core::bindable_property<std::string> descriptor{"placeholder"};
        return descriptor;
    }

    const maui::core::bindable_property<maui::graphics::color>& entry_cell::label_color_property()
    {
        static const maui::core::bindable_property<maui::graphics::color> descriptor{"label_color"};
        return descriptor;
    }

    const maui::core::bindable_property<maui::core::text_alignment>& entry_cell::horizontal_text_alignment_property()
    {
        // TextAlignmentElement.HorizontalTextAlignmentProperty: default Start.
        static const maui::core::bindable_property<maui::core::text_alignment> descriptor{
            "horizontal_text_alignment", maui::core::text_alignment::start};
        return descriptor;
    }

    const maui::core::bindable_property<maui::core::text_alignment>& entry_cell::vertical_text_alignment_property()
    {
        // TextAlignmentElement.VerticalTextAlignmentProperty: default Center.
        static const maui::core::bindable_property<maui::core::text_alignment> descriptor{
            "vertical_text_alignment", maui::core::text_alignment::center};
        return descriptor;
    }
} // namespace maui::controls
