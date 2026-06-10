// maui::hosting::maui_context — the registries pair handed to handlers (maui_context.hpp). Ported from
// src/Core/src/MauiContext.cs (the Services/Handlers accessor core; the platform-specific members are
// out of scope, i_maui_context.hpp).

#include "maui/hosting/maui_context.hpp"

namespace maui::hosting
{
    maui_context::maui_context(maui::core::service_registry& services, maui::core::handler_registry& handlers)
        : services_(&services), handlers_(&handlers)
    {
    }

    maui::core::service_registry& maui_context::services()
    {
        return *services_;
    }

    maui::core::handler_registry& maui_context::handlers()
    {
        return *handlers_;
    }
} // namespace maui::hosting
