// shape_view_handler — WinUI 3 platform recipe. C#'s Windows PlatformView is a Win2D W2DGraphicsView
// that replays the SAME ShapeDrawable this port already has (shape_drawable.hpp) through an ICanvas
// (ShapeViewHandler.Windows.cs). This port has no Win2D/Direct2D canvas backend, and the per-shape
// Windows partials it would otherwise mirror (BoxViewHandler/RectangleHandler/LineHandler/
// PathHandler/PolygonHandler/PolylineHandler/RoundRectangleHandler.Windows.cs) all funnel through
// that one Win2D view too — there is no "native XAML shape element" oracle to copy 1:1.
//
// So this backend takes the same approach as label_handler.cpp's Border-wrapped TextBlock: reach for
// the closest REAL WinUI element that renders the same pixels, rather than plumbing a whole Direct2D
// canvas. Microsoft.UI.Xaml.Shapes.Path already IS a general vector-shape renderer (arbitrary
// PathGeometry, Fill, Stroke, dash pattern, caps/joins, fill rule) — exactly ShapeDrawable's job,
// just addressed through the XAML object model instead of an immediate-mode canvas. The geometry
// comes from the SAME i_shape::path_for_bounds() the drawable calls (shape_drawable.cpp); this file
// never invents shape math, it only re-hosts the existing path in a native Path element.
//
// Fill/Stroke/dash/caps/joins mirror shape_drawable::draw_fill_path / draw_stroke_path member for
// member (see that file): Fill ?? Background for the interior, Stroke gated on stroke_thickness > 0,
// winding mode drives the fill rule.
//
// Native host: a Border (like label) wrapping a Path. The Border only ever paints when the shape sets
// BOTH Background and Fill (map_background's container-background case, src/core/shape_view_handler.cpp)
// — otherwise it stays transparent and the Path's own Fill carries the Fill ?? Background rule.

#include "maui/core/shape_view_handler.hpp"

#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Microsoft.UI.Xaml.Media.h>
#include <winrt/Microsoft.UI.Xaml.Shapes.h>
#include <winrt/Microsoft.UI.Xaml.h>
// The C++/WinRT include rule (see winui_interop.hpp): you need the FULL header for every
// namespace whose MEMBERS you call. Without this one, IVector<T>::Append is only
// forward-declared and every call fails with "error C3779: a function that returns 'auto'
// cannot be used before it is defined" - which does not read as "add an include".
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Foundation.h>

#include <cmath>
#include <cstddef>
#include <memory>
#include <optional>
#include <vector>

#include "maui/core/i_shape_view.hpp"
#include "maui/graphics/i_shape.hpp"
#include "maui/graphics/line_cap.hpp"
#include "maui/graphics/line_join.hpp"
#include "maui/graphics/matrix3x2.hpp"
#include "maui/graphics/paint.hpp"
#include "maui/graphics/path_f.hpp"
#include "maui/graphics/path_operation.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/winding_mode.hpp"
#include "winui_interop.hpp"
#include "winui_visual_ops.hpp"

namespace
{
    // Named `winui`, NOT `xaml` — see label_handler.cpp's note (maui::xaml would shadow the alias).
    namespace winui = winrt::Microsoft::UI::Xaml;
    using border = winui::Controls::Border;
    using shape_path = winui::Shapes::Path;

    // The void* slot boxes the BASE UIElement (see winui_interop.hpp / label_handler.cpp's as_host).
    border as_host(void* native)
    {
        return maui::platform::windows::ref<winui::UIElement>(native).as<border>();
    }

    shape_path as_path(void* native)
    {
        return as_host(native).Child().as<shape_path>();
    }

    winui::Media::PenLineCap to_pen_line_cap(maui::graphics::line_cap value)
    {
        switch (value)
        {
            case maui::graphics::line_cap::round:
                return winui::Media::PenLineCap::Round;
            case maui::graphics::line_cap::square:
                return winui::Media::PenLineCap::Square;
            case maui::graphics::line_cap::butt:
            default:
                return winui::Media::PenLineCap::Flat;
        }
    }

    winui::Media::PenLineJoin to_pen_line_join(maui::graphics::line_join value)
    {
        switch (value)
        {
            case maui::graphics::line_join::round:
                return winui::Media::PenLineJoin::Round;
            case maui::graphics::line_join::bevel:
                return winui::Media::PenLineJoin::Bevel;
            case maui::graphics::line_join::miter:
            default:
                return winui::Media::PenLineJoin::Miter;
        }
    }

