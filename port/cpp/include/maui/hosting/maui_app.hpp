#pragma once
// maui::hosting::maui_app  <=  Microsoft.Maui.Hosting.MauiApp
//   (+ Microsoft.Maui.Platform.ElementExtensions.ToHandler — attach_handler below)
//
// The built application: owns the registries the builder composed (services + handlers), the UI-thread
// dispatcher, the lifecycle registry, and the minted controls::application. Created exclusively by
// maui_app_builder::build() (the C# internal ctor); MauiApp.CreateBuilder is mirrored by the static
// create_builder(). MauiApp.Services maps to services(); the rest of MauiApp.cs is .NET host machinery
// the port declares OUT OF SCOPE (maui_app_builder.hpp): IConfiguration, and Dispose/DisposeAsync over
// the DI container — RAII covers teardown.
//
// open_window is the hosting-level door to Application.OpenWindow that FIRST bridges the window's
// lifecycle events into the lifecycle service (window_lifecycle_events.hpp) — the portable analog of
// the per-platform delegates C# invokes from the platform window callbacks — then lets the application
// open (and activate) the window, so the bridged Created/Activated delegates observe the open itself.
//
// Lifetime doctrine (PROFILE §8): the maui_app must outlive the handlers attach_handler mints (they
// keep a raw i_maui_context* into it) and must be destroyed while the windows it bridged are still
// alive (its scoped_connections detach from the windows' events) — i.e. construct the element tree
// first / destroy the maui_app first, or close_window each window before either dies. The samples and
// tests follow that order; close_window also unbridges explicitly.

#include <memory>
#include <stdexcept>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "maui/controls/application.hpp"
#include "maui/controls/window.hpp"
#include "maui/core/dispatcher.hpp"
#include "maui/core/event.hpp"
#include "maui/core/handler_registry.hpp"
#include "maui/core/i_element_handler.hpp"
#include "maui/core/i_maui_context.hpp"
#include "maui/core/service_registry.hpp"
#include "maui/hosting/lifecycle_event_service.hpp"
#include "maui/hosting/maui_context.hpp"

namespace maui::hosting
{
    class maui_app_builder;

    class maui_app
    {
    public:
        // MauiApp.CreateBuilder — defaults always on (the port has no useDefaults:false path; the C#
        // false path only skips the .NET host defaults that are out of scope here).
        [[nodiscard]] static maui_app_builder create_builder();

        maui_app(const maui_app&) = delete;
        maui_app(maui_app&&) = delete;
        maui_app& operator=(const maui_app&) = delete;
        maui_app& operator=(maui_app&&) = delete;
        ~maui_app() = default;

        // MauiApp.Services — the composed singletons (the dispatcher, the lifecycle service, and the
        // application are pre-registered by build(); builder-time singletons flow through).
        [[nodiscard]] maui::core::service_registry& services()
        {
            return services_;
        }
        [[nodiscard]] const maui::core::service_registry& services() const
        {
            return services_;
        }
        // The handler table the builder composed (the IMauiHandlersFactory resolution face).
        [[nodiscard]] maui::core::handler_registry& handlers()
        {
            return handlers_;
        }
        [[nodiscard]] const maui::core::handler_registry& handlers() const
        {
            return handlers_;
        }
        // The UI-thread dispatcher (ConfigureDispatching's product): GCD on apple/ios, manual on headless.
        [[nodiscard]] maui::core::i_dispatcher& dispatcher() const
        {
            return *dispatcher_;
        }
        // The lifecycle registry ConfigureLifecycleEvents composed.
        [[nodiscard]] lifecycle_event_service& lifecycle_events()
        {
            return *lifecycle_;
        }
        [[nodiscard]] const lifecycle_event_service& lifecycle_events() const
        {
            return *lifecycle_;
        }
        // The IMauiContext threaded into every handler attach_handler mints.
        [[nodiscard]] maui::core::i_maui_context& context()
        {
            return context_;
        }

        // The minted application (UseMauiApp<TApp>'s product) — null when use_maui_app was not called.
        [[nodiscard]] const std::shared_ptr<maui::controls::application>& application() const
        {
            return application_;
        }
        // The application down-cast to the concrete TApp it was minted as (null on mismatch / none).
        template <class TApp> [[nodiscard]] std::shared_ptr<TApp> application_as() const
        {
            return std::dynamic_pointer_cast<TApp>(application_);
        }

        // Open `window` through the application lifecycle (Application.OpenWindow: SendStart → Created →
        // Activated), bridging its lifecycle events into the lifecycle service FIRST so the registered
        // delegates observe the open. Throws when no application was configured.
        void open_window(maui::controls::window& window);
        // Close `window` (Application.CloseWindow — Destroying fires through the bridge) and unbridge it.
        void close_window(maui::controls::window& window);

        // ElementExtensions.ToHandler: mint the handler registered for View, thread the maui context
        // into it (SetMauiContext precedes SetVirtualView, as in C#), and attach it to `view` (the view
        // owns it, PROFILE §11). Throws when no handler is registered (C#'s HandlerNotFoundException).
        template <class View> std::shared_ptr<maui::core::i_element_handler> attach_handler(View& view)
        {
            std::shared_ptr<maui::core::i_element_handler> handler = handlers_.create_handler<View>();
            if (handler == nullptr)
            {
                throw std::runtime_error("maui_app: no handler is registered for this view type");
            }
            handler->set_maui_context(&context_);
            view.set_handler(handler);
            return handler;
        }

    private:
        friend class maui_app_builder; // the only creator (the C# internal MauiApp(IServiceProvider))
        maui_app(maui::core::service_registry services, maui::core::handler_registry handlers,
                 std::shared_ptr<maui::core::i_dispatcher> dispatcher,
                 std::shared_ptr<lifecycle_event_service> lifecycle,
                 std::shared_ptr<maui::controls::application> application);

        // Connect the window's seven lifecycle events to the named delegates (idempotent per window).
        void attach_window_lifecycle(maui::controls::window& window);
        // Run the delegates registered under `event_name` (both the window-payload and no-payload shapes).
        void invoke_window_event(std::string_view event_name, maui::controls::window& window) const;

        maui::core::service_registry services_;
        maui::core::handler_registry handlers_;
        std::shared_ptr<maui::core::i_dispatcher> dispatcher_;
        std::shared_ptr<lifecycle_event_service> lifecycle_;
        std::shared_ptr<maui::controls::application> application_;
        maui_context context_; // references services_/handlers_ — declared after them
        // One bridge per open window: the seven event subscriptions, detached on close_window or when
        // this maui_app is destroyed (so the bridged windows must still be alive then — see the header
        // doctrine above). Declared LAST so destruction detaches before the application (and its
        // subclass-owned windows) goes away.
        std::unordered_map<maui::controls::window*, std::vector<maui::core::scoped_connection>>
            window_lifecycle_connections_;
    };
} // namespace maui::hosting
