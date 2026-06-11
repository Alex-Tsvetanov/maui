// border_handler — cross-platform part: the shared mapper + command tables, the ctor, the
// size-change-aware platform_arrange, and the IBorderStroke snapshot (BorderHandler.cs +
// StrokeExtensions.UpdateMauiCALayer's read of the border surface). The platform recipe (create +
// set_content + update_border + arrange_native) lives in the per-backend partial.

#include "maui/core/border_handler.hpp"

#include <any>

#include "maui/core/command_mapper.hpp"
#include "maui/core/i_border_view.hpp"
#include "maui/core/property_mapper.hpp"
#include "maui/core/view_handler.hpp"
#include "maui/core/view_mapper.hpp"
#include "maui/graphics/paint.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

namespace maui::core
{
    // Mirrors C# BorderHandler.Mapper: Content (MapContent) plus the nine stroke entries — every stroke
    // key funnels into the single update_border() refresh, exactly as the C# Map* methods all call
    // StrokeExtensions' UpdateMauiCALayer. Chained onto the shared view_mapper so the generic IView
    // properties map first (C#'s MapBackground override — drawing the background into the border layer —
    // collapses onto the base background push: the border's clip mask bounds the background to the shape).
    property_mapper<i_border_view, border_handler>& border_handler::mapper()
    {
        static property_mapper<i_border_view, border_handler> table{
            view_mapper(),
            {
                {"content", &border_handler::map_content},
                {"stroke_shape", &border_handler::map_border_property},
                {"stroke", &border_handler::map_border_property},
                {"stroke_thickness", &border_handler::map_border_property},
                {"stroke_line_cap", &border_handler::map_border_property},
                {"stroke_line_join", &border_handler::map_border_property},
                {"stroke_dash_array", &border_handler::map_border_property},
                {"stroke_dash_offset", &border_handler::map_border_property},
                {"stroke_miter_limit", &border_handler::map_border_property},
            },
        };
        return table;
    }

    // The content-management command (the same runtime "set_content" funnel the other content hosts
    // use; C#'s CommandMapper for borders is empty because its Content set re-enters the property path).
    maui::core::command_mapper<i_border_view, border_handler>& border_handler::command_mapper()
    {
        static maui::core::command_mapper<i_border_view, border_handler> table{
            {"set_content", &border_handler::map_set_content},
        };
        return table;
    }

    border_handler::border_handler() : view_handler(&mapper(), &command_mapper())
    {
    }

    // C# StrokeExtensions.UpdateMauiCALayer's read of the IBorderStroke surface, as a value snapshot.
    border_stroke_spec border_handler::make_border_stroke_spec(const i_border_view& view)
    {
        border_stroke_spec spec;
        const maui::graphics::paint* const stroke = view.stroke();
        spec.has_stroke = stroke != nullptr;
        if (stroke != nullptr)
        {
            spec.stroke_color = stroke->background_color();
        }
        spec.thickness = view.stroke_thickness();
        spec.line_cap = view.stroke_line_cap();
        spec.line_join = view.stroke_line_join();
        spec.dash_pattern = view.stroke_dash_pattern();
        spec.dash_offset = view.stroke_dash_offset();
        spec.miter_limit = view.stroke_miter_limit();
        spec.shape = view.shape();
        return spec;
    }

    maui::graphics::size border_handler::get_desired_size(double /*width_constraint*/,
                                                          double /*height_constraint*/) const
    {
        // The border computes its own size through the control (Border.CrossPlatformMeasure), so the
        // handler reports nothing here.
        return {0, 0};
    }

    // C# BorderHandler.PlatformArrange: frame the host, then re-issue the (bounds-dependent) shape
    // mapping when the size changed.
    void border_handler::platform_arrange(const maui::graphics::rect& frame)
    {
        arrange_native(frame);
        const maui::graphics::size new_size{frame.width, frame.height};
        if (!(last_size_ == new_size))
        {
            last_size_ = new_size;
            update_border();
        }
    }

    // C# MapContent → UpdateContent (the property path; runs on connect and on a content change).
    void border_handler::map_content(border_handler& handler, i_border_view& /*view*/)
    {
        handler.set_content();
    }

    // Every stroke property funnels into one refresh (the C# StrokeExtensions.Update* → UpdateMauiCALayer
    // funnel).
    void border_handler::map_border_property(border_handler& handler, i_border_view& /*view*/)
    {
        handler.update_border();
    }

    // The "set_content" command path: identical re-host, invoked by the control on a runtime content
    // change.
    void border_handler::map_set_content(border_handler& handler, i_border_view& /*view*/, const std::any& /*args*/)
    {
        handler.set_content();
    }
} // namespace maui::core