    // Build a PathGeometry from an ALREADY-FLATTENED path_f (move/line/close only — see the
    // get_flattened_path(..., include_sub_paths=true) call below). Flattening quad/cubic/arc segments
    // to a fine polyline (path_f's own ArcFlattener-derived logic, ported already for bounds
    // computation) sidesteps hand-converting the port's center+angle arc representation into WinUI's
    // endpoint-parameterized ArcSegment — reusing tested port math instead of writing new arc trig.
    // ponytail: polyline approximation, not native ArcSegment/BezierSegment — upgrade to exact curve
    // segments only if a future parity pass finds visible faceting (path_f's default 0.001 flatness
    // is sub-pixel for gallery-sized shapes, so it shouldn't).
    winui::Media::PathGeometry build_path_geometry(const maui::graphics::path_f& flattened,
                                                   maui::graphics::winding_mode winding)
    {
        using maui::graphics::path_operation;

        winui::Media::PathGeometry geometry;
        geometry.FillRule(winding == maui::graphics::winding_mode::even_odd ? winui::Media::FillRule::EvenOdd
                                                                            : winui::Media::FillRule::Nonzero);

        const std::vector<maui::graphics::point_f>& points = flattened.points();
        std::size_t point_index = 0;
        winui::Media::PathFigure figure = nullptr;
        winui::Media::PolyLineSegment segment = nullptr;
        for (const path_operation op : flattened.segment_types())
        {
            switch (op)
            {
                case path_operation::move: {
                    if (figure != nullptr)
                    {
                        geometry.Figures().Append(figure);
                    }
                    const maui::graphics::point_f& p = points[point_index++];
                    figure = winui::Media::PathFigure{};
                    figure.StartPoint(winrt::Windows::Foundation::Point{p.x, p.y});
                    figure.IsClosed(false);
                    segment = winui::Media::PolyLineSegment{};
                    figure.Segments().Append(segment);
                    break;
                }
                case path_operation::line: {
                    const maui::graphics::point_f& p = points[point_index++];
                    if (segment != nullptr)
                    {
                        segment.Points().Append(winrt::Windows::Foundation::Point{p.x, p.y});
                    }
                    break;
                }
                case path_operation::close:
                    if (figure != nullptr)
                    {
                        figure.IsClosed(true);
                    }
                    break;
                case path_operation::quad:
                case path_operation::cubic:
                case path_operation::arc:
                    // Do not survive get_flattened_path() — unreachable here.
                    break;
            }
        }
        if (figure != nullptr)
        {
            geometry.Figures().Append(figure);
        }
        return geometry;
    }

    // The shared body of update_shape/invalidate_shape/arrange_native: rebuild the Path's geometry +
    // Fill + Stroke from the current virtual-view state. Always re-reads everything (like C#'s
    // InvalidateShape, which has no per-property granularity either) rather than tracking which single
    // property changed.
    void refresh_native_shape(maui::core::shape_view_platform& platform, const maui::core::i_shape_view* view)
    {
        if (view == nullptr || platform.native == nullptr)
        {
            return;
        }
        const shape_path content = as_path(platform.native);
        const border host = as_host(platform.native);

        // Bounds come from the LAST arranged size (Border.Width/Height, set by arrange_native — a
        // Canvas child has no other source of size). NaN means "not laid out yet"; nothing to draw
        // until the next arrange, matching the get_desired_size / platform_arrange precedent elsewhere
        // (the border/image handlers' "0×0 before first layout" guard).
        const double width = host.Width();
        const double height = host.Height();
        if (!std::isfinite(width) || !std::isfinite(height) || width <= 0 || height <= 0)
        {
            content.Data(nullptr);
            return;
        }

        const maui::graphics::i_shape* shape = view->shape();
        if (shape == nullptr)
        {
            content.Data(nullptr);
            content.Fill(nullptr);
            content.Stroke(nullptr);
            return;
        }

        // C# ShapeDrawable.Draw: PathForBounds, then ApplyTransform (the render-transform push the
        // port collapses onto i_shape_view — see the shape_view_handler.hpp collapse note).
        maui::graphics::path_f path = shape->path_for_bounds(maui::graphics::rect{0, 0, width, height});
        if (const std::optional<maui::graphics::matrix3x2> transform = view->render_transform_matrix())
        {
            path.transform(*transform);
        }
        const maui::graphics::path_f flattened = path.get_flattened_path(0.001F, /*include_sub_paths=*/true);
        content.Data(build_path_geometry(flattened, view->fill_winding()));

        // C# DrawFillPath: Fill ?? Background paints the interior; neither set means no fill (the
        // drawable stages a transparent fill color in that case — a null Path.Fill is the same result).
        const maui::graphics::paint* fill_paint = view->fill();
        if (fill_paint == nullptr)
        {
            fill_paint = view->background();
        }
        content.Fill(fill_paint != nullptr ? maui::platform::windows::brush_for(*fill_paint) : nullptr);

        // C# DrawStrokePath's early-return guard: no stroke brush or a non-positive thickness draws
        // nothing at all.
        const maui::graphics::paint* stroke = view->stroke();
        const double stroke_thickness = view->stroke_thickness();
        if (stroke == nullptr || stroke_thickness <= 0)
        {
            content.Stroke(nullptr);
            return;
        }
        content.Stroke(maui::platform::windows::brush_for(*stroke));
        content.StrokeThickness(stroke_thickness);
        // IStroke carries ONE line-cap value applied uniformly (like ICanvas.StrokeLineCap, one state
        // for the whole stroke), so all three WinUI cap slots get the same mapped value.
        const winui::Media::PenLineCap cap = to_pen_line_cap(view->stroke_line_cap());
        content.StrokeStartLineCap(cap);
        content.StrokeEndLineCap(cap);
        content.StrokeDashCap(cap);
        content.StrokeLineJoin(to_pen_line_join(view->stroke_line_join()));
        content.StrokeMiterLimit(view->stroke_miter_limit());

        // WinUI expresses Shape.StrokeDashArray / StrokeDashOffset as MULTIPLES of StrokeThickness;
        // the port's dash pattern/offset (mirroring ICanvas.StrokeDashPattern/StrokeDashOffset) are raw
        // lengths in the same units as StrokeThickness itself, so divide through. An empty pattern
        // (the C# null analog) becomes an empty DoubleCollection, i.e. a solid line.
        winui::Media::DoubleCollection dashes;
        for (const float d : view->stroke_dash_pattern())
        {
            dashes.Append(static_cast<double>(d) / stroke_thickness);
        }
        content.StrokeDashArray(dashes);
        content.StrokeDashOffset(static_cast<double>(view->stroke_dash_offset()) / stroke_thickness);
    }
} // namespace

