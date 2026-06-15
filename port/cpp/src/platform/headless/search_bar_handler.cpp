// search_bar_handler — headless platform recipe. A testable stand-in for a native search bar: every
// mapped property is mirrored into search_bar_platform, and the inbound hooks (on_text_changed /
// on_search_button_pressed) forward to the virtual view's send_* methods so tests can simulate a native
// edit / search press and observe it flow through to the control's events. The Apple backend
// (src/platform/apple/search_bar_handler.mm) is the real-native twin.

#include "maui/core/search_bar_handler.hpp"

#include <memory>
#include <string>

#include "maui/core/i_search_bar.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

namespace maui::core
{
    // Headless has no native view in the `native` slot, so destruction is trivial.
    search_bar_platform::~search_bar_platform() = default;

    std::unique_ptr<search_bar_platform> search_bar_handler::create_platform_view()
    {
        return std::make_unique<search_bar_platform>();
    }

    void search_bar_handler::on_connect_handler(search_bar_platform& platform)
    {
        // Mirror SearchBarHandler.iOS's MauiSearchBarProxy: TextSetOrChanged/EditingChanged →
        // send_text_changed(old, new); SearchButtonClicked → SearchButtonPressed. Headless tests invoke
        // these directly.
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
        platform.on_search_button_pressed = [this] {
            if (auto* view = virtual_view())
            {
                view->send_search_button_pressed();
            }
        };
    }

    void search_bar_handler::on_disconnect_handler(search_bar_platform& platform)
    {
        platform.on_text_changed = nullptr;
        platform.on_search_button_pressed = nullptr;
    }

    void search_bar_handler::map_text(search_bar_handler& handler, i_search_bar& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->text = std::string(view.text());
            platform->last_known_text = platform->text;
        }
    }

    void search_bar_handler::map_placeholder(search_bar_handler& handler, i_search_bar& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->placeholder = std::string(view.placeholder());
        }
    }

    void search_bar_handler::map_placeholder_color(search_bar_handler& handler, i_search_bar& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->placeholder_color = view.placeholder_color();
        }
    }

    void search_bar_handler::map_is_read_only(search_bar_handler& handler, i_search_bar& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->is_read_only = view.is_read_only();
        }
    }

    void search_bar_handler::map_max_length(search_bar_handler& handler, i_search_bar& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->max_length = view.max_length();
        }
    }

    void search_bar_handler::map_text_color(search_bar_handler& handler, i_search_bar& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->text_color = view.text_color();
        }
    }

    void search_bar_handler::map_font(search_bar_handler& handler, i_search_bar& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->text_font = view.font();
        }
    }

    void search_bar_handler::map_character_spacing(search_bar_handler& handler, i_search_bar& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->character_spacing = view.character_spacing();
        }
    }

    void search_bar_handler::map_horizontal_text_alignment(search_bar_handler& handler, i_search_bar& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->horizontal_alignment = view.horizontal_text_alignment();
        }
    }

    void search_bar_handler::map_vertical_text_alignment(search_bar_handler& handler, i_search_bar& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->vertical_alignment = view.vertical_text_alignment();
        }
    }

    void search_bar_handler::map_is_text_prediction_enabled(search_bar_handler& handler, i_search_bar& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->is_text_prediction_enabled = view.is_text_prediction_enabled();
        }
    }

    void search_bar_handler::map_is_spell_check_enabled(search_bar_handler& handler, i_search_bar& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->is_spell_check_enabled = view.is_spell_check_enabled();
        }
    }

    void search_bar_handler::map_keyboard(search_bar_handler& handler, i_search_bar& view)
    {
        // Headless keeps the mirror only (no soft keyboard); the iOS twin pushes UIKeyboardType + traits.
        if (auto* platform = handler.typed_platform_view())
        {
            platform->keyboard = view.keyboard();
        }
    }

    void search_bar_handler::map_cursor_position(search_bar_handler& handler, i_search_bar& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->cursor_position = view.cursor_position();
        }
    }

    void search_bar_handler::map_selection_length(search_bar_handler& handler, i_search_bar& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->selection_length = view.selection_length();
        }
    }

    void search_bar_handler::map_cancel_button_color(search_bar_handler& handler, i_search_bar& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->cancel_button_color = view.cancel_button_color();
        }
    }

    void search_bar_handler::map_search_icon_color(search_bar_handler& handler, i_search_bar& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->search_icon_color = view.search_icon_color();
        }
    }

    void search_bar_handler::map_return_type(search_bar_handler& handler, i_search_bar& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->bar_return_type = view.return_type();
        }
    }

    maui::graphics::size search_bar_handler::get_desired_size(double width_constraint,
                                                              double /*height_constraint*/) const
    {
        // Headless placeholder metric: a single-line bar ~200pt wide by default, clamped to a finite
        // width constraint, fixed line height.
        double width = 200.0;
        if (width_constraint > 0 && width_constraint < width)
        {
            width = width_constraint;
        }
        return {width, 30.0};
    }

    void search_bar_handler::platform_arrange(const maui::graphics::rect& /*frame*/)
    {
        // Headless: no native layout to apply.
    }
} // namespace maui::core
