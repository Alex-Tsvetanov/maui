// maui::controls::grid — out-of-line definitions: the shared bindable-property descriptors + the
// default-handler self-registration. See grid.hpp.

#include "maui/controls/grid.hpp"

#include "maui/core/bindable_property.hpp"
#include "maui/core/handler_registry.hpp"
#include "maui/core/layout_handler.hpp"
#include "maui/core/thickness.hpp"

namespace maui::controls
{
    const maui::core::bindable_property<double>& grid::row_spacing_property()
    {
        static const maui::core::bindable_property<double> descriptor{"row_spacing", 0.0};
        return descriptor;
    }

    const maui::core::bindable_property<double>& grid::column_spacing_property()
    {
        static const maui::core::bindable_property<double> descriptor{"column_spacing", 0.0};
        return descriptor;
    }

    const maui::core::bindable_property<maui::core::thickness>& grid::padding_property()
    {
        static const maui::core::bindable_property<maui::core::thickness> descriptor{"padding"};
        return descriptor;
    }
} // namespace maui::controls

// Self-register the default handler for grid — a grid is an i_layout, so it reuses layout_handler (the
// same handler the stack layouts use; no grid-specific handler). Opt-in, PROFILE §6.
MAUI_REGISTER_HANDLER(maui::controls::grid, maui::core::layout_handler)
