// maui::hosting::maui_app_builder — the fluent maui_app composer (maui_app_builder.hpp). Ported from
// src/Core/src/Hosting/MauiAppBuilder.cs (the registries core; the .NET host machinery is out of
// scope — see the header) + HandlerMauiAppBuilderExtensions.ConfigureMauiHandlers (the deferred
// registration replay) + LifecycleEvents/AppHostBuilderExtensions.ConfigureLifecycleEvents +
// AppHostBuilderExtensions.ConfigureDispatching (the platform-dispatcher default).

#include "maui/hosting/maui_app_builder.hpp"

#include <functional>
#include <memory>
#include <stdexcept>
#include <utility>

#include "maui/controls/application.hpp"
#include "maui/core/dispatcher.hpp"
#include "maui/core/handler_registry.hpp"
#include "maui/core/i_application.hpp"
#include "maui/hosting/i_lifecycle_builder.hpp"
#include "maui/hosting/i_lifecycle_event_service.hpp"
#include "maui/hosting/lifecycle_event_service.hpp"
#include "maui/hosting/maui_app.hpp"
#include "maui/hosting/maui_controls_handlers.hpp"
#include "maui/hosting/maui_handlers_collection.hpp"

#if defined(MAUI_PLATFORM_APPLE) || defined(MAUI_PLATFORM_IOS)
    #include "maui/core/gcd_dispatcher.hpp"
#else
    #include "maui/core/manual_dispatcher.hpp"
#endif

namespace maui::hosting
{
    namespace
    {
        // ConfigureDispatching's role: the backend's UI-thread dispatcher when none was supplied. The
        // backend is a compile-time fact (PROFILE §4 — one backend per build), so this is the hosting
        // analog of the per-platform Dispatcher partial: the GCD main queue on apple/ios (the macro pair
        // every backend build defines PUBLIC on maui_core), the virtual-clock manual_dispatcher on
        // headless (deterministic tests — no run loop to pump).
        std::shared_ptr<maui::core::i_dispatcher> make_platform_dispatcher()
        {
#if defined(MAUI_PLATFORM_APPLE) || defined(MAUI_PLATFORM_IOS)
            return std::make_shared<maui::core::gcd_dispatcher>();
#else
            return std::make_shared<maui::core::manual_dispatcher>();
#endif
        }
    } // namespace

    maui_app_builder::maui_app_builder()
    {
        // CreateBuilder(useDefaults: true): seed the controls handler table FIRST, so every later
        // configure_handlers callback can replace a default (last registration wins, matching the C#
        // service-collection resolution order).
        handler_registrations_.emplace_back(
            [](i_maui_handlers_collection& handlers) { add_maui_controls_handlers(handlers); });
    }

    maui_app_builder& maui_app_builder::configure_handlers(std::function<void(i_maui_handlers_collection&)> configure)
    {
        // C# tolerates a null configureDelegate (it only ensures the collection service exists).
        if (configure)
        {
            handler_registrations_.push_back(std::move(configure));
        }
        return *this;
    }

    maui_app_builder& maui_app_builder::configure_lifecycle_events(std::function<void(i_lifecycle_builder&)> configure)
    {
        if (configure)
        {
            lifecycle_registrations_.emplace_back(std::move(configure));
        }
        return *this;
    }

    maui_app_builder& maui_app_builder::use_dispatcher(std::shared_ptr<maui::core::i_dispatcher> dispatcher)
    {
        dispatcher_ = std::move(dispatcher);
        return *this;
    }

    void maui_app_builder::set_application_factory(
        std::function<std::shared_ptr<maui::controls::application>()> factory)
    {
        application_factory_ = std::move(factory); // last UseMauiApp wins (TryAddSingleton-last semantics)
    }

    std::unique_ptr<maui_app> maui_app_builder::build()
    {
        if (built_)
        {
            throw std::runtime_error("maui_app_builder: build() may be called only once");
        }
        built_ = true; // the C# MakeReadOnly flip

        // Replay the handler registrations against a fresh registry (HandlerServiceBuilder's ctor loop).
        maui::core::handler_registry handlers;
        maui_handlers_collection collection{handlers};
        for (const auto& configure : handler_registrations_)
        {
            configure(collection);
        }

        // The lifecycle registry, applying the collected registrations in order (LifecycleEventService).
        auto lifecycle = std::make_shared<lifecycle_event_service>(lifecycle_registrations_);

        std::shared_ptr<maui::core::i_dispatcher> dispatcher =
            dispatcher_ != nullptr ? std::move(dispatcher_) : make_platform_dispatcher();

        // Mint the application eagerly (C# defers to the first IApplication resolve; the port has no
        // lazy DI — the application either exists after build() or use_maui_app was never called).
        std::shared_ptr<maui::controls::application> application =
            application_factory_ ? application_factory_() : nullptr;

        // Pre-register the built singletons every consumer resolves from MauiApp.Services in C#.
        services_.add_singleton<maui::core::i_dispatcher>(dispatcher);
        services_.add_singleton<lifecycle_event_service>(lifecycle);
        services_.add_singleton<i_lifecycle_event_service>(lifecycle);
        if (application != nullptr)
        {
            services_.add_singleton<maui::core::i_application>(application);
        }

        return std::unique_ptr<maui_app>(new maui_app(std::move(services_), std::move(handlers), std::move(dispatcher),
                                                      std::move(lifecycle), std::move(application)));
    }
} // namespace maui::hosting
