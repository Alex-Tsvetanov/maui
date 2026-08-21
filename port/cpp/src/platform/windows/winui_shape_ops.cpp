// winui_shape_ops — see the header for what this is and why it is shared. The walk below is moved
// VERBATIM from shape_view_handler.cpp (the already-fixed, MSVC-verified copy) except for the
// FillRule branch, which is widened from an unconditional set to an optional one so
// border_handler.cpp's call site can opt out instead of silently getting a FillRule it never had.
// Do not "improve" this walk here — it is the single known-good implementation both callers now
// share; if something else looks wrong, fix it separately so this move stays attributable.

// THE D2D BLOCK COMES FIRST, BEFORE THIS TU'S OWN HEADER — deliberately, and it is the one ordering
// that compiles. d2d_geometry_source below implements a CLASSIC-COM interface through
// winrt::implements, and C++/WinRT hard-fails that with `static_assert: To implement classic COM
// interfaces, you must #include <unknwn.h> before including C++/WinRT headers` (winrt/base.h:7158),
// followed by two C2555s on AddRef/Release. d2d1.h is what pulls unknwn.h in, and winui_shape_ops.hpp
// pulls winrt/Microsoft.UI.Composition.h — so d2d1.h must come FIRST. This is border_handler.cpp's proven
// order, which had d2d1.h ahead of every winrt include for the same reason. MEASURED, not guessed: the
// own-header-first arrangement failed exactly this way on the guest (2026-08-22).
//
// d2d1.h also pulls in windows.h, whose GetCurrentTime function-like macro then eats the argument list
// of the projection's Timeline::GetCurrentTime (C4002); the #undef is immediate, so nothing after this
// block sees it. windows.graphics.interop.h must follow d2d1.h: it names ID2D1Geometry / ID2D1Factory.
#include <d2d1.h>
#undef GetCurrentTime
#include <windows.graphics.interop.h>

#include "winui_shape_ops.hpp"

#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Microsoft.UI.Xaml.h>
// The C++/WinRT include rule (see winui_interop.hpp): the FULL header for every namespace whose
// MEMBERS are called. Without this one, IVector<T>::Append is only forward-declared and every call
// fails with "error C3779: a function that returns 'auto' cannot be used before it is defined" —
// which does not read as "add an include".
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Graphics.h>

#include <cmath>
#include <cstddef>
#include <numbers>
#include <utility>
#include <vector>

#include "maui/graphics/path_f.hpp"
#include "maui/graphics/path_operation.hpp"
#include "maui/graphics/point_f.hpp"

namespace
{
    // Named `winui`, NOT `xaml` — see label_handler.cpp's note (maui::xaml would shadow the alias).
    namespace winui = winrt::Microsoft::UI::Xaml;

    // GeometryUtil.EllipseAngleToPoint(x, y, w, h, angleInDegrees): the point on the ellipse bounded by
    // (x, y, w, h) at the given angle. Oracle: src/Graphics/src/Graphics/GeometryUtil.cs:131-143.
    maui::graphics::point_f ellipse_angle_to_point(float x, float y, float w, float h, float angle_degrees)
    {
        const double radians = angle_degrees * (std::numbers::pi / 180.0);
        const float cx = x + (w / 2);
        const float cy = y + (h / 2);
        return {cx + ((w / 2) * static_cast<float>(std::cos(radians))),
                cy + ((h / 2) * static_cast<float>(std::sin(radians)))};
    }

    // GeometryUtil.GetSweep(angle1, angle2, clockwise): oracle src/Graphics/src/Graphics/GeometryUtil.cs:88-112.
    // Only feeds the ArcSegment.IsLargeArc flag below — the sweep direction itself is `clockwise` verbatim.
    float get_sweep(float angle1, float angle2, bool clockwise)
    {
        if (clockwise)
        {
            return angle2 > angle1 ? angle1 + (360 - angle2) : angle1 - angle2;
        }
        return angle1 > angle2 ? angle2 + (360 - angle1) : angle2 - angle1;
    }

