// maui::controls binding diagnostics — the process-wide binding-failure sink (binding_diagnostics.hpp).
#include "maui/controls/bindings/binding_diagnostics.hpp"

#include <string>
#include <utility>

namespace maui::controls
{
    namespace
    {
        binding_failure_handler& handler_slot()
        {
            static binding_failure_handler handler;
            return handler;
        }
    } // namespace

    void set_binding_failure_handler(binding_failure_handler handler)
    {
        handler_slot() = std::move(handler);
    }

    void send_binding_failure(const std::string& message)
    {
        if (const binding_failure_handler& handler = handler_slot())
        {
            handler(message);
        }
    }
} // namespace maui::controls
