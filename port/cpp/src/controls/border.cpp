// maui::controls::border — out-of-line definitions: the shared bindable-property descriptors (the C#
// Border.*Property defaults), the measure/arrange layout (Border.CrossPlatformMeasure/Arrange), and the
// default-handler self-registration. See border.hpp.

#include "maui/controls/border.hpp"

#include <memory>
#include <vector>

#include "maui/core/bindable_property.hpp"
#include "maui/core/border_handler.hpp"
#include "maui/core/handler_registry.hpp"
#include "maui/core/i_view.hpp"
#include "maui/core/i_view_handler.hpp"
#include "maui/core/safe_area_edges.hpp"
#include "maui/core/safe_area_regions.hpp"
#include "maui/core/thickness.hpp"
#include "maui/graphics/i_shape.hpp"
#include "maui/graphics/line_cap.hpp"
#include "maui/graphics/line_join.hpp"
#include "maui/graphics/paint.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/shapes/rectangle.hpp"
#include "maui/graphics/size.hpp"

namespace maui::controls
{
    const maui::core::bindable_property<maui::core::thickness>& border::padding_property()
    {
        // C# Border.PaddingDefaultValueCreator returns Thickness.Zero.
        static const maui::core::bindable_property<maui::core::thickness> descriptor{"padding"};
        return descriptor;
    }

    const maui::core::bindable_property<std::shared_ptr<maui::graphics::paint>>& border::stroke_property()
    {
        // C# Border.StrokeProperty default is null.
        static const maui::core::bindable_property<std::shared_ptr<maui::graphics::paint>> descriptor{"stroke"};
        return descriptor;
    }

    const maui::core::bindable_property<double>& border::stroke_thickness_property()
    {
        // C# Border.StrokeThicknessProperty default is 1.0.
        static const maui::core::bindable_property<double> descriptor{"stroke_thickness", 1.0};
        return descriptor;
    }

    const maui::core::bindable_property<std::shared_ptr<maui::graphics::i_shape>>& border::stroke_shape_property()
    {
        // C# Border.StrokeShapeProperty default is `new Rectangle()` — one shared immutable instance
        // here (the default rectangle has no mutable state).
        static const maui::core::bindable_property<std::shared_ptr<maui::graphics::i_shape>> descriptor{
            "stroke_shape", std::make_shared<maui::graphics::shapes::rectangle>()};
        return descriptor;
    }

    const maui::core::bindable_property<std::vector<double>>& border::stroke_dash_array_property()
    {
        // C# Border.StrokeDashArrayProperty default-value creator mints an empty DoubleCollection.
        static const maui::core::bindable_property<std::vector<double>> descriptor{"stroke_dash_array"};
        return descriptor;
    }

    const maui::core::bindable_property<double>& border::stroke_dash_offset_property()
    {
        // C# Border.StrokeDashOffsetProperty default is 0.0.
        static const maui::core::bindable_property<double> descriptor{"stroke_dash_offset", 0.0};
        return descriptor;
    }

    const maui::core::bindable_property<maui::graphics::line_cap>& border::stroke_line_cap_property()
    {
        // C# Border.StrokeLineCapProperty default is PenLineCap.Flat (= LineCap.Butt).
        static const maui::core::bindable_property<maui::graphics::line_cap> descriptor{"stroke_line_cap",
                                                                                        maui::graphics::line_cap::butt};
        return descriptor;
    }

    const maui::core::bindable_property<maui::graphics::line_join>& border::stroke_line_join_property()
    {
        // C# Border.StrokeLineJoinProperty default is PenLineJoin.Miter.
        static const maui::core::bindable_property<maui::graphics::line_join> descriptor{
            "stroke_line_join", maui::graphics::line_join::miter};
        return descriptor;
    }

    const maui::core::bindable_property<double>& border::stroke_miter_limit_property()
    {
        // C# Border.StrokeMiterLimitProperty default is 10.0.
        static const maui::core::bindable_property<double> descriptor{"stroke_miter_limit", 10.0};
        return descriptor;
    }

    const maui::core::bindable_property<maui::core::safe_area_edges>& border::safe_area_edges_property()
    {
        // C# Border.SafeAreaEdgesProperty default is SafeAreaEdges.None.
        static const maui::core::bindable_property<maui::core::safe_area_edges> descriptor{
            "safe_area_edges", maui::core::safe_area_edges::none()};
        return descriptor;
    }

    void border::set_safe_area_insets(const maui::core::thickness& value)
    {
        (void)value;
    }

    maui::core::safe_area_regions border::get_safe_area_regions_for_edge(int edge) const
    {
        const auto r = safe_area_edges_.get().edge(edge);
        if (r == maui::core::safe_area_regions::default_value)
        {
            return maui::core::safe_area_regions::none;
        }
        return r;
    }

    // C# Border.CrossPlatformMeasure: inset = Padding + StrokeThickness; LayoutExtensions.MeasureContent —
    // measure the content within the inset, then add the inset back (padding-only when no content).
    maui::graphics::size border::measure(double width_constraint, double height_constraint)
    {
        const double stroke_inset = stroke_thickness();
        const maui::core::thickness base = padding();
        const maui::core::thickness inset{base.left + stroke_inset, base.top + stroke_inset, base.right + stroke_inset,
                                          base.bottom + stroke_inset};
        maui::graphics::size content_size{0, 0};
        if (content_ != nullptr)
        {
            content_size = content_->measure(width_constraint - inset.horizontal_thickness(),
                                             height_constraint - inset.vertical_thickness());
        }
        const maui::graphics::size measured{content_size.width + inset.horizontal_thickness(),
                                            content_size.height + inset.vertical_thickness()};
        // Border : View — reconcile the measured inset against this view's own Width/HeightRequest
        // (clamped by Min/Max), the IView desired-size resolution every leaf measure runs.
        desired_size_ = {resolve_size_request(measured.width, width(), minimum_width(), maximum_width()),
                         resolve_size_request(measured.height, height(), minimum_height(), maximum_height())};
        return desired_size_;
    }

    // C# Border.CrossPlatformArrange: inset the bounds by StrokeThickness (Rect.Inset), then
    // ArrangeContent applies the Padding within it. The handler is framed first so the native host (and
    // its bounds-dependent stroke layer) is sized before the content lands (the content_page order).
    maui::graphics::size border::arrange(const maui::graphics::rect& bounds)
    {
        frame_ = bounds;
        if (auto* view_handler = dynamic_cast<maui::core::i_view_handler*>(handler().get()))
        {
            view_handler->platform_arrange(bounds);
        }
        if (content_ != nullptr)
        {
            const double stroke_inset = stroke_thickness();
            const maui::core::thickness pad = padding();
            const maui::graphics::rect target{bounds.x + stroke_inset + pad.left, bounds.y + stroke_inset + pad.top,
                                              bounds.width - (2 * stroke_inset) - pad.horizontal_thickness(),
                                              bounds.height - (2 * stroke_inset) - pad.vertical_thickness()};
            content_->arrange(target);
        }
        return {bounds.width, bounds.height};
    }
} // namespace maui::controls

// Self-register the default handler for border (opt-in, PROFILE §6).
MAUI_REGISTER_HANDLER(maui::controls::border, maui::core::border_handler)
