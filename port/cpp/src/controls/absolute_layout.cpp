// maui::controls::absolute_layout — out-of-line definitions: the shared padding descriptor + the
// default-handler self-registration. See absolute_layout.hpp.

#include "maui/controls/absolute_layout.hpp"

#include "maui/core/bindable_property.hpp"
#include "maui/core/handler_registry.hpp"
#include "maui/core/layout_handler.hpp"
#include "maui/core/thickness.hpp"

namespace maui::controls
{
    const maui::core::bindable_property<maui::core::thickness>& absolute_layout::padding_property()
    {
        static const maui::core::bindable_property<maui::core::thickness> descriptor{"padding"};
        return descriptor;
    }
} // namespace maui::controls

// Self-register the default handler for absolute_layout — an absolute_layout is an i_layout, so it reuses
// layout_handler (the same handler the stack/grid layouts use). Opt-in, PROFILE §6.
MAUI_REGISTER_HANDLER(maui::controls::absolute_layout, maui::core::layout_handler)
