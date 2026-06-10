// Tests for maui::hosting's lifecycle-events subsystem (M-L unit hosting-app-builder): the
// named-delegate registry (lifecycle_event_service — AddEvent / GetEventDelegates / ContainsEvent with
// C#'s OfType-filter semantics + the registration replay order), the invoke_events helpers, the
// per-platform builder shells (add_apple / add_ios), and the hosting bridge that maps the EXISTING
// cross-platform window events (created/activated/.../backgrounding) into ConfigureLifecycleEvents
// registrations through maui_app::open_window. Backend-agnostic (headless + apple).
#include "maui/hosting/lifecycle_event_service.hpp"

#include <functional>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "maui/controls/application.hpp"
#include "maui/controls/window.hpp"
#include "maui/hosting/apple_lifecycle.hpp"
#include "maui/hosting/i_lifecycle_builder.hpp"
#include "maui/hosting/i_lifecycle_event_service.hpp"
#include "maui/hosting/ios_lifecycle.hpp"
#include "maui/hosting/maui_app.hpp"
#include "maui/hosting/maui_app_builder.hpp"
#include "maui/hosting/window_lifecycle_events.hpp"

namespace
{
    using maui::hosting::lifecycle_action;
    using maui::hosting::lifecycle_event_registration;
    using maui::hosting::lifecycle_event_service;
    using maui::hosting::window_lifecycle_action;
    namespace window_events = maui::hosting::window_lifecycle_events;

    TEST(lifecycle_event_service, add_event_and_get_event_delegates_roundtrip)
    {
        lifecycle_event_service service;
        int ran = 0;
        service.add_event("custom", [&ran] { ++ran; });

        const auto delegates = service.get_event_delegates<lifecycle_action>("custom");
        ASSERT_EQ(delegates.size(), 1U);
        delegates.front()();
        EXPECT_EQ(ran, 1);
    }

    TEST(lifecycle_event_service, contains_event_reports_registered_names_only)
    {
        lifecycle_event_service service;
        EXPECT_FALSE(service.contains_event("custom"));
        service.add_event("custom", [] {});
        EXPECT_TRUE(service.contains_event("custom"));
        EXPECT_FALSE(service.contains_event("other"));
    }

    TEST(lifecycle_event_service, get_event_delegates_filters_by_delegate_type)
    {
        // C#'s OfType<TDelegate>(): a delegate registered under the name but with another delegate type
        // is skipped silently by the typed view.
        lifecycle_event_service service;
        service.add_event("mixed", [] {});                                                   // lifecycle_action
        service.add_event<window_lifecycle_action>("mixed", [](maui::controls::window&) {}); // window shape

        EXPECT_EQ(service.get_event_delegates<lifecycle_action>("mixed").size(), 1U);
        EXPECT_EQ(service.get_event_delegates<window_lifecycle_action>("mixed").size(), 1U);
        EXPECT_EQ(service.event_delegates("mixed").size(), 2U); // the erased channel sees both
    }

    TEST(lifecycle_event_service, delegates_run_in_registration_order)
    {
        lifecycle_event_service service;
        std::vector<int> order;
        service.add_event("ordered", [&order] { order.push_back(1); });
        service.add_event("ordered", [&order] { order.push_back(2); });
        service.add_event("ordered", [&order] { order.push_back(3); });
        maui::hosting::invoke_events(service, "ordered");
        EXPECT_EQ(order, (std::vector<int>{1, 2, 3}));
    }

    TEST(lifecycle_event_service, constructor_replays_registrations_in_order)
    {
        // The C# ctor loop over the collected LifecycleEventRegistration list.
        std::vector<int> order;
        std::vector<lifecycle_event_registration> registrations;
        registrations.emplace_back([&order](maui::hosting::i_lifecycle_builder& builder) {
            builder.add_event("boot", [&order] { order.push_back(1); });
        });
        registrations.emplace_back([&order](maui::hosting::i_lifecycle_builder& builder) {
            builder.add_event("boot", [&order] { order.push_back(2); });
        });

        const lifecycle_event_service service{registrations};
        maui::hosting::invoke_events(service, "boot");
        EXPECT_EQ(order, (std::vector<int>{1, 2}));
    }

    TEST(lifecycle_event_service, typed_invoke_events_hands_each_delegate_to_the_invoker)
    {
        lifecycle_event_service service;
        maui::controls::window window;
        std::vector<maui::controls::window*> seen;
        service.add_event<window_lifecycle_action>("typed",
                                                   [&seen](maui::controls::window& value) { seen.push_back(&value); });
        service.add_event<window_lifecycle_action>("typed",
                                                   [&seen](maui::controls::window& value) { seen.push_back(&value); });

        maui::hosting::invoke_events<window_lifecycle_action>(
            service, "typed", [&window](const window_lifecycle_action& action) { action(window); });
        EXPECT_EQ(seen, (std::vector<maui::controls::window*>{&window, &window}));
    }

    TEST(lifecycle_shells, add_ios_registers_under_the_ios_names)
    {
        lifecycle_event_service service;
        int activated = 0;
        int terminated = 0;
        maui::hosting::add_ios(service, [&](maui::hosting::ios_lifecycle_builder& ios) {
            ios.on_activated([&activated] { ++activated; }).will_terminate([&terminated] { ++terminated; });
        });

        EXPECT_TRUE(service.contains_event(maui::hosting::ios_lifecycle_events::on_activated));
        maui::hosting::invoke_events(service, maui::hosting::ios_lifecycle_events::on_activated);
        maui::hosting::invoke_events(service, maui::hosting::ios_lifecycle_events::will_terminate);
        EXPECT_EQ(activated, 1);
        EXPECT_EQ(terminated, 1);
    }

