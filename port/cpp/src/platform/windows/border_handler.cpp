// border_handler — WinUI 3 platform recipe. C#'s Windows PlatformView is a ContentPanel (a custom
// Panel hosting a stroke Path + the content child) — see src/Core/src/Platform/Windows/ContentPanel.cs +
// BorderExtensions.cs/StrokeExtensions.cs (the oracle for every push below) + BorderHandler.Windows.cs
// (UpdateContent). This port has no custom-Panel seam yet (no C++/WinRT DependencyObject subclass), so
// the host is a plain Canvas — the same simplification content_page_handler.cpp / layout_handler.cpp
// already make for "a panel that just holds children the cross-platform layout positions itself" — with
// TWO PERMANENT children, both appended once in create_platform_view and never removed: the stroke Path
// (index 0) and a content-host Canvas (index 1) that holds the hosted content and carries the content
// clip (see THE CONTENT CLIP below). Both indices are fixed for the platform view's whole lifetime, so
// as_path() / as_content_host() below need no separate boxed handles.
//
// Background routing (BorderHandler.cs's MapBackground + ViewExtensions.UpdateBorderBackground):
// Border.Shape defaults to a Rectangle (border.cpp's stroke_shape_property()) and is never null in this
// port, so C#'s "hasBorder" branch always applies — Background paints the STROKE PATH's Fill (so it
// follows the border's shape, e.g. rounded corners), not the host's own rectangular Background. That is
// why border_platform::update_background is NOT delegated to the shared apply_background helper the way
// every other Windows control's is (that would paint the Canvas's full rectangular bounds instead).
//
// get_desired_size / platform_arrange are NOT per-backend here: a Border computes its own size and
// content placement through the control (Border::measure/arrange in border.cpp), exactly like
// content_page/layout — both already implemented once, cross-platform, in src/core/border_handler.cpp.
// Only the per-backend HALF of platform_arrange (arrange_native: frame the host) lives here.
//
// THE CONTENT CLIP (ContentPanel.UpdateClip), previously deferred here and now ported: C# masks the
// CONTENT — not the stroke — to the border's INNER shape, so a photo or an oversized glyph cannot spill
// past a circular/triangular/rounded border. Microsoft.UI.Xaml.UIElement.Clip only accepts a
// RectangleGeometry, so a non-rectangular clip has to go through Composition: a CompositionGeometricClip
// over a CompositionPathGeometry, set on the visual behind the content.
//
// TWO deviations from the C# shape, both structural, neither changing the resulting geometry:
//
//  1. THE CLIPPED VISUAL. C# puts the clip on the CONTENT's own visual and then cancels the content's
//     position out of it (`geometricClip.Offset = strokeThickness - Content.ActualOffset`), which lands
//     the clip at (T, T) in the panel's space whatever the content's alignment did. This port gives the
//     host a THIRD permanent child instead — a content-host Canvas spanning the whole border box, with
//     the content inside it — and clips THAT. Its visual's origin IS the border's origin, so the same
//     placement is a plain translate by (T, T) with no content-position term at all. That matters here
//     for a reason C# does not have: border::arrange (src/controls/border.cpp) frames the handler BEFORE
//     it arranges the content, so a content-relative offset read during arrange_native would be one
//     layout pass stale — C#'s ArrangeOverride runs AFTER base.ArrangeOverride has placed the children.
//     A permanent clipped visual also survives a content change for free (C# has to re-clip, since a new
//     content element means a new visual).
//  2. THE GEOMETRY SOURCE. CompositionPath takes a Windows.Graphics.IGeometrySource2D, and C# gets one
//     from Win2D (`CanvasDevice.GetSharedDevice()` + `PathF.AsPath(device)`). Win2D has no C++/WinRT
//     projection in this build, so d2d_geometry_source below implements that interface directly over a
//     Direct2D geometry — d2d1.lib ships in the Windows SDK, so this adds NO new dependency (one extra
//     entry beside WindowsApp.lib in CMakeLists.txt; vcpkg.json is untouched).

#include "maui/core/border_handler.hpp"

