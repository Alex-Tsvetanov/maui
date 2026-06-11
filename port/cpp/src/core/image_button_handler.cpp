// image_button_handler — cross-platform part: the shared mapper tables + ctor + the source routing
// (ImageButtonHandler.cs). The platform recipe (create/connect/disconnect/map/measure + the source
// primitives) lives in the per-backend partial.

#include "maui/core/image_button_handler.hpp"

#include <utility>

#include "maui/core/command_mapper.hpp"
#include "maui/core/i_image_button.hpp"
#include "maui/core/i_image_source.hpp"
#include "maui/core/image_source_loader.hpp"
#include "maui/core/image_source_result.hpp"
#include "maui/core/property_mapper.hpp"
#include "maui/core/view_handler.hpp"
#include "maui/core/view_mapper.hpp"

namespace maui::core
{
    // ImageButtonHandler.Mapper chains ImageMapper (= ImageHandler.Mapper) and adds the IButtonStroke +
    // Padding keys. The port's contract flattens i_image into i_image_button (see i_image_button.hpp),
    // so the chain collapses to listing the image keys here, in the chained-first position (the image
    // keys before the button's own keys, after the generic IView keys from the shared view_mapper).
    property_mapper<i_image_button, image_button_handler>& image_button_handler::mapper()
    {
        static property_mapper<i_image_button, image_button_handler> table{
            view_mapper(),
            {
                // ---- the ImageMapper chain (image_handler's keys, keyed on i_image_button) ----
                {"aspect", &image_button_handler::map_aspect},
                {"is_opaque", &image_button_handler::map_is_opaque},
                {"source", &image_button_handler::map_source},
                // ---- ImageButtonHandler.Mapper's own keys ----
                {"stroke_thickness", &image_button_handler::map_stroke_thickness},
                {"stroke_color", &image_button_handler::map_stroke_color},
                {"corner_radius", &image_button_handler::map_corner_radius},
                {"padding", &image_button_handler::map_padding},
            },
        };
        return table;
    }

    // The type must be qualified inside the body: the method name `command_mapper` shadows the template.
    maui::core::command_mapper<i_image_button, image_button_handler>& image_button_handler::command_mapper()
    {
        static maui::core::command_mapper<i_image_button, image_button_handler> table{};
        return table;
    }

    image_button_handler::image_button_handler() : view_handler(&mapper(), &command_mapper())
    {
        // Per-backend loader wiring (the image_handler convention).
        configure_loader(source_loader_);
    }

    // Cross-platform source routing — image_handler::map_source's twin over the image_button platform
    // primitives (ImageButtonHandler shares ImageHandler's MapSource through IImageHandler): the file
    // fast-path is synchronous; every other source (uri/stream/font) goes through the handler-owned
    // loader (async, with the source-identity recheck); a null/empty source cancels + clears.
    void image_button_handler::map_source(image_button_handler& handler, i_image_button& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }

        i_image_button* const view_ptr = &view;
        auto on_loading = [view_ptr](bool is_loading) { view_ptr->update_is_loading(is_loading); };

        i_image_source* const src = view.source();
        if (src == nullptr || src->is_empty())
        {
            handler.source_loader_.update_source(nullptr, nullptr, std::move(on_loading));
            clear_source_native(*platform);
            return;
        }

        if (const auto* file_src = dynamic_cast<const i_file_image_source*>(src))
        {
            handler.source_loader_.update_source(nullptr, nullptr);
            load_file_source_sync(*platform, *file_src);
            view.update_is_loading(false);
            return;
        }

        handler.source_loader_.update_source(
            src, [platform](const image_source_result& result) { apply_loaded_result(*platform, result); },
            std::move(on_loading));
    }
} // namespace maui::core
