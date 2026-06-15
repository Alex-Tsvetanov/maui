// view_focus_ops — Apple (AppKit / macOS) platform recipe behind the shared view_command_mapper's
// Focus / Unfocus commands (view_focus_ops.hpp).
//
// AppKit has no per-view BecomeFirstResponder/ResignFirstResponder call the way UIKit does; the window
// owns the first responder, so focus is window.makeFirstResponder:(view) and unfocus is
// window.makeFirstResponder:nil (the standard AppKit "resign" — it moves first responder to the window
// itself). makeFirstResponder: returns whether the change was accepted (a non-key/off-screen view, or a
// view not yet in a window, returns NO) — that bool is the focus result the mapper records, the faithful
// AppKit analog of UIView.BecomeFirstResponder's return.
//
// Soft-keyboard concepts (KeyboardType, the Done input-accessory bar) have NO AppKit equivalent and are
// documented no-ops in the entry/editor/search keyboard maps; focus is the real AppKit behavior here.
// Compiled as Objective-C++ with ARC for the apple backend.

#include "maui/core/view_focus_ops.hpp"

#import <AppKit/AppKit.h>

namespace maui::core
{
    bool focus_native_view(void* native_view)
    {
        if (native_view == nullptr)
        {
            return false;
        }
        NSView* const view = (__bridge NSView*)native_view;
        NSWindow* const window = view.window;
        if (window == nil)
        {
            // Not in a window: there is no responder chain to join (AppKit cannot focus a detached view).
            return false;
        }
        return [window makeFirstResponder:view] == YES;
    }

    void unfocus_native_view(void* native_view)
    {
        if (native_view == nullptr)
        {
            return;
        }
        NSView* const view = (__bridge NSView*)native_view;
        NSWindow* const window = view.window;
        if (window == nil)
        {
            return;
        }
        NSResponder* const first = window.firstResponder;
        // Resign when this view is first responder directly, OR when an editable control (NSTextField) is
        // being edited — then the window's first responder is its field editor (an NSText whose delegate is
        // the control), not the control's NSView. Clearing to nil moves first responder back to the window.
        bool is_focused = (first == view);
        if (!is_focused && [first isKindOfClass:[NSText class]])
        {
            id const editor_delegate = ((NSText*)first).delegate;
            is_focused = (editor_delegate == (id)view);
        }
        if (is_focused)
        {
            [window endEditingFor:view]; // tear down the field editor cleanly before resigning
            [window makeFirstResponder:nil];
        }
    }
} // namespace maui::core
