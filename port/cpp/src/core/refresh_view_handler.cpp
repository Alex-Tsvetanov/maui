// refresh_view_handler — cross-platform part: the shared mapper + command tables and the ctor
// (RefreshViewHandler.cs). The platform recipe (create + content host + the update_* pushes +
// request_refresh) lives in the per-backend partial.

#include "maui/core/refresh_view_handler.hpp"

#include <any>

#include "maui/core/command_mapper.hpp"
#include "maui/core/i_refresh_view.hpp"
#include "maui/core/property_mapper.hpp"
#include "maui/core/view_handler.hpp"
#include "maui/core/view_mapper.hpp"
#include "maui/graphics/size.hpp"

namespace maui::core
{
    // Mirrors C# RefreshViewHandler.Mapper: IsRefreshing + Content + RefreshColor + IsRefreshEnabled,
    // chained onto the shared view_mapper. (C#'s extra IView.Background / IView.IsEnabled re-maps are
    // covered by the shared view_mapper here.)
    property_mapper<i_refresh_view, refresh_view_handler>& refresh_view_handler::mapper()
    {
        static property_mapper<i_refresh_view, refresh_view_handler> table{
            view_mapper(),
            {
                {"is_refreshing", &refresh_view_handler::map_is_refreshing},
                {"content", &refresh_view_handler::map_content},
                {"refresh_color", &refresh_view_handler::map_refresh_color},
                {"is_refresh_enabled", &refresh_view_handler::map_is_refresh_enabled},
            },
        };
        return table;
    }

    // C# RefreshViewHandler.CommandMapper is empty; the port adds the "set_content" runtime content funnel.
    maui::core::command_mapper<i_refresh_view, refresh_view_handler>& refresh_view_handler::command_mapper()
    {
        static maui::core::command_mapper<i_refresh_view, refresh_view_handler> table{
            {"set_content", &refresh_view_handler::map_set_content},
        };
        return table;
    }

    refresh_view_handler::refresh_view_handler() : view_handler(&mapper(), &command_mapper())
    {
    }

    maui::graphics::size refresh_view_handler::get_desired_size(double /*width_constraint*/,
                                                                double /*height_constraint*/) const
    {
        // The refresh view computes its own size through the control (which ports MeasureContent).
        return {0, 0};
    }

    void refresh_view_handler::map_is_refreshing(refresh_view_handler& handler, i_refresh_view& /*view*/)
    {
        handler.update_is_refreshing();
    }

    void refresh_view_handler::map_content(refresh_view_handler& handler, i_refresh_view& /*view*/)
    {
        handler.set_content();
    }

    void refresh_view_handler::map_refresh_color(refresh_view_handler& handler, i_refresh_view& /*view*/)
    {
        handler.update_refresh_color();
    }

    void refresh_view_handler::map_is_refresh_enabled(refresh_view_handler& handler, i_refresh_view& /*view*/)
    {
        handler.update_is_refresh_enabled();
    }

    void refresh_view_handler::map_set_content(refresh_view_handler& handler, i_refresh_view& /*view*/,
                                               const std::any& /*args*/)
    {
        handler.set_content();
    }
} // namespace maui::core