    TEST(lifecycle_shells, add_apple_registers_under_the_apple_names)
    {
        lifecycle_event_service service;
        int became_active = 0;
        maui::hosting::add_apple(service, [&](maui::hosting::apple_lifecycle_builder& apple) {
            apple.did_become_active([&became_active] { ++became_active; });
        });

        EXPECT_TRUE(service.contains_event(maui::hosting::apple_lifecycle_events::did_become_active));
        maui::hosting::invoke_events(service, maui::hosting::apple_lifecycle_events::did_become_active);
        EXPECT_EQ(became_active, 1);
    }

    TEST(window_lifecycle_bridge, open_window_invokes_created_then_activated_with_the_window)
    {
        maui::controls::window window; // outlives the app (the bridge detaches at app destruction)
        std::vector<std::string> trace;
        const auto app = maui::hosting::maui_app::create_builder()
                             .use_maui_app<maui::controls::application>()
                             .configure_lifecycle_events([&](maui::hosting::i_lifecycle_builder& lifecycle) {
                                 lifecycle.add_event<window_lifecycle_action>(
                                     window_events::created, [&trace, &window](maui::controls::window& value) {
                                         EXPECT_EQ(&value, &window); // the delegate receives the window it concerns
                                         trace.emplace_back("created");
                                     });
                                 lifecycle.add_event<window_lifecycle_action>(
                                     window_events::activated,
                                     [&trace](maui::controls::window& /*value*/) { trace.emplace_back("activated"); });
                             })
                             .build();

        app->open_window(window);
        EXPECT_EQ(trace, (std::vector<std::string>{"created", "activated"})); // OpenWindow's drive order
    }

    TEST(window_lifecycle_bridge, no_payload_registrations_fire_too)
    {
        // The port convenience documented in window_lifecycle_events.hpp: a plain Action registered
        // under a window event name is invoked alongside the typed shape.
        maui::controls::window window;
        int created = 0;
        const auto app = maui::hosting::maui_app::create_builder()
                             .use_maui_app<maui::controls::application>()
                             .configure_lifecycle_events([&](maui::hosting::i_lifecycle_builder& lifecycle) {
                                 lifecycle.add_event(window_events::created, [&created] { ++created; });
                             })
                             .build();
        app->open_window(window);
        EXPECT_EQ(created, 1);
    }

    TEST(window_lifecycle_bridge, resume_and_stop_reach_the_delegates)
    {
        maui::controls::window window;
        std::vector<std::string> trace;
        const auto app =
            maui::hosting::maui_app::create_builder()
                .use_maui_app<maui::controls::application>()
                .configure_lifecycle_events([&](maui::hosting::i_lifecycle_builder& lifecycle) {
                    lifecycle.add_event(window_events::resumed, [&trace] { trace.emplace_back("resumed"); });
                    lifecycle.add_event(window_events::stopped, [&trace] { trace.emplace_back("stopped"); });
                    lifecycle.add_event(window_events::backgrounding,
                                        [&trace] { trace.emplace_back("backgrounding"); });
                })
                .build();
        app->open_window(window);

        window.send_stopped();
        window.send_backgrounding();
        window.send_resumed();
        EXPECT_EQ(trace, (std::vector<std::string>{"stopped", "backgrounding", "resumed"}));
    }

    TEST(window_lifecycle_bridge, close_window_fires_destroying_then_unbridges)
    {
        maui::controls::window window;
        int destroying = 0;
        int deactivated = 0;
        const auto app = maui::hosting::maui_app::create_builder()
                             .use_maui_app<maui::controls::application>()
                             .configure_lifecycle_events([&](maui::hosting::i_lifecycle_builder& lifecycle) {
                                 lifecycle.add_event(window_events::destroying, [&destroying] { ++destroying; });
                                 lifecycle.add_event(window_events::deactivated, [&deactivated] { ++deactivated; });
                             })
                             .build();
        app->open_window(window);
        app->close_window(window);
        EXPECT_EQ(destroying, 1);  // Destroying fired through the still-connected bridge
        EXPECT_GE(deactivated, 1); // CloseWindow deactivates before destroying

        const int destroying_after_close = destroying;
        window.send_created(); // re-drive the raw window events: the bridge is gone, so no delegates run
        window.send_activated();
        window.send_destroying();
        EXPECT_EQ(destroying, destroying_after_close);
    }

    TEST(window_lifecycle_bridge, built_lifecycle_service_is_resolvable_from_services)
    {
        const auto app = maui::hosting::maui_app::create_builder()
                             .configure_lifecycle_events([](maui::hosting::i_lifecycle_builder& lifecycle) {
                                 lifecycle.add_event("custom", [] {});
                             })
                             .build();
        // Both the concrete service and the i_* face resolve (the C# ILifecycleEventService singleton).
        const auto service = app->services().get_service<lifecycle_event_service>();
        ASSERT_NE(service, nullptr);
        EXPECT_TRUE(service->contains_event("custom"));
        const auto face = app->services().get_service<maui::hosting::i_lifecycle_event_service>();
        ASSERT_NE(face, nullptr);
        EXPECT_TRUE(face->contains_event("custom"));
        EXPECT_EQ(service.get(), &app->lifecycle_events());
    }
} // namespace
