// maui::hosting::lifecycle_event_service — the named-delegate registry (lifecycle_event_service.hpp).
// Ported from src/Core/src/LifecycleEvents/LifecycleEventService.cs.

#include "maui/hosting/lifecycle_event_service.hpp"

#include <any>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace maui::hosting
{
    lifecycle_event_service::lifecycle_event_service(const std::vector<lifecycle_event_registration>& registrations)
    {
        // The C# ctor loop: replay every recorded ConfigureLifecycleEvents callback, in order.
        for (const lifecycle_event_registration& registration : registrations)
        {
            registration.add_registration(*this);
        }
    }

    void lifecycle_event_service::add_event_delegate(std::string_view event_name, std::any action)
    {
        const auto found = mapper_.find(event_name);
        if (found != mapper_.end())
        {
            found->second.push_back(std::move(action));
            return;
        }
        mapper_.try_emplace(std::string(event_name)).first->second.push_back(std::move(action));
    }

    bool lifecycle_event_service::contains_event(std::string_view event_name) const
    {
        const auto found = mapper_.find(event_name);
        return found != mapper_.end() && !found->second.empty();
    }

    std::vector<std::any> lifecycle_event_service::event_delegates(std::string_view event_name) const
    {
        const auto found = mapper_.find(event_name);
        if (found == mapper_.end())
        {
            return {};
        }
        return found->second; // snapshot copy (each std::any holds a std::function — cheap to copy)
    }
} // namespace maui::hosting
