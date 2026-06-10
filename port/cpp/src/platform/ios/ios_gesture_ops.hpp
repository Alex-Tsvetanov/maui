#pragma once
// Shared UIKit operations for the gesture platform manager — the platform side of
// maui::controls::gesture_platform_manager's native attach, ported from
// src/Controls/src/Core/Platform/GestureManager/GesturePlatformManager.iOS.cs. Objective-C++ only —
// include exclusively from .mm files (it references UIGestureRecognizer / UIEvent types).

#import <UIKit/UIKit.h>
#import <objc/runtime.h>

#include "maui/controls/gestures/buttons_mask.hpp"

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
