// maui::controls::entry — out-of-line definitions: the shared bindable-property descriptors (one instance
// per type, like Entry.*Property / InputView.*Property) and the default-handler self-registration. See
// entry.hpp. Defaults mirror InputView/Entry: empty text/placeholder, IsPassword/IsReadOnly false,
// MaxLength = int.MaxValue (no cap), start/center alignment, IsTextPredictionEnabled/IsSpellCheckEnabled
// true, CursorPosition/SelectionLength 0, ReturnType.Default, ClearButtonVisibility.Never.

#include "maui/controls/entry.hpp"

#include <limits>
#include <optional> // --- W2-24: the cursor_color() return type ---
#include <string>

#include "maui/controls/platform_configuration/ios_specific/entry.hpp" // --- W2-24 ---
#include "maui/core/bindable_property.hpp"
#include "maui/core/clear_button_visibility.hpp"
#include "maui/core/entry_handler.hpp"
#include "maui/core/font.hpp"
#include "maui/core/handler_registry.hpp"
#include "maui/core/keyboard.hpp"
#include "maui/core/return_type.hpp"
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

    const maui::core::bindable_property<bool>& entry::is_text_prediction_enabled_property()
    {
        // C# InputView default: true.
        static const maui::core::bindable_property<bool> descriptor{"is_text_prediction_enabled", true};
        return descriptor;
    }

    const maui::core::bindable_property<bool>& entry::is_spell_check_enabled_property()
    {
        // C# InputView default: true.
        static const maui::core::bindable_property<bool> descriptor{"is_spell_check_enabled", true};
        return descriptor;
    }

    const maui::core::bindable_property<int>& entry::cursor_position_property()
    {
        // C# InputView default: 0 (validated >= 0; the control's setters clamp the floor).
        static const maui::core::bindable_property<int> descriptor{"cursor_position", 0};
        return descriptor;
    }

    const maui::core::bindable_property<int>& entry::selection_length_property()
    {
        static const maui::core::bindable_property<int> descriptor{"selection_length", 0};
        return descriptor;
    }

    const maui::core::bindable_property<maui::core::return_type>& entry::return_type_property()
    {
        static const maui::core::bindable_property<maui::core::return_type> descriptor{
            "return_type", maui::core::return_type::default_};
        return descriptor;
    }

    const maui::core::bindable_property<maui::core::clear_button_visibility>& entry::clear_button_visibility_property()
    {
        static const maui::core::bindable_property<maui::core::clear_button_visibility> descriptor{
            "clear_button_visibility", maui::core::clear_button_visibility::never};
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

    const maui::core::bindable_property<maui::core::keyboard>& entry::keyboard_property()
    {
        // C# InputView.KeyboardProperty default is Keyboard.Default (its coerceValue maps null -> Default;
        // a value-type keyboard can never be null, so the default alone reproduces that).
        static const maui::core::bindable_property<maui::core::keyboard> descriptor{
            "keyboard", maui::core::keyboard::default_keyboard()};
        return descriptor;
    }

    // --- platform configuration (W2-24): the iOSSpecific Entry.CursorColor face ----------------------

    // C# entry.IsSet(iOSSpecific.Entry.CursorColorProperty).
    bool entry::cursor_color_set() const
    {
        return has_platform_spec(platform_configuration::ios_specific::entry::cursor_color_key);
    }

    // C# iOSSpecific.Entry.GetCursorColor.
    std::optional<maui::graphics::color> entry::cursor_color() const
    {
        return platform_configuration::ios_specific::entry::get_cursor_color(*this);
    }

    // C# iOSSpecific.Entry.AdjustsFontSizeToFitWidth() — reads the bool knob (default false). No IsSet
    // probe: TextExtensions.UpdateAdjustsFontSizeToFitWidth pushes the value unconditionally.
    bool entry::adjusts_font_size_to_fit_width() const
    {
        return platform_configuration::ios_specific::entry::get_adjusts_font_size_to_fit_width(*this);
    }
    // --- end platform configuration (W2-24) -----------------------------------------------------------
} // namespace maui::controls

// Self-register the default handler for entry (opt-in, PROFILE §6).
MAUI_REGISTER_HANDLER(maui::controls::entry, maui::core::entry_handler)
