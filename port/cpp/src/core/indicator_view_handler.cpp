// indicator_view_handler — cross-platform part: the shared mapper tables, the ctor, and the
// IndicatorViewExtensions.GetMaximumVisible helper. The platform recipe (create / map_* / connect /
// measure) lives in the per-backend partial. Ported from IndicatorViewHandler.cs +
// IndicatorViewExtensions.cs.

#include "maui/core/indicator_view_handler.hpp"

#include <algorithm>

#include "maui/core/command_mapper.hpp"
#include "maui/core/i_indicator_view.hpp"
#include "maui/core/property_mapper.hpp"
#include "maui/core/view_handler.hpp"
#include "maui/core/view_mapper.hpp"

namespace maui::core
{
    // IndicatorViewExtensions.GetMaximumVisible: min(MaximumVisible, Count) floored at 0, with the
    // HideSingle collapse (a single dot vanishes when HideSingle is set).
    int max_visible_indicators(const i_indicator_view& view)
    {
        const int min_value = std::min(view.maximum_visible(), view.count());
        int maximum_visible = min_value <= 0 ? 0 : min_value;
        if (maximum_visible == 1 && view.hide_single())
        {
            maximum_visible = 0;
        }
        return maximum_visible;
    }

    // IndicatorViewHandler.Mapper: Count / Position / HideSingle / MaximumVisible / IndicatorSize /
    // IndicatorColor / SelectedIndicatorColor / IndicatorsShape over ViewHandler.ViewMapper.
    property_mapper<i_indicator_view, indicator_view_handler>& indicator_view_handler::mapper()
    {
        static property_mapper<i_indicator_view, indicator_view_handler> table{
            view_mapper(),
            {
                {"count", &indicator_view_handler::map_count},
                {"position", &indicator_view_handler::map_position},
                {"hide_single", &indicator_view_handler::map_hide_single},
                {"maximum_visible", &indicator_view_handler::map_maximum_visible},
                {"indicator_size", &indicator_view_handler::map_indicator_size},
                {"indicator_color", &indicator_view_handler::map_indicator_color},
                {"selected_indicator_color", &indicator_view_handler::map_selected_indicator_color},
                {"indicators_shape", &indicator_view_handler::map_indicator_shape},
            }};
        return table;
    }

    // No indicator-specific commands (C#'s CommandMapper is empty). Qualified return type: the method
    // name `command_mapper` shadows the `command_mapper` template.
    maui::core::command_mapper<i_indicator_view, indicator_view_handler>& indicator_view_handler::command_mapper()
    {
        static maui::core::command_mapper<i_indicator_view, indicator_view_handler> table{};
        return table;
    }

    indicator_view_handler::indicator_view_handler() : view_handler(&mapper(), &command_mapper())
    {
    }
} // namespace maui::core
