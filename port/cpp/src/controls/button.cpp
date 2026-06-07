// maui::controls::button — out-of-line definitions. The Text bindable-property descriptor lives here
// (one shared instance per type, like Button.TextProperty). See button.hpp for the design.

#include "maui/controls/button.hpp"

#include <string>

#include "maui/core/bindable_property.hpp"
#include "maui/core/button_handler.hpp"
#include "maui/core/handler_registry.hpp"

namespace maui::controls
{
    const maui::core::bindable_property<std::string>& button::text_property()
    {
        static const maui::core::bindable_property<std::string> descriptor{"text", std::string{}};
        return descriptor;
    }
} // namespace maui::controls

// Self-register the default handler for button (opt-in, PROFILE §6). This TU is always linked (button's
// out-of-line members above are referenced by every user of the control), so the registrar runs.
MAUI_REGISTER_HANDLER(maui::controls::button, maui::core::button_handler)
