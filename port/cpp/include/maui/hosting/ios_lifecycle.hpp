#pragma once
// maui::hosting::ios_lifecycle_builder  <=  Microsoft.Maui.LifecycleEvents.IiOSLifecycleBuilder
//   (+ iOSLifecycleBuilderExtensions / iOSLifecycleExtensions.AddiOS / the iOSLifecycle delegate names)
//
// The iOS-flavoured registration shell: named fluent hooks that write platform-prefixed events into the
// same lifecycle registry (the C# pattern: AddiOS wraps the ILifecycleBuilder in an IiOSLifecycleBuilder
// whose extension methods AddEvent under the delegate's name). REGISTRATION is cross-platform — this
// type compiles on every backend, exactly like the C# builder types — while the INVOKE side belongs to
// the platform's UIApplicationDelegate: the ios sample's delegate drives these via invoke_events; a
// library-side driver arrives with the platform application object (out of scope, STATUS.md). Delegates
// are no-payload at this layer (the C# delegates receive the UIApplication; the port has no
// cross-platform face for it yet). The scene-phase delegates (SceneWillEnterForeground, …) are omitted
// with it. The hook subset kept is the classic app-delegate set the C# cross-platform mapping uses
// (AppHostBuilderExtensions.iOS.cs): launch, activate/resign, foreground/background, terminate.

#include <functional>
#include <string_view>
#include <utility>

#include "maui/hosting/i_lifecycle_builder.hpp"

namespace maui::hosting
{
    // The names the shell registers under ("ios_"-prefixed: the C# names live in one flat namespace per
    // platform assembly; the port shares one registry across the shells, so each prefixes its own).
    namespace ios_lifecycle_events
    {
        inline constexpr std::string_view will_finish_launching = "ios_will_finish_launching";
        inline constexpr std::string_view finished_launching = "ios_finished_launching";
        inline constexpr std::string_view on_activated = "ios_on_activated";
        inline constexpr std::string_view on_resign_activation = "ios_on_resign_activation";
        inline constexpr std::string_view will_enter_foreground = "ios_will_enter_foreground";
        inline constexpr std::string_view did_enter_background = "ios_did_enter_background";
        inline constexpr std::string_view will_terminate = "ios_will_terminate";
    } // namespace ios_lifecycle_events

    class ios_lifecycle_builder
    {
    public:
        explicit ios_lifecycle_builder(i_lifecycle_builder& builder) : builder_(&builder)
        {
        }

        // iOSLifecycleBuilderExtensions — one fluent hook per UIApplicationDelegate moment.
        ios_lifecycle_builder& will_finish_launching(lifecycle_action action);
        ios_lifecycle_builder& finished_launching(lifecycle_action action);
        ios_lifecycle_builder& on_activated(lifecycle_action action);
        ios_lifecycle_builder& on_resign_activation(lifecycle_action action);
        ios_lifecycle_builder& will_enter_foreground(lifecycle_action action);
        ios_lifecycle_builder& did_enter_background(lifecycle_action action);
        ios_lifecycle_builder& will_terminate(lifecycle_action action);

    private:
        i_lifecycle_builder* builder_; // NON-owning (the shell only lives inside an add_ios callback)
    };

    // iOSLifecycleExtensions.AddiOS: run `configure` against the iOS shell over `builder`. Fluent.
    i_lifecycle_builder& add_ios(i_lifecycle_builder& builder,
                                 const std::function<void(ios_lifecycle_builder&)>& configure);
} // namespace maui::hosting
