// slider_handler — cross-platform part: the shared mapper tables + ctor (SliderHandler.cs). The
// platform recipe (create/connect/disconnect/map_*/measure) lives in the per-backend partial.

#include "maui/core/slider_handler.hpp"

#include "maui/core/command_mapper.hpp"
#include "maui/core/i_slider.hpp"
#include "maui/core/property_mapper.hpp"
#include "maui/core/view_handler.hpp"
#include "maui/core/view_mapper.hpp"

namespace maui::core
{
    // SliderHandler.Mapper (C# key order): Maximum / MaximumTrackColor / Minimum / MinimumTrackColor /
    // ThumbColor / Value over ViewHandler.ViewMapper. ThumbImageSource is deferred (see the header).
    property_mapper<i_slider, slider_handler>& slider_handler::mapper()
    {
        static property_mapper<i_slider, slider_handler> table{
            view_mapper(),
            {
                {"maximum", &slider_handler::map_maximum},
                {"maximum_track_color", &slider_handler::map_maximum_track_color},
                {"minimum", &slider_handler::map_minimum},
                {"minimum_track_color", &slider_handler::map_minimum_track_color},
                {"thumb_color", &slider_handler::map_thumb_color},
                {"value", &slider_handler::map_value},
            }};
        return table;
    }

    // No slider-specific commands (C#'s CommandMapper is empty). Qualified return type: the method name
    // `command_mapper` shadows the `command_mapper` template.
    maui::core::command_mapper<i_slider, slider_handler>& slider_handler::command_mapper()
    {
        static maui::core::command_mapper<i_slider, slider_handler> table{};
        return table;
    }

    slider_handler::slider_handler() : view_handler(&mapper(), &command_mapper())
    {
    }
} // namespace maui::core
