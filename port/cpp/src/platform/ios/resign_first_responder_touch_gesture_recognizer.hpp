#pragma once
// resign_first_responder_touch_gesture_recognizer — the C++-callable seam over the UITapGestureRecognizer
// subclass ported from Microsoft.Maui.Platform.ResignFirstResponderTouchGestureRecognizer
// (src/Controls/src/Core/ContentPage/HideSoftInputOnTappedChanged/
//  ResignFirstResponderTouchGestureRecognizer.iOS.cs). Objective-C++ only — include exclusively from .mm
// files compiled with ARC.
//
// resign_first_responder_update(uiView) is the C# static ResignFirstResponderTouchGestureRecognizer
// .Update(UIView): it hooks the editing-begin/end events on the text input, immediately arms the gesture
// if the input is already first responder, and returns a cleanup closure (C#'s ActionDisposable) that
// unhooks those events and removes any armed recognizer from the window. An empty/null closure is
// returned when the input has no window (nothing was wired). The armed recognizer holds WEAK refs to its
// UIView/UIWindow under ARC and removes itself on tap (resigning the focused input) or on editing-end /
// window removal, so it never outlives its target.

#import <UIKit/UIKit.h>

#include <functional>

namespace maui::platform::ios
{
    // <= ResignFirstResponderTouchGestureRecognizer.Update(UIView). Returns the cleanup token (run on the
    // next focus change / teardown); the returned std::function is empty when nothing was wired.
    std::function<void()> resign_first_responder_update(UIView* ui_view);

    // Test seam (no-touch-synthesis, mirroring gesture_ios_tests' fire_registered_target): run the
    // tap-Ended branch (OnTapped) of every armed recognizer on `window` — resign the focused input + remove
    // the recognizer — without synthesizing a real UITouch (which UIGestureRecognizer cannot accept from
    // unit tests). Returns the number of recognizers fired.
    int resign_first_responder_fire_for_testing(UIWindow* window);
} // namespace maui::platform::ios
