// image_button_handler — headless platform recipe. A testable stand-in for a native image button: the
// image surface mirrors into image_button_platform (the image_handler convention) and the inbound
// touch hooks (on_click/on_press/on_release) forward to the virtual view's send_* methods (the
// button_handler convention). The Apple backend (src/platform/apple/image_button_handler.mm) is the
// real-native twin.

#include "maui/core/image_button_handler.hpp"

#include <memory>
#include <string>

#include "maui/core/i_image_button.hpp"
#include "maui/core/i_image_source.hpp"
#include "maui/core/image_source_loader.hpp" // configure_loader parameter type
#include "maui/core/image_source_result.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

namespace maui::core
{
    // Headless has no native view in the `native` slot, so destruction just clears the unused slot
    // (the image_platform convention — an explicit body mirroring the Apple RAII shape).
    image_button_platform::~image_button_platform()
    {
        native = nullptr;
    }

    std::unique_ptr<image_button_platform> image_button_handler::create_platform_view()
    {
        return std::make_unique<image_button_platform>();
    }

    // Headless: leave the loader on its defaults (synchronous read_uri_bytes, disk layer off).
    void image_button_handler::configure_loader(image_source_loader& /*loader*/)
    {
    }

    void image_button_handler::on_connect_handler(image_button_platform& platform)
    {
        // Mirror ImageButtonHandler.iOS's ImageButtonProxy: TouchDown → Pressed; TouchUpInside →
        // Released + Clicked; TouchUpOutside → Released. Headless tests invoke these directly.
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

    void image_button_handler::on_disconnect_handler(image_button_platform& platform)
    {
        platform.on_press = nullptr;
        platform.on_release = nullptr;
        platform.on_click = nullptr;
    }

    void image_button_handler::map_aspect(image_button_handler& handler, i_image_button& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->image_aspect = view.aspect();
        }
    }

    void image_button_handler::map_is_opaque(image_button_handler& handler, i_image_button& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->opaque = view.is_opaque();
        }
    }

    // IsAnimationPlaying (headless mirror; the Apple/iOS twins push to the native button's image). The
    // image_handler convention — ImageButton pins the value false, so the mirror stays false in practice.
    void image_button_handler::map_is_animation_playing(image_button_handler& handler, i_image_button& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->animation_playing = view.is_animation_playing();
        }
    }

    void image_button_handler::map_padding(image_button_handler& handler, i_image_button& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->padding = view.padding();
        }
    }

    void image_button_handler::map_stroke_color(image_button_handler& handler, i_image_button& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->stroke_color = view.stroke_color();
        }
    }

    void image_button_handler::map_stroke_thickness(image_button_handler& handler, i_image_button& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->stroke_thickness = view.stroke_thickness();
        }
    }

    void image_button_handler::map_corner_radius(image_button_handler& handler, i_image_button& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->corner_radius = view.corner_radius();
        }
    }

    // ---- per-backend source primitives (the cross-platform map_source routes here) ----

    void image_button_handler::load_file_source_sync(image_button_platform& platform,
                                                     const i_file_image_source& file_src)
    {
        platform.source_kind = "file";
        platform.source_file = std::string(file_src.file());
        platform.source_loaded = true;
    }

    void image_button_handler::apply_loaded_result(image_button_platform& platform, const image_source_result& result)
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

    void image_button_handler::clear_source_native(image_button_platform& platform)
    {
        platform.source_kind.clear();
        platform.source_file.clear();
        platform.source_loaded = false;
    }

    maui::graphics::size image_button_handler::get_desired_size(double /*width_constraint*/,
                                                                double /*height_constraint*/) const
    {
        // No image bytes are decoded headless, so there is no intrinsic content size to report.
        return {0, 0};
    }

    void image_button_handler::platform_arrange(const maui::graphics::rect& /*frame*/)
    {
        // Headless: no native layout to apply.
    }
} // namespace maui::core
