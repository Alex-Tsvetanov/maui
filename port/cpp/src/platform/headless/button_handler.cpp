// button_handler — headless platform recipe. A testable stand-in for a native button: text is mirrored
// into button_platform::title, and the inbound event callbacks (on_click/on_press/on_release) are wired
// to the virtual view's send_* methods so tests can simulate a tap and observe it flow through to the
// control's events. The Apple backend (src/platform/apple/button_handler.mm) is the real-native twin.

#include "maui/core/button_handler.hpp"

#include <memory>
#include <string>

#include "maui/core/i_button.hpp"
#include "maui/core/i_text_button.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

namespace maui::core
{
    std::unique_ptr<button_platform> button_handler::create_platform_view()
    {
        return std::make_unique<button_platform>();
    }

    void button_handler::on_connect_handler(button_platform& platform)
    {
        // Mirror ButtonHandler.iOS's ButtonEventProxy: TouchDown → Pressed; TouchUpInside →
        // Released + Clicked; TouchUpOutside/Cancel → Released. Headless tests invoke these directly.
        platform.on_press = [this] {
            if (auto* view = virtual_view())
            {
                view->send_pressed();
            }
        };
        platform.on_release = [this] {
            if (auto* view = virtual_view())
            {
                view->send_released();
            }
        };
        platform.on_click = [this] {
            if (auto* view = virtual_view())
            {
                view->send_released();
                view->send_clicked();
            }
        };
    }

    void button_handler::on_disconnect_handler(button_platform& platform)
    {
        platform.on_press = nullptr;
        platform.on_release = nullptr;
        platform.on_click = nullptr;
    }

    void button_handler::map_text(button_handler& handler, i_text_button& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->title = std::string(view.text());
        }
    }

    maui::graphics::size button_handler::get_desired_size(double /*width_constraint*/,
                                                          double /*height_constraint*/) const
    {
        // Headless placeholder metric (no real text layout): ~8pt per character, fixed height.
        const auto* platform = typed_platform_view();
        const double width = platform != nullptr ? static_cast<double>(platform->title.size()) * 8.0 : 0.0;
        return {width, 20.0};
    }

    void button_handler::platform_arrange(const maui::graphics::rect& /*frame*/)
    {
        // Headless: no native layout to apply.
    }
} // namespace maui::core
