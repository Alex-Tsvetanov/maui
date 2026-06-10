#pragma once
// maui::controls binding diagnostics  <=  Microsoft.Maui.Controls.Xaml.Diagnostics.BindingDiagnostics
//                                         (+ the Application logger BindingExpression failures land in)
//
// The binding-failure sink: every "property not found" / "cannot convert" the engine hits is reported
// here instead of throwing (binding failures are diagnostics in MAUI, never exceptions). The default
// handler is a no-op; tests (and apps) install one to observe failures — the port's stand-in for the
// C# unit tests' MockApplication.MockLogger assertions.

#include <functional>
#include <string>

namespace maui::controls
{
    using binding_failure_handler = std::function<void(const std::string& message)>;

    // Install (or clear, with an empty function) the process-wide failure handler.
    void set_binding_failure_handler(binding_failure_handler handler);

    // Report a binding failure to the installed handler (a no-op when none is installed).
    void send_binding_failure(const std::string& message);
} // namespace maui::controls
