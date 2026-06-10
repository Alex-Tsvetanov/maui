#pragma once
// maui::hosting::maui_app_builder  <=  Microsoft.Maui.Hosting.MauiAppBuilder
//   (+ Microsoft.Maui.Controls.Hosting.AppHostBuilderExtensions.UseMauiApp<TApp>,
//      Microsoft.Maui.Hosting.HandlerMauiAppBuilderExtensions.ConfigureMauiHandlers,
//      Microsoft.Maui.LifecycleEvents.MauiAppHostBuilderExtensions.ConfigureLifecycleEvents,
//      Microsoft.Maui.Hosting.AppHostBuilderExtensions.ConfigureDispatching)
//
// The fluent composer for a maui_app: use_maui_app<TApp>() mints the controls::application subclass,
// configure_handlers collects ConfigureMauiHandlers callbacks (replayed against a fresh handler_registry
// at build), configure_lifecycle_events collects the lifecycle registrations, services() exposes the
// builder-time service_registry (builder.Services), and build() assembles the maui_app — once, like the
// C# read-only flip on the service collection.
//
// OUT OF SCOPE (documented, M-L): the .NET generic-host machinery MauiAppBuilder also fronts —
// IHostApplicationBuilder, ConfigurationManager, ILoggingBuilder, IMetricsBuilder, Properties,
// ConfigureContainer (third-party DI), IMauiInitializeService — none has a C++ counterpart in the port;
// the builder composes exactly the registries the port owns. Defaults: the CONSTRUCTOR seeds the
// controls handler table (C# seeds it inside UseMauiApp → SetupDefaults; folding it into the ctor
// mirrors CreateBuilder(useDefaults: true) — the only creation path — and removes the call-order
// hazard), and build() supplies the platform dispatcher when use_dispatcher was not called
// (ConfigureDispatching's role): the GCD main-queue dispatcher on the apple/ios backends, the
// deterministic manual_dispatcher on headless.

#include <functional>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

#include "maui/controls/application.hpp"
#include "maui/core/dispatcher.hpp"
#include "maui/core/service_registry.hpp"
#include "maui/hosting/lifecycle_event_service.hpp"

namespace maui::hosting
{
    class i_maui_handlers_collection;
    class maui_app;

    class maui_app_builder
    {
    public:
        maui_app_builder(); // seeds the default controls handler table (see header note)

        // UseMauiApp<TApp>: mint TApp (a controls::application subclass) as THE application at build().
        template <class TApp> maui_app_builder& use_maui_app()
        {
            return use_maui_app<TApp>([] { return std::make_shared<TApp>(); });
        }
        // UseMauiApp<TApp>(implementationFactory): mint through the caller's factory instead.
        template <class TApp> maui_app_builder& use_maui_app(std::function<std::shared_ptr<TApp>()> factory)
        {
            static_assert(std::is_base_of_v<maui::controls::application, TApp>,
                          "TApp must derive maui::controls::application");
            set_application_factory(
                [create = std::move(factory)]() -> std::shared_ptr<maui::controls::application> { return create(); });
            return *this;
        }

        // ConfigureMauiHandlers: collect a handler-registration callback (replayed in order at build;
        // a later add_handler replaces an earlier one, so callbacks can override the seeded defaults).
        maui_app_builder& configure_handlers(std::function<void(i_maui_handlers_collection&)> configure);
        // ConfigureLifecycleEvents: collect a lifecycle registration (replayed in order at build).
        maui_app_builder& configure_lifecycle_events(std::function<void(i_lifecycle_builder&)> configure);
        // Supply the dispatcher explicitly (tests / custom loops); otherwise build() mints the platform one.
        maui_app_builder& use_dispatcher(std::shared_ptr<maui::core::i_dispatcher> dispatcher);

        // builder.Services: singletons registered here flow into the built maui_app's services().
        [[nodiscard]] maui::core::service_registry& services()
        {
            return services_;
        }

        // Build the maui_app (MauiAppBuilder.Build). Callable once — the registries move into the app.
        [[nodiscard]] std::unique_ptr<maui_app> build();

    private:
        void set_application_factory(std::function<std::shared_ptr<maui::controls::application>()> factory);

        maui::core::service_registry services_;
        std::vector<std::function<void(i_maui_handlers_collection&)>> handler_registrations_;
        std::vector<lifecycle_event_registration> lifecycle_registrations_;
        std::function<std::shared_ptr<maui::controls::application>()> application_factory_;
        std::shared_ptr<maui::core::i_dispatcher> dispatcher_;
        bool built_ = false;
    };
} // namespace maui::hosting
