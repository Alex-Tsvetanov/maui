// view_focus_ops — iOS (UIKit) platform recipe behind the shared view_command_mapper's Focus / Unfocus
// commands (view_focus_ops.hpp). The DIRECT port of ViewExtensions.iOS:
//   Focus(this UIView, FocusRequest)  -> request.TrySetResult(platformView.BecomeFirstResponder())
//   Unfocus(this UIView, IView)       -> platformView.ResignFirstResponder()
// becomeFirstResponder returns whether the view actually became first responder (it needs a window /
// key window and a view that canBecomeFirstResponder) — that bool is the focus result the mapper records.
// Compiled as Objective-C++ with ARC for the ios backend.

#include "maui/core/view_focus_ops.hpp"

#import <UIKit/UIKit.h>

namespace maui::core
{
    bool focus_native_view(void* native_view)
    {
        if (native_view == nullptr)
        {
            return false;
        }
        UIView* const view = (__bridge UIView*)native_view;
        return [view becomeFirstResponder] == YES;
    }

    void unfocus_native_view(void* native_view)
    {
        if (native_view == nullptr)
        {
            return;
        }
        UIView* const view = (__bridge UIView*)native_view;
        [view resignFirstResponder];
    }
} // namespace maui::core
