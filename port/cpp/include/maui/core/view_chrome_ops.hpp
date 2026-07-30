#pragma once
// maui::core::view_chrome_ops  <=  the per-platform native-view chrome extensions behind
// Microsoft.Maui.Handlers.ViewHandler.MapToolTip / MapContextFlyout / MapClip (ToolTipExtensions.
// UpdateToolTip / the per-platform context-flyout attach / WrapperView.UpdateClip on Windows).
//
// Free functions the SHARED view_mapper calls with the handler's native view — exactly C#'s shape,
// where MapToolTip calls `handler.ToPlatform().UpdateToolTip(...)`, an extension on the NATIVE view.
// Because the native view comes from i_view_handler::native_view(), every control gets the behavior
// uniformly with NO per-control platform-struct override. One definition per backend:
//   - headless (src/platform/headless/view_chrome_ops.cpp): no-ops — the view_platform_base mirrors
//     are the observable state;
//   - apple (src/platform/apple/view_chrome_ops.mm): NSView.toolTip + NSView.menu (the AppKit
//     right-click menu, built from the flyout's i_menu_element tree — REAL and inspectable); clip is a
//     no-op here — apple already pushes Clip per-control via apple_visual_ops::apply_clip, called from
//     each handler's own platform_arrange, so a second uniform push here would double-apply;
//   - ios (src/platform/ios/view_chrome_ops.mm): tooltip = documented no-op (C# materializes tooltips
//     on desktop/Catalyst only); context flyout = a UIContextMenuInteraction attached to the view
//     (menu materialization needs user interaction — the delegate builds the UIMenu on demand); clip is
//     a no-op here for the same per-control-already-does-it reason as apple;
//   - windows (src/platform/windows/view_chrome_ops.cpp): tooltip/context-flyout stay no-ops (not wired
//     up on this backend yet); clip is the REAL push — a Microsoft.UI.Composition geometric clip
//     installed on the element's own visual (WrapperView.UpdateClip's effect, ported without a
//     WrapperView container — see that file's header for the Win2D-free DOCUMENTED DEVIATION).

#include <optional>
#include <string>

namespace maui::graphics
{
    class i_shape;
} // namespace maui::graphics

namespace maui::core
{
    class i_flyout;

    // Push (or clear, when nullopt/empty) the tooltip text onto the native view. `native_view` may be
    // null (headless platform views) — then this is a no-op.
    void apply_native_tool_tip(void* native_view, const std::optional<std::string>& text);

    // Materialize (or clear, when null) the context flyout on the native view.
    void apply_native_context_flyout(void* native_view, const i_flyout* flyout);

    // Push (or clear, when null) the IView.Clip shape onto the native view (ViewHandler.MapClip ->
    // WrapperView.UpdateClip on Windows; a documented no-op on backends that already push Clip
    // per-control — see the per-backend notes above). Called from view_mapper.cpp's map_clip (whenever
    // the Clip PROPERTY changes) AND from a Windows handler's platform_arrange (whenever the view's SIZE
    // changes — the geometry for a bounds-relative shape depends on it, and even a bounds-independent
    // one needs the "has this view been arranged yet" guard); see the windows .cpp for why no
    // SizeChanged hook is needed here.
    void apply_native_clip(void* native_view, const maui::graphics::i_shape* shape);
} // namespace maui::core
