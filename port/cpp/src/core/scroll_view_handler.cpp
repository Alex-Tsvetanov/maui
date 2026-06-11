// scroll_view_handler — cross-platform part: the shared mapper + command tables and the ctor
// (ScrollViewHandler.cs). The platform recipe (create + content/orientation/bar pushes + scroll_to +
// platform_arrange) lives in the per-backend partial.

#include "maui/core/scroll_view_handler.hpp"

#include <any>

#include "maui/core/command_mapper.hpp"
#include "maui/core/i_scroll_view.hpp"
#include "maui/core/property_mapper.hpp"
#include "maui/core/scroll_to_request.hpp"
#include "maui/core/view_handler.hpp"
#include "maui/core/view_mapper.hpp"

namespace maui::core
{
    // Mirrors C# ScrollViewHandler.Mapper: Content + the two scroll-bar visibilities + Orientation,
    // chained onto the shared view_mapper (C# builds on ViewMapper the same way). The iOS-only IsEnabled
    // re-map collapses into update_orientation (both funnel to the same native scroll-ability there).
    property_mapper<i_scroll_view, scroll_view_handler>& scroll_view_handler::mapper()
    {
        static property_mapper<i_scroll_view, scroll_view_handler> table{
            view_mapper(),
            {
                {"content", &scroll_view_handler::map_content},
                {"horizontal_scroll_bar_visibility", &scroll_view_handler::map_horizontal_scroll_bar_visibility},
                {"vertical_scroll_bar_visibility", &scroll_view_handler::map_vertical_scroll_bar_visibility},
                {"orientation", &scroll_view_handler::map_orientation},
            },
        };
        return table;
    }

    // Mirrors C# ScrollViewHandler.CommandMapper: RequestScrollTo — plus the port's "set_content"
    // runtime funnel every content host shares (C#'s Content set re-enters the property path instead).
    maui::core::command_mapper<i_scroll_view, scroll_view_handler>& scroll_view_handler::command_mapper()
    {
        static maui::core::command_mapper<i_scroll_view, scroll_view_handler> table{
            {"request_scroll_to", &scroll_view_handler::map_request_scroll_to},
            {"set_content", &scroll_view_handler::map_set_content},
        };
        return table;
    }

    scroll_view_handler::scroll_view_handler() : view_handler(&mapper(), &command_mapper())
    {
    }

    maui::graphics::size scroll_view_handler::get_desired_size(double /*width_constraint*/,
                                                               double /*height_constraint*/) const
    {
        // The scroll view computes its own size through the control (which ports the handler-side
        // CrossPlatformMeasure), so the handler reports nothing here — like the other content hosts.
        return {0, 0};
    }

    void scroll_view_handler::map_content(scroll_view_handler& handler, i_scroll_view& /*view*/)
    {
        handler.set_content();
    }

    void scroll_view_handler::map_orientation(scroll_view_handler& handler, i_scroll_view& /*view*/)
    {
        handler.update_orientation();
    }

    void scroll_view_handler::map_horizontal_scroll_bar_visibility(scroll_view_handler& handler,
                                                                   i_scroll_view& /*view*/)
    {
        handler.update_horizontal_scroll_bar_visibility();
    }

    void scroll_view_handler::map_vertical_scroll_bar_visibility(scroll_view_handler& handler, i_scroll_view& /*view*/)
    {
        handler.update_vertical_scroll_bar_visibility();
    }

    // C# MapRequestScrollTo: unwrap the ScrollToRequest payload and execute it natively.
    void scroll_view_handler::map_request_scroll_to(scroll_view_handler& handler, i_scroll_view& /*view*/,
                                                    const std::any& args)
    {
        const auto* request = std::any_cast<scroll_to_request>(&args);
        if (request == nullptr)
        {
            return;
        }
        handler.scroll_to(*request);
    }

    void scroll_view_handler::map_set_content(scroll_view_handler& handler, i_scroll_view& /*view*/,
                                              const std::any& /*args*/)
    {
        handler.set_content();
    }
} // namespace maui::core
