#pragma once
// Shared AppKit operations for the gesture platform manager — the platform side of
// maui::controls::gesture_platform_manager's native attach (the GesturePlatformManager.iOS.cs role,
// adapted to AppKit: MAUI's own macOS support is Mac Catalyst/UIKit, so there is no AppKit
// GesturePlatformManager in the read-only C# source to port verbatim; the cross-platform contract is
// faithful and these are the standard AppKit equivalents of the UIKit recipe). Objective-C++ only —
// include exclusively from .mm files (it references NSGestureRecognizer / NSView types).
//
// AppKit translation notes (each also documented at the use site in gesture_platform_manager.mm):
//   - tap   → NSClickGestureRecognizer (numberOfClicksRequired + buttonMask — the direct analog of
//             UITapGestureRecognizer.numberOfTapsRequired + buttonMaskRequired);
//   - pan   → NSPanGestureRecognizer (translationInView, like UIKit);
//   - pinch → NSMagnificationGestureRecognizer — AppKit reports a CUMULATIVE `magnification` delta
//             from 1 (so scale = 1 + magnification), fed through the shared pinch_scale_delta math;
//   - swipe → AppKit has NO swipe recognizer (NSSwipeGestureRecognizer does not exist), so the swipe
//             is SYNTHESIZED from a dedicated NSPanGestureRecognizer's deltas via the recognizer's own
//             cross-platform accumulate-then-detect seam (ISwipeGestureController.SendSwipe +
//             DetectSwipe) — the same model the C# Windows/Android bridges use;
//   - pointer → enter/exit/move via an NSTrackingArea (AppKit's hover facility; UIKit's
//             UIHoverGestureRecognizer equivalent) and pressed/released via an
//             NSPressGestureRecognizer with minimumPressDuration = 0 (Began on mouse-down → pressed,
//             Ended/Cancelled/Failed → released).

#import <AppKit/AppKit.h>

#include "maui/controls/gestures/buttons_mask.hpp"

namespace maui::platform::apple
{
    // maui buttons_mask → NSGestureRecognizer.buttonMask (bit 0 = primary, bit 1 = secondary — the
    // same bit layout AppKit documents for click/press/pan recognizers).
    inline NSUInteger to_ns_button_mask(maui::controls::buttons_mask mask)
    {
        NSUInteger result = 0;
        if (maui::controls::contains(mask, maui::controls::buttons_mask::primary))
        {
            result |= 0x1U;
        }
        if (maui::controls::contains(mask, maui::controls::buttons_mask::secondary))
        {
            result |= 0x2U;
        }
        return result;
    }

    // The button a recognized press/click reports: AppKit's recognizer action carries no button
    // identity — the bridge uses the SHARED maui::controls::effective_button policy (the C# Mac
    // Catalyst fallback; see gesture_platform_manager.hpp).

    // A pan translation in MAUI coordinates: MAUI's TotalY grows DOWNWARD (the UIKit convention the
    // C# bridge reads from translationInView); a non-flipped AppKit view's y grows upward, so negate.
    inline double pan_total_y(NSView* view, double translation_y)
    {
        return view.isFlipped ? translation_y : -translation_y;
    }
} // namespace maui::platform::apple
