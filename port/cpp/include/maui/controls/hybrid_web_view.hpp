#pragma once
// maui::controls::hybrid_web_view  <=  Microsoft.Maui.Controls.HybridWebView
//
// A view that presents local HTML content and lets JavaScript and host code communicate via raw messages
// and method invocation. Ported from src/Controls/src/Core/HybridWebView/HybridWebView.cs. Same API shape
// as the other controls: method accessors backed by private property<T> engines whose changes flow through
// view::on_property_changed to the handler; the reverse direction is the raw_message_received() the
// handler's platform bridge calls (it raises the public raw_message_received event).
//
//   - default_file / hybrid_root are bindable (DefaultFileProperty default "index.html",
//     HybridRootProperty default "wwwroot");
//   - send_raw_message(message) dispatches the "send_raw_message" handler command
//     (Handler?.Invoke(nameof(SendRawMessage), new HybridWebViewRawMessage{Message=…}));
//   - invoke_js(method_name, params, on_result) ports InvokeJavaScriptAsync as the callback idiom (per the
//     port's async model — see web_view::eval_js / evaluate_java_script_request): the params are the
//     already-JSON-encoded argument strings (C# JSON-serializes each value; C++23 has no reflection-driven
//     System.Text.Json, so the caller owns serialization) and on_result receives the RAW json result
//     (nullopt for a null/undefined/void/failed call);
//   - evaluate_js(script, on_result) ports EvaluateJavaScriptAsync — the identical escape/wrap/unquote
//     pipeline web_view::eval_js uses, routed through the "evaluate_java_script" command;
//   - raw_message_received raises HybridWebView.RawMessageReceived; web_view_initializing /
//     web_view_initialized / web_resource_requested raise the IInitializationAwareWebView /
//     IWebRequestInterceptingWebView events.
//
// OUT OF SCOPE (documented, not stubbed — PROFILE §6 no-reflection): SetInvokeJavaScriptTarget +
// InvokeDotNet (JavaScript → .NET method invocation by reflection); the typed InvokeJavaScriptAsync<T>
// JSON deserialization (the port returns the raw json — the caller deserializes). The custom-config /
// custom-response payloads of the initializing / web-resource-requested args (WKWebViewConfiguration /
// the platform request/response) are not exposed — the events fire at the faithful points.

#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "maui/controls/hybrid_web_view_raw_message_received_event_args.hpp"
#include "maui/controls/view.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/event.hpp"
#include "maui/core/i_hybrid_web_view.hpp"
#include "maui/core/move_only_function.hpp"
#include "maui/core/property.hpp"

namespace maui::controls
{
    class hybrid_web_view : public view<maui::core::i_hybrid_web_view>
    {
    public:
        // The InvokeJavaScript / EvaluateJavaScript completion: nullopt when the call yielded
        // null/undefined/void or failed, otherwise the RAW json (invoke) / unquoted (evaluate) result.
        using js_result_callback = maui::core::move_only_function<void(std::optional<std::string> result)>;

        // Declare the style TargetType so an implicit / class style targeting `hybrid_web_view` matches.
        hybrid_web_view()
        {
            this->set_style_target_type<hybrid_web_view>();
        }

        // Shared bindable-property descriptors (HybridWebView.DefaultFileProperty / HybridRootProperty).
        static const maui::core::bindable_property<std::string>& default_file_property();
        static const maui::core::bindable_property<std::string>& hybrid_root_property();

        // ---- i_hybrid_web_view ----
        [[nodiscard]] std::string default_file() const override
        {
            return default_file_.get();
        }
        [[nodiscard]] std::string hybrid_root() const override
        {
            return hybrid_root_.get();
        }
        // C# IHybridWebView.RawMessageReceived: the handler's bridge calls this when a raw message arrives;
        // raise the public event.
        void raw_message_received(std::string_view raw_message) override;

        // ---- i_initialization_aware_web_view (the handler calls these around platform-view creation) ----
        void web_view_initialization_started() override;
        void web_view_initialization_completed() override;

        // ---- i_web_request_intercepting_web_view (the handler's scheme handler calls this) ----
        bool web_resource_requested(std::string_view url) override;

        // ---- the developer-facing config surface ----
        void set_default_file(std::string value)
        {
            default_file_.set(std::move(value));
        }
        void set_hybrid_root(std::string value)
        {
            hybrid_root_.set(std::move(value));
        }

        // ---- messaging + scripting (handler commands) ----
        // C# HybridWebView.SendRawMessage: send a raw string to the page (no processing).
        void send_raw_message(std::string_view raw_message);

        // C# HybridWebView.InvokeJavaScriptAsync (callback form): invoke the global JS function
        // `method_name` with the already-JSON-encoded `param_values`; on_result gets the raw json result
        // (nullopt for null/undefined/void/failed). An empty method_name is a no-op (C# throws
        // ArgumentException; the port reports nullopt to the callback instead — the callback channel has no
        // faulted state).
        void invoke_js(std::string_view method_name, std::vector<std::string> param_values,
                       js_result_callback on_result);
        // Convenience overload for a parameterless invocation.
        void invoke_js(std::string_view method_name, js_result_callback on_result);

        // C# HybridWebView.EvaluateJavaScriptAsync (callback form): run arbitrary script and return its
        // value (the web_view::eval_js escape/wrap/unquote pipeline).
        void evaluate_js(std::string_view script, js_result_callback on_result);

        // ---- developer-facing events ----
        // C# HybridWebView.RawMessageReceived.
        maui::core::event<const hybrid_web_view_raw_message_received_event_args&> raw_message_received_event;
        // C# HybridWebView.WebViewInitializing / WebViewInitialized.
        maui::core::event<> web_view_initializing;
        maui::core::event<> web_view_initialized;
        // C# HybridWebView.WebResourceRequested. The args are mutable shared state — a subscriber sets
        // `handled` so the platform stops its own resolution (WebViewWebResourceRequestedEventArgs.Handled).
        maui::core::event<std::string_view, bool&> web_resource_requested_event;

    private:
        maui::core::property<std::string> default_file_{*this, default_file_property()};
        maui::core::property<std::string> hybrid_root_{*this, hybrid_root_property()};
    };
} // namespace maui::controls
