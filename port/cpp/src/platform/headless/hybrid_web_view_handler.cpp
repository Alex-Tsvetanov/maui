// hybrid_web_view_handler — headless platform recipe. A testable stand-in for the WKWebView +
// WKScriptMessageHandler bridge: create/connect wire the virtual view; the three commands build the SAME
// native→JS scripts the .mm evaluates (recorded in evaluated_scripts) and, for send_raw_message, also
// record the json-quoted message; an evaluate_java_script command completes through the script_responder
// answer seam. The JS→native direction is reached by calling message_received() directly (the test plays
// the page). The shared Apple .mm (src/platform/apple_shared/hybrid_web_view_handler.mm) is the
// real-native twin.

#include "maui/controls/hybrid_web_view_handler.hpp"

#include <any>
#include <cmath>
#include <memory>
#include <optional>
#include <string>
#include <utility>

#include "maui/core/evaluate_java_script_request.hpp"
#include "maui/core/hybrid_web_view_protocol.hpp"
#include "maui/core/i_hybrid_web_view.hpp"
#include "maui/core/invoke_java_script_request.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

namespace maui::controls
{
    // Headless has no native view in the `native` slot, so destruction is trivial.
    hybrid_web_view_platform::~hybrid_web_view_platform() = default;

    std::unique_ptr<hybrid_web_view_platform> hybrid_web_view_handler::create_platform_view()
    {
        // HybridWebViewHandler.CreatePlatformView fires the IInitializationAwareWebView hooks around
        // platform-view creation; the headless mirror fires them at the same points so the control's
        // lifecycle events are exercised without a real WKWebView (the apple_shared .mm is the twin).
        if (auto* view = virtual_view())
        {
            view->web_view_initialization_started();
        }
        auto platform = std::make_unique<hybrid_web_view_platform>();
        if (auto* view = virtual_view())
        {
            view->web_view_initialization_completed();
        }
        return platform;
    }

    void hybrid_web_view_handler::on_connect_handler(hybrid_web_view_platform& platform)
    {
        platform.connected_view = virtual_view();
    }

    void hybrid_web_view_handler::on_disconnect_handler(hybrid_web_view_platform& platform)
    {
        platform.connected_view = nullptr;
    }

    // HybridWebViewHandler.MapSendRawMessage → MauiHybridWebView.SendRawMessage: evaluate
    // window.external.receiveMessage(<json message>).
    void hybrid_web_view_handler::map_send_raw_message(hybrid_web_view_handler& handler,
                                                       maui::core::i_hybrid_web_view& /*view*/, const std::any& args)
    {
        auto* platform = handler.typed_platform_view();
        const auto* message = std::any_cast<std::string>(&args);
        if (platform == nullptr || message == nullptr)
        {
            return;
        }
        platform->sent_raw_messages.push_back(*message);
        platform->evaluated_scripts.push_back(maui::core::build_send_raw_message_script(*message));
    }

    // HybridWebViewHandler.MapInvokeJavaScriptAsync → HybridWebViewHelper.ProcessInvokeJavaScriptAsync:
    // mint a task id, evaluate window.HybridWebView.__InvokeJavaScript(...). The completion arrives later
    // through message_received (the "__InvokeJavaScriptCompleted" raw message).
    void hybrid_web_view_handler::map_invoke_java_script(hybrid_web_view_handler& handler,
                                                         maui::core::i_hybrid_web_view& /*view*/, const std::any& args)
    {
        const auto* request_ptr = std::any_cast<std::shared_ptr<maui::core::invoke_java_script_request>>(&args);
        if (request_ptr == nullptr || *request_ptr == nullptr)
        {
            return;
        }
        const std::shared_ptr<maui::core::invoke_java_script_request> request = *request_ptr;
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            // C# SetCanceled when the platform view is gone — the port surfaces it as a null result.
            request->complete(std::nullopt);
            return;
        }
        const std::string task_id = handler.create_invoke_task(request);
        platform->evaluated_scripts.push_back(
            maui::core::build_invoke_java_script_script(task_id, request->method_name(), request->param_values()));
    }

    // HybridWebViewHandler.MapEvaluateJavaScriptAsync: record the (already escaped+wrapped) script and
    // complete the request through the answer seam (unset => "null", the WKWebView value for an
    // errored/void script). The same pipeline as web_view's headless map_evaluate_java_script.
    void hybrid_web_view_handler::map_evaluate_java_script(hybrid_web_view_handler& handler,
                                                           maui::core::i_hybrid_web_view& /*view*/,
                                                           const std::any& args)
    {
        const auto* request = std::any_cast<std::shared_ptr<maui::core::evaluate_java_script_request>>(&args);
        if (request == nullptr || *request == nullptr)
        {
            return;
        }
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            (*request)->complete("null");
            return;
        }
        platform->evaluated_scripts.push_back((*request)->script());
        std::string result = platform->script_responder ? platform->script_responder((*request)->script()) : "null";
        (*request)->complete(std::move(result));
    }

    maui::graphics::size hybrid_web_view_handler::get_desired_size(double width_constraint,
                                                                   double height_constraint) const
    {
        // The web_view precedent: the platform measure (0 for the headless stand-in) falls back to
        // MinimumSize (44) per dimension when the constraint is unbounded or non-positive.
        double width = 0;
        double height = 0;
        if (width_constraint <= 0 || !std::isfinite(width_constraint))
        {
            width = minimum_size;
        }
        if (height_constraint <= 0 || !std::isfinite(height_constraint))
        {
            height = minimum_size;
        }
        return {width, height};
    }

    void hybrid_web_view_handler::platform_arrange(const maui::graphics::rect& /*frame*/)
    {
        // Headless: no native layout to apply.
    }
} // namespace maui::controls
