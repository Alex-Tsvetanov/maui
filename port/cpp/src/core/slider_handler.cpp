// slider_handler — cross-platform part: the shared mapper tables, the ctor, and the cross-platform
// ThumbImageSource routing (through the handler-owned image_source_loader). The platform recipe
// (create/connect/disconnect/map_*/measure + the native thumb-image apply / UpdateOnTap gesture) lives
// in the per-backend partial. Ported from SliderHandler.cs + SliderExtensions.UpdateThumbImageSourceAsync
// + Slider.Mapper.cs's UpdateOnTap remap.

#include "maui/core/slider_handler.hpp"

#include "maui/core/command_mapper.hpp"
#include "maui/core/i_image_source.hpp"
#include "maui/core/i_slider.hpp"
#include "maui/core/image_source_loader.hpp"
#include "maui/core/image_source_result.hpp"
#include "maui/core/property_mapper.hpp"
#include "maui/core/view_handler.hpp"
#include "maui/core/view_mapper.hpp"

namespace maui::core
{
    // SliderHandler.Mapper (C# key order): Maximum / MaximumTrackColor / Minimum / MinimumTrackColor /
    // ThumbColor / ThumbImageSource / Value over ViewHandler.ViewMapper, plus the iOSSpecific UpdateOnTap
    // remap (Slider.Mapper.cs ReplaceMapping). The UpdateOnTap key is the namespaced platform-spec key, so
    // setting iOSSpecific.Slider.UpdateOnTap re-runs map_update_on_tap (the platform-config seam, W2-24).
    property_mapper<i_slider, slider_handler>& slider_handler::mapper()
    {
        static property_mapper<i_slider, slider_handler> table{
            view_mapper(),
            {
                {"maximum", &slider_handler::map_maximum},
                {"maximum_track_color", &slider_handler::map_maximum_track_color},
                {"minimum", &slider_handler::map_minimum},
                {"minimum_track_color", &slider_handler::map_minimum_track_color},
                {"thumb_color", &slider_handler::map_thumb_color},
                {"thumb_image_source", &slider_handler::map_thumb_image_source},
                {"value", &slider_handler::map_value},
                {"ios.Slider.UpdateOnTap", &slider_handler::map_update_on_tap},
            }};
        return table;
    }

    // No slider-specific commands (C#'s CommandMapper is empty). Qualified return type: the method name
    // `command_mapper` shadows the `command_mapper` template.
    maui::core::command_mapper<i_slider, slider_handler>& slider_handler::command_mapper()
    {
        static maui::core::command_mapper<i_slider, slider_handler> table{};
        return table;
    }

    slider_handler::slider_handler() : view_handler(&mapper(), &command_mapper())
    {
        // Per-backend loader wiring (apple/ios: NSURLSession async fetch + cache dir; headless: a no-op).
        configure_thumb_loader(thumb_image_loader_);
    }

    // C# SliderHandler.DisconnectHandler teardown: cancel the in-flight thumb-image load BEFORE the base
    // destroys the platform view, so a queued apply closure (which captured the platform by raw pointer)
    // is gated out by the loader's cancelled token instead of dereferencing the freed platform. See the
    // header note. update_source(nullptr, nullptr) calls begin_load() → cancels the previous token; the
    // already-queued deliver() closure then bails on its `!token.is_cancelled()` recheck.
    void slider_handler::disconnect_handler()
    {
        thumb_image_loader_.update_source(nullptr, nullptr);
        view_handler::disconnect_handler();
    }

    // SliderHandler.MapThumbImageSource / SliderExtensions.UpdateThumbImageSourceAsync: a null/empty
    // source clears the native thumb image and falls back to the thumb color (UpdateThumbColor's else
    // branch); a real source loads through the handler-owned loader and, when the result arrives and is
    // still current (the loader's source-identity recheck), applies it as the thumb image. The apply
    // closure captures the platform + view by pointer — the loader is a handler member, so its liveness
    // token guards the marshalled apply and the view outlives it (the view owns the handler).
    void slider_handler::map_thumb_image_source(slider_handler& handler, i_slider& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        i_image_source* const src = view.thumb_image_source();
        if (src == nullptr || src->is_empty())
        {
            // No image: cancel any in-flight load and restore the thumb color (the C# else branch).
            handler.thumb_image_loader_.update_source(nullptr, nullptr);
            clear_thumb_image(*platform, view);
            return;
        }
        i_slider* const view_ptr = &view;
        handler.thumb_image_loader_.update_source(src, [platform, view_ptr](const image_source_result& result) {
            if (result.loaded())
            {
                apply_thumb_image(*platform, *view_ptr, result);
            }
            else
            {
                clear_thumb_image(*platform, *view_ptr);
            }
        });
    }
} // namespace maui::core
