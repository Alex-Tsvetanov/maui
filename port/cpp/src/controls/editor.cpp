// maui::controls::editor — out-of-line definitions: the shared bindable-property descriptors (one
// instance per type, like Editor.*Property / InputView.*Property) and the default-handler
// self-registration. See editor.hpp. Defaults mirror InputView/Editor: empty text/placeholder,
// IsReadOnly false, MaxLength = int.MaxValue (no cap), Start/Start alignment (Editor's own
// VerticalTextAlignmentProperty defaults to Start, unlike Entry's center),
// IsTextPredictionEnabled/IsSpellCheckEnabled true, CursorPosition/SelectionLength 0,
// AutoSize = EditorAutoSizeOption.Disabled.

#include "maui/controls/editor.hpp"

#include <limits>
#include <string>

#include "maui/controls/editor_auto_size_option.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/editor_handler.hpp"
#include "maui/core/font.hpp"
#include "maui/core/handler_registry.hpp"
#include "maui/core/text_alignment.hpp"
#include "maui/graphics/color.hpp"

namespace maui::controls
{
    const maui::core::bindable_property<std::string>& editor::text_property()
    {
        static const maui::core::bindable_property<std::string> descriptor{"text", std::string{}};
        return descriptor;
    }

    const maui::core::bindable_property<std::string>& editor::placeholder_property()
    {
        static const maui::core::bindable_property<std::string> descriptor{"placeholder", std::string{}};
        return descriptor;
    }

    const maui::core::bindable_property<maui::graphics::color>& editor::placeholder_color_property()
    {
        static const maui::core::bindable_property<maui::graphics::color> descriptor{"placeholder_color"};
        return descriptor;
    }

    const maui::core::bindable_property<bool>& editor::is_read_only_property()
    {
        static const maui::core::bindable_property<bool> descriptor{"is_read_only", false};
        return descriptor;
    }

    const maui::core::bindable_property<int>& editor::max_length_property()
    {
        // C# default is int.MaxValue (no effective cap).
        static const maui::core::bindable_property<int> descriptor{"max_length", std::numeric_limits<int>::max()};
        return descriptor;
    }

    const maui::core::bindable_property<bool>& editor::is_text_prediction_enabled_property()
    {
        // C# InputView default: true.
        static const maui::core::bindable_property<bool> descriptor{"is_text_prediction_enabled", true};
        return descriptor;
    }

    const maui::core::bindable_property<bool>& editor::is_spell_check_enabled_property()
    {
        // C# InputView default: true.
        static const maui::core::bindable_property<bool> descriptor{"is_spell_check_enabled", true};
        return descriptor;
    }

    const maui::core::bindable_property<int>& editor::cursor_position_property()
    {
        // C# InputView default: 0 (validated >= 0; the control's setters clamp the floor).
        static const maui::core::bindable_property<int> descriptor{"cursor_position", 0};
        return descriptor;
    }

    const maui::core::bindable_property<int>& editor::selection_length_property()
    {
        static const maui::core::bindable_property<int> descriptor{"selection_length", 0};
        return descriptor;
    }

    const maui::core::bindable_property<maui::graphics::color>& editor::text_color_property()
    {
        static const maui::core::bindable_property<maui::graphics::color> descriptor{"text_color"};
        return descriptor;
    }

    const maui::core::bindable_property<maui::core::font>& editor::font_property()
    {
        static const maui::core::bindable_property<maui::core::font> descriptor{"font"};
        return descriptor;
    }

    const maui::core::bindable_property<double>& editor::character_spacing_property()
    {
        static const maui::core::bindable_property<double> descriptor{"character_spacing", 0.0};
        return descriptor;
    }

    const maui::core::bindable_property<maui::core::text_alignment>& editor::horizontal_text_alignment_property()
    {
        static const maui::core::bindable_property<maui::core::text_alignment> descriptor{
            "horizontal_text_alignment", maui::core::text_alignment::start};
        return descriptor;
    }

    const maui::core::bindable_property<maui::core::text_alignment>& editor::vertical_text_alignment_property()
    {
        // Editor's own VerticalTextAlignmentProperty defaults to Start (Editor.cs), unlike Entry's center.
        static const maui::core::bindable_property<maui::core::text_alignment> descriptor{
            "vertical_text_alignment", maui::core::text_alignment::start};
        return descriptor;
    }

    const maui::core::bindable_property<editor_auto_size_option>& editor::auto_size_property()
    {
        // C# default: EditorAutoSizeOption.Disabled.
        static const maui::core::bindable_property<editor_auto_size_option> descriptor{
            "auto_size", editor_auto_size_option::disabled};
        return descriptor;
    }
} // namespace maui::controls

// Self-register the default handler for editor (opt-in, PROFILE §6).
MAUI_REGISTER_HANDLER(maui::controls::editor, maui::core::editor_handler)
