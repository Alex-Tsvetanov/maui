// maui::controls::vertical_stack_layout — out-of-line definitions: the shared bindable-property
// descriptors + the default-handler self-registration. See vertical_stack_layout.hpp.

#include "maui/controls/vertical_stack_layout.hpp"

#include "maui/core/bindable_property.hpp"
#include "maui/core/handler_registry.hpp"
#include "maui/core/layout_handler.hpp"
#include "maui/core/thickness.hpp"

namespace maui::controls
{
    const maui::core::bindable_property<double>& vertical_stack_layout::spacing_property()
    {
        static const maui::core::bindable_property<double> descriptor{"spacing", 0.0};
        return descriptor;
    }

    const maui::core::bindable_property<maui::core::thickness>& vertical_stack_layout::padding_property()
    {
        static const maui::core::bindable_property<maui::core::thickness> descriptor{"padding"};
        return descriptor;
    }
} // namespace maui::controls

// Self-register the default handler for vertical_stack_layout (opt-in, PROFILE §6).
MAUI_REGISTER_HANDLER(maui::controls::vertical_stack_layout, maui::core::layout_handler)
