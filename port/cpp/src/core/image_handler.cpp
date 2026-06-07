// image_handler — cross-platform part: the shared mapper table + ctor (ImageHandler.cs, minimal cut). The
// platform recipe (create/map/measure) lives in the per-backend partial.

#include "maui/core/image_handler.hpp"

#include "maui/core/command_mapper.hpp"
#include "maui/core/i_image.hpp"
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
} // namespace maui::core