// d2d1.h pulls in windows.h, whose GetCurrentTime function-like macro then eats the argument list of the
// projection's Timeline::GetCurrentTime (C4002) — host_run.cpp carries the same #undef for the same
// reason. windows.graphics.interop.h must follow d2d1.h: it names ID2D1Geometry / ID2D1Factory.
#include <d2d1.h>
#undef GetCurrentTime
#include <windows.graphics.interop.h>

#include <winrt/Microsoft.UI.Composition.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Microsoft.UI.Xaml.Hosting.h>
#include <winrt/Microsoft.UI.Xaml.Media.h>
#include <winrt/Microsoft.UI.Xaml.Shapes.h>
#include <winrt/Microsoft.UI.Xaml.h>
// The C++/WinRT include rule (see winui_interop.hpp): the FULL header for every namespace whose MEMBERS
// are called. Without this one, IVector<T>::Append/GetAt are only forward-declared and every call fails
// with "error C3779: a function that returns 'auto' cannot be used before it is defined."
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Graphics.h>

#include <algorithm>
#include <cmath>
#include <memory>
#include <string_view>
#include <utility>

#include "maui/core/i_border_view.hpp"
#include "maui/core/i_view.hpp"
#include "maui/core/i_view_handler.hpp"
#include "maui/core/view_chrome_ops.hpp"
#include "maui/graphics/corner_radius.hpp"
#include "maui/graphics/i_shape.hpp"
#include "maui/graphics/line_cap.hpp"
#include "maui/graphics/line_join.hpp"
#include "maui/graphics/matrix3x2.hpp"
#include "maui/graphics/paint.hpp"
#include "maui/graphics/path_f.hpp"
#include "maui/graphics/path_operation.hpp"
#include "maui/graphics/point_f.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/shapes/round_rectangle.hpp"
#include "winui_interop.hpp"
#include "winui_shape_ops.hpp"
#include "winui_visual_ops.hpp"

namespace
{
    // Named `winui`, NOT `xaml` — see label_handler.cpp's note (maui::xaml would shadow the alias).
    namespace winui = winrt::Microsoft::UI::Xaml;
    using canvas = winui::Controls::Canvas;
    using shape_path = winui::Shapes::Path;

    // The void* slot boxes the BASE UIElement (see winui_interop.hpp / content_page_handler.cpp's
    // as_host — a Canvas, like content_page's and layout's hosts, since the port has no custom-Panel
    // seam yet).
    canvas as_host(void* native)
    {
        return maui::platform::windows::ref<winui::UIElement>(native).as<canvas>();
    }

    // The host's two children are BOTH permanent, appended once in create_platform_view and never
    // removed, so these indices are stable for the platform view's whole lifetime: [0] the stroke Path
    // (painted behind), [1] the content-host Canvas that holds the hosted content and carries the
    // Composition content clip (see the file header). set_content() swaps children INSIDE [1].
    shape_path as_path(void* native)
    {
        return as_host(native).Children().GetAt(0).as<shape_path>();
    }

    canvas as_content_host(void* native)
    {
        return as_host(native).Children().GetAt(1).as<canvas>();
    }

    // The child's native UIElement via its view-handler's native_view() — the content_page/layout twin.
    // Null when the child is unattached, has no handler, or its handler produced no native view yet.
    winui::UIElement native_child(maui::core::i_view* view)
    {
        if (view == nullptr)
        {
            return nullptr;
        }
        auto* handler = dynamic_cast<maui::core::i_view_handler*>(view->handler().get());
        if (handler == nullptr || handler->native_view() == nullptr)
        {
            return nullptr;
        }
        return maui::platform::windows::ref<winui::UIElement>(handler->native_view());
    }

    // IStroke.StrokeLineCap/LineJoin → WinUI's PenLineCap/PenLineJoin — the same mapping
    // shape_view_handler.cpp uses (both read the identical maui::graphics enums), duplicated here rather
    // than shared: neither file exports these translators, and this slice does not touch shape_view_
    // handler.cpp (already-working, unbuildable-here-to-reverify) to extract one.
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

