// maui::controls::image_cell — the shared bindable-property descriptor. See image_cell.hpp; ported from
// src/Controls/src/Core/Cells/ImageCell.cs.

#include "maui/controls/cells/image_cell.hpp"

#include <memory>

#include "maui/core/bindable_property.hpp"
#include "maui/core/i_image_source.hpp"

namespace maui::controls
{
    const maui::core::bindable_property<std::shared_ptr<maui::core::i_image_source>>& image_cell::
        image_source_property()
    {
        static const maui::core::bindable_property<std::shared_ptr<maui::core::i_image_source>> descriptor{
            "image_source"};
        return descriptor;
    }
} // namespace maui::controls
