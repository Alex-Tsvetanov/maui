// shape_view_handler — Windows (WinUI 3) platform partial: a REAL Microsoft.UI.Xaml.Controls.Canvas
// CONTAINER holding the current shape as its single Microsoft.UI.Xaml.Shapes child. The windows twin
// of src/platform/apple/shape_view_handler.mm (NSView drawing host) / the android MauiShapeView
// canvas-replay partial, and the real-native sibling of the headless drawable mirror
// (src/platform/headless/shape_view_handler.cpp). box_view is NOT a handler of its own: BoxView is
// its own IShapeView AND its own IShape and renders through this shared handler (the android twin's
// note) — the box_view pages are this partial's priority path.
//
// Ported from ShapeViewHandler.cs + ShapeViewHandler.Windows.cs, with ShapeDrawable.cs as the render
// spec (fill-then-stroke, Fill ?? Background, stroke.ToColor(), the winding/render-transform pushes).
//
// THE CENTRAL DOCUMENTED DEVIATION: C#'s Windows partial renders every shape through a Win2D
// graphics view (CreatePlatformView => new W2DGraphicsView(), whose drawable replays the SAME
// ShapeDrawable the headless twin holds). This port has no Win2D/D2D canvas host, so the
// FAITHFUL-ENOUGH v1 maps the port's shape model onto native Microsoft.UI.Xaml.Shapes primitives —
// per concrete kind of the virtual view:
//   - controls::box_view            → Shapes.Rectangle (solid Fill from Color + RadiusX/RadiusY)
//   - controls::shapes::rectangle   → Shapes.Rectangle (RadiusX/RadiusY)
//   - controls::shapes::ellipse     → Shapes.Ellipse
//   - controls::shapes::line        → Shapes.Line (X1/Y1/X2/Y2)
//   - controls::shapes::polygon     → Shapes.Polygon (Points + FillRule)
//   - controls::shapes::polyline    → Shapes.Polyline (Points + FillRule)
//   - controls::shapes::path        → Shapes.Path (PathGeometry rebuilt from the port's path_f model)
// The KIND can change after create (map_shape re-runs for any shape swap; box_view's "color" funnels
// into it too), so the native is the stable Canvas container and the child element is rebuilt from
// scratch on EVERY update_shape/invalidate_shape (cheap — the indicator-dots rebuild precedent);
// measure/arrange target the container exactly like every other windows handler.
//
// PER-KIND FIDELITY NOTES / DEFERRALS (each with the C# reference):
//   - Rectangle radii: Rectangle.cs GetPath appends max(RadiusX, RadiusY) as ONE corner radius (the
//     C# "TODO: consider both radii"), so MAUI's rendered output collapses the two — the native push
//     mirrors that collapse (RadiusX = RadiusY = max) rather than the lossless two-radii form.
//   - box_view per-corner radii (BoxView.PathForBounds carries all four): Shapes.Rectangle is
//     uniform-radius only — the largest corner stands in. // deferred: a Path geometry child for
//     distinct corners.
//   - Gradient fills: // deferred (Paint.ToPlatform) — the paint's resolved background_color() (the
//     first-gradient-stop projection, the border_stroke_spec precedent) stands in as a solid brush so
//     the shape stays visible. The stroke ALWAYS uses that projection — that IS C#'s stroke.ToColor()
//     (ShapeDrawable.DrawStrokePath), not a deviation.
//   - path_operation::arc (center+angles) has no direct XAML ArcSegment analog (endpoint+radii); the
//     geometry walk flattens arc-bearing paths first (PathF.GetFlattenedPath). SVG "A" data never
//     hits this (svg_arc_to decomposes to cubics upstream).
//   - path_aspect::center has no XAML Stretch analog → Stretch::None (// deferred: the centering
//     translate). none/stretch/aspect_fit/aspect_fill map to None/Fill/Uniform/UniformToFill.
//   - The path control's RenderTransform is applied to the path_f BEFORE the geometry is built —
//     exactly ShapeDrawable.Draw's ApplyTransform (geometry-space, stroke width untransformed) —
//     rather than to the element (Shape.RenderTransform would scale the stroke too).
//   - Stroke inset: the port's rectangle/ellipse GetPath insets by half the stroke thickness so the
//     stroke stays inside the bounds; XAML Rectangle/Ellipse natively fit the stroke inside their
//     layout slot — the same rendered result, so the raw kind maps without the inset.
//
// THE HEADLESS MIRROR STAYS LIVE (the android twin's dual-drive rule): every shared write the
// headless twin makes (drawable.update_shape_view, refresh_drawable_state's winding/render-transform
// pushes, the invalidations counter) is preserved, so the windows preset's XAML-less cross-platform
// suite (create_platform_view degrades to a null native on the bare test host) observes exactly the
// headless partial's behavior; the native rebuild is layered ON TOP behind the null-native guard.

