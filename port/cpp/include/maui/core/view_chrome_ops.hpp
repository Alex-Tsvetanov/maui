#pragma once
// maui::core::view_chrome_ops  <=  the per-platform native-view chrome extensions behind
// Microsoft.Maui.Handlers.ViewHandler.MapToolTip / MapContextFlyout (ToolTipExtensions.UpdateToolTip /
// the per-platform context-flyout attach).
//
// Two free functions the SHARED view_mapper calls with the handler's native view — exactly C#'s shape,
// where MapToolTip calls `handler.ToPlatform().UpdateToolTip(...)`, an extension on the NATIVE view.
// Because the native view comes from i_view_handler::native_view(), every control gets the behavior
// uniformly with NO per-control platform-struct override. One definition per backend:
//   - headless (src/platform/headless/view_chrome_ops.cpp): no-ops — the view_platform_base mirrors
//     are the observable state;
//   - apple (src/platform/apple/view_chrome_ops.mm): NSView.toolTip + NSView.menu (the AppKit
//     right-click menu, built from the flyout's i_menu_element tree — REAL and inspectable);
//   - ios (src/platform/ios/view_chrome_ops.mm): tooltip = documented no-op (C# materializes tooltips
//     on desktop/Catalyst only); context flyout = a UIContextMenuInteraction attached to the view
//     (menu materialization needs user interaction — the delegate builds the UIMenu on demand).

#include <optional>
#include <string>

namespace maui::core
{
    class i_flyout;

    // Push (or clear, when nullopt/empty) the tooltip text onto the native view. `native_view` may be
    // null (headless platform views) — then this is a no-op.
    void apply_native_tool_tip(void* native_view, const std::optional<std::string>& text);

    // Materialize (or clear, when null) the context flyout on the native view.
    void apply_native_context_flyout(void* native_view, const i_flyout* flyout);
} // namespace maui::core
