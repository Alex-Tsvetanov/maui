// ios_keyboard_auto_manager — AppKit (apple/macOS) stub of the keyboard scroll-avoidance engine. macOS
// has no soft keyboard (AppKit windows never present an on-screen keyboard that overlaps content), so the
// engine has nothing to observe or scroll away from. Both contract methods are no-ops, mirroring the C#
// situation where KeyboardAutoManagerScroll is an iOS-only type. Compiled for the `apple` backend; the
// real engine lives in src/platform/ios/ios_keyboard_auto_manager.mm.

#include "maui/core/keyboard_auto_manager.hpp"

namespace maui::core
{
    void keyboard_auto_manager::connect_scroll_handler()
    {
        // No soft keyboard on macOS — nothing to observe.
    }

    void keyboard_auto_manager::disconnect_scroll_handler()
    {
        // No-op: no observers were ever registered.
    }
} // namespace maui::core
