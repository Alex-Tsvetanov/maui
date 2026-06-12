// graphics_view_handler — cross-platform part: the shared mapper + command tables, the ctor, and
// the measure/arrange seam (GraphicsViewHandler.cs). The platform recipe (create + update_drawable +
// invalidate_drawable + arrange_native) lives in the per-backend partial.

#include "maui/core/graphics_view_handler.hpp"

#include <any>

#include "maui/core/command_mapper.hpp"
#include "maui/core/i_graphics_view.hpp"
#include "maui/core/property_mapper.hpp"
#include "maui/core/view_handler.hpp"
#include "maui/core/view_mapper.hpp"
#include "maui/graphics/paint.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

namespace maui::core
{
    // Mirrors C# GraphicsViewHandler.Mapper: Drawable plus the Background / FlowDirection overrides
    // (which ALSO invalidate the drawable), chained onto the shared view_mapper so the remaining
    // generic IView properties map first.
    property_mapper<i_graphics_view, graphics_view_handler>& graphics_view_handler::mapper()
    {
        static property_mapper<i_graphics_view, graphics_view_handler> table{
            view_mapper(),
            {
                {"background", &graphics_view_handler::map_background},
                {"drawable", &graphics_view_handler::map_drawable},
                {"flow_direction", &graphics_view_handler::map_flow_direction},
            },
        };
        return table;
    }

    // C# GraphicsViewHandler.CommandMapper: Invalidate.
    maui::core::command_mapper<i_graphics_view, graphics_view_handler>& graphics_view_handler::command_mapper()
    {
        static maui::core::command_mapper<i_graphics_view, graphics_view_handler> table{
            {"invalidate", &graphics_view_handler::map_invalidate},
        };
        return table;
    }

    graphics_view_handler::graphics_view_handler() : view_handler(&mapper(), &command_mapper())
    {
    }

    maui::graphics::size graphics_view_handler::get_desired_size(double /*width_constraint*/,
                                                                 double /*height_constraint*/) const
    {
        // The base C# GetDesiredSize asks the native host, and a plain canvas host has no intrinsic
        // size — the developer's size requests drive the layout.
        return {0, 0};
    }

    void graphics_view_handler::platform_arrange(const maui::graphics::rect& frame)
    {
        arrange_native(frame);
    }

    // C# MapDrawable → UpdateDrawable.
    void graphics_view_handler::map_drawable(graphics_view_handler& handler, i_graphics_view& /*view*/)
    {
        handler.update_drawable();
    }

    // C# MapBackground (iOS): a set background invalidates the drawable. The chained generic push is
    // replaced (this entry overrides the view_mapper's "background" key), exactly like C#'s mapper
    // replacement — the drawable is expected to render the background itself.
    void graphics_view_handler::map_background(graphics_view_handler& handler, i_graphics_view& view)
    {
        if (view.background() != nullptr)
        {
            handler.invalidate_drawable();
        }
    }

    // C# MapFlowDirection (iOS): push the platform flow direction, then invalidate.
    void graphics_view_handler::map_flow_direction(graphics_view_handler& handler, i_graphics_view& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->update_flow_direction(view.flow_direction());
        }
        handler.invalidate_drawable();
    }

    // C# MapInvalidate (command) → InvalidateDrawable.
    void graphics_view_handler::map_invalidate(graphics_view_handler& handler, i_graphics_view& /*view*/,
                                               const std::any& /*args*/)
    {
        handler.invalidate_drawable();
    }
} // namespace maui::core