    // RoundRectangle.InnerPathForBounds(bounds, st) — UpdateClip's IRoundRectangle branch: GetInnerPath's
    // box (inset st/2, sized W−st, radii each reduced by st), then Shape.TransformPathForBounds' Stretch.
    // Fill rescale back onto `bounds` (RoundRectangle's ctor sets Aspect = Fill). Net effect versus a
    // plain path_for_bounds: the same box, with the corner radii pulled in by the stroke — which is
    // exactly the "rounded corners peek past the clip" difference this branch exists for.
    //
    // Built here rather than in the shared shape layer: i_shape carries only PathForBounds (i_shape.hpp
    // records IRoundRectangle as not ported), this is its ONE caller, and a shared-layer change to this
    // exact geometry was already reverted once (f1a5a17658) for leaking into paths MAUI does not inset.
    // NOT applied, for the same reason: TransformPathForBounds' own `viewBounds.X += StrokeThickness/2`
    // term — i.e. maui::core::shape_self_inset, which update_border below DOES apply to the STROKE.
    // DO NOT extend that helper to this clip. Measured on the guest (border_resize_content, light,
    // 2026-07-31): the circle+image cell's ENTIRE residual after this fix is 114 px, every one of them
    // between 0.94 and 1.04 of the clip radius (median exactly 1.00), zero interior, zero outside, and
    // discontinuous around the circumference — clip-EDGE antialiasing. A missing 0.5 DIP inset would
    // instead read as a continuous ~270 px ring biased to one side. The clip is placed correctly as-is.
    maui::graphics::path_f inner_round_rectangle_path(const maui::graphics::shapes::round_rectangle& shape,
                                                      const maui::graphics::rect& bounds, double stroke_thickness)
    {
        maui::graphics::path_f path;
        const double width = bounds.width - stroke_thickness;
        const double height = bounds.height - stroke_thickness;
        if (width <= 0 || height <= 0)
        {
            return path;
        }
        const maui::graphics::corner_radius radii = shape.corner_radius();
        const auto reduce = [stroke_thickness](double radius) {
            return static_cast<float>(std::max(0.0, radius - stroke_thickness));
        };
        const auto inset = static_cast<float>(stroke_thickness / 2.0);
        path.append_rounded_rectangle(inset, inset, static_cast<float>(width), static_cast<float>(height),
                                      reduce(radii.top_left), reduce(radii.top_right), reduce(radii.bottom_left),
                                      reduce(radii.bottom_right));
        // Stretch.Fill: scale the inner path's bounds (inset, inset, width, height) back onto `bounds`.
        const auto scale_x = static_cast<float>(bounds.width / width);
        const auto scale_y = static_cast<float>(bounds.height / height);
        path.transform(maui::graphics::matrix3x2::create_scale(scale_x, scale_y) *
                       maui::graphics::matrix3x2::create_translation(static_cast<float>(bounds.x) - (scale_x * inset),
                                                                     static_cast<float>(bounds.y) - (scale_y * inset)));
        return path;
    }

    // ContentPanel.UpdateClip itself: mask the content-host visual to the border's INNER shape.
    //
    // The bounds are UpdateClip's own and deliberately NOT shared with update_border's: this deflates by
    // TWICE the thickness (`Rect(0, 0, width - strokeThickness * 2, height - strokeThickness * 2)`) where
    // UpdatePath deflates by one and re-centers by half. A shared bounds helper between the two would be a
    // half-stroke error on all four sides.
    void apply_content_clip(void* native, const maui::core::border_stroke_spec& spec)
    {
        const canvas host = as_host(native);
        const auto visual = winui::Hosting::ElementCompositionPreview::GetElementVisual(as_content_host(native));
        const double width = host.Width();
        const double height = host.Height();
        const double thickness = spec.thickness;
        const maui::graphics::rect path_size{0, 0, width - (2 * thickness), height - (2 * thickness)};
        // No shape, not yet laid out (NaN before the first arrange_native), or a stroke thick enough to
        // swallow the box: no clip at all, matching C#'s early returns (which leave Visual.Clip unset).
        if (spec.shape == nullptr || !std::isfinite(width) || !std::isfinite(height) || path_size.width <= 0 ||
            path_size.height <= 0)
        {
            visual.Clip(nullptr);
            return;
        }
        const auto* round_rect = dynamic_cast<const maui::graphics::shapes::round_rectangle*>(spec.shape);
        maui::graphics::path_f clip = round_rect != nullptr
                                          ? inner_round_rectangle_path(*round_rect, path_size, thickness / 2.0)
                                          : spec.shape->path_for_bounds(
                                                maui::core::shape_self_inset(path_size, thickness));
        // C#'s `geometricClip.Offset = strokeThickness - Content.ActualOffset` places the clip at (T, T)
        // in the PANEL's space; this visual already starts at the host's origin (file header, deviation
        // 1), so the placement is the translate alone.
        const auto offset = static_cast<float>(thickness);
        clip.transform(maui::graphics::matrix3x2::create_translation(offset, offset));
        const winrt::com_ptr<ID2D1PathGeometry> geometry = build_clip_geometry(clip);
        if (!geometry)
        {
            visual.Clip(nullptr);
            return;
        }
        const auto compositor = visual.Compositor();
        const winrt::Microsoft::UI::Composition::CompositionPath composition_path{
            winrt::make<d2d_geometry_source>(geometry)};
        visual.Clip(compositor.CreateGeometricClip(compositor.CreatePathGeometry(composition_path)));
    }

} // namespace

