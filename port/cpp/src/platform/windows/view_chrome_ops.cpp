// view_chrome_ops — WinUI 3 (Composition) platform recipe: the REAL native push behind the shared
// view_mapper's "clip" map (view_chrome_ops.hpp). ToolTip/ContextFlyout stay documented no-ops here (not
// wired up on this backend slice yet) — this file REPLACES src/platform/headless/view_chrome_ops.cpp in
// the windows build (CMakeLists.txt's MAUI_WINDOWS_SWAPS foreach), so both no-op bodies live here
// instead of silently falling back to the headless TU's.
//
// apply_native_clip ports the EFFECT of WrapperView.UpdateClip (src/Core/src/Platform/Windows/
// WrapperView.cs:104-126, reached via ViewExtensions.cs(Windows):72-78's UpdateClip extension and
// ViewHandler.cs:461-473's MapClip): masking the element to an arbitrary shape via a
// Microsoft.UI.Composition geometric clip installed on the element's OWN visual
// (ElementCompositionPreview.GetElementVisual) — ported without a WrapperView container, since
// ViewExtensions.cs:111-114's NeedsContainer (Clip-or-Shadow) is not modeled as a dynamic per-view
// opt-in on this backend (view_handler.hpp's needs_container() is a static per-handler-type flag); the
// clip installs directly on the control's own native element instead of a separate wrapper.
//
// DOCUMENTED DEVIATION from the oracle's MECHANISM (not its intended result): WrapperView.cs:117-120
// converts ANY IShape into a CompositionPath via `clipGeometry.PathForBounds(...).AsPath(CanvasDevice)`.
// Win2D's CanvasDevice/CanvasGeometry are the only IGeometrySource2D producer C# has for that call, and
// Win2D is not linked on this backend (border_handler.cpp's identical note for its own deferred content
// clip; also image_source_services.cpp:314, button_handler.cpp:734, image_button_handler.cpp:677) — so
// there is no way to feed an arbitrary flattened path into Compositor.CreatePathGeometry(CompositionPath)
// generically. Instead build_geometry() below dispatches on the shape's CONCRETE type and builds the
// matching native Composition geometry primitive directly (Compositor.CreateEllipseGeometry /
// CreateRectangleGeometry, which CompositionGeometricClip.Geometry accepts directly — it takes the
// CompositionGeometry base class, not specifically a CompositionPathGeometry). This is exact (not an
// approximation) for the shapes it covers — ellipse/rectangle, absolute or bounds-relative — including
// this page's shared EllipseGeometry; the same spirit as android_clip_ops.hpp's own convex-only honest
// degradation. RoundRectangleGeometry/round_rectangle ARE covered, but only when all four corner radii
// are EQUAL: CreateRoundedRectangleGeometry takes a single uniform Vector2 corner size, while MAUI's
// CornerRadius carries four INDEPENDENT radii, so a genuinely non-uniform radius set (e.g.
// clip_corner_radius_page's 4-slider demo once dragged apart) cannot be expressed exactly —
// approximating would silently render a wrong shape, so that case falls through to the same
// documented-gap return as an unsupported shape instead (honest degradation, matching the convex-only
// precedent above). PathGeometry/GeometryGroup are not covered either (same Win2D gap). Any of these
// leaves an existing clip untouched — TODO: verify against WrapperView.cs:117-120 once Win2D is linked
// here.
//
// NOTE on that "leaves an existing clip untouched" fallback for round_rectangle_geometry specifically:
// because it is a GeometryGroup, callers like clip_corner_radius_page mutate the SAME instance
// (set_corner_radius) and re-push it (set_clip) to make the change observable. So the moment the four
// sliders diverge, apply_native_clip below hits this unsupported-shape early-return and the VISUAL'S
// CLIP IS NOT UPDATED — the image keeps showing the last uniform-radius clip it had, not "no clip" and
// not the new (non-uniform) shape. That is a stale-clip symptom, distinct from PathGeometry/GeometryGroup
// (which were never clippable in the first place and so read as "unclipped" from frame one).
//
// LAYERING NOTE: build_geometry() is, to my knowledge, the first file under src/core or src/platform to
// #include maui/controls/shapes/*_geometry.hpp — every other clip path (android_clip_ops.hpp, apple/ios
// apply_clip) stays entirely inside the maui::graphics::i_shape contract via path_for_bounds, never
// naming a concrete controls-layer geometry type. That genericity is exactly what Win2D would have
// bought here too; without it, naming the concrete type is the only way to recover a typed
// center/radius or rect for Composition's factory methods. The include is header-only (ellipse_geometry
// / rectangle_geometry are fully inline, deriving maui::core::bindable_object which is already part of
// maui_core) — no new link dependency, one-directional (controls_shapes never references this file).
//
// TWO CALL SITES (both needed — see view_chrome_ops.hpp): view_mapper.cpp's map_clip pushes whenever the
// Clip PROPERTY changes; each Windows handler's platform_arrange/arrange_native re-pushes whenever the
// view's SIZE changes, passing the SAME virtual-view clip again. As of the 2026-07-30 sweep this is EVERY
// MAUI_WINDOWS_SWAPS handler whose arrange stamps Width/Height on a real element: button, entry, layout,
// search_bar (the original four), plus image, label, shape_view, border, content_page, picker, slider,
// scroll_view, swipe_view, image_button. NOT covered: window_handler (IWindow has no Clip — Clip is an
// i_view member, and i_window derives i_element, not i_view, so there is nothing to push) and
// collection_view_handler (owned by a concurrent edit at the time of this sweep — same "stamps Width/
// Height, virtual_view derives i_view" shape as the rest of this list, so it needs the identical call once
// that edit lands). A bounds-RELATIVE shape (maui::graphics::shapes::ellipse/rectangle, which fills
// whatever rect it is given) genuinely needs the resize-time rebuild; even a bounds-INDEPENDENT one
// (controls::shapes::ellipse_geometry, the clip_gallery page's shape) still needs it for the "has this
// view been arranged yet" guard below, since map_clip's own push always runs before the first layout pass.
//
// NO SizeChanged HOOK (a deliberate divergence from WrapperView.cs:131-135/188-195's own re-apply
// mechanism): both call sites read the element's OWN Width/Height properties (FrameworkElement's plain
// get/set DPs, NOT ActualWidth/ActualHeight). This port's cross-platform layout_manager computes the
// final frame itself and stamps Width/Height onto the native element IMPERATIVELY from platform_arrange
// (see layout_handler.cpp's identical ClipsToBounds precedent, which rebuilds its clip rectangle the same
// way at arrange time using the frame argument) — so Width/Height already equal the final arranged size
// synchronously, with no XAML measure/arrange pass to wait for. C#'s WrapperView has no such imperative
// stamp of its own (it is measured/arranged BY XAML, like any other element in a real layout panel), which
// is exactly why it needs the SizeChanged hook to learn its own ActualWidth/ActualHeight later; this
// backend's Canvas-push architecture makes that hook unnecessary.

