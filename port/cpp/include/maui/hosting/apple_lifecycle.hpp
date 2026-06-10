#pragma once
// maui::hosting::apple_lifecycle_builder — the AppKit (macOS) analog of ios_lifecycle.hpp.
//   <=  the AppKit twin of Microsoft.Maui.LifecycleEvents.IiOSLifecycleBuilder (net-new: MAUI has no
//       AppKit target — its macOS story is Mac Catalyst/UIKit — but the port's primary dev backend is
//       AppKit, PROFILE §4, so the shell mirrors the iOS one over the NSApplication delegate moments).
//
// Same contract as the iOS shell: registration is cross-platform (compiles on every backend); the
// INVOKE side belongs to the NSApplicationDelegate (the macOS samples can drive these via
// invoke_events; a library-side driver arrives with the platform application object — out of scope,
// STATUS.md). Hooks map 1:1 to NSApplication notifications: DidFinishLaunching / DidBecomeActive /
// DidResignActive / WillTerminate.

#include <functional>
#include <string_view>
#include <utility>

#include "maui/hosting/i_lifecycle_builder.hpp"

namespace maui::hosting
{
    // The names the shell registers under ("apple_"-prefixed; see ios_lifecycle.hpp on prefixing).
    namespace apple_lifecycle_events
    {
        inline constexpr std::string_view did_finish_launching = "apple_did_finish_launching";
        inline constexpr std::string_view did_become_active = "apple_did_become_active";
        inline constexpr std::string_view did_resign_active = "apple_did_resign_active";
        inline constexpr std::string_view will_terminate = "apple_will_terminate";
    } // namespace apple_lifecycle_events

    class apple_lifecycle_builder
    {
    public:
        explicit apple_lifecycle_builder(i_lifecycle_builder& builder) : builder_(&builder)
        {
        }

        // One fluent hook per NSApplicationDelegate moment.
        apple_lifecycle_builder& did_finish_launching(lifecycle_action action);
        apple_lifecycle_builder& did_become_active(lifecycle_action action);
        apple_lifecycle_builder& did_resign_active(lifecycle_action action);
        apple_lifecycle_builder& will_terminate(lifecycle_action action);

    private:
        i_lifecycle_builder* builder_; // NON-owning (the shell only lives inside an add_apple callback)
    };

    // The AddiOS twin: run `configure` against the AppKit shell over `builder`. Fluent.
    i_lifecycle_builder& add_apple(i_lifecycle_builder& builder,
                                   const std::function<void(apple_lifecycle_builder&)>& configure);
} // namespace maui::hosting
