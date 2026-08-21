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
// MECHANISM NOTE. WrapperView.cs:117-120 converts ANY IShape into a CompositionPath via
// `clipGeometry.PathForBounds(...).AsPath(CanvasDevice)` — one route, no shape-kind switch. Win2D's
// CanvasGeometry is C#'s only IGeometrySource2D producer for that call; this backend answers the same
// interface over a Direct2D path geometry instead (winui_shape_ops.cpp's build_composition_clip_geometry,
// moved there out of border_handler.cpp, which had needed exactly this for its own content clip).
// build_geometry() below therefore has the oracle's general route as its LAST branch, and every earlier
// branch is a shortcut, not a gate: for an ellipse/rectangle/uniform-radius round rect, Composition can
// build the primitive natively (CreateEllipseGeometry / CreateRectangleGeometry /
// CreateRoundedRectangleGeometry, all accepted by CompositionGeometricClip.Geometry, which takes the
// CompositionGeometry base) with no flattening at all, so those keep their exact form. A non-uniform
// CornerRadius (CreateRoundedRectangleGeometry takes ONE uniform Vector2), a PathGeometry and a
// GeometryGroup all fall through to the path route and are now clipped correctly.
//
// THIS WAS THE D1 DEFECT (docs/comparison/PHASE_TRIAGE.md). The last branch used to `return nullptr` and
// apply_native_clip read that as "unsupported — leave any existing clip alone", i.e. NO CLIP: windows/clip
// and windows/clip_gallery rendered their GeometryGroup- and PathGeometry-clipped images as full,
// uncropped bitmaps below the fold. The board could not see it because the at-rest still photographs only
// the top of those pages, where the EllipseGeometry/RectangleGeometry rows do clip.
//
// A null return now means only that Direct2D refused to build a geometry (no factory, or an Open/Close
// failure) — the caller still leaves any existing clip alone, which is the conservative reading.
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
#include "maui/graphics/path_f.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/shapes/ellipse.hpp"
#include "maui/graphics/shapes/rectangle.hpp"
#include "maui/graphics/shapes/round_rectangle.hpp"
#include "winui_interop.hpp"
#include "winui_shape_ops.hpp"

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

    // Build the native Composition geometry for `shape` (see the file header). The typed branches below
    // are exact-form shortcuts; the final branch is the oracle's own universal path route, so every shape
    // kind is covered. `width`/`height` are the element's current size —
    // needed only by the BOUNDS-RELATIVE shapes (maui::graphics::shapes::ellipse/rectangle, which fill
    // whatever rect they are given, mirroring their own path_for_bounds); the controls::shapes::
    // *_geometry family ignores them entirely (absolute coordinates — geometry.hpp's path_for_bounds
    // contract), so those two branches read the geometry object's own fields directly instead of
    // round-tripping through path_for_bounds/path_f for a value Composition needs typed anyway (a
    // center+radius or an offset+size, not a flattened point list). A null return now means only that
    // Direct2D refused to build the geometry — the caller leaves any existing clip alone.
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
            // Non-uniform radii: CreateRoundedRectangleGeometry cannot express them — fall through to the
            // path route below, which can (it was 'unsupported' before that branch existed).
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
            // Non-uniform radii: CreateRoundedRectangleGeometry cannot express them — fall through to the
            // path route below, which can (it was 'unsupported' before that branch existed).
        }
        // EVERY REMAINING KIND — PathGeometry, GeometryGroup, and a round rect whose four radii are not
        // uniform — goes the ORACLE'S OWN route, which has no shape-kind switch at all: WrapperView.cs:
        // 117-120 turns any IShape into `PathForBounds(pathSize).AsPath(device)` -> CompositionPath ->
        // CreatePathGeometry, for the ellipse and rectangle cases above just as much as for these. The
        // exact-geometry branches above are kept because Composition can express those shapes natively
        // (no flattening at all), not because the path route cannot; this is the general case they are
        // shortcuts for.
        //
        // Until this existed the function returned nullptr here and apply_native_clip left the element
        // UNCLIPPED — the whole of PHASE_TRIAGE.md D1: on windows/clip and windows/clip_gallery the
        // GeometryGroup- and PathGeometry-clipped images below the fold rendered as full, uncropped
        // bitmaps (19669 px / 18896 px differing at the `scrolled-down` step) while MAUI showed page
        // background around the clipped artwork.
        //
        // `pathSize` is WrapperView.cs:113's `new Rect(0, 0, width, height)` verbatim — the geometry is
        // built in the element's own local space, and apply_native_clip below applies the -ActualOffset
        // translate the oracle applies at :124.
        const maui::graphics::path_f path = shape.path_for_bounds(
            maui::graphics::rect{0.0, 0.0, static_cast<double>(width), static_cast<double>(height)});
        return maui::platform::windows::build_composition_clip_geometry(compositor, path);
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
        //
        // ACTUALOFFSET TIMING: unlike Width/Height (plain DPs, read back synchronously the instant this
        // function's own caller sets them), ActualOffset is ARRANGE-computed and only reflects WinUI's own
        // automatic layout pass, which this call does not force -- so the FIRST apply_native_clip for a
        // freshly-arranged element (boot's drive_layout, before WinUI has ever arranged this Border/Child
        // pair) can read a stale/zero ActualOffset. Not forced synchronously here (a mid-tree-walk
        // UpdateLayout() would risk laying out sibling elements this same drive_layout pass has not
        // finished stamping yet). This backend does not need to get it right on that first call: host_run.
        // cpp's SizeChanged handler unconditionally replays drive_layout once the E2E harness pins the
        // window to its capture rect ("Without this the capture would show the boot layout" -- the SAME
        // reasoning applies to ActualOffset here), and that resize is a REAL WinUI layout event, so by the
        // time it fires WinUI has necessarily arranged the tree at least once. Since clip.xaml/clip_gallery.
        // xaml's images use fixed WidthRequest/HeightRequest (unaffected by window size), the host stays
        // 200x200 across both passes, so the child's Center-aligned overflow offset this second call reads
        // is already the final, settled one. TODO: verify on the VM -- if a future clip target's host size
        // legitimately changes between these two passes (unlike this page), this is the assumption to
        // revisit.
        comp::CompositionGeometricClip geometric_clip = compositor.CreateGeometricClip(geometry);
        if (clip_target_is_border_child)
        {
            const winrt::Windows::Foundation::Numerics::float3 child_offset = clip_target.ActualOffset();
            geometric_clip.Offset({-child_offset.x, -child_offset.y});
        }
        visual.Clip(geometric_clip);
    }
} // namespace maui::core