#include "maui/core/shape_view_handler.hpp"

#include <algorithm>
#include <cmath>
#include <memory>
#include <optional>
#include <string_view>
#include <vector>

#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Microsoft.UI.Xaml.Media.h>
#include <winrt/Microsoft.UI.Xaml.Shapes.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Windows.Foundation.Collections.h> // Children / Points / Figures IVector consume methods
#include <winrt/Windows.Foundation.h>
#include <winrt/base.h>

#include "maui/controls/box_view.hpp"
#include "maui/controls/shapes/ellipse.hpp"
#include "maui/controls/shapes/line.hpp"
#include "maui/controls/shapes/path.hpp"
#include "maui/controls/shapes/path_segment.hpp" // point_collection
#include "maui/controls/shapes/polygon.hpp"
#include "maui/controls/shapes/polyline.hpp"
#include "maui/controls/shapes/rectangle.hpp"
#include "maui/core/i_shape_view.hpp"
#include "maui/core/path_aspect.hpp"
#include "maui/core/view_platform_base.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/corner_radius.hpp"
#include "maui/graphics/i_canvas.hpp"
#include "maui/graphics/line_cap.hpp"
#include "maui/graphics/line_join.hpp"
#include "maui/graphics/matrix3x2.hpp"
#include "maui/graphics/paint.hpp"
#include "maui/graphics/path_f.hpp"
#include "maui/graphics/path_operation.hpp"
#include "maui/graphics/point.hpp"
#include "maui/graphics/point_f.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/rect_f.hpp"
#include "maui/graphics/solid_paint.hpp"
#include "maui/graphics/winding_mode.hpp"
#include "windows_native.hpp"

namespace
{
    namespace muxc = winrt::Microsoft::UI::Xaml::Controls;
    namespace muxm = winrt::Microsoft::UI::Xaml::Media;
    namespace muxs = winrt::Microsoft::UI::Xaml::Shapes;
    namespace wf = winrt::Windows::Foundation;
    namespace wnative = maui::platform::win;

    // PathF's default flattening tolerance (the arc-subdivision flatness GetFlattenedPath defaults).
    constexpr float k_flatten_tolerance = 0.001F;

    [[nodiscard]] muxc::Canvas host_of(const maui::core::shape_view_platform& platform)
    {
        return wnative::borrow<muxc::Canvas>(platform.native);
    }

    // WindingMode → Microsoft.UI.Xaml.Media.FillRule (the drawable's ClipPath/FillPath winding).
    [[nodiscard]] muxm::FillRule to_fill_rule(maui::graphics::winding_mode winding)
    {
        return winding == maui::graphics::winding_mode::even_odd ? muxm::FillRule::EvenOdd
                                                                 : muxm::FillRule::Nonzero;
    }

    // PathAspect → Shape.Stretch: None/Fill/Uniform/UniformToFill — the native expression of the
    // aspect fitting Shape.TransformPathForBounds performs in the drawable pipeline. center has no
    // XAML analog (// deferred: the centering translate — header note).
    [[nodiscard]] muxm::Stretch to_stretch(maui::core::path_aspect aspect)
    {
        switch (aspect)
        {
            case maui::core::path_aspect::stretch:
                return muxm::Stretch::Fill;
            case maui::core::path_aspect::aspect_fit:
                return muxm::Stretch::Uniform;
            case maui::core::path_aspect::aspect_fill:
                return muxm::Stretch::UniformToFill;
            case maui::core::path_aspect::none:
            case maui::core::path_aspect::center:
            default:
                return muxm::Stretch::None;
        }
    }

    // LineCap → PenLineCap (Butt = Flat, the Shape.cs PenLineCap collapse note).
    [[nodiscard]] muxm::PenLineCap to_pen_cap(maui::graphics::line_cap cap)
    {
        switch (cap)
        {
            case maui::graphics::line_cap::round:
                return muxm::PenLineCap::Round;
            case maui::graphics::line_cap::square:
                return muxm::PenLineCap::Square;
            case maui::graphics::line_cap::butt:
            default:
                return muxm::PenLineCap::Flat;
        }
    }

