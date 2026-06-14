// maui::controls::hybrid_web_view — out-of-line definitions: the shared bindable-property descriptors,
// the inbound raw_message_received → public event, the initialization / web-resource event hooks, the
// messaging/scripting command dispatch, and the default-handler self-registration. See hybrid_web_view.hpp.

#include "maui/controls/hybrid_web_view.hpp"

#include <cstddef>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "maui/controls/hybrid_web_view_handler.hpp"
#include "maui/controls/hybrid_web_view_raw_message_received_event_args.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/evaluate_java_script_request.hpp"
#include "maui/core/event.hpp"
#include "maui/core/handler_registry.hpp"
#include "maui/core/invoke_java_script_request.hpp"
#include "maui/core/web_view_helper.hpp"

namespace maui::controls
{
    const maui::core::bindable_property<std::string>& hybrid_web_view::default_file_property()
    {
        // HybridWebView.DefaultFileProperty: default "index.html".
        static const maui::core::bindable_property<std::string> descriptor{"default_file", std::string{"index.html"}};
        return descriptor;
    }

    const maui::core::bindable_property<std::string>& hybrid_web_view::hybrid_root_property()
    {
        // HybridWebView.HybridRootProperty: default "wwwroot".
        static const maui::core::bindable_property<std::string> descriptor{"hybrid_root", std::string{"wwwroot"}};
        return descriptor;
    }

    void hybrid_web_view::raw_message_received(std::string_view raw_message)
    {
        // C# HybridWebView.RawMessageReceived: raise with new HybridWebViewRawMessageReceivedEventArgs(msg).
        hybrid_web_view_raw_message_received_event_args args;
        args.message = std::string(raw_message);
        raw_message_received_event.raise(args);
    }

    void hybrid_web_view::web_view_initialization_started()
    {
        // C# IInitializationAwareWebView.WebViewInitializationStarted → raise WebViewInitializing.
        web_view_initializing.raise();
    }

    void hybrid_web_view::web_view_initialization_completed()
    {
        // C# IInitializationAwareWebView.WebViewInitializationCompleted → raise WebViewInitialized.
        web_view_initialized.raise();
    }

    bool hybrid_web_view::web_resource_requested(std::string_view url)
    {
        // C# IWebRequestInterceptingWebView.WebResourceRequested: raise WebResourceRequested, then return
        // the args' Handled flag (a subscriber sets it to stop the platform's own resolution).
        bool handled = false;
        web_resource_requested_event.raise(url, handled);
        return handled;
    }

    void hybrid_web_view::send_raw_message(std::string_view raw_message)
    {
        // C# HybridWebView.SendRawMessage: Handler?.Invoke(nameof(SendRawMessage),
        // new HybridWebViewRawMessage { Message = rawMessage }).
        if (const auto& element_handler = handler())
        {
            element_handler->invoke("send_raw_message", std::string(raw_message));
        }
    }

    void hybrid_web_view::invoke_js(std::string_view method_name, std::vector<std::string> param_values,
                                    js_result_callback on_result)
    {
        if (!on_result)
        {
            return;
        }
        if (method_name.empty())
        {
            // C# InvokeJavaScriptAsync throws ArgumentException for an empty method name; the port has no
            // faulted callback channel, so it reports a null result instead.
            on_result(std::nullopt);
            return;
        }
        auto request = std::make_shared<maui::core::invoke_java_script_request>(
            std::string(method_name), std::move(param_values),
            [callback = std::move(on_result)](std::optional<std::string> result, bool failed) mutable {
                // A failed call (the "__InvokeJavaScriptFailed" channel) is reported as a null result —
                // the callback channel cannot carry the exception (the documented deviation). The detail
                // json carried by `result` on failure is dropped; only a successful result is delivered.
                callback(failed ? std::nullopt : std::move(result));
            });

        if (const auto& element_handler = handler())
        {
            element_handler->invoke("invoke_java_script", std::move(request));
            return;
        }
        // DEVIATION: without a handler C# would fault the awaited task (Handler is null); the port's
        // callback channel completes with the null result instead.
        request->complete(std::nullopt);
    }

    void hybrid_web_view::invoke_js(std::string_view method_name, js_result_callback on_result)
    {
        invoke_js(method_name, std::vector<std::string>{}, std::move(on_result));
    }

    void hybrid_web_view::evaluate_js(std::string_view script, js_result_callback on_result)
    {
        if (!on_result)
        {
            return;
        }
        // C# EvaluateJavaScriptAsync: escape the script, then wrap it so the platform returns a
        // JSON.stringify'd value (or 'null' when it errors) — the web_view::eval_js pipeline.
        std::string wrapped = "try{JSON.stringify(eval('";
        wrapped += maui::core::escape_js_string(script);
        wrapped += "'))}catch(e){'null'};";

        auto request = std::make_shared<maui::core::evaluate_java_script_request>(
            std::move(wrapped), [callback = std::move(on_result)](std::string result) mutable {
                // C#: a "null" result (errored/undefined script) is a null return; any other result has
                // the JSON.stringify quotes trimmed (Trim('"')).
                if (result == "null")
                {
                    callback(std::nullopt);
                    return;
                }
                std::size_t begin = 0;
                std::size_t end = result.size();
                while (begin < end && result[begin] == '"')
                {
                    ++begin;
                }
                while (end > begin && result[end - 1] == '"')
                {
                    --end;
                }
                callback(result.substr(begin, end - begin));
            });

        if (const auto& element_handler = handler())
        {
            element_handler->invoke("evaluate_java_script", request);
            return;
        }
        // DEVIATION (web_view precedent): without a handler the callback completes with the null result.
        request->complete("null");
    }
} // namespace maui::controls

// Opt-in self-registration: hybrid_web_view resolves to hybrid_web_view_handler in the default registry.
MAUI_REGISTER_HANDLER(maui::controls::hybrid_web_view, maui::controls::hybrid_web_view_handler)
