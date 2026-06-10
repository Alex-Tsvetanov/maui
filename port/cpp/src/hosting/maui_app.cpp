// maui::hosting::maui_app — the built application (maui_app.hpp). Ported from
// src/Core/src/Hosting/MauiApp.cs, plus the window lifecycle bridge (window_lifecycle_events.hpp) that
// maps the cross-platform window events into the ConfigureLifecycleEvents registry.

#include "maui/hosting/maui_app.hpp"

#include <memory>
#include <stdexcept>
#include <string_view>
#include <utility>
#include <vector>

#include "maui/controls/application.hpp"
#include "maui/controls/window.hpp"
#include "maui/core/dispatcher.hpp"
#include "maui/core/event.hpp"
#include "maui/core/handler_registry.hpp"
#include "maui/core/service_registry.hpp"
#include "maui/hosting/i_lifecycle_builder.hpp"
#include "maui/hosting/lifecycle_event_service.hpp"
#include "maui/hosting/maui_app_builder.hpp"
#include "maui/hosting/window_lifecycle_events.hpp"

namespace maui::hosting
{
    maui_app::maui_app(maui::core::service_registry services, maui::core::handler_registry handlers,
                       std::shared_ptr<maui::core::i_dispatcher> dispatcher,
                       std::shared_ptr<lifecycle_event_service> lifecycle,
                       std::shared_ptr<maui::controls::application> application)
        : services_(std::move(services)), handlers_(std::move(handlers)), dispatcher_(std::move(dispatcher)),
          lifecycle_(std::move(lifecycle)), application_(std::move(application)), context_(services_, handlers_)
    {
    }

    maui_app_builder maui_app::create_builder()
    {
        return {};
    }

    void maui_app::open_window(maui::controls::window& window)
    {
        if (application_ == nullptr)
        {
            throw std::runtime_error("maui_app: no application was configured — call use_maui_app<TApp>()");
        }
        attach_window_lifecycle(window); // BEFORE the open, so Created/Activated reach the delegates
        application_->open_window(window);
    }

    void maui_app::close_window(maui::controls::window& window)
    {
        if (application_ != nullptr)
        {
            application_->close_window(window); // Destroying still fires through the bridge
        }
        window_lifecycle_connections_.erase(&window); // then unbridge (scoped_connections disconnect)
    }

    void maui_app::attach_window_lifecycle(maui::controls::window& window)
    {
        if (window_lifecycle_connections_.contains(&window))
        {
            return; // already bridged (re-opening an open window is a no-op upstream too)
        }
        std::vector<maui::core::scoped_connection>& connections = window_lifecycle_connections_[&window];
        const auto bridge = [this, &window, &connections](maui::core::event<>& source, std::string_view event_name) {
            connections.push_back(maui::core::connect_scoped(
                source, [this, &window, event_name] { invoke_window_event(event_name, window); }));
        };
        bridge(window.created, window_lifecycle_events::created);
        bridge(window.activated, window_lifecycle_events::activated);
        bridge(window.deactivated, window_lifecycle_events::deactivated);
        bridge(window.destroying, window_lifecycle_events::destroying);
        bridge(window.resumed, window_lifecycle_events::resumed);
        bridge(window.stopped, window_lifecycle_events::stopped);
        bridge(window.backgrounding, window_lifecycle_events::backgrounding);
    }

    void maui_app::invoke_window_event(std::string_view event_name, maui::controls::window& window) const
    {
        // The typed shape first (the bridge's own delegate), then the no-payload registrations under the
        // same name (the AddEvent(string, Action) shape — see window_lifecycle_events.hpp).
        for (const window_lifecycle_action& action :
             lifecycle_->get_event_delegates<window_lifecycle_action>(event_name))
        {
            if (action)
            {
                action(window);
            }
        }
        for (const lifecycle_action& action : lifecycle_->get_event_delegates<lifecycle_action>(event_name))
        {
            if (action)
            {
                action();
            }
        }
    }
} // namespace maui::hosting