    // LineJoin → PenLineJoin.
    [[nodiscard]] muxm::PenLineJoin to_pen_join(maui::graphics::line_join join)
    {
        switch (join)
        {
            case maui::graphics::line_join::round:
                return muxm::PenLineJoin::Round;
            case maui::graphics::line_join::bevel:
                return muxm::PenLineJoin::Bevel;
            case maui::graphics::line_join::miter:
            default:
                return muxm::PenLineJoin::Miter;
        }
    }

    // PointCollection (the port's plain vector<point>) → the winrt Points collection.
    [[nodiscard]] muxm::PointCollection to_points(const maui::controls::shapes::point_collection& points)
    {
        muxm::PointCollection collection;
        for (const maui::graphics::point& value : points)
        {
            collection.Append(wf::Point{static_cast<float>(value.x), static_cast<float>(value.y)});
        }
        return collection;
    }

    // path_f → Microsoft.UI.Xaml.Media.PathGeometry: one PathFigure per Move sub-path, with
    // Line/QuadraticBezier/Bezier segments and Close → IsClosed. Arc ops (center+angles) have no
    // direct ArcSegment analog (endpoint+radii), so an arc-bearing path is flattened to lines first
    // (header note — rare: SVG arc data decomposes to cubics upstream).
    [[nodiscard]] muxm::PathGeometry to_path_geometry(const maui::graphics::path_f& source,
                                                      maui::graphics::winding_mode winding)
    {
        const std::vector<maui::graphics::path_operation>& source_ops = source.segment_types();
        const bool has_arcs =
            std::find(source_ops.begin(), source_ops.end(), maui::graphics::path_operation::arc) !=
            source_ops.end();
        maui::graphics::path_f flattened;
        const maui::graphics::path_f* path = &source;
        if (has_arcs)
        {
            flattened = source.get_flattened_path(k_flatten_tolerance, /*include_sub_paths=*/true);
            path = &flattened;
        }

        muxm::PathGeometry geometry;
        geometry.FillRule(to_fill_rule(winding));

        muxm::PathFigure figure{nullptr};
        int point_index = 0;
        const auto next_point = [&path, &point_index]() {
            const maui::graphics::point_f value = (*path)[point_index];
            point_index++;
            return wf::Point{value.x, value.y};
        };
        for (const maui::graphics::path_operation op : path->segment_types())
        {
            switch (op)
            {
                case maui::graphics::path_operation::move:
                {
                    figure = muxm::PathFigure{};
                    figure.StartPoint(next_point());
                    geometry.Figures().Append(figure);
                    break;
                }
                case maui::graphics::path_operation::line:
                {
                    if (figure == nullptr) // defensive: a well-formed path_f always opens with a Move
                    {
                        point_index += 1;
                        break;
                    }
                    muxm::LineSegment segment;
                    segment.Point(next_point());
                    figure.Segments().Append(segment);
                    break;
                }
                case maui::graphics::path_operation::quad:
                {
                    if (figure == nullptr)
                    {
                        point_index += 2;
                        break;
                    }
                    muxm::QuadraticBezierSegment segment;
                    segment.Point1(next_point());
                    segment.Point2(next_point());
                    figure.Segments().Append(segment);
                    break;
                }
                case maui::graphics::path_operation::cubic:
                {
                    if (figure == nullptr)
                    {
                        point_index += 3;
                        break;
                    }
                    muxm::BezierSegment segment;
                    segment.Point1(next_point());
                    segment.Point2(next_point());
                    segment.Point3(next_point());
                    figure.Segments().Append(segment);
                    break;
                }
                case maui::graphics::path_operation::arc:
                {
                    point_index += 2; // unreachable after the flatten; skip the two stored corners
                    break;
                }
                case maui::graphics::path_operation::close:
                {
                    if (figure != nullptr)
                    {
                        figure.IsClosed(true);
                    }
                    break;
                }
            }
        }
        return geometry;
    }

