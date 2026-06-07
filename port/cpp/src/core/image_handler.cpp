// image_handler — cross-platform part: the shared mapper table + ctor (ImageHandler.cs, minimal cut). The
// platform recipe (create/map/measure) lives in the per-backend partial.

#include "maui/core/image_handler.hpp"

#include "maui/core/command_mapper.hpp"
#include "maui/core/i_image.hpp"
#include "maui/core/property_mapper.hpp"
#include "maui/core/view_handler.hpp"

namespace maui::core
{
    // Keyed on i_image; this minimal cut maps only aspect (Source / IsAnimationPlaying / IsOpaque are the
    // deferred async-source subsystem).
    property_mapper<i_image, image_handler>& image_handler::mapper()
    {
        static property_mapper<i_image, image_handler> table{
            {"aspect", &image_handler::map_aspect},
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
