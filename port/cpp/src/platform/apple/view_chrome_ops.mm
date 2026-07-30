// view_chrome_ops — Apple (AppKit / macOS) platform recipe: the REAL native push behind the shared
// view_mapper's "tool_tip" / "context_flyout" maps (view_chrome_ops.hpp).
//   - apply_native_tool_tip → NSView.toolTip (the AppKit tooltip — ToolTipExtensions.UpdateToolTip's
//     desktop materialization);
//   - apply_native_context_flyout → NSView.menu (the AppKit right-click menu), built whole from the
//     flyout's i_menu_element tree by apple_menu_ops (rebuilt on every change; null clears it).
// Works uniformly for every control because the native view comes from the handler's native_view() —
// no per-control platform override needed. Compiled as Objective-C++ with ARC for the apple backend.
//
// apply_native_clip is a documented NO-OP here: this backend already pushes Clip per-control, via each
// handler's own `update_clip` override storing the shape and its own `platform_arrange` re-resolving it
// against the live bounds through apple_visual_ops::apply_clip (button_handler.mm:405 and friends). A
// second uniform push from the shared view_mapper would double-apply the mask, so this stays a no-op —
// the Windows twin (src/platform/windows/view_chrome_ops.cpp) is where this function actually pushes.

#include "maui/core/view_chrome_ops.hpp"

#import <AppKit/AppKit.h>
#import <objc/runtime.h>

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

    namespace
    {
        // Tags the NSMenus THIS op installs (associated object on the menu) so a null flyout only
        // clears what the op owns: menu-bearing controls (NSPopUpButton, NSDatePicker, ...) expose
        // their ITEM LIST through NSView.menu, and the shared view_mapper pushes every key on attach
        // — blindly assigning nil would destroy the control's own menu (it emptied the picker).
        const void* context_flyout_tag()
        {
            static const char key = 0;
            return &key;
        }
    } // namespace

    void apply_native_context_flyout(void* native_view, const i_flyout* flyout)
    {
        if (native_view == nullptr)
        {
            return;
        }
        NSView* const view = as_view(native_view);
        if (flyout != nullptr)
        {
            NSMenu* const menu = maui::platform::apple::build_menu_from_flyout(flyout);
            objc_setAssociatedObject(menu, context_flyout_tag(), @YES, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
            view.menu = menu; // a ContextFlyout on a menu-bearing control replaces its menu (documented)
            return;
        }
        NSMenu* const current = view.menu;
        if (current != nil && objc_getAssociatedObject(current, context_flyout_tag()) != nil)
        {
            view.menu = nil; // cleared flyout: remove only the menu the op installed
        }
    }

    void apply_native_clip(void* /*native_view*/, const maui::graphics::i_shape* /*shape*/)
    {
        // No-op — see the file header: apple already pushes Clip per-control from each handler's own
        // platform_arrange (apple_visual_ops::apply_clip), so this uniform seam must not double-apply.
    }
} // namespace maui::core
