#pragma once
// maui::hosting::i_lifecycle_event_service  <=  Microsoft.Maui.LifecycleEvents.ILifecycleEventService
//   (+ Microsoft.Maui.LifecycleEvents.LifecycleEventServiceExtensions.InvokeEvents)
//
// The consumption face of the lifecycle registry: the platform (or the maui_app's window bridge) asks
// for the delegates registered under an event name and invokes them. get_event_delegates<TDelegate>
// mirrors C#'s OfType<TDelegate>() filter — a delegate registered under the name but with a different
// delegate type is skipped silently. The erased channel (event_delegates) returns a SNAPSHOT, so a
// delegate registered mid-invocation is not observed until the next lookup.

#include <any>
#include <functional>
#include <string_view>
#include <vector>

#include "maui/hosting/i_lifecycle_builder.hpp"

namespace maui::hosting
{
    class i_lifecycle_event_service
    {
    public:
        virtual ~i_lifecycle_event_service() = default;

        // ILifecycleEventService.ContainsEvent: at least one delegate is registered under event_name.
        [[nodiscard]] virtual bool contains_event(std::string_view event_name) const = 0;
        // The erased channel: a snapshot of the delegates registered under event_name (empty when none).
        [[nodiscard]] virtual std::vector<std::any> event_delegates(std::string_view event_name) const = 0;

        // ILifecycleEventService.GetEventDelegates<TDelegate>: the typed view (OfType<TDelegate>).
        template <class TDelegate>
        [[nodiscard]] std::vector<TDelegate> get_event_delegates(std::string_view event_name) const
        {
            std::vector<TDelegate> matched;
            for (const std::any& erased : event_delegates(event_name))
            {
                if (const auto* typed = std::any_cast<TDelegate>(&erased))
                {
                    matched.push_back(*typed);
                }
            }
            return matched;
        }

    protected:
        i_lifecycle_event_service() = default;
        i_lifecycle_event_service(const i_lifecycle_event_service&) = default;
        i_lifecycle_event_service(i_lifecycle_event_service&&) = default;
        i_lifecycle_event_service& operator=(const i_lifecycle_event_service&) = default;
        i_lifecycle_event_service& operator=(i_lifecycle_event_service&&) = default;
    };

    // LifecycleEventServiceExtensions.InvokeEvents<TDelegate>(eventName, action): hand each registered
    // TDelegate to the invoker (which supplies the delegate's arguments).
    template <class TDelegate>
    void invoke_events(const i_lifecycle_event_service& service, std::string_view event_name,
                       const std::function<void(const TDelegate&)>& invoke)
    {
        for (const TDelegate& delegate : service.get_event_delegates<TDelegate>(event_name))
        {
            if (delegate && invoke)
            {
                invoke(delegate);
            }
        }
    }

    // LifecycleEventServiceExtensions.InvokeEvents(eventName): run every no-payload delegate.
    inline void invoke_events(const i_lifecycle_event_service& service, std::string_view event_name)
    {
        for (const lifecycle_action& action : service.get_event_delegates<lifecycle_action>(event_name))
        {
            if (action)
            {
                action();
            }
        }
    }
} // namespace maui::hosting
