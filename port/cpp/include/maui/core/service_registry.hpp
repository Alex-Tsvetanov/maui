#pragma once
// maui::core::service_registry — a type-keyed service locator (PROFILE §5/§6): the C++ stand-in for
// MAUI's IServiceProvider (Microsoft.Extensions.DependencyInjection). Registration is explicit (no
// reflection/scan): add a singleton instance, resolve it by its type.
//
// Type-erasure is deliberately CONFINED here — a service locator is inherently dynamic. Instances are
// held as shared_ptr<void> keyed by type_tag and recovered with static_pointer_cast back to the exact
// type they were registered under; this is safe because the key guarantees the stored dynamic type
// (we only ever cast back to the same type used as the key). This is the boundary-confined erasure
// PROFILE §7 endorses, not the value-system erasure it forbids.

#include <memory>
#include <stdexcept>
#include <unordered_map>
#include <utility>

#include "maui/core/type_tag.hpp"

namespace maui::core
{
    class service_registry
    {
    public:
        // Register (or replace) the singleton instance for Service.
        template <class Service> void add_singleton(std::shared_ptr<Service> instance)
        {
            services_[type_tag::of<Service>()] = std::move(instance);
        }

        // Resolve Service, or nullptr if none registered (mirrors GetService).
        template <class Service> [[nodiscard]] std::shared_ptr<Service> get_service() const
        {
            const auto found = services_.find(type_tag::of<Service>());
            if (found == services_.end())
            {
                return nullptr;
            }
            return std::static_pointer_cast<Service>(found->second);
        }

        // Resolve Service, throwing if none registered (mirrors GetRequiredService).
        template <class Service> [[nodiscard]] std::shared_ptr<Service> get_required_service() const
        {
            auto service = get_service<Service>();
            if (!service)
            {
                throw std::runtime_error("service_registry: required service is not registered");
            }
            return service;
        }

        [[nodiscard]] bool contains(type_tag service_type) const
        {
            return services_.contains(service_type);
        }

    private:
        std::unordered_map<type_tag, std::shared_ptr<void>> services_;
    };
} // namespace maui::core