    // ---- the content clip (ContentPanel.UpdateClip) ----------------------------------------------
    // One process-wide Direct2D factory for the clip geometries. SINGLE_THREADED: every call below is on
    // the UI thread. Deliberately never released — a function-local com_ptr would release at static
    // teardown, after COM is already gone.
    ID2D1Factory* d2d_factory()
    {
        static ID2D1Factory* const factory = []() -> ID2D1Factory* {
            ID2D1Factory* created = nullptr;
            if (FAILED(::D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &created)))
            {
                return nullptr;
            }
            return created;
        }();
        return factory;
    }

    // The Win2D stand-in (file header, deviation 2). Windows.Graphics.IGeometrySource2D is a pure marker
    // interface — CompositionPath does all of its real work through the classic-COM
    // IGeometrySource2DInterop it QIs for, which is what these two methods answer.
    struct d2d_geometry_source : winrt::implements<d2d_geometry_source, winrt::Windows::Graphics::IGeometrySource2D,
                                                   ABI::Windows::Graphics::IGeometrySource2DInterop>
    {
        explicit d2d_geometry_source(winrt::com_ptr<ID2D1PathGeometry> geometry) : geometry_{std::move(geometry)}
        {
        }

        IFACEMETHODIMP GetGeometry(ID2D1Geometry** value) noexcept final
        {
            if (value == nullptr)
            {
                return E_POINTER;
            }
            // A plain base-class upcast (single inheritance), so it cannot fail — no QI. The member keeps
            // the PATH geometry type rather than the base because ::Stream below is a PathGeometry member
            // (ID2D1Geometry's own streaming face is Simplify, which would re-approximate).
            ID2D1Geometry* const geometry = geometry_.get();
            geometry->AddRef();
            *value = geometry;
            return S_OK;
        }

        // The compositor asks for the geometry on ITS OWN D2D factory first. Rather than bet on a
        // foreign-factory geometry being accepted, replay ours into a path geometry the caller's factory
        // owns — ID2D1PathGeometry::Stream re-emits the figures into any sink, so this is exact, not a
        // re-approximation.
        IFACEMETHODIMP TryGetGeometryUsingFactory(ID2D1Factory* factory, ID2D1Geometry** value) noexcept final
        {
            if (value == nullptr)
            {
                return E_POINTER;
            }
            *value = nullptr;
            if (factory == nullptr || !geometry_)
            {
                return S_OK; // "cannot" — the caller falls back to GetGeometry
            }
            winrt::com_ptr<ID2D1PathGeometry> replayed;
            if (const HRESULT hr = factory->CreatePathGeometry(replayed.put()); FAILED(hr))
            {
                return hr;
            }
            winrt::com_ptr<ID2D1GeometrySink> sink;
            if (const HRESULT hr = replayed->Open(sink.put()); FAILED(hr))
            {
                return hr;
            }
            if (const HRESULT hr = geometry_->Stream(sink.get()); FAILED(hr))
            {
                return hr;
            }
            if (const HRESULT hr = sink->Close(); FAILED(hr))
            {
                return hr;
            }
            *value = replayed.detach();
            return S_OK;
        }

    private:
        winrt::com_ptr<ID2D1PathGeometry> geometry_;
    };

    // maui::graphics::path_f → an ID2D1PathGeometry, FLATTENED to line segments. Unlike the stroke path
    // (winui_shape_ops' walk, which emits real quad/cubic/arc segments because the stroke is RENDERED
    // from it), a clip is a mask, so path_f's own curve flattener — the port of MAUI's PathF.
    // GetFlattenedPath, the same code MAUI would use — keeps this walk to BeginFigure/AddLine at a
    // 0.001 px error bound, two orders of magnitude under a pixel. `include_sub_paths = true` is REQUIRED:
    // the default stops at the FIRST Close, silently dropping every later sub-path of a multi-figure shape.
    winrt::com_ptr<ID2D1PathGeometry> build_clip_geometry(const maui::graphics::path_f& path)
    {
        ID2D1Factory* const factory = d2d_factory();
        if (factory == nullptr)
        {
            return nullptr;
        }
        winrt::com_ptr<ID2D1PathGeometry> geometry;
        if (FAILED(factory->CreatePathGeometry(geometry.put())))
        {
            return nullptr;
        }
        winrt::com_ptr<ID2D1GeometrySink> sink;
        if (FAILED(geometry->Open(sink.put())))
        {
            return nullptr;
        }
        // EvenOdd — WinUI's own PathGeometry default, which the stroke path leaves standing too (see the
        // build_path_geometry call site below on why Border has no winding surface to set it from). D2D
        // requires the fill mode BEFORE the first figure.
        sink->SetFillMode(D2D1_FILL_MODE_ALTERNATE);
        const maui::graphics::path_f flat = path.get_flattened_path(0.001F, true);
        bool in_figure = false;
        int point_index = 0;
        for (const maui::graphics::path_operation op : flat.segment_types())
        {
            switch (op)
            {
                case maui::graphics::path_operation::move: {
                    if (in_figure)
                    {
                        sink->EndFigure(D2D1_FIGURE_END_CLOSED);
                    }
                    const maui::graphics::point_f point = flat[point_index++];
                    sink->BeginFigure(D2D1::Point2F(point.x, point.y), D2D1_FIGURE_BEGIN_FILLED);
                    in_figure = true;
                    break;
                }
                case maui::graphics::path_operation::line: {
                    const maui::graphics::point_f point = flat[point_index++];
                    if (in_figure)
                    {
                        sink->AddLine(D2D1::Point2F(point.x, point.y));
                    }
                    break;
                }
                case maui::graphics::path_operation::close:
                    if (in_figure)
                    {
                        sink->EndFigure(D2D1_FIGURE_END_CLOSED);
                        in_figure = false;
                    }
                    break;
                default:
                    break; // flattening leaves only move/line/close
            }
        }
        // Every figure ends CLOSED, including one the path left open: this geometry is a FILL region (a
        // mask), and D2D fills an open figure by implicitly closing it anyway.
        if (in_figure)
        {
            sink->EndFigure(D2D1_FIGURE_END_CLOSED);
        }
        if (FAILED(sink->Close()))
        {
            return nullptr;
        }
        return geometry;
    }
} // namespace

