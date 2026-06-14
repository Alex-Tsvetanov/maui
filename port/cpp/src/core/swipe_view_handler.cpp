// swipe_view_handler — cross-platform part: the shared mapper + command tables and the ctor
// (SwipeViewHandler.cs). The platform recipe (create + content host + the state-machine drivers) lives in
// the per-backend partial.

#include "maui/core/swipe_view_handler.hpp"

#include <any>

#include "maui/core/command_mapper.hpp"
#include "maui/core/i_swipe_view.hpp"
#include "maui/core/property_mapper.hpp"
#include "maui/core/swipe_view_requests.hpp"
#include "maui/core/view_handler.hpp"
#include "maui/core/view_mapper.hpp"
#include "maui/graphics/size.hpp"

namespace maui::core
{
    // Mirrors C# SwipeViewHandler.Mapper: Content + SwipeTransitionMode + the four item sets, chained onto
    // the shared view_mapper. The four item entries mirror C#'s empty MapLeftItems/... (the gesture-time
    // UpdateSwipeItems does the real work) — the port routes them to update_items so a runtime collection
    // change re-touches the platform cache.
    property_mapper<i_swipe_view, swipe_view_handler>& swipe_view_handler::mapper()
    {
        static property_mapper<i_swipe_view, swipe_view_handler> table{
            view_mapper(),
            {
                {"content", &swipe_view_handler::map_content},
                {"swipe_transition_mode", &swipe_view_handler::map_transition_mode},
                {"left_items", &swipe_view_handler::map_items},
                {"top_items", &swipe_view_handler::map_items},
                {"right_items", &swipe_view_handler::map_items},
                {"bottom_items", &swipe_view_handler::map_items},
            },
        };
        return table;
    }

    // Mirrors C# SwipeViewHandler.CommandMapper: RequestOpen / RequestClose — plus the port's "set_content"
    // runtime funnel every content host shares.
    maui::core::command_mapper<i_swipe_view, swipe_view_handler>& swipe_view_handler::command_mapper()
    {
        static maui::core::command_mapper<i_swipe_view, swipe_view_handler> table{
            {"request_open", &swipe_view_handler::map_request_open},
            {"request_close", &swipe_view_handler::map_request_close},
            {"set_content", &swipe_view_handler::map_set_content},
        };
        return table;
    }

    swipe_view_handler::swipe_view_handler() : view_handler(&mapper(), &command_mapper())
    {
    }

    maui::graphics::size swipe_view_handler::get_desired_size(double /*width_constraint*/,
                                                              double /*height_constraint*/) const
    {
        // The swipe view computes its own size through the control (which ports MeasureContent).
        return {0, 0};
    }

    void swipe_view_handler::map_content(swipe_view_handler& handler, i_swipe_view& /*view*/)
    {
        handler.set_content();
    }

    void swipe_view_handler::map_transition_mode(swipe_view_handler& handler, i_swipe_view& /*view*/)
    {
        handler.update_transition_mode();
    }

    void swipe_view_handler::map_items(swipe_view_handler& handler, i_swipe_view& /*view*/)
    {
        handler.update_items();
    }

    // C# MapRequestOpen: unwrap the SwipeViewOpenRequest payload and open programmatically.
    void swipe_view_handler::map_request_open(swipe_view_handler& handler, i_swipe_view& /*view*/, const std::any& args)
    {
        const auto* request = std::any_cast<swipe_view_open_request>(&args);
        if (request == nullptr)
        {
            return;
        }
        handler.programmatically_open(*request);
    }

    // C# MapRequestClose: unwrap the SwipeViewCloseRequest payload and reset (close) the swipe.
    void swipe_view_handler::map_request_close(swipe_view_handler& handler, i_swipe_view& /*view*/,
                                               const std::any& args)
    {
        const auto* request = std::any_cast<swipe_view_close_request>(&args);
        if (request == nullptr)
        {
            return;
        }
        handler.reset_swipe(request->animated);
    }

    void swipe_view_handler::map_set_content(swipe_view_handler& handler, i_swipe_view& /*view*/,
                                             const std::any& /*args*/)
    {
        handler.set_content();
    }
} // namespace maui::core