#include "maui/core/view_chrome_ops.hpp"

#include <winrt/Microsoft.UI.Composition.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Microsoft.UI.Xaml.Hosting.h>
#include <winrt/Microsoft.UI.Xaml.h>
// Not actually needed for an IVector/IMap member here -- CompositionRectangleGeometry::Size(...) below
// textually matches check_winrt_includes.py's IVector::Size heuristic (a documented false-positive
// shape the lint owns up to). Included anyway: it is a real, harmless header and keeps the lint at 0.
#include <winrt/Windows.Foundation.Collections.h>
// float2/float3 (UIElement::ActualOffset() below returns a float3; CompositionGeometricClip::Offset()
// takes a float2) -- named explicitly now (apply_native_clip's offset-compensation fix), so the owning
// header is included by name rather than relied on transitively via Composition/Xaml.
#include <winrt/Windows.Foundation.Numerics.h>

#include <cmath>
#include <optional>
#include <string>

#include "maui/controls/shapes/ellipse_geometry.hpp"
#include "maui/controls/shapes/rectangle_geometry.hpp"
#include "maui/controls/shapes/round_rectangle_geometry.hpp"
#include "maui/graphics/i_shape.hpp"
#include "maui/graphics/shapes/ellipse.hpp"
#include "maui/graphics/shapes/rectangle.hpp"
#include "maui/graphics/shapes/round_rectangle.hpp"
#include "winui_interop.hpp"

