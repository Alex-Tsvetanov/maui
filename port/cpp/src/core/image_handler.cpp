// image_handler — cross-platform part: the shared mapper table + ctor (ImageHandler.cs, minimal cut). The
// platform recipe (create/map/measure) lives in the per-backend partial.

#include "maui/core/image_handler.hpp"

#include <utility>

#include "maui/core/command_mapper.hpp"
#include "maui/core/i_image.hpp"
#include "maui/core/i_image_source.hpp"
#include "maui/core/image_source_loader.hpp"
#include "maui/core/image_source_result.hpp"
#include "maui/core/property_mapper.hpp"
#include "maui/core/view_handler.hpp"
#include "maui/core/view_mapper.hpp"

namespace maui::core
{
    // Keyed on i_image; maps aspect + source + is_opaque + is_animation_playing. Chained onto the shared
    // view_mapper so the generic IView properties (Visibility/Opacity/IsEnabled/AutomationId) map first
    // (keys() walks the chain first). The keys match the image control's bindable-property names exactly
    // (so a property change drives the right MapXxx — mirrors ImageHandler.Mapper's nameof(IImage.*) keys).
    property_mapper<i_image, image_handler>& image_handler::mapper()
    {
        static property_mapper<i_image, image_handler> table{
            view_mapper(),
            {
                {"aspect", &image_handler::map_aspect},
                {"is_animation_playing", &image_handler::map_is_animation_playing},
                {"is_opaque", &image_handler::map_is_opaque},
                {"source", &image_handler::map_source},
            },
        };
        return table;
    }

    // The type must be qualified inside the body: the method name `command_mapper` shadows the template.
    maui::core::command_mapper<i_image, image_handler>& image_handler::command_mapper()
    {
        static maui::core::command_mapper<i_image, image_handler> table{};
        return table;
    }

    image_handler::image_handler() : view_handler(&mapper(), &command_mapper())
    {
        // Per-backend loader wiring (apple: NSURLSession async fetch + NSCachesDirectory disk cache;
        // headless: a no-op, leaving the synchronous-read_uri_bytes defaults).
        configure_loader(source_loader_);
    }

    // Cross-platform source routing (ImageHandler.MapSource): the file fast-path is synchronous; every
    // other source (uri/stream/font) goes through the handler-owned loader (async, with the source-identity
    // recheck). Only the per-backend primitives (load_file_source_sync / apply_loaded_result /
    // clear_source_native) touch the native view or the headless mirror — the routing + async wiring live
    // here once. The loader pushes the in-flight loading state back via view.update_is_loading (C#'s
    // IsLoading lifecycle); the loading callback captures the view by pointer (the property change that
    // triggered map_source keeps the view alive for the synchronous start, and the loader's liveness token
    // guards the marshalled completion).
    // C# ImageHandler.OnWindowChanged: SourceLoader.SourceManager.RequiresReload(PlatformView) → re-issue
    // the source. The loader compares the current display density against the one captured at load time;
    // when it differs and the last result was resolution-dependent (a font image), re-run map_source so the
    // glyph re-rasterizes at the new density.
    void image_handler::on_window_changed()
    {
        if (source_loader_.requires_reload(query_display_scale()))
        {
            if (auto* view = virtual_view())
            {
                map_source(*this, *view);
            }
        }
    }

    // Push the current display density into the loader (captured at complete_load into CurrentResolution).
    void image_handler::refresh_display_scale()
    {
        source_loader_.set_scale(query_display_scale());
    }

    void image_handler::map_source(image_handler& handler, i_image& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }

        // Capture the current display density so the loader records it at complete_load (the basis for a
        // later RequiresReload comparison — C#'s CompleteLoad reads uiContext.GetDisplayDensity()).
        handler.refresh_display_scale();

        i_image* const view_ptr = &view;
        auto on_loading = [view_ptr](bool is_loading) { view_ptr->update_is_loading(is_loading); };

        i_image_source* const src = view.source();
        if (src == nullptr || src->is_empty())
        {
            // Nothing to load: cancel any in-flight load and clear the view (matches BeginLoad +
            // SetImageSource(null) when imageSource.Source is null). The loader sets IsLoading false.
            handler.source_loader_.update_source(nullptr, nullptr, std::move(on_loading));
            clear_source_native(*platform);
            return;
        }

        // File fast-path: a local file is cheap, so it loads synchronously (kept from the first cut). Still
        // cancel any in-flight async load so a pending uri/stream apply can't clobber the file we just set;
        // then mark loading finished (the sync load completes immediately — C#'s file load still toggles
        // IsLoading true→false, which collapses to "not loading" once the synchronous decode returns).
        if (const auto* file_src = dynamic_cast<const i_file_image_source*>(src))
        {
            handler.source_loader_.update_source(nullptr, nullptr);
            load_file_source_sync(*platform, *file_src);
            view.update_is_loading(false);
            return;
        }

        // Async path (uri / stream / font): hand the source to the loader; the apply runs only if this
        // source is still current when the result arrives (the loader's identity recheck). The apply closure
        // captures the platform view + the handler/view (by pointer — the loader is a handler member, so the
        // liveness token guarding the marshalled apply also guarantees the handler/view outlive it). After
        // the image is applied, the animation state is re-pushed so a freshly-loaded ANIMATED image starts
        // cycling if IsAnimationPlaying is already set (C# SetImageSource → UpdateValue(IsAnimationPlaying)).
        image_handler* const handler_ptr = &handler;
        handler.source_loader_.update_source(
            src,
            [platform, handler_ptr, view_ptr](const image_source_result& result) {
                apply_loaded_result(*platform, result);
                map_is_animation_playing(*handler_ptr, *view_ptr);
            },
            std::move(on_loading));
    }
} // namespace maui::core
