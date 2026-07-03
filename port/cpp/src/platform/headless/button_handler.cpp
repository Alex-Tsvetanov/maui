// button_handler — headless platform recipe. A testable stand-in for a native button: text is mirrored
// into button_platform::title, and the inbound event callbacks (on_click/on_press/on_release) are wired
// to the virtual view's send_* methods so tests can simulate a tap and observe it flow through to the
// control's events. The Apple backend (src/platform/apple/button_handler.mm) is the real-native twin.

#include "maui/core/button_handler.hpp"

#include <memory>
#include <string>

#include "maui/core/i_button.hpp"
#include "maui/core/i_image_source.hpp"
#include "maui/core/i_text_button.hpp"
#include "maui/core/image_source_loader.hpp"
#include "maui/core/image_source_result.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

namespace maui::core
{
    // Headless has no native view in the `native` slot, so destruction is trivial.
    button_platform::~button_platform() = default;

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

    void button_handler::map_text_color(button_handler& handler, i_text_button& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->text_color = view.text_color();
        }
    }

    void button_handler::map_font(button_handler& handler, i_text_button& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->text_font = view.font();
        }
    }

    void button_handler::map_character_spacing(button_handler& handler, i_text_button& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->character_spacing = view.character_spacing();
        }
    }

    void button_handler::map_padding(button_handler& handler, i_button& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->padding = view.padding();
        }
    }

    void button_handler::map_stroke_color(button_handler& handler, i_button& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->stroke_color = view.stroke_color();
        }
    }

    void button_handler::map_stroke_thickness(button_handler& handler, i_button& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->stroke_thickness = view.stroke_thickness();
        }
    }

    void button_handler::map_corner_radius(button_handler& handler, i_button& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->corner_radius = view.corner_radius();
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

    // Headless mirrors the cross-platform ResolveConstraints contract (an explicit size request clamps the
    // measured size) — the intrinsic-content floor is a native iOS/macOS behavior, not a unit-test one.
    bool button_handler::content_is_minimum_size() const
    {
        return false;
    }

    void button_handler::platform_arrange(const maui::graphics::rect& /*frame*/)
    {
        // Headless: no native layout to apply.
    }

    // ---- per-backend image-source primitives (the cross-platform map_image_source routes here) ----
    // Headless records the source mirrors (kind/file/loaded) so tests observe the load; there is no
    // display. Mirrors image_handler / image_button_handler's headless source primitives.

    // Headless: leave the loader on its defaults (synchronous read_uri_bytes, disk layer off).
    void button_handler::configure_loader(maui::core::image_source_loader& /*loader*/)
    {
    }

    void button_handler::load_file_source_sync(button_platform& platform, const i_file_image_source& file_src)
    {
        platform.source_kind = "file";
        platform.source_file = std::string(file_src.file());
        platform.source_loaded = true;
    }

    void button_handler::apply_loaded_result(button_platform& platform, const image_source_result& result)
    {
        if (!result.loaded())
        {
            clear_source_native(platform);
            return;
        }
        platform.source_kind = result.kind();
        platform.source_file = result.detail();
        platform.source_loaded = true;
    }

    void button_handler::clear_source_native(button_platform& platform)
    {
        platform.source_kind.clear();
        platform.source_file.clear();
        platform.source_loaded = false;
    }
} // namespace maui::core