namespace
{
    // Named `winui`, NOT `xaml` — see winui_visual_ops.cpp's identical note (the port's own maui::xaml
    // XAML-loader namespace would shadow a file-scope `xaml` alias inside namespace maui::*).
    namespace winui = winrt::Microsoft::UI::Xaml;
    // Microsoft.UI.Composition — no existing alias precedent in this backend (this is the first unit to
    // touch the Composition API surface; border_handler.cpp's header note flagged it as needed-but-not-
    // yet-linked for its own deferred content clip). Named for what it is, matching the `winui` style.
    namespace comp = winrt::Microsoft::UI::Composition;

    winui::UIElement element_of(void* native)
    {
        if (native == nullptr)
        {
            return nullptr;
        }
        return maui::platform::windows::ref<winui::UIElement>(native);
    }

    // Build the native Composition geometry for the shape kinds this backend can express WITHOUT Win2D
    // (see the file header's DOCUMENTED DEVIATION). `width`/`height` are the element's current size —
    // needed only by the BOUNDS-RELATIVE shapes (maui::graphics::shapes::ellipse/rectangle, which fill
    // whatever rect they are given, mirroring their own path_for_bounds); the controls::shapes::
    // *_geometry family ignores them entirely (absolute coordinates — geometry.hpp's path_for_bounds
    // contract), so those two branches read the geometry object's own fields directly instead of
    // round-tripping through path_for_bounds/path_f for a value Composition needs typed anyway (a
    // center+radius or an offset+size, not a flattened point list). A null return means "not one of the
    // shapes this backend supports yet" (see the file header) — the caller leaves any existing clip alone.
    comp::CompositionGeometry build_geometry(const comp::Compositor& compositor, const maui::graphics::i_shape& shape,
                                             float width, float height)
    {
        using maui::controls::shapes::ellipse_geometry;
        using maui::controls::shapes::rectangle_geometry;
        using maui::controls::shapes::round_rectangle_geometry;
        namespace graphics_shapes = maui::graphics::shapes;

        // Shared by both round-rect branches below: CreateRoundedRectangleGeometry only takes ONE
        // uniform Vector2 corner size, so a non-uniform maui::graphics::corner_radius (four independent
        // fields) cannot be expressed exactly — nullopt here means "fall through to unsupported" (see
        // the file header's honest-degradation note), never an approximation. DELIBERATELY compares the
        // four fields directly rather than via corner_radius::equals()/operator==: that equals() special-
        // cases TWO default-constructed (non-parameterized) instances as equal without a field compare,
        // which is the wrong question here (an own-fields-equal check on ONE instance, not an instance
        // equality check against another) and would be easy to reach for by mistake.
        auto uniform_radius = [](const maui::graphics::corner_radius& corner) -> std::optional<float> {
            if (corner.top_left == corner.top_right && corner.top_left == corner.bottom_left &&
                corner.top_left == corner.bottom_right)
            {
                return static_cast<float>(corner.top_left);
            }
            return std::nullopt;
        };

        if (const auto* ellipse = dynamic_cast<const ellipse_geometry*>(&shape))
        {
            comp::CompositionEllipseGeometry geometry = compositor.CreateEllipseGeometry();
            geometry.Center({static_cast<float>(ellipse->center().x), static_cast<float>(ellipse->center().y)});
            geometry.Radius({static_cast<float>(ellipse->radius_x()), static_cast<float>(ellipse->radius_y())});
            return geometry;
        }
        if (dynamic_cast<const graphics_shapes::ellipse*>(&shape) != nullptr)
        {
            comp::CompositionEllipseGeometry geometry = compositor.CreateEllipseGeometry();
            geometry.Center({width / 2.0F, height / 2.0F});
            geometry.Radius({width / 2.0F, height / 2.0F});
            return geometry;
        }
        if (const auto* rect = dynamic_cast<const rectangle_geometry*>(&shape))
        {
            comp::CompositionRectangleGeometry geometry = compositor.CreateRectangleGeometry();
            geometry.Offset({static_cast<float>(rect->rect().x), static_cast<float>(rect->rect().y)});
            geometry.Size({static_cast<float>(rect->rect().width), static_cast<float>(rect->rect().height)});
            return geometry;
        }
        if (dynamic_cast<const graphics_shapes::rectangle*>(&shape) != nullptr)
        {
            comp::CompositionRectangleGeometry geometry = compositor.CreateRectangleGeometry();
            geometry.Offset({0.0F, 0.0F});
            geometry.Size({width, height});
            return geometry;
        }
        if (const auto* round_rect_geom = dynamic_cast<const round_rectangle_geometry*>(&shape))
        {
            if (const std::optional<float> radius = uniform_radius(round_rect_geom->corner_radius()))
            {
                comp::CompositionRoundedRectangleGeometry geometry = compositor.CreateRoundedRectangleGeometry();
                geometry.Offset(
                    {static_cast<float>(round_rect_geom->rect().x), static_cast<float>(round_rect_geom->rect().y)});
                geometry.Size({static_cast<float>(round_rect_geom->rect().width),
                               static_cast<float>(round_rect_geom->rect().height)});
                geometry.CornerRadius({*radius, *radius});
                return geometry;
            }
            // Non-uniform radii: honest degradation (see the file header) — fall through to unsupported.
        }
        if (const auto* round_rect = dynamic_cast<const graphics_shapes::round_rectangle*>(&shape))
        {
            if (const std::optional<float> radius = uniform_radius(round_rect->corner_radius()))
            {
                comp::CompositionRoundedRectangleGeometry geometry = compositor.CreateRoundedRectangleGeometry();
                geometry.Offset({0.0F, 0.0F});
                geometry.Size({width, height});
                geometry.CornerRadius({*radius, *radius});
                return geometry;
            }
            // Non-uniform radii: honest degradation (see the file header) — fall through to unsupported.
        }
        // PathGeometry and GeometryGroup (beyond round_rectangle[_geometry] above) need the Win2D route
        // the file header documents as missing. TODO: verify against WrapperView.cs:117-120 once Win2D
        // (CanvasDevice/CanvasGeometry) is linked on this backend.
        return nullptr;
    }
} // namespace

