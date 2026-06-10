// editor_handler — headless platform recipe. A testable stand-in for an editable native multi-line
// view: every mapped property is mirrored into editor_platform, and the inbound hooks (on_text_changed /
// on_completed) forward to the virtual view's send_* methods so tests can simulate a native edit /
// end-of-edit and observe it flow through to the control's events. The Apple backend
// (src/platform/apple/editor_handler.mm) is the real-native twin.

#include "maui/core/editor_handler.hpp"

#include <memory>
#include <string>

#include "maui/core/i_editor.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

namespace maui::core
{
    // Headless has no native view in the `native` slot, so destruction is trivial.
    editor_platform::~editor_platform() = default;

    std::unique_ptr<editor_platform> editor_handler::create_platform_view()
    {
        return std::make_unique<editor_platform>();
    }

    void editor_handler::on_connect_handler(editor_platform& platform)
    {
        // Mirror EditorHandler.iOS's MauiTextViewEventProxy: TextSetOrChanged → send_text_changed(old,
        // new); Ended → Completed. Headless tests invoke these directly.
        platform.on_text_changed = [this](const std::string& old_value, const std::string& new_value) {
            if (auto* platform_view = typed_platform_view())
            {
                platform_view->last_known_text = new_value;
            }
            if (auto* view = virtual_view())
            {
                view->send_text_changed(old_value, new_value);
            }
        };
        platform.on_completed = [this] {
            if (auto* view = virtual_view())
            {
                view->send_completed();
            }
        };
    }

    void editor_handler::on_disconnect_handler(editor_platform& platform)
    {
        platform.on_text_changed = nullptr;
        platform.on_completed = nullptr;
    }

    void editor_handler::map_text(editor_handler& handler, i_editor& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->text = std::string(view.text());
            platform->last_known_text = platform->text;
        }
    }

    void editor_handler::map_placeholder(editor_handler& handler, i_editor& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->placeholder = std::string(view.placeholder());
        }
    }

    void editor_handler::map_placeholder_color(editor_handler& handler, i_editor& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->placeholder_color = view.placeholder_color();
        }
    }

    void editor_handler::map_is_read_only(editor_handler& handler, i_editor& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->is_read_only = view.is_read_only();
        }
    }

    void editor_handler::map_max_length(editor_handler& handler, i_editor& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->max_length = view.max_length();
        }
    }

    void editor_handler::map_text_color(editor_handler& handler, i_editor& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->text_color = view.text_color();
        }
    }

    void editor_handler::map_font(editor_handler& handler, i_editor& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->text_font = view.font();
        }
    }

    void editor_handler::map_character_spacing(editor_handler& handler, i_editor& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->character_spacing = view.character_spacing();
        }
    }

    void editor_handler::map_horizontal_text_alignment(editor_handler& handler, i_editor& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->horizontal_alignment = view.horizontal_text_alignment();
        }
    }

    void editor_handler::map_vertical_text_alignment(editor_handler& handler, i_editor& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->vertical_alignment = view.vertical_text_alignment();
        }
    }

    void editor_handler::map_is_text_prediction_enabled(editor_handler& handler, i_editor& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->is_text_prediction_enabled = view.is_text_prediction_enabled();
        }
    }

    void editor_handler::map_is_spell_check_enabled(editor_handler& handler, i_editor& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->is_spell_check_enabled = view.is_spell_check_enabled();
        }
    }

    void editor_handler::map_cursor_position(editor_handler& handler, i_editor& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->cursor_position = view.cursor_position();
        }
    }

    void editor_handler::map_selection_length(editor_handler& handler, i_editor& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->selection_length = view.selection_length();
        }
    }

    maui::graphics::size editor_handler::get_desired_size(double width_constraint, double /*height_constraint*/) const
    {
        // Headless placeholder metric (no real text layout): a multi-line view ~150pt wide by default,
        // clamped to a finite width constraint, a few lines tall.
        double width = 150.0;
        if (width_constraint > 0 && width_constraint < width)
        {
            width = width_constraint;
        }
        return {width, 66.0};
    }

    void editor_handler::platform_arrange(const maui::graphics::rect& /*frame*/)
    {
        // Headless: no native layout to apply.
    }
} // namespace maui::core
