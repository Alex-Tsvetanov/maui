// border_handler — WinUI 3 platform recipe. C#'s Windows PlatformView is a ContentPanel (a custom
// Panel hosting a stroke Path + the content child) — see src/Core/src/Platform/Windows/ContentPanel.cs +
// BorderExtensions.cs/StrokeExtensions.cs (the oracle for every push below) + BorderHandler.Windows.cs
// (UpdateContent). This port has no custom-Panel seam yet (no C++/WinRT DependencyObject subclass), so
// the host is a plain Canvas — the same simplification content_page_handler.cpp / layout_handler.cpp
// already make for "a panel that just holds children the cross-platform layout positions itself" — with
// TWO children: the stroke Path (index 0, permanent, appended once in create_platform_view and never
// removed) and the hosted content (index 1, re-set by set_content). The Path is always Children()[0],
// so as_path() below needs no separate boxed handle for it.
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
// Not ported yet (documented deviation, like the iOS/Apple partials' own scope notes in border_handler.
// hpp): ContentPanel.UpdateClip's Composition geometric clip, which masks the CONTENT to the border's
// INNER shape (round-rectangle inset by half the stroke thickness) via ElementCompositionPreview /
// CanvasDevice / CompositionPath. That needs the Composition API surface (Microsoft.UI.Composition +
// Win2D's CanvasDevice) no other Windows handler in this port touches yet, and only matters when content
// overflows into a rounded corner — a much smaller visual gap than the missing stroke + background this
// slice closes. The stroke geometry, fill, and content hosting below are the dominant fix.

#include "maui/core/border_handler.hpp"

#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Microsoft.UI.Xaml.Media.h>
#include <winrt/Microsoft.UI.Xaml.Shapes.h>
#include <winrt/Microsoft.UI.Xaml.h>
// The C++/WinRT include rule (see winui_interop.hpp): the FULL header for every namespace whose MEMBERS
// are called. Without this one, IVector<T>::Append/GetAt are only forward-declared and every call fails
// with "error C3779: a function that returns 'auto' cannot be used before it is defined."
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Foundation.h>

#include <algorithm>
#include <cmath>
#include <memory>
#include <string_view>

#include "maui/core/i_border_view.hpp"
#include "maui/core/i_view.hpp"
#include "maui/core/i_view_handler.hpp"
#include "maui/core/view_chrome_ops.hpp"
#include "maui/graphics/i_shape.hpp"
#include "maui/graphics/line_cap.hpp"
#include "maui/graphics/line_join.hpp"
#include "maui/graphics/matrix3x2.hpp"
#include "maui/graphics/paint.hpp"
#include "maui/graphics/path_f.hpp"
#include "maui/graphics/rect.hpp"
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

    // The stroke Path is always Children()[0] — appended once in create_platform_view and never
    // removed (set_content() below clears + re-adds it FIRST, ahead of the content child, on every
    // content change), so this index is stable for the platform view's whole lifetime.
    shape_path as_path(void* native)
    {
        return as_host(native).Children().GetAt(0).as<shape_path>();
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
        // The stroke path is the host's ONE permanent child, appended once here at Children()[0] — see
        // as_path()'s note on why that index never moves.
        host.Children().Append(path);
        platform->native = maui::platform::windows::take<winui::UIElement>(host);
        return platform;
    }

    // C# UpdateContent: CachedChildren.Clear() + EnsureBorderPath() + re-parent Content. Clearing
    // everything and re-adding the path FIRST (so it stays index 0, painted behind the content) then the
    // new content keeps a re-set (the mapper re-runs on every Content change) from stacking two
    // generations of content on top of each other — the content_page_handler.cpp precedent, with the
    // extra permanent path child border alone needs.
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
        const canvas host = as_host(platform->native);
        const shape_path path = as_path(platform->native); // a strong ref — survives the Clear() below
        host.Children().Clear();
        host.Children().Append(path);
        if (const winui::UIElement element = native_child(platform->hosted_content))
        {
            host.Children().Append(element);
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
            return;
        }

        // BorderExtensions.UpdatePath: PathForBounds fitted to (width - thickness, height - thickness),
        // then a RenderTransform translate of (thickness/2, thickness/2) centers the stroke ON the shape
        // edge. The port bakes the same translate into the geometry via path_f::transform instead of a
        // separate XAML RenderTransform object — the identical net position, one fewer native object.
        const double thickness = spec.thickness;
        const maui::graphics::rect path_bounds{0, 0, std::max(0.0, width - thickness),
                                               std::max(0.0, height - thickness)};
        maui::graphics::path_f geometry = spec.shape->path_for_bounds(path_bounds);
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
        // Clip is bounds-dependent (view_chrome_ops.cpp's apply_native_clip reads the just-set Width/
        // Height back); map_clip's own push (view_mapper.cpp) always runs before the first arrange, so
        // this re-invoke is what actually installs the clip once the border has a real size. `native`
        // boxes a plain Canvas (this file's header — no custom-Panel seam), NOT a Border, so
        // apply_native_clip's host-vs-child redirect does not fire here; the clip masks the whole
        // Canvas (stroke path + content), which is the generic IView.Clip — distinct from, and unrelated
        // to, this file's still-unported ContentPanel.UpdateClip content-to-inner-shape clip (see the
        // file header).
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
