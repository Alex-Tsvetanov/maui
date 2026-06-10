// maui::hosting::maui_handlers_collection — the write-through facade over the builder's
// handler_registry (maui_handlers_collection.hpp). Ported from Hosting/Internal/MauiHandlersCollection.cs
// + the AddHandler/TryAddHandler extension behavior (MauiHandlersCollectionExtensions.cs).

#include "maui/hosting/maui_handlers_collection.hpp"

#include <utility>

#include "maui/core/handler_registry.hpp"
#include "maui/core/type_tag.hpp"

namespace maui::hosting
{
    maui_handlers_collection::maui_handlers_collection(maui::core::handler_registry& registry) : registry_(&registry)
    {
    }

    void maui_handlers_collection::add_handler(maui::core::type_tag view_type,
                                               maui::core::handler_registry::factory factory)
    {
        registry_->register_factory(view_type, std::move(factory));
    }

    bool maui_handlers_collection::try_add_handler(maui::core::type_tag view_type,
                                                   maui::core::handler_registry::factory factory)
    {
        if (registry_->is_registered(view_type))
        {
            return false; // TryAddTransient: keep the existing registration
        }
        registry_->register_factory(view_type, std::move(factory));
        return true;
    }

    bool maui_handlers_collection::is_registered(maui::core::type_tag view_type) const
    {
        return registry_->is_registered(view_type);
    }
} // namespace maui::hosting
