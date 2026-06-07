// maui::controls::entry — out-of-line definitions: the shared bindable-property descriptors (one instance
// per type, like Entry.*Property / InputView.*Property) and the default-handler self-registration. See
// entry.hpp. Defaults mirror InputView/Entry: empty text/placeholder, IsPassword/IsReadOnly false,
// MaxLength = int.MaxValue (no cap), start/center alignment.

#include "maui/controls/entry.hpp"

#include <limits>
#include <string>

#include "maui/core/bindable_property.hpp"
#include "maui/core/entry_handler.hpp"
#include "maui/core/font.hpp"
#include "maui/core/handler_registry.hpp"
#include "maui/core/text_alignment.hpp"
#include "maui/graphics/color.hpp"

namespace maui::controls
{
    const maui::core::bindable_property<std::string>& entry::text_property()
    {
        static const maui::core::bindable_property<std::string> descriptor{"text", std::string{}};
        return descriptor;
    }

    const maui::core::bindable_property<std::string>& entry::placeholder_property()
    {
        static const maui::core::bindable_property<std::string> descriptor{"placeholder", std::string{}};
        return descriptor;
    }

    const maui::core::bindable_property<maui::graphics::color>& entry::placeholder_color_property()
    {
        static const maui::core::bindable_property<maui::graphics::color> descriptor{"placeholder_color"};
        return descriptor;
    }

    const maui::core::bindable_property<bool>& entry::is_password_property()
    {
        static const maui::core::bindable_property<bool> descriptor{"is_password", false};
        return descriptor;
    }

    const maui::core::bindable_property<bool>& entry::is_read_only_property()
    {
        static const maui::core::bindable_property<bool> descriptor{"is_read_only", false};
        return descriptor;
    }

    const maui::core::bindable_property<int>& entry::max_length_property()
    {
        // C# default is int.MaxValue (no effective cap).
        static const maui::core::bindable_property<int> descriptor{"max_length", std::numeric_limits<int>::max()};
        return descriptor;
    }

    const maui::core::bindable_property<maui::graphics::color>& entry::text_color_property()
    {
        static const maui::core::bindable_property<maui::graphics::color> descriptor{"text_color"};
        return descriptor;
    }

    const maui::core::bindable_property<maui::core::font>& entry::font_property()
    {
        static const maui::core::bindable_property<maui::core::font> descriptor{"font"};
        return descriptor;
    }

    const maui::core::bindable_property<double>& entry::character_spacing_property()
    {
        static const maui::core::bindable_property<double> descriptor{"character_spacing", 0.0};
        return descriptor;
    }

    const maui::core::bindable_property<maui::core::text_alignment>& entry::horizontal_text_alignment_property()
    {
        static const maui::core::bindable_property<maui::core::text_alignment> descriptor{
            "horizontal_text_alignment", maui::core::text_alignment::start};
        return descriptor;
    }

    const maui::core::bindable_property<maui::core::text_alignment>& entry::vertical_text_alignment_property()
    {
        static const maui::core::bindable_property<maui::core::text_alignment> descriptor{
            "vertical_text_alignment", maui::core::text_alignment::center};
        return descriptor;
    }
} // namespace maui::controls

// Self-register the default handler for entry (opt-in, PROFILE §6).
MAUI_REGISTER_HANDLER(maui::controls::entry, maui::core::entry_handler)
