// entry_handler — headless platform recipe. A testable stand-in for an editable native field: every
// mapped property is mirrored into entry_platform, and the inbound hooks (on_text_changed / on_completed)
// forward to the virtual view's send_* methods so tests can simulate a native edit / end-of-edit and
// observe it flow through to the control's events. The Apple backend (src/platform/apple/entry_handler.mm)
// is the real-native twin.

#include "maui/core/entry_handler.hpp"
#include "maui/core/i_ios_entry_specifics.hpp" // --- platform configuration (W2-24) ---

#include <memory>
#include <string>

#include "maui/core/i_entry.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

namespace maui::core
{
    // Headless has no native view in the `native` slot, so destruction is trivial.
    entry_platform::~entry_platform() = default;

    std::unique_ptr<entry_platform> entry_handler::create_platform_view()
    {
        return std::make_unique<entry_platform>();
    }

    void entry_handler::on_connect_handler(entry_platform& platform)
    {
        // Mirror EntryHandler.iOS's MauiTextFieldProxy: EditingChanged → send_text_changed(old, new);
        // EditingDidEnd / ShouldReturn → Completed. Headless tests invoke these directly.
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

    void entry_handler::on_disconnect_handler(entry_platform& platform)
    {
        platform.on_text_changed = nullptr;
        platform.on_completed = nullptr;
    }

    void entry_handler::map_text(entry_handler& handler, i_entry& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->text = std::string(view.text());
            platform->last_known_text = platform->text;
        }
    }

    void entry_handler::map_placeholder(entry_handler& handler, i_entry& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->placeholder = std::string(view.placeholder());
        }
    }

    void entry_handler::map_placeholder_color(entry_handler& handler, i_entry& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->placeholder_color = view.placeholder_color();
        }
    }

    void entry_handler::map_is_password(entry_handler& handler, i_entry& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->is_password = view.is_password();
        }
    }

    void entry_handler::map_is_read_only(entry_handler& handler, i_entry& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->is_read_only = view.is_read_only();
        }
    }

    void entry_handler::map_max_length(entry_handler& handler, i_entry& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->max_length = view.max_length();
        }
    }

    void entry_handler::map_text_color(entry_handler& handler, i_entry& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->text_color = view.text_color();
        }
    }

    void entry_handler::map_font(entry_handler& handler, i_entry& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->text_font = view.font();
        }
    }

    void entry_handler::map_character_spacing(entry_handler& handler, i_entry& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->character_spacing = view.character_spacing();
        }
    }

    void entry_handler::map_horizontal_text_alignment(entry_handler& handler, i_entry& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->horizontal_alignment = view.horizontal_text_alignment();
        }
    }

    void entry_handler::map_vertical_text_alignment(entry_handler& handler, i_entry& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->vertical_alignment = view.vertical_text_alignment();
        }
    }

    void entry_handler::map_is_text_prediction_enabled(entry_handler& handler, i_entry& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->is_text_prediction_enabled = view.is_text_prediction_enabled();
        }
    }

    void entry_handler::map_is_spell_check_enabled(entry_handler& handler, i_entry& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->is_spell_check_enabled = view.is_spell_check_enabled();
        }
    }

    void entry_handler::map_keyboard(entry_handler& handler, i_entry& view)
    {
        // Headless keeps the mirror only (no soft keyboard); the iOS twin pushes UIKeyboardType + traits.
        if (auto* platform = handler.typed_platform_view())
        {
            platform->keyboard = view.keyboard();
        }
    }

    void entry_handler::map_return_type(entry_handler& handler, i_entry& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->entry_return_type = view.return_type();
        }
    }

    void entry_handler::map_clear_button_visibility(entry_handler& handler, i_entry& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->clear_button = view.clear_button_visibility();
        }
    }

    void entry_handler::map_cursor_position(entry_handler& handler, i_entry& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->cursor_position = view.cursor_position();
        }
    }

    void entry_handler::map_selection_length(entry_handler& handler, i_entry& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->selection_length = view.selection_length();
        }
    }

    // --- platform configuration (W2-24): the iOSSpecific Entry.CursorColor map — headless keeps the
    // mirror only, guarded by the IsSet probe like TextExtensions.UpdateCursorColor (an untouched entry
    // leaves the mirror nullopt). The value crosses on the i_ios_entry_specifics face.
    void entry_handler::map_cursor_color(entry_handler& handler, i_entry& view)
    {
        auto* platform = handler.typed_platform_view();
        const auto* specifics = dynamic_cast<const i_ios_entry_specifics*>(&view);
        if (platform == nullptr || specifics == nullptr || !specifics->cursor_color_set())
        {
            return;
        }
        platform->cursor_color = specifics->cursor_color();
    }

    maui::graphics::size entry_handler::get_desired_size(double width_constraint, double /*height_constraint*/) const
    {
        // Headless placeholder metric (no real text layout): a single-line field ~150pt wide by default,
        // clamped to a finite width constraint, fixed line height.
        double width = 150.0;
        if (width_constraint > 0 && width_constraint < width)
        {
            width = width_constraint;
        }
        return {width, 22.0};
    }

    void entry_handler::platform_arrange(const maui::graphics::rect& /*frame*/)
    {
        // Headless: no native layout to apply.
    }
} // namespace maui::core