namespace maui::platform::windows
{
    // Build a PathGeometry straight from the shape's own path — no pre-flattening. Mirrors
    // GraphicsExtensions.AsCanvasGeometry (src/Graphics/src/Graphics/Platforms/Windows/
    // GraphicsExtensions.cs:119-248) operation by operation, swapping Win2D's imperative
    // CanvasPathBuilder for WinUI's PathFigure/PathSegment object model: Line becomes a running
    // PolyLineSegment (AddLine), Quad a QuadraticBezierSegment (AddQuadraticBezier, :165-173), Cubic a
    // BezierSegment (AddCubicBezier, :174-183), Arc an ArcSegment (AddArc, :184-232). A curve/arc
    // segment ends whatever PolyLineSegment was running — WinUI needs a distinct segment OBJECT per
    // kind, unlike Win2D's single builder — so `segment` is nulled after each one and lazily
    // re-created by the next `line`, the same lazy pattern `move` already uses for `figure`.
    winui::Media::PathGeometry build_path_geometry(const maui::graphics::path_f& path,
                                                   std::optional<maui::graphics::winding_mode> winding)
    {
        using maui::graphics::path_operation;

        winui::Media::PathGeometry geometry;
        // nullopt (border_handler.cpp's call) leaves FillRule at WinUI's own default, untouched — see
        // this file's header comment for why Border has nothing to set it from.
        if (winding.has_value())
        {
            geometry.FillRule(*winding == maui::graphics::winding_mode::even_odd ? winui::Media::FillRule::EvenOdd
                                                                                 : winui::Media::FillRule::Nonzero);
        }

        const std::vector<maui::graphics::point_f>& points = path.points();
        std::size_t point_index = 0;
        int arc_angle_index = 0;
        int arc_cw_index = 0;
        std::optional<path_operation> previous_op;
        winui::Media::PathFigure figure = nullptr;
        winui::Media::PolyLineSegment segment = nullptr;
        for (const path_operation op : path.segment_types())
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
                    segment = nullptr;
                    break;
                }
                case path_operation::line: {
                    const maui::graphics::point_f& p = points[point_index++];
                    if (figure != nullptr)
                    {
                        if (segment == nullptr)
                        {
                            segment = winui::Media::PolyLineSegment{};
                            figure.Segments().Append(segment);
                        }
                        segment.Points().Append(winrt::Windows::Foundation::Point{p.x, p.y});
                    }
                    break;
                }
                case path_operation::quad: {
                    // GraphicsExtensions.cs:165-173 (AddQuadraticBezier): control point, then end point.
                    const maui::graphics::point_f& control_point = points[point_index++];
                    const maui::graphics::point_f& end_point = points[point_index++];
                    if (figure != nullptr)
                    {
                        winui::Media::QuadraticBezierSegment quad_segment;
                        quad_segment.Point1(winrt::Windows::Foundation::Point{control_point.x, control_point.y});
                        quad_segment.Point2(winrt::Windows::Foundation::Point{end_point.x, end_point.y});
                        figure.Segments().Append(quad_segment);
                        segment = nullptr;
                    }
                    break;
                }
                case path_operation::cubic: {
                    // GraphicsExtensions.cs:174-183 (AddCubicBezier): control1, control2, then end point.
                    const maui::graphics::point_f& control1 = points[point_index++];
                    const maui::graphics::point_f& control2 = points[point_index++];
                    const maui::graphics::point_f& end_point = points[point_index++];
                    if (figure != nullptr)
                    {
                        winui::Media::BezierSegment bezier_segment;
                        bezier_segment.Point1(winrt::Windows::Foundation::Point{control1.x, control1.y});
                        bezier_segment.Point2(winrt::Windows::Foundation::Point{control2.x, control2.y});
                        bezier_segment.Point3(winrt::Windows::Foundation::Point{end_point.x, end_point.y});
                        figure.Segments().Append(bezier_segment);
                        segment = nullptr;
                    }
                    break;
                }
                case path_operation::arc: {
                    // GraphicsExtensions.cs:184-232 (AddArc): endpoint-parameterized — the ellipse's own
                    // start/end points (GeometryUtil.EllipseAngleToPoint) plus the large-arc flag
                    // (GeometryUtil.GetSweep); sweep direction is `clockwise` directly (:230).
                    const maui::graphics::point_f& top_left = points[point_index++];
                    const maui::graphics::point_f& bottom_right = points[point_index++];
                    float start_angle = path.get_arc_angle(arc_angle_index++);
                    float end_angle = path.get_arc_angle(arc_angle_index++);
                    const bool clockwise = path.get_arc_clockwise(arc_cw_index++);
                    // GraphicsExtensions.cs:192-200: fold negative angles up into [0, 360) first.
                    while (start_angle < 0)
                    {
                        start_angle += 360;
                    }
                    while (end_angle < 0)
                    {
                        end_angle += 360;
                    }

                    const float rect_width = bottom_right.x - top_left.x;
                    const float rect_height = bottom_right.y - top_left.y;
                    const maui::graphics::point_f start_point =
                        ellipse_angle_to_point(top_left.x, top_left.y, rect_width, rect_height, -start_angle);
                    const maui::graphics::point_f end_point =
                        ellipse_angle_to_point(top_left.x, top_left.y, rect_width, rect_height, -end_angle);

                    // path_f::add_arc (path_f.cpp:333-349) only opens a NEW sub-path when it's the very
                    // first operation or follows a `close` — mirrored here instead of the Win2D builder's
                    // own figureOpen bookkeeping (an artifact of CanvasPathBuilder's imperative
                    // Begin/EndFigure API that has no equivalent question once ported to WinUI's
                    // PathFigure/PathSegment object model).
                    // TODO: verify against GraphicsExtensions.cs if a real path is ever built with a
                    // `close` immediately followed by a bare arc (no intervening move) — unreached by any
                    // current shape or PathGeometry today (path_geometry.cpp's ArcSegment flattens straight
                    // to lines, mirroring PathGeometry.AddArc, so it never emits path_operation::arc), so
                    // this branch has no oracle render to check against.
                    if (figure == nullptr || previous_op == path_operation::close)
                    {
                        if (figure != nullptr)
                        {
                            geometry.Figures().Append(figure);
                        }
                        figure = winui::Media::PathFigure{};
                        figure.StartPoint(winrt::Windows::Foundation::Point{start_point.x, start_point.y});
                        figure.IsClosed(false);
                        segment = nullptr;
                    }
                    else
                    {
                        if (segment == nullptr)
                        {
                            segment = winui::Media::PolyLineSegment{};
                            figure.Segments().Append(segment);
                        }
                        segment.Points().Append(winrt::Windows::Foundation::Point{start_point.x, start_point.y});
                    }

                    const float rotation = get_sweep(start_angle, end_angle, clockwise);
                    winui::Media::ArcSegment arc_segment;
                    arc_segment.Point(winrt::Windows::Foundation::Point{end_point.x, end_point.y});
                    arc_segment.Size(winrt::Windows::Foundation::Size{rect_width / 2, rect_height / 2});
                    arc_segment.RotationAngle(0.0);
                    arc_segment.IsLargeArc(std::abs(rotation) >= 180);
                    arc_segment.SweepDirection(clockwise ? winui::Media::SweepDirection::Clockwise
                                                         : winui::Media::SweepDirection::Counterclockwise);
                    figure.Segments().Append(arc_segment);
                    segment = nullptr;
                    break;
                }
                case path_operation::close:
                    if (figure != nullptr)
                    {
                        figure.IsClosed(true);
                    }
                    break;
            }
            previous_op = op;
        }
        if (figure != nullptr)
        {
            geometry.Figures().Append(figure);
        }
        return geometry;
    }

    winrt::Microsoft::UI::Composition::CompositionPathGeometry build_composition_clip_geometry(
        const winrt::Microsoft::UI::Composition::Compositor& compositor, const maui::graphics::path_f& path)
    {
        // WrapperView.cs:117-120, the three lines after AsPath: CompositionPath over the geometry source,
        // then CreatePathGeometry. The caller wraps it in CreateGeometricClip (WrapperView.cs:121).
        const winrt::com_ptr<ID2D1PathGeometry> geometry = build_clip_geometry(path);
        if (!geometry)
        {
            return nullptr;
        }
        const winrt::Microsoft::UI::Composition::CompositionPath composition_path{
            winrt::make<d2d_geometry_source>(geometry)};
        return compositor.CreatePathGeometry(composition_path);
    }
} // namespace maui::platform::windows
