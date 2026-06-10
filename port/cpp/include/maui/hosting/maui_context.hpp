#pragma once
// maui::hosting::maui_context  <=  Microsoft.Maui.MauiContext
//
// The concrete i_maui_context the built maui_app threads into every handler it attaches
// (i_element_handler::set_maui_context): the service locator + handler factory pair, exactly the two
// accessors the Core contract declares. NON-owning — it references the registries the owning maui_app
// holds (C#'s MauiContext wraps the built IServiceProvider the same way); the maui_app outlives every
// handler it contexts (PROFILE §8 — handlers keep a raw i_maui_context* back into it).

#include "maui/core/i_maui_context.hpp"

namespace maui::core
{
    class handler_registry;
    class service_registry;
} // namespace maui::core

namespace maui::hosting
{
    class maui_context final : public maui::core::i_maui_context
    {
    public:
        maui_context(maui::core::service_registry& services, maui::core::handler_registry& handlers);

        [[nodiscard]] maui::core::service_registry& services() override;
        [[nodiscard]] maui::core::handler_registry& handlers() override;

    private:
        maui::core::service_registry* services_; // NON-owning (the maui_app owns both registries)
        maui::core::handler_registry* handlers_;
    };
} // namespace maui::hosting
