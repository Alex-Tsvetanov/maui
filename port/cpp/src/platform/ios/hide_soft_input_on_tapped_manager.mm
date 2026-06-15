// hide_soft_input_on_tapped_manager — iOS (UIKit) backend hook, ported from
// Microsoft.Maui.Controls.HideSoftInputOnTappedChangedManager.iOS.cs (SetupHideSoftInputOnTapped(UIView)).
// Compiled as Objective-C++ with ARC.
//
// The cross-platform tracking (UpdatePage / UpdateFocusForView / FeatureEnabled / ...) lives in the shared
// src/core/hide_soft_input_on_tapped_manager.cpp. This TU defines only the ONE backend-specific step:
// setup_native(focused_view) finds the focused input's native UITextField/UITextView (searching the
// subtree when the host itself is not one — C# FindDescendantView) and arms the resign-first-responder
// tap gesture on its window via resign_first_responder_touch_gesture_recognizer, returning the cleanup
// token (empty when there is no native view / no window — the C# null returns).

#import <UIKit/UIKit.h>

#include "maui/platform/ios/hide_soft_input_on_tapped_manager.hpp"

#include <functional>

#include "maui/core/i_view.hpp"
#include "maui/core/i_view_handler.hpp"
#include "resign_first_responder_touch_gesture_recognizer.hpp"

namespace maui::platform::ios
{
    namespace
    {
        // The native UIView behind an i_view (its handler's platform view), or nil when detached.
        UIView* native_view_of(maui::core::i_view& view)
        {
            const auto& element_handler = view.handler();
            if (!element_handler)
            {
                return nil;
            }
            auto* const view_handler = dynamic_cast<maui::core::i_view_handler*>(element_handler.get());
            if (view_handler == nullptr)
            {
                return nil;
            }
            return (__bridge UIView*)view_handler->native_view();
        }

        // ViewExtensions.FindDescendantView<UIView>(view, predicate) restricted to the text-input probe the
        // C# SetupHideSoftInputOnTapped uses: the first UITextField/UITextView in the subtree, BFS order.
        UIView* find_text_input_descendant(UIView* root)
        {
            if (root == nil)
            {
                return nil;
            }
            NSMutableArray<UIView*>* const queue = [NSMutableArray arrayWithObject:root];
            while (queue.count > 0)
            {
                UIView* const view = queue.firstObject;
                [queue removeObjectAtIndex:0];
                if ([view isKindOfClass:[UITextField class]] || [view isKindOfClass:[UITextView class]])
                {
                    return view;
                }
                [queue addObjectsFromArray:view.subviews];
            }
            return nil;
        }
    } // namespace

    // C# HideSoftInputOnTappedChangedManager.SetupHideSoftInputOnTapped(UIView): wire the tap gesture onto
    // the (text-input) view's window, searching the subtree for the input when the host itself is not one.
    // Returns the cleanup token (empty when nothing was wired — no native view / no window / no input).
    std::function<void()> hide_soft_input_on_tapped_manager::setup_native(maui::core::i_view& focused_view)
    {
        UIView* const host = native_view_of(focused_view);
        if (host == nil || ![host.window isKindOfClass:[UIWindow class]])
        {
            return {};
        }
        UIView* text = host;
        if (!([text isKindOfClass:[UITextField class]] || [text isKindOfClass:[UITextView class]]))
        {
            text = find_text_input_descendant(host);
        }
        if (text == nil)
        {
            return {};
        }
        return resign_first_responder_update(text);
    }
} // namespace maui::platform::ios
