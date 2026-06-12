// shape_view_handler — cross-platform part: the shared mapper table (the C# ShapeViewHandler keys
// UNIONED with the per-shape sub-handler keys — see the header collapse note), the ctor, the
// drawable-state refresh, and the measure/arrange seam. The platform recipe (create + update_shape +
// invalidate_shape + arrange_native) lives in the per-backend partial.

#include "maui/core/shape_view_handler.hpp"

#include "maui/core/command_mapper.hpp"
#include "maui/core/i_shape_view.hpp"
#include "maui/core/property_mapper.hpp"
#include "maui/core/view_handler.hpp"
#include "maui/core/view_mapper.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

namespace maui::core
{
    // Mirrors C# ShapeViewHandler.Mapper (Shape/Aspect/Fill/Stroke + the six stroke detail keys +
    // the Background/FlowDirection overrides) UNIONED with the Controls sub-handler keys —
    // RectangleHandler's RadiusX/RadiusY, RoundRectangleHandler's CornerRadius, LineHandler's
    // X1/Y1/X2/Y2, Polyline/PolygonHandler's Points/FillRule, PathHandler's Data/RenderTransform —
    // every one funnelling into InvalidateShape exactly as the C# Map* bodies do (fill rule and
    // render transform re-read off the contract inside refresh_drawable_state). "stroke_dash_array"
    // is the control's property name behind C#'s StrokeDashPattern remap (Shape.Mapper.cs).
    property_mapper<i_shape_view, shape_view_handler>& shape_view_handler::mapper()
    {
        static property_mapper<i_shape_view, shape_view_handler> table{
            view_mapper(),
            {
                {"background", &shape_view_handler::map_background},
                {"flow_direction", &shape_view_handler::map_flow_direction},
                {"shape", &shape_view_handler::map_shape},
                {"aspect", &shape_view_handler::map_invalidate_shape},
                {"fill", &shape_view_handler::map_invalidate_shape},
                {"stroke", &shape_view_handler::map_invalidate_shape},
                {"stroke_thickness", &shape_view_handler::map_invalidate_shape},
                {"stroke_dash_array", &shape_view_handler::map_invalidate_shape},
                {"stroke_dash_offset", &shape_view_handler::map_invalidate_shape},
                {"stroke_line_cap", &shape_view_handler::map_invalidate_shape},
                {"stroke_line_join", &shape_view_handler::map_invalidate_shape},
                {"stroke_miter_limit", &shape_view_handler::map_invalidate_shape},
                // ---- the absorbed sub-handler keys (the header collapse note) ----
                {"radius_x", &shape_view_handler::map_invalidate_shape},
                {"radius_y", &shape_view_handler::map_invalidate_shape},
                {"corner_radius", &shape_view_handler::map_invalidate_shape},
                {"x1", &shape_view_handler::map_invalidate_shape},
                {"y1", &shape_view_handler::map_invalidate_shape},
                {"x2", &shape_view_handler::map_invalidate_shape},
                {"y2", &shape_view_handler::map_invalidate_shape},
                {"points", &shape_view_handler::map_invalidate_shape},
                {"fill_rule", &shape_view_handler::map_invalidate_shape},
                {"data", &shape_view_handler::map_invalidate_shape},
                {"render_transform", &shape_view_handler::map_invalidate_shape},
                // BoxView.OnPropertyChanged funnels its Color change into UpdateValue(Shape).
                {"color", &shape_view_handler::map_shape},
            },
        };
        return table;
    }

    // C# ShapeViewHandler.CommandMapper is empty.
    maui::core::command_mapper<i_shape_view, shape_view_handler>& shape_view_handler::command_mapper()
    {
        static maui::core::command_mapper<i_shape_view, shape_view_handler> table{};
        return table;
    }

    shape_view_handler::shape_view_handler() : view_handler(&mapper(), &command_mapper())
    {
    }

    maui::graphics::size shape_view_handler::get_desired_size(double /*width_constraint*/,
                                                              double /*height_constraint*/) const
    {
        // The shape controls compute their own size (Shape.MeasureOverride in the control), so the
        // handler reports nothing — the border handler precedent (see the header note).
        return {0, 0};
    }

    void shape_view_handler::platform_arrange(const maui::graphics::rect& frame)
    {
        arrange_native(frame);
    }

    // The C# MapFillRule / MapRenderTransform pushes, funneled: re-read both off the virtual view
    // into the host's drawable (runs inside every update_shape/invalidate_shape).
    void shape_view_handler::refresh_drawable_state()
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || virtual_view() == nullptr)
        {
            return;
        }
        platform->drawable.update_winding_mode(virtual_view()->fill_winding());
        platform->drawable.update_render_transform(virtual_view()->render_transform_matrix());
    }

    // C# MapShape → UpdateShape.
    void shape_view_handler::map_shape(shape_view_handler& handler, i_shape_view& /*view*/)
    {
        handler.update_shape();
    }

    // C#'s per-property Map* bodies: InvalidateShape.
    void shape_view_handler::map_invalidate_shape(shape_view_handler& handler, i_shape_view& /*view*/)
    {
        handler.invalidate_shape();
    }

    // C# MapBackground: a set background (or fill) invalidates the shape; the Fill+Background
    // container split (UpdateValue(ContainerView) + UpdateBackground on the wrapper) is deferred
    // with the container seam (documented in STATUS).
    void shape_view_handler::map_background(shape_view_handler& handler, i_shape_view& view)
    {
        if (view.background() != nullptr || view.fill() != nullptr)
        {
            handler.invalidate_shape();
        }
    }

    // C# MapFlowDirection: push the platform flow direction, then invalidate.
    void shape_view_handler::map_flow_direction(shape_view_handler& handler, i_shape_view& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->update_flow_direction(view.flow_direction());
        }
        handler.invalidate_shape();
    }
} // namespace maui::core
