#pragma once
// maui::hosting::maui_handlers_collection  <=  Microsoft.Maui.Hosting.Internal.MauiHandlersCollection
//
// The concrete i_maui_handlers_collection: a thin write-through facade over a caller-owned
// maui::core::handler_registry (the builder's registry-under-construction). NON-owning — the registry
// outlives the facade, which only lives for the duration of the ConfigureMauiHandlers callbacks
// maui_app_builder::build() replays (HandlerMauiAppBuilderExtensions.HandlerServiceBuilder's role).

#include "maui/core/handler_registry.hpp"
#include "maui/core/type_tag.hpp"
#include "maui/hosting/i_maui_handlers_collection.hpp"

namespace maui::hosting
{
    class maui_handlers_collection final : public i_maui_handlers_collection
    {
    public:
        explicit maui_handlers_collection(maui::core::handler_registry& registry);

        // Keep the inherited template sugar visible next to the erased overrides below.
        using i_maui_handlers_collection::add_handler;
        using i_maui_handlers_collection::is_registered;
        using i_maui_handlers_collection::try_add_handler;

        void add_handler(maui::core::type_tag view_type, maui::core::handler_registry::factory factory) override;
        bool try_add_handler(maui::core::type_tag view_type, maui::core::handler_registry::factory factory) override;
        [[nodiscard]] bool is_registered(maui::core::type_tag view_type) const override;

    private:
        maui::core::handler_registry* registry_; // NON-owning (the builder owns the registry)
    };
} // namespace maui::hosting
