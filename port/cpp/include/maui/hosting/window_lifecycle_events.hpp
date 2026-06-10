#pragma once
// maui::hosting::window_lifecycle_events — the CROSS-PLATFORM window lifecycle event names + delegate.
//
// MAUI's lifecycle service is keyed by per-platform delegate names (iOSLifecycle.OnActivated, …) and the
// cross-platform Window lifecycle reaches app code only as the Window.Created/Activated/… C# events. The
// port's hosting layer ADDITIONALLY bridges those window events into the lifecycle service —
// maui_app::open_window connects each window event to the matching name below before the application
// opens the window — so ConfigureLifecycleEvents observes the portable lifecycle on every backend
// without a platform shim (the per-platform shells in apple_lifecycle.hpp / ios_lifecycle.hpp carry the
// platform-specific names). Each bridged delegate receives the cross-platform window the event concerns
// (C#'s platform delegates receive the platform window; the portable bridge hands over the
// maui::controls::window instead).

#include <functional>
#include <string_view>

namespace maui::controls
{
    class window;
}

namespace maui::hosting
{
    // The delegate every bridged window lifecycle event receives. The bridge ALSO runs plain
    // lifecycle_action registrations under these names (a port convenience — C# would skip the
    // mismatched delegate type silently; the no-payload shape is too common to drop).
    using window_lifecycle_action = std::function<void(maui::controls::window&)>;

    // One name per window event (controls/window.hpp): Created / Activated / Deactivated / Destroying
    // + Resumed / Stopped / Backgrounding.
    namespace window_lifecycle_events
    {
        inline constexpr std::string_view created = "window_created";
        inline constexpr std::string_view activated = "window_activated";
        inline constexpr std::string_view deactivated = "window_deactivated";
        inline constexpr std::string_view destroying = "window_destroying";
        inline constexpr std::string_view resumed = "window_resumed";
        inline constexpr std::string_view stopped = "window_stopped";
        inline constexpr std::string_view backgrounding = "window_backgrounding";
    } // namespace window_lifecycle_events
} // namespace maui::hosting