    // ShapeDrawable.DrawFillPath: Fill ?? Background paints the interior (a Background WITHOUT a
    // Fill IS the shape fill; the both-set container split happens in map_background upstream). A
    // null result leaves the XAML Shape's default null Fill — nothing painted, like the drawable's
    // transparent fill.
    void apply_fill(const muxs::Shape& element, const maui::core::i_shape_view& view)
    {
        const maui::graphics::paint* fill = view.fill();
        if (fill == nullptr)
        {
            fill = view.background();
        }
        if (fill == nullptr)
        {
            return;
        }
        if (const auto* solid = dynamic_cast<const maui::graphics::solid_paint*>(fill))
        {
            element.Fill(wnative::to_brush(solid->color()));
            return;
        }
        // deferred: gradient / image / pattern fills as real XAML brushes (Paint.ToPlatform) — the
        // resolved background color (the first-gradient-stop projection) stands in (header note).
        element.Fill(wnative::to_brush(fill->background_color()));
    }

    // ShapeDrawable.DrawStrokePath: no stroke paint or a non-positive thickness draws no outline
    // (the C# early-return); otherwise the full IStroke surface lands on the Shape's native stroke
    // properties.
    void apply_stroke(const muxs::Shape& element, const maui::core::i_shape_view& view)
    {
        const maui::graphics::paint* stroke = view.stroke();
        if (stroke == nullptr || view.stroke_thickness() <= 0)
        {
            return;
        }
        // C# stroke.ToColor() — the solid/first-gradient-stop projection (paint::background_color(),
        // the border_stroke_spec precedent). deferred: gradient strokes as real brushes.
        element.Stroke(wnative::to_brush(stroke->background_color()));
        element.StrokeThickness(view.stroke_thickness());
        // StrokeDashPattern → StrokeDashArray: both speak stroke-thickness multiples (the D2D
        // dash-style unit XAML Shapes and Win2D's CanvasStrokeStyle share), so the values and the
        // offset pass through unscaled. Empty = solid (the C# null analog) — leave the default.
        const std::vector<float> dashes = view.stroke_dash_pattern();
        if (!dashes.empty())
        {
            muxm::DoubleCollection dash_array;
            for (const float dash : dashes)
            {
                dash_array.Append(static_cast<double>(dash));
            }
            element.StrokeDashArray(dash_array);
            element.StrokeDashOffset(static_cast<double>(view.stroke_dash_offset()));
        }
        // The one MAUI cap feeds all three XAML cap slots (start/end/dash) — the Win2D
        // CanvasStrokeStyle shape the C# render produces.
        const muxm::PenLineCap cap = to_pen_cap(view.stroke_line_cap());
        element.StrokeStartLineCap(cap);
        element.StrokeEndLineCap(cap);
        element.StrokeDashCap(cap);
        element.StrokeLineJoin(to_pen_join(view.stroke_line_join()));
        if (view.stroke_miter_limit() > 0)
        {
            element.StrokeMiterLimit(static_cast<double>(view.stroke_miter_limit()));
        }
    }