namespace maui::core
{
    void apply_native_tool_tip(void* /*native_view*/, const std::optional<std::string>& /*text*/)
    {
        // Documented no-op on this backend slice (see the file header — this TU replaces the headless
        // one in the windows build): the Windows ToolTipExtensions push is not wired up yet.
    }

    void apply_native_context_flyout(void* /*native_view*/, const i_flyout* /*flyout*/)
    {
        // Documented no-op on this backend slice: the Windows MenuFlyout push is not wired up yet.
    }

    void apply_native_clip(void* native_view, const maui::graphics::i_shape* shape)
    {
        const winui::UIElement element = element_of(native_view);
        if (element == nullptr)
        {
            return;
        }
        const auto framework_element = element.try_as<winui::FrameworkElement>();
        if (framework_element == nullptr)
        {
            return;
        }
        // HOST-VS-CHILD (image_handler.cpp's CONTAINER note is the canonical writeup): several handlers
        // on this backend (image/label/shape_view, and any future Border-host handler) box a chromeless
        // Border WRAPPING the real control instead of the bare control itself -- the Border plays MAUI's
        // WrapperView/ContainerView role and is what Background paints onto (ImageHandler.Windows.cs:133
        // ToPlatform()=ContainerView) and what THIS backend's platform_arrange stamps Width/Height on. But
        // WrapperView.cs:112/126 installs its geometric clip on `GetElementVisual(Child)` -- the CHILD's
        // visual, not its own -- so the mask must land on the Border's child, not the Border. Width/Height
        // are still read from `framework_element` below (the passed-in host), since that is the element
        // this backend's arrange path actually sizes; only the VISUAL the clip attaches to changes here.
        // A non-Border element (button/entry/layout/search_bar/... which pass their own bare control, no
        // host) falls through unchanged -- try_as<Border> fails and `clip_target` stays `element`.
        winui::UIElement clip_target = element;
        bool clip_target_is_border_child = false;
        if (const auto host = element.try_as<winui::Controls::Border>(); host != nullptr && host.Child() != nullptr)
        {
            clip_target = host.Child();
            clip_target_is_border_child = true;
        }
        const comp::Visual visual = winui::Hosting::ElementCompositionPreview::GetElementVisual(clip_target);
        if (visual == nullptr)
        {
            return;
        }
        if (shape == nullptr)
        {
            visual.Clip(nullptr); // WrapperView.cs's DisposeClip path — clear, no size guard needed.
            return;
        }
        // WrapperView.cs:104-110's guard: not yet arranged -> no clip. map_clip's own push always hits
        // this branch (it runs before the first layout pass); the handler's platform_arrange re-invokes
        // once the element has its real size (see the file header for why that is enough here, with no
        // SizeChanged hook).
        const auto width = static_cast<float>(framework_element.Width());
        const auto height = static_cast<float>(framework_element.Height());
        if (!(width > 0.0F) && !(height > 0.0F))
        {
            visual.Clip(nullptr);
            return;
        }
        // PORT-SPECIFIC guard the oracle does not need. WrapperView reads ActualWidth/ActualHeight, which
        // XAML computes and which are never NaN, so `height <= 0 && width <= 0` is a sufficient test there.
        // This backend reads the Width/Height DPs instead (the port stamps them itself in platform_arrange
        // — see this file's header), and an UNSET DP reads back NaN. The guard above only rejects the case
        // where BOTH are unset, faithfully mirroring the oracle's `&&`; a view that pins ONE axis and
        // leaves the other unset would slip through with a NaN extent and hand NaN to CreateEllipseGeometry
        // /CreateRectangleGeometry. Require both finite before building any geometry from them.
        if (!std::isfinite(width) || !std::isfinite(height))
        {
            visual.Clip(nullptr);
            return;
        }
        const comp::Compositor compositor = visual.Compositor();
        const comp::CompositionGeometry geometry = build_geometry(compositor, *shape, width, height);
        if (geometry == nullptr)
        {
            return; // an unsupported shape kind (see build_geometry) — leave any existing clip alone.
        }
        // WrapperView.cs:125: `geometricClip.Offset = -Child.ActualOffset`. CORRECTED (was previously
        // skipped here on the assumption every Border-host child stretches to fill its host, citing
        // image_handler's "default Stretch alignment" -- that premise is false for AspectFill: image_
        // handler.cpp's map_aspect (ImageViewExtensions.UpdateAspect ported 1:1) Center-aligns the child
        // Image on BOTH axes for Aspect::aspect_fill, and an AspectFill-scaled child routinely overflows
        // its host on one axis (e.g. dotnet_bot.png is 1200x694 -- scaled to fill a 200x200 host it comes
        // out ~346x200, landing the child ~73 DIP negative on X once centered). The clip geometry above is
        // built in the DEVELOPER's coordinate space (element-local, origin at THIS element's own top-left
        // -- e.g. RectangleGeometry's Rect="0,15,150,150" assumes (0,0) is the element's corner), but for
        // a Border-host element the visual actually being clipped is the CHILD's, whose own local origin
        // sits whatever ActualOffset away from the host once alignment shifts it off (0,0). Left
        // uncompensated, that shift silently crops content the developer's clip never asked to hide --
        // this was the entire clip/clip_gallery page diff (RectangleGeometry/EllipseGeometry/
        // RoundRectangleGeometry clips on AspectFill images losing their right-hand content). For
        // clip_target == element (no Border host: button/entry/layout/search_bar/...), the element IS the
        // visual being positioned, so its own local origin already matches the developer's coordinate
        // space and no correction applies -- clip_target_is_border_child stays false there.
        comp::CompositionGeometricClip geometric_clip = compositor.CreateGeometricClip(geometry);
        if (clip_target_is_border_child)
        {
            const winrt::Windows::Foundation::Numerics::float3 child_offset = clip_target.ActualOffset();
            geometric_clip.Offset({-child_offset.x, -child_offset.y});
        }
        visual.Clip(geometric_clip);
    }
} // namespace maui::core
