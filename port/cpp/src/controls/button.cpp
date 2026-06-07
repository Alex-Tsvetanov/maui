// maui::controls::button — out-of-line definitions. The Text bindable-property descriptor lives here
// (one shared instance per type, like Button.TextProperty). See button.hpp for the design.

#include "maui/controls/button.hpp"

#include <string>

#include "maui/core/bindable_property.hpp"

namespace maui::controls
{
    const maui::core::bindable_property<std::string>& button::text_property()
    {
        static const maui::core::bindable_property<std::string> descriptor{"text", std::string{}};
        return descriptor;
    }
} // namespace maui::controls