    // Build the matching Shapes element for the virtual view's concrete kind (the per-kind table in
    // the header). Empty (nullptr) for an unknown kind — the headless drawable mirror still records
    // it; a faithful render of arbitrary custom IShapeViews needs the Win2D-style canvas host
    // (C#'s W2DGraphicsView — deferred).
    [[nodiscard]] muxs::Shape make_shape_element(const maui::core::i_shape_view& view)
    {
        if (const auto* box = dynamic_cast<const maui::controls::box_view*>(&view))
        {
            // BoxView → Rectangle: the solid (optionally rounded) box — the priority path. The fill
            // arrives through apply_fill (BoxView.Fill => Color?.AsPaint()); no stroke (the BoxView
            // IStroke surface is all empty/zero).
            muxs::Rectangle rect;
            const maui::graphics::corner_radius radii = box->corner_radius();
            // Shapes.Rectangle is uniform-radius only; the largest corner stands in (// deferred:
            // a Path geometry for distinct corners — BoxView.PathForBounds carries all four).
            const double radius =
                (std::max)({radii.top_left, radii.top_right, radii.bottom_left, radii.bottom_right});
            rect.RadiusX(radius);
            rect.RadiusY(radius);
            return rect;
        }
        if (const auto* rectangle_shape = dynamic_cast<const maui::controls::shapes::rectangle*>(&view))
        {
            // Rectangle.cs GetPath appends max(RadiusX, RadiusY) as ONE radius (the C# TODO), so
            // MAUI's rendered output collapses the two — mirrored here (header note).
            muxs::Rectangle rect;
            const double radius = (std::max)(rectangle_shape->radius_x(), rectangle_shape->radius_y());
            rect.RadiusX(radius);
            rect.RadiusY(radius);
            return rect;
        }
        if (dynamic_cast<const maui::controls::shapes::ellipse*>(&view) != nullptr)
        {
            return muxs::Ellipse{};
        }
        if (const auto* line_shape = dynamic_cast<const maui::controls::shapes::line*>(&view))
        {
            muxs::Line native_line;
            native_line.X1(line_shape->x1());
            native_line.Y1(line_shape->y1());
            native_line.X2(line_shape->x2());
            native_line.Y2(line_shape->y2());
            native_line.Stretch(to_stretch(view.aspect()));
            return native_line;
        }
        if (const auto* polygon_shape = dynamic_cast<const maui::controls::shapes::polygon*>(&view))
        {
            muxs::Polygon native_polygon;
            native_polygon.Points(to_points(polygon_shape->points()));
            // The C# PolygonHandler.MapFillRule push, read off the fill_winding() port extension.
            native_polygon.FillRule(to_fill_rule(view.fill_winding()));
            native_polygon.Stretch(to_stretch(view.aspect()));
            return native_polygon;
        }
        if (const auto* polyline_shape = dynamic_cast<const maui::controls::shapes::polyline*>(&view))
        {
            muxs::Polyline native_polyline;
            native_polyline.Points(to_points(polyline_shape->points()));
            native_polyline.FillRule(to_fill_rule(view.fill_winding()));
            native_polyline.Stretch(to_stretch(view.aspect()));
            return native_polyline;
        }
        if (const auto* path_shape = dynamic_cast<const maui::controls::shapes::path*>(&view))
        {
            muxs::Path native_path;
            maui::graphics::path_f figure_path = path_shape->get_path();
            // ShapeDrawable.Draw's ApplyTransform: the RenderTransform maps the GEOMETRY (stroke
            // width untransformed), so it lands on the path_f before the winrt geometry is built —
            // NOT on the element's RenderTransform (which would scale the stroke too).
            if (const std::optional<maui::graphics::matrix3x2> transform = view.render_transform_matrix())
            {
                figure_path.transform(*transform);
            }
            native_path.Data(to_path_geometry(figure_path, view.fill_winding()));
            native_path.Stretch(to_stretch(view.aspect()));
            return native_path;
        }
        return muxs::Shape{nullptr}; // unknown kind (header note)
    }

    // The redraw analog of C#'s Drawable-setter/Invalidate round trip: swap the Canvas's single
    // Shapes child for a freshly built one (the kind may have changed) and re-apply fill/stroke.
    void rebuild_native_shape(maui::core::shape_view_platform& platform, const maui::core::i_shape_view* view)
    {
        auto host = host_of(platform);
        if (host == nullptr)
        {
            return; // XAML-less: the headless mirrors are the whole push (header note)
        }
        host.Children().Clear();
        wnative::release(platform.shape_element);
        if (view == nullptr || view->shape() == nullptr)
        {
            return; // C# ShapeDrawable.Draw: a null Shape draws nothing
        }
        const muxs::Shape element = make_shape_element(*view);
        if (element == nullptr)
        {
            return; // unknown kind — deferred (make_shape_element note)
        }
        apply_fill(element, *view);
        apply_stroke(element, *view);
        host.Children().Append(element);
        platform.shape_element = wnative::store(element); // released on rebuild / in the dtor
        // Pin the child to the container's arranged size when one exists (Width/Height are NaN before
        // the first arrange — arrange_native pins it right after, in the same layout pass).
        const double width = host.Width();
        const double height = host.Height();
        if (!std::isnan(width) && !std::isnan(height) && width >= 0 && height >= 0)
        {
            wnative::arrange_native(platform.shape_element, maui::graphics::rect{0, 0, width, height});
        }
    }
} // namespace

namespace maui::core
{
    // Releases the strong refs pinning the Canvas container + the current Shapes child (the wnative
    // shape of the pimpl-owned-native doctrine; the android twin deletes its JNI global ref here).
    shape_view_platform::~shape_view_platform()
    {
        wnative::release(shape_element);
        wnative::release(native);
    }

    // The portable replay seat (shared with headless — the golden-op tests replay the drawable into a
    // recording_canvas). On windows the REAL render is the native Shapes child; this stays live so any
    // recording-canvas host observes the same shape ops (the android twin's rule).
    void shape_view_platform::replay(maui::graphics::i_canvas& canvas, const maui::graphics::rect_f& dirty_rect)
    {
        drawable.draw(canvas, dirty_rect);
    }

