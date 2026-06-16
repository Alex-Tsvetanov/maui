#pragma once
// Shared UIKit operations for the gesture platform manager — the platform side of
// maui::controls::gesture_platform_manager's native attach, ported from
// src/Controls/src/Core/Platform/GestureManager/GesturePlatformManager.iOS.cs. Objective-C++ only —
// include exclusively from .mm files (it references UIGestureRecognizer / UIEvent types).

#import <UIKit/UIKit.h>
#import <objc/runtime.h>

#include "maui/controls/gestures/buttons_mask.hpp"
#include "maui/core/i_view.hpp"

namespace maui::platform::ios
{
    // maui buttons_mask → UIEventButtonMask (the CreateTapRecognizer mapping: Primary →
    // UIEventButtonMask.Primary, Secondary → .Secondary).
    inline UIEventButtonMask to_ui_button_mask(maui::controls::buttons_mask mask)
    {
        UIEventButtonMask result = static_cast<UIEventButtonMask>(0);
        if (maui::controls::contains(mask, maui::controls::buttons_mask::primary))
        {
            result |= UIEventButtonMaskPrimary;
        }
        if (maui::controls::contains(mask, maui::controls::buttons_mask::secondary))
        {
            result |= UIEventButtonMaskSecondary;
        }
        return result;
    }

    // ShouldReceiveTouchProxy.ShouldReceiveTouch (GesturePlatformManager.iOS.cs lines 834-873): the pure
    // decision the shared arbitration delegate makes for every native recognizer on the platform view.
    // `virtual_view` is manager._handler?.VirtualView; `platform_view` is manager.PlatformView;
    // `touch_view` is touch.View (read off the live UITouch at the call site — factored out so the
    // on-simulator lane can exercise the decision without synthesizing a UITouch).
    //   - null virtual/platform view → false (the WeakReference-dead + handler-gone guards);
    //   - InputTransparent → false; !IsEnabled → false;
    //   - touch.View == platformView → true;
    //   - touch.View is a descendant of platformView AND (touch.View OR platformView already carries
    //     gesture recognizers) → true (let a wrapped UIView's own recognizers through);
    //   - otherwise false.
    [[nodiscard]] inline bool should_receive_touch(const maui::core::i_view* virtual_view, UIView* platform_view,
                                                   UIView* touch_view)
    {
        if (virtual_view == nullptr || platform_view == nil)
        {
            return false;
        }
        if (virtual_view->input_transparent())
        {
            return false;
        }
        if (!virtual_view->is_enabled())
        {
            return false;
        }
        if (touch_view == platform_view)
        {
            return true;
        }
        if (touch_view != nil && [touch_view isDescendantOfView:platform_view] &&
            (touch_view.gestureRecognizers.count > 0 || platform_view.gestureRecognizers.count > 0))
        {
            return true;
        }
        return false;
    }

    // ShouldRecognizeTapsTogether (GesturePlatformManager.iOS.cs lines 574-607): two
    // UITapGestureRecognizers may recognize together iff both are taps on the SAME view with matching
    // taps-required and touches-required (so multiple TapGestureRecognizers added to one MAUI element
    // all fire). A non-tap `gesture`/`other` → false.
    [[nodiscard]] inline bool should_recognize_taps_together(UIGestureRecognizer* gesture, UIGestureRecognizer* other)
    {
        UITapGestureRecognizer* const tap =
            [gesture isKindOfClass:[UITapGestureRecognizer class]] ? (UITapGestureRecognizer*)gesture : nil;
        if (tap == nil)
        {
            return false;
        }
        UITapGestureRecognizer* const other_tap =
            [other isKindOfClass:[UITapGestureRecognizer class]] ? (UITapGestureRecognizer*)other : nil;
        if (other_tap == nil)
        {
            return false;
        }
        if (tap.view != other_tap.view)
        {
            return false;
        }
        if (tap.numberOfTapsRequired != other_tap.numberOfTapsRequired)
        {
            return false;
        }
        return tap.numberOfTouchesRequired == other_tap.numberOfTouchesRequired;
    }

    // The button a recognized press reports when the event carries no button identity: the bridge
    // uses the SHARED maui::controls::effective_button policy (the C# Mac Catalyst fallback; see
    // gesture_platform_manager.hpp).

    // The associated-object key under which the bridge stores each native recognizer's trampoline
    // target (UIGestureRecognizer does not retain its targets, and — unlike AppKit — exposes no
    // target/action properties to query). An inline function-local static merges across TUs (ODR), so
    // the on-simulator tests can retrieve the registered target and fire its -onGesture: selector
    // directly — the documented no-touch-synthesis test seam.
    inline const void* gesture_target_key()
    {
        static const char key = 0;
        return &key;
    }
} // namespace maui::platform::ios
