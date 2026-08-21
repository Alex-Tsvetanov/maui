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
        // …and this Border's OWN MARGIN, which C# LayoutExtensions.ComputeDesiredSize (LayoutExtensions.cs
        // :11-32) folds into EVERY IView's desired size: the constraint loses it, the reported size regains
        // it, so the PARENT reserves the gap. Neither MAUI's layout managers nor the port's add child
        // margins themselves — both rely on the child's Measure() already including it.
        //
        // Omitting it here was not merely an under-reserve, it was UNBALANCED — the same defect layout.hpp
        // :175-180 documents and fixes for the layout override. arrange calls the shared compute_frame (the
        // ComputeFrame port), which SUBTRACTS the margin from the desired size REGARDLESS (view.hpp
        // :1069/1076, "DesiredSize already INCLUDES the margin"), so a Border with a Margin lost 2x that
        // margin outright. Measured on border_clip_playground: a Margin=5 Border resolved to 100pt and
        // compute_frame handed it 90pt, a 266px stroke bbox where MAUI draws 296px at 3x. view<>::measure
        // always added it; this override was half-wired. A no-op at zero margin, which is why it survived.
        const maui::core::thickness view_margin = margin();
        const double margin_h = view_margin.horizontal_thickness();
        const double margin_v = view_margin.vertical_thickness();
        maui::graphics::size content_size{0, 0};
        if (content_ != nullptr)
        {
            content_size = content_->measure(width_constraint - margin_h - inset.horizontal_thickness(),
                                             height_constraint - margin_v - inset.vertical_thickness());
        }
        const maui::graphics::size measured{content_size.width + inset.horizontal_thickness(),
                                            content_size.height + inset.vertical_thickness()};
        // Border : View — reconcile the measured inset against this view's own Width/HeightRequest
        // (clamped by Min/Max), the IView desired-size resolution every leaf measure runs.
        // The margin is added AFTER the size-request resolution, not before: in C# the Width/HeightRequest
        // clamp happens inside the handler's GetDesiredSize, and ComputeDesiredSize adds the margin to
        // whatever that returned. Folding it in first would let an explicit WidthRequest swallow the margin.
        desired_size_ = {resolve_size_request(measured.width, width(), minimum_width(), maximum_width()) + margin_h,
                         resolve_size_request(measured.height, height(), minimum_height(), maximum_height()) +
                             margin_v};
        return desired_size_;
    }

    // C# Border.CrossPlatformArrange: inset the bounds by StrokeThickness (Rect.Inset), then
    // ArrangeContent applies the Padding within it. The handler is framed first so the native host (and
    // its bounds-dependent stroke layer) is sized before the content lands (the content_page order).
    //
    // The content is hosted as a SUBVIEW of the border host (BorderHandler.UpdateContent), and that host
    // is framed at `bounds` by platform_arrange above. A native subview's frame is expressed in its
    // superview's coordinate space — which, for the host, starts at the host's own origin (0,0), not the
    // page origin. So the content is arranged HOST-RELATIVE: the stroke+padding inset measured from the
    // host's top-left, with `bounds.x/bounds.y` dropped. Carrying the absolute page origin here would
    // double-offset the content (host frame origin + the same origin again) and push it outside the host's
    // shape-clip mask — invisible on iOS/AppKit. (In C# the native LayoutSubviews passes each container its
    // own 0-origin Bounds, so the absolute origin never enters the child rect; the port drives arrange
    // top-down with absolute coordinates, so the container subtracts its origin instead.) Mirrors
    // templated_view::arrange (content_view), the sibling single-content host.
    maui::graphics::size border::arrange(const maui::graphics::rect& bounds)
    {
        // Border : View — resolve this view's own aligned, size-requested FRAME within the allotted
        // `bounds`, the same LayoutExtensions.ComputeFrame the leaf view<>::arrange runs: it honors this
        // Border's WidthRequest/HeightRequest (+ Min/Max) and HorizontalOptions/VerticalOptions. Without it
        // a Border with an explicit Width and a non-Fill alignment filled the whole cell (e.g. the
        // alignment demo's 160-wide Start/Center/End borders all stretched edge-to-edge). A Fill border
        // with no explicit size still resolves to `bounds` (compute_frame returns it), so the common case
        // is unchanged. The native host + its bounds-dependent stroke layer are then framed to that frame.
        const maui::graphics::rect frame = compute_frame(bounds);
        frame_ = frame;
        if (auto* view_handler = dynamic_cast<maui::core::i_view_handler*>(handler().get()))
        {
            view_handler->platform_arrange(frame);
        }
        if (content_ != nullptr)
        {
            const double stroke_inset = stroke_thickness();
            const maui::core::thickness pad = padding();
            const maui::graphics::rect target{stroke_inset + pad.left, stroke_inset + pad.top,
                                              frame.width - (2 * stroke_inset) - pad.horizontal_thickness(),
                                              frame.height - (2 * stroke_inset) - pad.vertical_thickness()};
            content_->arrange(target);
        }
        return {frame.width, frame.height};
    }
} // namespace maui::controls

// Self-register the default handler for border (opt-in, PROFILE §6).
MAUI_REGISTER_HANDLER(maui::controls::border, maui::core::border_handler)