    // The generic-IView pushes (the shared view_mapper calls these through view_platform_base). Each
    // calls the base body FIRST — the headless mirrors must stay live for the XAML-less cross-platform
    // suite (header note) — then pushes to the real elements when they exist.

    void shape_view_platform::update_visibility(maui::core::visibility value)
    {
        view_platform_base::update_visibility(value);
        // ViewExtensions.UpdateVisibility (Windows): Hidden rides Opacity 0, Collapsed collapses.
        wnative::apply_visibility(native, value, alpha);
    }

    void shape_view_platform::update_opacity(double value)
    {
        view_platform_base::update_opacity(value);
        // ViewExtensions.UpdateOpacity: the Hidden state pins opacity 0 (apply_visibility restores it).
        if (!hidden)
        {
            wnative::apply_opacity(native, value);
        }
    }

    void shape_view_platform::update_automation_id(std::string_view value)
    {
        view_platform_base::update_automation_id(value);
        // ViewExtensions.UpdateAutomationId: AutomationProperties.SetAutomationId.
        wnative::apply_automation_id(native, value);
    }

    void shape_view_platform::update_background(const maui::graphics::paint* value)
    {
        view_platform_base::update_background(value);
        auto host = wnative::borrow<muxc::Canvas>(native);
        if (host == nullptr)
        {
            return;
        }
        // map_background's container split (C# MapBackground: with BOTH Background and Fill set the
        // background paints the CONTAINER behind the shape — UpdateValue(ContainerView)); the Canvas
        // is the port's container (no WrapperView seam). A Background WITHOUT a Fill never reaches
        // here — it becomes the shape fill (Fill ?? Background in apply_fill / the drawable).
        if (value == nullptr)
        {
            host.ClearValue(muxc::Panel::BackgroundProperty());
            return;
        }
        if (const auto* solid = dynamic_cast<const maui::graphics::solid_paint*>(value))
        {
            host.Background(wnative::to_brush(solid->color()));
            return;
        }
        // deferred: gradient / image-source paints (Paint.ToPlatform) — the base mirror above keeps
        // the borrow observable.
    }

    std::unique_ptr<shape_view_platform> shape_view_handler::create_platform_view()
    {
        auto platform = std::make_unique<shape_view_platform>();
        try
        {
            // The stable, kind-agnostic seat (C#'s W2DGraphicsView is likewise one stable element the
            // drawable re-renders into): a Canvas whose single child is the current Shapes element.
            const muxc::Canvas host;
            platform->native = wnative::store(host); // released in ~shape_view_platform
        }
        catch (const winrt::hresult_error&)
        {
            platform->native = nullptr; // XAML-less degradation (header note)
        }
        return platform;
    }

    // C# UpdateShape (ShapeViewExtensions): re-point the host's drawable at the virtual view + redraw.
    // The shared headless mirror (drawable + winding/render-transform + invalidations) is kept live;
    // the native redraw is the child-element rebuild (the kind may have changed — header note).
    void shape_view_handler::update_shape()
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->drawable.update_shape_view(virtual_view());
        refresh_drawable_state();
        platform->invalidations++;
        rebuild_native_shape(*platform, virtual_view());
    }

    // C# InvalidateShape: refresh the drawable pushes + count the redraw request, then re-render —
    // here the same full child rebuild (every stroke/fill/points/radius key funnels through this).
    void shape_view_handler::invalidate_shape()
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        refresh_drawable_state();
        platform->invalidations++;
        rebuild_native_shape(*platform, virtual_view());
    }

    void shape_view_handler::arrange_native(const maui::graphics::rect& frame)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return; // XAML-less: no native host to frame (the headless twin's no-op)
        }
        // The shared Canvas recipe: Canvas.SetLeft/SetTop + explicit Width/Height pin the container to
        // the frame (ViewHandlerExtensions.Windows.cs PlatformArrangeHandler on the Canvas layout model).
        wnative::arrange_native(platform->native, frame);
        // The child spans the container (C# draws the shape over the graphics view's full bounds):
        // 0,0 + the frame extent — the Stretch / radius fitting is native from here.
        if (platform->shape_element != nullptr)
        {
            wnative::arrange_native(platform->shape_element,
                                    maui::graphics::rect{0, 0, frame.width, frame.height});
        }
    }
} // namespace maui::core
