#pragma once
// maui::core::i_maui_context  <=  Microsoft.Maui.IMauiContext
//
// The hosting context threaded into every handler (SetMauiContext): the service locator plus the
// handler factory. Ported from src/Core/src/IMauiContext.cs. Minimal for M1 — only the two accessors
// handlers actually store/forward are declared; platform-specific members (e.g. the Android Context)
// are added per-backend later. The registries are returned by reference, so forward declarations
// suffice here.

namespace maui::core
{
    class service_registry;
    class handler_registry;

    class i_maui_context
    {
    public:
        virtual ~i_maui_context() = default;

        [[nodiscard]] virtual service_registry& services() = 0;
        [[nodiscard]] virtual handler_registry& handlers() = 0;

    protected:
        i_maui_context() = default;
        i_maui_context(const i_maui_context&) = default;
        i_maui_context(i_maui_context&&) = default;
        i_maui_context& operator=(const i_maui_context&) = default;
        i_maui_context& operator=(i_maui_context&&) = default;
    };
} // namespace maui::core