namespace maui::core
{
    border_platform::~border_platform()
    {
        if (native != nullptr)
        {
            maui::platform::windows::drop<winui::UIElement>(native);
        }
    }

    std::unique_ptr<border_platform> border_handler::create_platform_view()
    {
        auto platform = std::make_unique<border_platform>();
        canvas host;
        shape_path path;
        canvas content_host;
        // Both children are permanent, appended once here in painting order — see as_path() /
        // as_content_host()'s note on why those indices never move.
        host.Children().Append(path);
        host.Children().Append(content_host);
        platform->native = maui::platform::windows::take<winui::UIElement>(host);
        return platform;
    }

    // C# UpdateContent: CachedChildren.Clear() + EnsureBorderPath() + re-parent Content. Here the stroke
    // path and the content host are permanent, so a content re-set (the mapper re-runs on every Content
    // change) only swaps the content host's single child — which cannot stack two generations of content,
    // and leaves the content host's Composition clip in place rather than losing it with the old visual.
    void border_handler::set_content()
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->hosted_content = virtual_view() != nullptr ? virtual_view()->content() : nullptr;
        if (platform->native == nullptr)
        {
            return;
        }
        const canvas content_host = as_content_host(platform->native);
        content_host.Children().Clear();
        if (const winui::UIElement element = native_child(platform->hosted_content))
        {
            content_host.Children().Append(element);
        }
    }

    // Every stroke property funnels into this ONE refresh (border_handler.hpp's collapsing of C#'s nine
    // separate Map* calls, each of which funnels into StrokeExtensions' Update* + BorderExtensions'
    // UpdatePath). Always re-reads and re-applies everything, like the headless/apple/ios partials.
    void border_handler::update_border()
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr || virtual_view() == nullptr)
        {
            return;
        }
        platform->border = make_border_stroke_spec(*virtual_view());
        const border_stroke_spec& spec = platform->border;

        const canvas host = as_host(platform->native);
        const shape_path path = as_path(platform->native);

        const double width = host.Width();
        const double height = host.Height();
        // No shape, or not yet laid out (NaN before the first arrange_native, or a collapsed 0-size),
        // draws nothing — matching apple_border_ops.hpp's "No shape... removes the stroke sublayer"
        // full-refresh contract this port's other backends already established, rather than
        // BorderExtensions.UpdatePath's literal "leave the previous Data" (a quirk of C#'s per-property
        // incremental update model this port's single full-refresh update_border does not reproduce —
        // moot in practice anyway, since StrokeShapeProperty defaults to Rectangle and is never null).
        if (spec.shape == nullptr || !std::isfinite(width) || !std::isfinite(height) || width <= 0 || height <= 0)
        {
            path.Data(nullptr);
            path.Stroke(nullptr);
            apply_content_clip(platform->native, spec); // clears the clip on this same "nothing to draw"
            return;
        }

        // BorderExtensions.UpdatePath: PathForBounds fitted to (width - thickness, height - thickness),
        // then a RenderTransform translate of (thickness/2, thickness/2) centers the stroke ON the shape
        // edge. The port bakes the same translate into the geometry via path_f::transform instead of a
        // separate XAML RenderTransform object — the identical net position, one fewer native object.
        const double thickness = spec.thickness;
        const maui::graphics::rect path_bounds{0, 0, std::max(0.0, width - thickness),
                                               std::max(0.0, height - thickness)};

        // Second, SEPARATE deflate: the default StrokeShape's OWN 0.5 DIP/side self-inset, stacked on top
        // of the Border-level deflate above (which uses the BORDER's thickness, not the shape's — Border
        // never propagates StrokeThickness to StrokeShape). The derivation, the Border.UpdateStrokeShape
        // latch behind the `thickness > 0` gate, and the measured evidence all live in shape_self_inset
        // (core/border_handler.hpp), shared with the iOS/Apple/Android partials, which carried the same
        // defect for the same reason. Scoped to the border handlers, so the CLIP paths MAUI never deflates
        // (view_chrome_ops.cpp) stay untouched — the shared-graphics-layer attempt that leaked into them
        // was reverted (`f1a5a17658`).
        //
        // NOTE this handler's earlier comment called the gate "MEASURED, not derived from src/", on the
        // reading that Shape.TransformPathForBounds applies unconditionally. That was incomplete: the gate
        // IS in src/ (Border.cs:433-439). See the shared helper.
        maui::graphics::path_f geometry =
            spec.shape->path_for_bounds(maui::core::shape_self_inset(path_bounds, thickness));
        const auto half_thickness = static_cast<float>(thickness / 2.0);
        geometry.transform(maui::graphics::matrix3x2::create_translation(half_thickness, half_thickness));
        // winui_shape_ops::build_path_geometry, no winding argument: Border has no winding-mode surface
        // at all (i_border_stroke exposes none), so there is nothing to set PathGeometry.FillRule from —
        // WinUI's own default (EvenOdd) is left standing, matching GraphicsExtensions.AsPathGeometry
        // (src/Core/src/Platform/Windows/GraphicsExtensions.cs), which never touches FillRule either.
        // (shape_view_handler.cpp's call DOES pass a winding, because that control's own oracle carries
        // one — see winui_shape_ops.hpp's header comment for the shared walk and this FillRule split.)
        path.Data(maui::platform::windows::build_path_geometry(geometry));

        // UpdateStroke/UpdateStrokeThickness: like every backend's border_stroke_spec mirror, only the
        // RESOLVED SOLID color survives (border_handler.hpp: "Gradient strokes are out of scope") — a
        // SolidColorBrush built directly from spec.stroke_color (the same to_ui_color-then-SolidColorBrush
        // shape label_handler.cpp's map_text_color already uses), NOT brush_for (which needs the ORIGINAL
        // paint* and is reserved for update_background below — the one push that still carries it).
        path.StrokeThickness(thickness);
        path.Stroke(spec.has_stroke
                        ? winui::Media::SolidColorBrush{maui::platform::windows::to_ui_color(spec.stroke_color)}
                        : nullptr);

        // UpdateStrokeLineCap/LineJoin/MiterLimit: IStroke's line-cap/join are ONE value applied
        // uniformly, so all three WinUI cap slots get the same mapped value (shape_view_handler.cpp's
        // identical stroke push).
        const winui::Media::PenLineCap cap = to_pen_line_cap(spec.line_cap);
        path.StrokeStartLineCap(cap);
        path.StrokeEndLineCap(cap);
        path.StrokeDashCap(cap);
        path.StrokeLineJoin(to_pen_line_join(spec.line_join));
        path.StrokeMiterLimit(spec.miter_limit);

        // UpdateStrokeDashPattern/UpdateBorderDashOffset (BorderExtensions.cs): RAW lengths, copied
        // straight into StrokeDashArray/StrokeDashOffset with NO division by StrokeThickness — unlike
        // shape_view_handler.cpp's Windows stroke push (whose own oracle DOES divide by thickness, per
        // WinUI's Shape.StrokeDashArray semantics generally). Read directly off BorderExtensions.cs:
        // UpdateStrokeDashPattern's `foreach (double value in array) borderPath.StrokeDashArray.Add(value)`
        // and UpdateBorderDashOffset's `borderPath.StrokeDashOffset = borderDashOffset` — neither divides
        // anywhere. Faithfully reproduced as the oracle actually reads, not "fixed" to match the
        // shape_view technique. An empty pattern (the C# null analog, per i_stroke.hpp) still yields an
        // empty DoubleCollection — a solid line.
        winui::Media::DoubleCollection dashes;
        for (const float d : spec.dash_pattern)
        {
            dashes.Append(static_cast<double>(d));
        }
        path.StrokeDashArray(dashes);
        path.StrokeDashOffset(spec.dash_offset);

        // ContentPanel.UpdateBorder's tail: the same shape + thickness that just built the stroke also
        // define the content clip, so every stroke push re-issues it (C#'s UpdateBorder → UpdateClip).
        apply_content_clip(platform->native, spec);
    }

    // The backend half of platform_arrange (border_handler.cpp's cross-platform half calls this, then
    // conditionally update_border() when the size changed) — frame the host, nothing else. Matches
    // content_page_handler.cpp / label_handler.cpp's identical framing exactly.
    void border_handler::arrange_native(const maui::graphics::rect& frame)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        // PlatformArrangeHandler's guard, WIDENED to non-finite — see label_handler.cpp's identical
        // guard for why (a NaN reaching XAML's Canvas layout is an unrecoverable stowed exception).
        if (!std::isfinite(frame.x) || !std::isfinite(frame.y) || !std::isfinite(frame.width) ||
            !std::isfinite(frame.height) || frame.width < 0 || frame.height < 0)
        {
            return;
        }
        const canvas host = as_host(platform->native);
        canvas::SetLeft(host, frame.x);
        canvas::SetTop(host, frame.y);
        host.Width(frame.width);
        host.Height(frame.height);
        // The content host spans the whole border box at the host's origin — that is what makes its
        // visual the border's own coordinate space for the content clip (file header, deviation 1). Its
        // children keep being positioned by their own handlers in exactly the host-relative coordinates
        // they used when the content was a direct child (border::arrange arranges content host-relative).
        const canvas content_host = as_content_host(platform->native);
        canvas::SetLeft(content_host, 0);
        canvas::SetTop(content_host, 0);
        content_host.Width(frame.width);
        content_host.Height(frame.height);
        // ContentPanel.ArrangeOverride's own UpdateClip call: the clip is bounds-dependent, and this is
        // the push that installs it once the border has a real size (platform_arrange only re-runs
        // update_border when the SIZE changed, so this cannot be left to that path alone).
        apply_content_clip(platform->native, platform->border);
        // The generic IView.Clip, a DIFFERENT clip on a different element by a different mechanism —
        // view_chrome_ops.cpp's RectangleGeometry on the host UIElement, masking the whole Canvas (stroke
        // path AND content), where the content clip above is a Composition geometric clip on the content
        // host alone. It is bounds-dependent too (apply_native_clip reads the just-set Width/Height back)
        // and map_clip's own push (view_mapper.cpp) always runs before the first arrange, so this
        // re-invoke is what actually installs it once the border has a real size. `native` boxes a plain
        // Canvas (this file's header — no custom-Panel seam), NOT a Border, so apply_native_clip's
        // host-vs-child redirect does not fire here.
        if (const auto* view = virtual_view(); view != nullptr)
        {
            apply_native_clip(platform->native, view->clip());
        }
    }

    // ---- generic-IView property pushes (view_platform_base overrides) ---------------------------
    void border_platform::update_visibility(maui::core::visibility value)
    {
        maui::platform::windows::apply_visibility(native, value);
    }

    void border_platform::update_opacity(double value)
    {
        maui::platform::windows::apply_opacity(native, value);
    }

    void border_platform::update_is_enabled(bool value)
    {
        maui::platform::windows::apply_is_enabled(native, value);
    }

    void border_platform::update_automation_id(std::string_view value)
    {
        maui::platform::windows::apply_automation_id(native, value);
    }

    // NOT delegated to the shared apply_background (see the file header): Border.Shape is never null in
    // this port, so C#'s ViewExtensions.UpdateBorderBackground always takes the "hasBorder" branch —
    // ContentPanel.UpdateBackground(Paint?) routes straight to the STROKE PATH's Fill, so the fill
    // follows the border's shape (rounded corners etc.) instead of the host Canvas's rectangular bounds.
    void border_platform::update_background(const maui::graphics::paint* value)
    {
        if (native == nullptr)
        {
            return;
        }
        const shape_path path = as_path(native);
        path.Fill(value != nullptr ? maui::platform::windows::brush_for(*value) : nullptr);
    }
} // namespace maui::core