namespace maui::core
{
    shape_view_platform::~shape_view_platform()
    {
        if (native != nullptr)
        {
            maui::platform::windows::drop<winui::UIElement>(native);
        }
    }

    // The headless drawRect twin — not the real Windows paint path (that's refresh_native_shape
    // above), but declared unconditionally in the header, so every backend defines it (the golden-op
    // replay tests call it directly against a recording_canvas).
    void shape_view_platform::replay(maui::graphics::i_canvas& canvas, const maui::graphics::rect_f& dirty_rect)
    {
        drawable.draw(canvas, dirty_rect);
    }

    std::unique_ptr<shape_view_platform> shape_view_handler::create_platform_view()
    {
        auto platform = std::make_unique<shape_view_platform>();
        border host;
        shape_path content;
        content.HorizontalAlignment(winui::HorizontalAlignment::Stretch);
        content.VerticalAlignment(winui::VerticalAlignment::Stretch);
        host.Child(content);
        platform->native = maui::platform::windows::take<winui::UIElement>(host);
        return platform;
    }

    // C# UpdateShape (ShapeViewExtensions): re-point the drawable at the virtual view, refresh the
    // winding/render-transform pushes, then redraw.
    void shape_view_handler::update_shape()
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        platform->drawable.update_shape_view(virtual_view());
        refresh_drawable_state();
        platform->invalidations++;
        refresh_native_shape(*platform, virtual_view());
    }

    // C# InvalidateShape: refresh the drawable pushes, then redraw.
    void shape_view_handler::invalidate_shape()
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        refresh_drawable_state();
        platform->invalidations++;
        refresh_native_shape(*platform, virtual_view());
    }

    void shape_view_handler::arrange_native(const maui::graphics::rect& frame)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        // PlatformArrangeHandler's guard, widened to non-finite — see label_handler.cpp's identical
        // guard for why (a NaN reaching XAML's Canvas layout is an unrecoverable stowed exception).
        if (!std::isfinite(frame.x) || !std::isfinite(frame.y) || !std::isfinite(frame.width) ||
            !std::isfinite(frame.height) || frame.width < 0 || frame.height < 0)
        {
            return;
        }
        const border host = as_host(platform->native);
        winui::Controls::Canvas::SetLeft(host, frame.x);
        winui::Controls::Canvas::SetTop(host, frame.y);
        host.Width(frame.width);
        host.Height(frame.height);
        // The shape's geometry is computed FOR these bounds (path_for_bounds), so a resize has to
        // rebuild it — unlike label/button, where the native control lays out its own content.
        refresh_native_shape(*platform, virtual_view());
    }

    // ---- generic-IView property pushes (view_platform_base overrides) ---------------------------
    // Delegated to the shared winui_visual_ops free functions, same as every other Windows control's
    // platform struct (label_handler.cpp / button_handler.cpp).
    void shape_view_platform::update_visibility(maui::core::visibility value)
    {
        maui::platform::windows::apply_visibility(native, value);
    }

    void shape_view_platform::update_opacity(double value)
    {
        maui::platform::windows::apply_opacity(native, value);
    }

    void shape_view_platform::update_is_enabled(bool value)
    {
        maui::platform::windows::apply_is_enabled(native, value);
    }

    void shape_view_platform::update_automation_id(std::string_view value)
    {
        maui::platform::windows::apply_automation_id(native, value);
    }

    void shape_view_platform::update_background(const maui::graphics::paint* value)
    {
        maui::platform::windows::apply_background(native, value);
    }
} // namespace maui::core
