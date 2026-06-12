// view_chrome_ops — Apple (AppKit / macOS) platform recipe: the REAL native push behind the shared
// view_mapper's "tool_tip" / "context_flyout" maps (view_chrome_ops.hpp).
//   - apply_native_tool_tip → NSView.toolTip (the AppKit tooltip — ToolTipExtensions.UpdateToolTip's
//     desktop materialization);
//   - apply_native_context_flyout → NSView.menu (the AppKit right-click menu), built whole from the
//     flyout's i_menu_element tree by apple_menu_ops (rebuilt on every change; null clears it).
// Works uniformly for every control because the native view comes from the handler's native_view() —
// no per-control platform override needed. Compiled as Objective-C++ with ARC for the apple backend.

#include "maui/core/view_chrome_ops.hpp"

#import <AppKit/AppKit.h>

#include <optional>
#include <string>

#include "apple_menu_ops.hpp"
#include "maui/core/i_flyout.hpp"

namespace
{
    // Typed view of the handler's native slot (routing the cast through a helper keeps direct casts out
    // of variable initializers, the convention the other .mm partials follow).
    NSView* as_view(void* native)
    {
        return (__bridge NSView*)native;
    }
} // namespace

namespace maui::core
{
    void apply_native_tool_tip(void* native_view, const std::optional<std::string>& text)
    {
        if (native_view == nullptr)
        {
            return;
        }
        NSView* const view = as_view(native_view);
        if (!text.has_value())
        {
            view.toolTip = nil; // never set (or cleared) → no tooltip
            return;
        }
        NSString* const value = [NSString stringWithUTF8String:text->c_str()];
        view.toolTip = value;
    }

    void apply_native_context_flyout(void* native_view, const i_flyout* flyout)
    {
        if (native_view == nullptr)
        {
            return;
        }
        as_view(native_view).menu = flyout != nullptr ? maui::platform::apple::build_menu_from_flyout(flyout) : nil;
    }
} // namespace maui::core
