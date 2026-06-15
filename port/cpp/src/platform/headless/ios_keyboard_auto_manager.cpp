// ios_keyboard_auto_manager — headless stub of the keyboard scroll-avoidance engine. The headless backend
// has no native windowing or soft keyboard, so the engine has nothing to wire; both contract methods are
// no-ops. This lets the shared handler code call connect/disconnect unconditionally and lets the
// headless test suite link and run. The real engine is the iOS partial
// (src/platform/ios/ios_keyboard_auto_manager.mm); the AppKit twin is a no-op for the same reason as here.

#include "maui/core/keyboard_auto_manager.hpp"

namespace maui::core
{
    void keyboard_auto_manager::connect_scroll_handler()
    {
        // Headless has no keyboard — nothing to observe.
    }

    void keyboard_auto_manager::disconnect_scroll_handler()
    {
        // No-op.
    }
} // namespace maui::core
