// view_focus_ops — headless platform recipe: there is no native first responder, so a focus request
// always "succeeds" (returns true) and unfocus is a no-op. The observable result is the virtual view's
// is_focused state, which the shared view_command_mapper sets from this return value. The Apple twin
// (window makeFirstResponder:) is src/platform/apple/view_focus_ops.mm; the iOS twin
// (UIView becomeFirstResponder) is src/platform/ios/view_focus_ops.mm.

#include "maui/core/view_focus_ops.hpp"

namespace maui::core
{
    bool focus_native_view(void* /*native_view*/)
    {
        return true;
    }

    void unfocus_native_view(void* /*native_view*/)
    {
    }
} // namespace maui::core
