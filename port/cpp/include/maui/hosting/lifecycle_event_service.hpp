#pragma once
// maui::hosting::lifecycle_event_service  <=  Microsoft.Maui.LifecycleEvents.LifecycleEventService
// maui::hosting::lifecycle_event_registration  <=  Microsoft.Maui.LifecycleEvents.LifecycleEventRegistration
//   (Hosting/LifecycleEvents/AppHostBuilderExtensions.cs)
//
// The concrete named-delegate registry: both the registration face (i_lifecycle_builder — the
// ConfigureLifecycleEvents delegates write through it) and the consumption face (i_lifecycle_event_service
// — the platform / the maui_app window bridge reads from it). Constructed from the ordered registration
// list the builder collected, applying each in registration order (the C# ctor loop). The registration
// type shares this header — it exists only to be replayed against the service (a two-type cluster,
// PROFILE §3).

#include <any>
#include <functional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

#include "maui/hosting/i_lifecycle_builder.hpp"
#include "maui/hosting/i_lifecycle_event_service.hpp"

namespace maui::hosting
{
    // One deferred ConfigureLifecycleEvents callback: recorded by the builder, replayed at build().
    class lifecycle_event_registration
    {
    public:
        explicit lifecycle_event_registration(std::function<void(i_lifecycle_builder&)> register_action)
            : register_action_(std::move(register_action))
        {
        }

        // LifecycleEventRegistration.AddRegistration: apply the recorded callback to the builder face.
        void add_registration(i_lifecycle_builder& builder) const
        {
            if (register_action_)
            {
                register_action_(builder);
            }
        }

    private:
        std::function<void(i_lifecycle_builder&)> register_action_;
    };

    class lifecycle_event_service final : public i_lifecycle_event_service, public i_lifecycle_builder
    {
    public:
        lifecycle_event_service() = default;
        explicit lifecycle_event_service(const std::vector<lifecycle_event_registration>& registrations);

        void add_event_delegate(std::string_view event_name, std::any action) override;
        [[nodiscard]] bool contains_event(std::string_view event_name) const override;
        [[nodiscard]] std::vector<std::any> event_delegates(std::string_view event_name) const override;

    private:
        // Heterogeneous (string_view) lookup into the string-keyed map — no per-lookup allocation.
        struct name_hash
        {
            using is_transparent = void;
            [[nodiscard]] std::size_t operator()(std::string_view name) const noexcept
            {
                return std::hash<std::string_view>{}(name);
            }
        };

        // C# LifecycleEventService._mapper: event name -> ordered delegate list (Ordinal comparison).
        std::unordered_map<std::string, std::vector<std::any>, name_hash, std::equal_to<>> mapper_;
    };
} // namespace maui::hosting
