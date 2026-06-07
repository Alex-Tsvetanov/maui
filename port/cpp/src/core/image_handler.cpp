// image_handler — cross-platform part: the shared mapper table + ctor (ImageHandler.cs, minimal cut). The
// platform recipe (create/map/measure) lives in the per-backend partial.

#include "maui/core/image_handler.hpp"

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
    // Keyed on i_image; this cut maps aspect + source (IsAnimationPlaying / IsOpaque remain deferred).
    // Chained onto the shared view_mapper so the generic IView properties (Visibility/Opacity/IsEnabled/
    // AutomationId) map first (keys() walks the chain first). The "aspect"/"source" keys match the image
    // control's bindable-property names.
    property_mapper<i_image, image_handler>& image_handler::mapper()
    {
        static property_mapper<i_image, image_handler> table{
            view_mapper(),
            {
                {"aspect", &image_handler::map_aspect},
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
    }

    // Cross-platform source routing (ImageHandler.MapSource): the file fast-path is synchronous; every
    // other source goes through the handler-owned loader (async, with the source-identity recheck). Only
    // the per-backend primitives (load_file_source_sync / apply_loaded_result / clear_source_native) touch
    // the native view or the headless mirror — the routing + async wiring live here once.
    void image_handler::map_source(image_handler& handler, i_image& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }

        i_image_source* const src = view.source();
        if (src == nullptr || src->is_empty())
        {
            // Nothing to load: cancel any in-flight load and clear the view (matches BeginLoad +
            // SetImageSource(null) when imageSource.Source is null).
            handler.source_loader_.update_source(nullptr, nullptr);
            clear_source_native(*platform);
            return;
        }

        // File fast-path: a local file is cheap, so it loads synchronously (kept from the first cut). Still
        // cancel any in-flight async load so a pending uri/stream apply can't clobber the file we just set.
        if (const auto* file_src = dynamic_cast<const i_file_image_source*>(src))
        {
            handler.source_loader_.update_source(nullptr, nullptr);
            load_file_source_sync(*platform, *file_src);
            return;
        }

        // Async path (uri / stream): hand the source to the loader; the apply runs only if this source is
        // still current when the result arrives (the loader's identity recheck). The closure captures the
        // platform view; the loader's liveness token guards against the handler being torn down first.
        handler.source_loader_.update_source(
            src, [platform](const image_source_result& result) { apply_loaded_result(*platform, result); });
    }
} // namespace maui::core
