// maui::controls::search_bar — out-of-line definitions: the shared bindable-property descriptors (one
// instance per type, like SearchBar.*Property / InputView.*Property) and the default-handler
// self-registration. See search_bar.hpp. Defaults mirror InputView/SearchBar: empty text/placeholder,
// IsReadOnly false, MaxLength = int.MaxValue (no cap), Start/Center alignment (TextAlignmentElement),
// prediction/spellcheck true, CursorPosition/SelectionLength 0, ReturnType.Search, and default-color
// CancelButtonColor/SearchIconColor (C# null — platform default).

#include "maui/controls/search_bar.hpp"

#include <limits>
#include <string>

#include "maui/core/bindable_property.hpp"
#include "maui/core/font.hpp"
#include "maui/core/handler_registry.hpp"
#include "maui/core/return_type.hpp"
#include "maui/core/search_bar_handler.hpp"
#include "maui/core/text_alignment.hpp"
#include "maui/graphics/color.hpp"

namespace maui::controls
{
    const maui::core::bindable_property<std::string>& search_bar::text_property()
    {
        static const maui::core::bindable_property<std::string> descriptor{"text", std::string{}};
        return descriptor;
    }

    const maui::core::bindable_property<std::string>& search_bar::placeholder_property()
    {
        static const maui::core::bindable_property<std::string> descriptor{"placeholder", std::string{}};
        return descriptor;
    }

    const maui::core::bindable_property<maui::graphics::color>& search_bar::placeholder_color_property()
    {
        static const maui::core::bindable_property<maui::graphics::color> descriptor{"placeholder_color"};
        return descriptor;
    }

    const maui::core::bindable_property<maui::graphics::color>& search_bar::cancel_button_color_property()
    {
        // C# default(Color) — null, the platform default; the default-constructed color stands in.
        static const maui::core::bindable_property<maui::graphics::color> descriptor{"cancel_button_color"};
        return descriptor;
    }

    const maui::core::bindable_property<maui::graphics::color>& search_bar::search_icon_color_property()
    {
        static const maui::core::bindable_property<maui::graphics::color> descriptor{"search_icon_color"};
        return descriptor;
    }

    const maui::core::bindable_property<bool>& search_bar::is_read_only_property()
    {
        static const maui::core::bindable_property<bool> descriptor{"is_read_only", false};
        return descriptor;
    }

    const maui::core::bindable_property<int>& search_bar::max_length_property()
    {
        // C# default is int.MaxValue (no effective cap).
        static const maui::core::bindable_property<int> descriptor{"max_length", std::numeric_limits<int>::max()};
        return descriptor;
    }

    const maui::core::bindable_property<bool>& search_bar::is_text_prediction_enabled_property()
    {
        static const maui::core::bindable_property<bool> descriptor{"is_text_prediction_enabled", true};
        return descriptor;
    }

    const maui::core::bindable_property<bool>& search_bar::is_spell_check_enabled_property()
    {
        static const maui::core::bindable_property<bool> descriptor{"is_spell_check_enabled", true};
        return descriptor;
    }

    const maui::core::bindable_property<int>& search_bar::cursor_position_property()
    {
        static const maui::core::bindable_property<int> descriptor{"cursor_position", 0};
        return descriptor;
    }

    const maui::core::bindable_property<int>& search_bar::selection_length_property()
    {
        static const maui::core::bindable_property<int> descriptor{"selection_length", 0};
        return descriptor;
    }

    const maui::core::bindable_property<maui::core::return_type>& search_bar::return_type_property()
    {
        // C# SearchBar default: ReturnType.Search.
        static const maui::core::bindable_property<maui::core::return_type> descriptor{"return_type",
                                                                                       maui::core::return_type::search};
        return descriptor;
    }

    const maui::core::bindable_property<maui::graphics::color>& search_bar::text_color_property()
    {
        static const maui::core::bindable_property<maui::graphics::color> descriptor{"text_color"};
        return descriptor;
    }

    const maui::core::bindable_property<maui::core::font>& search_bar::font_property()
    {
        static const maui::core::bindable_property<maui::core::font> descriptor{"font"};
        return descriptor;
    }

    const maui::core::bindable_property<double>& search_bar::character_spacing_property()
    {
        static const maui::core::bindable_property<double> descriptor{"character_spacing", 0.0};
        return descriptor;
    }

    const maui::core::bindable_property<maui::core::text_alignment>& search_bar::horizontal_text_alignment_property()
    {
        static const maui::core::bindable_property<maui::core::text_alignment> descriptor{
            "horizontal_text_alignment", maui::core::text_alignment::start};
        return descriptor;
    }

    const maui::core::bindable_property<maui::core::text_alignment>& search_bar::vertical_text_alignment_property()
    {
        // TextAlignmentElement.VerticalTextAlignmentProperty default: Center.
        static const maui::core::bindable_property<maui::core::text_alignment> descriptor{
            "vertical_text_alignment", maui::core::text_alignment::center};
        return descriptor;
    }
} // namespace maui::controls

// Self-register the default handler for search_bar (opt-in, PROFILE §6).
MAUI_REGISTER_HANDLER(maui::controls::search_bar, maui::core::search_bar_handler)
