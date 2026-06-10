// Tests for maui::hosting::maui_app_builder / maui_app (the builder boot, M-L unit hosting-app-builder).
// Characterizes the MauiAppBuilder/MauiApp oracle surface the port keeps: the seeded default controls
// handler table + ConfigureMauiHandlers replay order (AddHandler replaces, TryAddHandler keeps),
// UseMauiApp<TApp> minting (default ctor + factory overloads, IApplication service registration),
// builder.Services flow-through, the ConfigureDispatching default + use_dispatcher override, Build()'s
// one-shot contract, and the hosting extras: open_window driving Application.OpenWindow and
// attach_handler (the ToHandler analog: registry resolve + SetMauiContext + view owns handler).
// Backend-agnostic — no platform-view internals are asserted, so the suite runs on headless AND apple.
#include "maui/hosting/maui_app_builder.hpp"

#include <any>
#include <memory>
#include <stdexcept>
#include <string_view>

#include <gtest/gtest.h>

#include "maui/controls/application.hpp"
#include "maui/controls/button.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/window.hpp"
#include "maui/core/button_handler.hpp"
#include "maui/core/dispatcher.hpp"
#include "maui/core/i_application.hpp"
#include "maui/core/i_element.hpp"
#include "maui/core/i_element_handler.hpp"
#include "maui/core/i_maui_context.hpp"
#include "maui/core/manual_dispatcher.hpp"
#include "maui/core/type_tag.hpp"
#include "maui/hosting/i_maui_handlers_collection.hpp"
#include "maui/hosting/maui_app.hpp"

namespace
{
    using maui::core::type_tag;

    // A minimal stand-in handler for replacement tests (never drives a platform view).
    class stub_handler final : public maui::core::i_element_handler
    {
    public:
        void set_maui_context(maui::core::i_maui_context* context) override
        {
            context_ = context;
        }
        void set_virtual_view(maui::core::i_element& view) override
        {
            view_ = &view;
        }
        void update_value(std::string_view /*property*/) override
        {
        }
        void invoke(std::string_view /*command*/, const std::any& /*args*/) override
        {
        }
        void disconnect_handler() override
        {
            view_ = nullptr;
        }
        [[nodiscard]] void* platform_view() const override
        {
            return nullptr;
        }
        [[nodiscard]] maui::core::i_element* virtual_view() const override
        {
            return view_;
        }
        [[nodiscard]] maui::core::i_maui_context* maui_context() const override
        {
            return context_;
        }

    private:
        maui::core::i_element* view_ = nullptr;
        maui::core::i_maui_context* context_ = nullptr;
    };

    // The application subclass UseMauiApp<TApp> mints in these tests.
    class test_app final : public maui::controls::application
    {
    public:
        explicit test_app(int marker = 0) : marker_(marker)
        {
        }
        [[nodiscard]] int marker() const
        {
            return marker_;
        }

    private:
        int marker_;
    };

    // A control type nothing registers a handler for (a distinct type_tag from button's).
    class unregistered_button final : public maui::controls::button
    {
    };

    TEST(app_builder, seeds_the_default_controls_handler_table)
    {
        const auto app = maui::hosting::maui_app::create_builder().build();
        EXPECT_TRUE(app->handlers().is_registered(type_tag::of<maui::controls::button>()));
        EXPECT_TRUE(app->handlers().is_registered(type_tag::of<maui::controls::label>()));
        EXPECT_TRUE(app->handlers().is_registered(type_tag::of<maui::controls::window>()));
        EXPECT_NE(app->handlers().create_handler<maui::controls::button>(), nullptr);
    }

    TEST(app_builder, configure_handlers_add_handler_replaces_a_default)
    {
        const auto app = maui::hosting::maui_app::create_builder()
                             .configure_handlers([](maui::hosting::i_maui_handlers_collection& handlers) {
                                 handlers.add_handler<maui::controls::button, stub_handler>();
                             })
                             .build();
        const auto handler = app->handlers().create_handler<maui::controls::button>();
        EXPECT_NE(dynamic_cast<stub_handler*>(handler.get()), nullptr); // last registration won
    }

    TEST(app_builder, configure_handlers_try_add_handler_keeps_the_default)
    {
        const auto app = maui::hosting::maui_app::create_builder()
                             .configure_handlers([](maui::hosting::i_maui_handlers_collection& handlers) {
                                 handlers.try_add_handler<maui::controls::button, stub_handler>();
                             })
                             .build();
        const auto handler = app->handlers().create_handler<maui::controls::button>();
        EXPECT_EQ(dynamic_cast<stub_handler*>(handler.get()), nullptr); // the seeded default survived
    }

    TEST(app_builder, configure_handlers_factory_overload_registers)
    {
        int factory_runs = 0;
        const auto app =
            maui::hosting::maui_app::create_builder()
                .configure_handlers([&](maui::hosting::i_maui_handlers_collection& handlers) {
                    handlers.add_handler<unregistered_button>([&factory_runs] {
                        ++factory_runs;
                        return std::unique_ptr<maui::core::i_element_handler>(std::make_unique<stub_handler>());
                    });
                })
                .build();
        const auto handler = app->handlers().create_handler<unregistered_button>();
        EXPECT_NE(dynamic_cast<stub_handler*>(handler.get()), nullptr);
        EXPECT_EQ(factory_runs, 1);
    }

