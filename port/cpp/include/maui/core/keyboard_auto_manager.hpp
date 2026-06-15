#pragma once
// keyboard_auto_manager  <=  Microsoft.Maui.Platform.KeyboardAutoManagerScroll
// (src/Core/src/Platform/iOS/KeyboardAutoManagerScroll.cs) — the cross-platform contract for the iOS
// keyboard scroll-avoidance engine. The engine observes the soft keyboard's WillShow/WillHide/DidHide
// notifications plus the UITextField/UITextView DidBeginEditing notifications, debounces an
// AdjustPosition pass, computes the editing cursor's rect in window coordinates against the keyboard
// frame, and animates the enclosing UIScrollView (ContentInset / ContentOffset) so the cursor stays
// visible above the keyboard — restoring the original insets/offsets when the keyboard hides.
//
// This is the iOS twin only. The AppKit (apple) backend has no soft keyboard, and the headless backend
// has no native windowing, so both compile a no-op stub of this contract (so the engine can be wired
// from the shared handler code on every backend, with the iOS partial the only one that does work).
//
// connect_scroll_handler() installs the five NSNotificationCenter observers ONCE (idempotent — a second
// call while connected is a no-op, mirroring C#'s `if (TextFieldToken is not null) return;`).
// disconnect_scroll_handler() removes them and tears down the observer tokens. Both are main-thread-only,
// matching the UIKit notification + animation surface they drive. The next-responder return-key walk
// (KeyboardAutoManager.GoToNextResponderOrResign) is a separate, purely UIView-tree subsystem that lives
// in the iOS handler partials via src/platform/ios/ios_keyboard_manager_ops.hpp.

namespace maui::core
{
    // keyboard_auto_manager  <=  Microsoft.Maui.Platform.KeyboardAutoManagerScroll
    struct keyboard_auto_manager
    {
        // KeyboardAutoManagerScroll.Connect — register the keyboard + text-input observers (idempotent).
        static void connect_scroll_handler();

        // KeyboardAutoManagerScroll.Disconnect — remove the observers and clear the tracked tokens.
        static void disconnect_scroll_handler();
    };
} // namespace maui::core
