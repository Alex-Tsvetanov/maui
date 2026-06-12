// radio_button_handler — headless platform recipe. A testable stand-in for a native radio button: every
// mapped property mirrors into radio_button_platform, and the inbound on_select hook forwards a
// simulated native tap to the virtual view's send_is_checked(true) (a radio tap SELECTS — the group's
// mutual exclusion unchecks the others at the Controls layer). The Apple backend
// (src/platform/apple/radio_button_handler.mm) is the real-native twin.

#include "maui/core/radio_button_handler.hpp"

#include <memory>
#include <string>

#include "maui/core/i_radio_button.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

namespace maui::core
{
    // Headless has no native view in the `native` slot, so destruction just clears the unused slot
    // (an explicit body mirroring the Apple RAII shape).
    radio_button_platform::~radio_button_platform()
    {
        native = nullptr;
    }

    std::unique_ptr<radio_button_platform> radio_button_handler::create_platform_view()
    {
        return std::make_unique<radio_button_platform>();
    }

    void radio_button_handler::on_connect_handler(radio_button_platform& platform)
    {
        // RadioButton.SelectRadioButton: a native tap checks the button (from-handler write); the
        // mapped mirror then follows through map_is_checked.
        platform.on_select = [this] {
            if (auto* view = virtual_view())
            {
                view->send_is_checked(true);
            }
        };
    }

    void radio_button_handler::on_disconnect_handler(radio_button_platform& platform)
    {
        platform.on_select = nullptr;
    }

    void radio_button_handler::map_is_checked(radio_button_handler& handler, i_radio_button& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->is_checked = view.is_checked();
        }
    }

    void radio_button_handler::map_content(radio_button_handler& handler, i_radio_button& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->content = std::string(view.content_as_string());
        }
    }

    void radio_button_handler::map_text_color(radio_button_handler& handler, i_radio_button& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->text_color = view.text_color();
        }
    }

    void radio_button_handler::map_character_spacing(radio_button_handler& handler, i_radio_button& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->character_spacing = view.character_spacing();
        }
    }

    void radio_button_handler::map_font(radio_button_handler& handler, i_radio_button& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->text_font = view.font();
        }
    }

    void radio_button_handler::map_stroke_color(radio_button_handler& handler, i_radio_button& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->stroke_color = view.stroke_color();
        }
    }

    void radio_button_handler::map_stroke_thickness(radio_button_handler& handler, i_radio_button& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->stroke_thickness = view.stroke_thickness();
        }
    }

    void radio_button_handler::map_corner_radius(radio_button_handler& handler, i_radio_button& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->corner_radius = view.corner_radius();
        }
    }

    maui::graphics::size radio_button_handler::get_desired_size(double /*width_constraint*/,
                                                                double /*height_constraint*/) const
    {
        // No native control headless, so there is no intrinsic content size to report.
        return {0, 0};
    }

    void radio_button_handler::platform_arrange(const maui::graphics::rect& /*frame*/)
    {
        // Headless: no native layout to apply.
    }
} // namespace maui::core