    TEST(app_builder, use_maui_app_mints_the_application_subclass)
    {
        const auto app = maui::hosting::maui_app::create_builder().use_maui_app<test_app>().build();
        ASSERT_NE(app->application(), nullptr);
        EXPECT_NE(app->application_as<test_app>(), nullptr);
        // ...and registers it as THE IApplication singleton (UseMauiPrimaryApp's TryAddSingleton).
        const auto face = app->services().get_service<maui::core::i_application>();
        EXPECT_EQ(face.get(), static_cast<maui::core::i_application*>(app->application().get()));
    }

    TEST(app_builder, use_maui_app_factory_overload_mints_through_the_factory)
    {
        const auto app = maui::hosting::maui_app::create_builder()
                             .use_maui_app<test_app>([] { return std::make_shared<test_app>(42); })
                             .build();
        const auto typed = app->application_as<test_app>();
        ASSERT_NE(typed, nullptr);
        EXPECT_EQ(typed->marker(), 42);
    }

    TEST(app_builder, application_is_null_when_use_maui_app_was_not_called)
    {
        const auto app = maui::hosting::maui_app::create_builder().build();
        EXPECT_EQ(app->application(), nullptr);
        EXPECT_EQ(app->services().get_service<maui::core::i_application>(), nullptr);
    }

    TEST(app_builder, builder_services_flow_into_the_built_app)
    {
        struct my_service
        {
            int value = 7;
        };
        auto builder = maui::hosting::maui_app::create_builder();
        builder.services().add_singleton<my_service>(std::make_shared<my_service>());
        const auto app = builder.build();
        const auto service = app->services().get_service<my_service>();
        ASSERT_NE(service, nullptr);
        EXPECT_EQ(service->value, 7);
    }

    TEST(app_builder, build_registers_a_default_dispatcher)
    {
        const auto app = maui::hosting::maui_app::create_builder().build();
        const auto dispatcher = app->services().get_service<maui::core::i_dispatcher>();
        ASSERT_NE(dispatcher, nullptr); // the platform default (GCD on apple/ios, manual on headless)
        EXPECT_EQ(dispatcher.get(), &app->dispatcher());
    }

    TEST(app_builder, use_dispatcher_overrides_the_platform_default)
    {
        const auto dispatcher = std::make_shared<maui::core::manual_dispatcher>();
        const auto app = maui::hosting::maui_app::create_builder().use_dispatcher(dispatcher).build();
        EXPECT_EQ(&app->dispatcher(), dispatcher.get());
        EXPECT_EQ(app->services().get_service<maui::core::i_dispatcher>(), dispatcher);
    }

    TEST(app_builder, build_may_only_be_called_once)
    {
        auto builder = maui::hosting::maui_app::create_builder();
        const auto app = builder.build();
        EXPECT_NE(app, nullptr);
        EXPECT_THROW((void)builder.build(), std::runtime_error); // the C# read-only flip
    }

    TEST(maui_app, open_window_drives_the_application_lifecycle)
    {
        maui::controls::window window; // declared BEFORE the app: it must outlive the bridge (maui_app.hpp)
        const auto app = maui::hosting::maui_app::create_builder().use_maui_app<test_app>().build();
        app->open_window(window);
        EXPECT_TRUE(window.is_created());   // SendStart -> Created
        EXPECT_TRUE(window.is_activated()); // ... -> Activated
        ASSERT_EQ(app->application()->windows_typed().size(), 1U);
        EXPECT_EQ(app->application()->windows_typed().front(), &window);

        app->close_window(window);
        EXPECT_TRUE(app->application()->windows_typed().empty());
    }

    TEST(maui_app, open_window_throws_without_an_application)
    {
        maui::controls::window window;
        const auto app = maui::hosting::maui_app::create_builder().build();
        EXPECT_THROW(app->open_window(window), std::runtime_error);
    }

    TEST(maui_app, attach_handler_resolves_from_the_registry_and_threads_the_context)
    {
        maui::controls::button control; // outlives the app's context (destroyed after — declared first)
        const auto app = maui::hosting::maui_app::create_builder().build();
        const auto handler = app->attach_handler(control);
        ASSERT_NE(handler, nullptr);
        EXPECT_EQ(control.handler(), handler);               // the view owns the handler (PROFILE §11)
        EXPECT_EQ(handler->maui_context(), &app->context()); // SetMauiContext preceded SetVirtualView
        EXPECT_EQ(handler->virtual_view(), &control);        // ...which connected the seam
        EXPECT_NE(dynamic_cast<maui::core::button_handler*>(handler.get()), nullptr);
        // The context resolves back to the app's own registries.
        EXPECT_EQ(&app->context().handlers(), &app->handlers());
        EXPECT_EQ(&app->context().services(), &app->services());
        control.set_handler(nullptr); // release the seam before the app (and its context) is destroyed
    }

    TEST(maui_app, attach_handler_throws_when_no_handler_is_registered)
    {
        unregistered_button control;
        const auto app = maui::hosting::maui_app::create_builder().build();
        EXPECT_THROW((void)app->attach_handler(control), std::runtime_error);
    }
} // namespace
