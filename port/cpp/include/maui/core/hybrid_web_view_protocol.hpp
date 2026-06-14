#pragma once
// maui::core::hybrid_web_view_protocol  <=  Microsoft.Maui.Handlers.HybridWebViewHelper.ProcessRawMessage
//                                            + HybridWebView.js
//
// The cross-platform message protocol both hybrid-web-view backends share. The injected JavaScript
// (HybridWebView.js) and the host exchange strings of the form `<type>|<content>`; this module ports the
// host-side parsing (HybridWebViewHelper.ProcessRawMessage) plus the small string builders the handler
// uses to drive the JS side. Pure (no platform types) so it is unit-testable and identical on every
// backend.
//
// Message types (HybridWebView.js):
//   "__RawMessage|<content>"                       — a raw message the page sent (sendRawMessage)
//   "__InvokeJavaScriptCompleted|<taskId>|<json>"  — an InvokeJavaScript call finished (result json)
//   "__InvokeJavaScriptFailed|<taskId>|<error>"    — an InvokeJavaScript call threw (JSInvokeError json)
//
// The native→JS channels the handler builds:
//   send_raw_message  → `window.external.receiveMessage(<json-quoted message>)`  (MauiHybridWebView.SendRawMessage)
//   invoke_java_script→ `window.HybridWebView.__InvokeJavaScript(<taskId>, <methodName>, [<params...>])`
//                        (HybridWebViewHelper.ProcessInvokeJavaScriptAsync)

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace maui::core
{
    // The parsed shape of an inbound "<type>|<content>" message.
    enum class hybrid_message_type : std::uint8_t
    {
        invalid = 0,      // empty, or no pipe, or an unrecognized type
        raw_message,      // "__RawMessage"
        invoke_completed, // "__InvokeJavaScriptCompleted"
        invoke_failed,    // "__InvokeJavaScriptFailed"
    };

    struct hybrid_parsed_message
    {
        hybrid_message_type type = hybrid_message_type::invalid;
        // For raw_message: the message content. For invoke_*: the task id (the content's first pipe field).
        std::string content_or_task_id;
        // For invoke_*: the result/error json (the content after the second pipe). Empty/absent otherwise.
        std::string invoke_result;
    };

    // HybridWebViewHelper.ProcessRawMessage: split "<type>|<content>"; for the invoke types, split the
    // content again into "<taskId>|<result>". An unrecognized / malformed message returns type==invalid
    // (C# throws ArgumentException; the port reports invalid so the caller can drop it without faulting
    // the seam — see hybrid_web_view_handler::message_received).
    [[nodiscard]] hybrid_parsed_message parse_hybrid_message(std::string_view raw_message);

    // The native→JS SendRawMessage script: `window.external.receiveMessage(<json-quoted message>)`.
    // MauiHybridWebView.SendRawMessage JSON-serializes the message string (so embedded quotes/newlines
    // are escaped); json_quote ports that single-string serialization.
    [[nodiscard]] std::string build_send_raw_message_script(std::string_view message);

    // The native→JS InvokeJavaScript script:
    // `window.HybridWebView.__InvokeJavaScript(<taskId>, <methodName>, [<param0>, <param1>, ...])`.
    // The param values are spliced verbatim (already JSON-encoded by the caller; a null param is "null").
    [[nodiscard]] std::string build_invoke_java_script_script(std::string_view task_id, std::string_view method_name,
                                                              const std::vector<std::string>& param_values);

    // Serialize a single string as a JSON string literal (quotes + the standard escapes), matching
    // System.Text.Json's default string encoding for the characters HybridWebView messages contain.
    [[nodiscard]] std::string json_quote(std::string_view value);
} // namespace maui::core
