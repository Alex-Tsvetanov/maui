// Tests for the hybrid_web_view control + its headless handler seam (G5). The control half characterizes
// HybridWebView.cs (DefaultFile/HybridRoot defaults, the RawMessageReceived event, SendRawMessage /
// InvokeJavaScriptAsync / EvaluateJavaScriptAsync command dispatch, the initialization + web-resource
// hooks); the seam half mirrors the WKWebView + WKScriptMessageHandler bridge the headless partial
// records synchronously (the native→JS scripts built for send_raw_message / invoke_java_script, the
// JS→native protocol routing through message_received, and the invoke-task completion round trip). The
// shared Apple .mm is the real-native twin verified on the ios simulator. The protocol parsing/building
// half ports HybridWebViewHelper.ProcessRawMessage + HybridWebView.js.
#include "maui/controls/hybrid_web_view.hpp"

#include <limits>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "maui/controls/hybrid_web_view_handler.hpp"
#include "maui/controls/hybrid_web_view_raw_message_received_event_args.hpp"
#include "maui/core/hybrid_web_view_protocol.hpp"
#include "maui/core/invoke_java_script_request.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::controls::hybrid_web_view;
    using maui::controls::hybrid_web_view_handler;
    using maui::controls::hybrid_web_view_raw_message_received_event_args;
    using maui::core::hybrid_message_type;

    // ---- the control in isolation (HybridWebView.cs) ----

    TEST(hybrid_web_view, default_file_and_root_defaults)
    {
        const hybrid_web_view control;
        EXPECT_EQ(control.default_file(), "index.html"); // DefaultFileProperty default
        EXPECT_EQ(control.hybrid_root(), "wwwroot");     // HybridRootProperty default
    }

    TEST(hybrid_web_view, default_file_and_root_are_settable)
    {
        hybrid_web_view control;
        control.set_default_file("main.html");
        control.set_hybrid_root("hybrid_root");
        EXPECT_EQ(control.default_file(), "main.html");
        EXPECT_EQ(control.hybrid_root(), "hybrid_root");
    }

    // RawMessageReceived raises the event with the message content.
    TEST(hybrid_web_view, raw_message_received_raises_event)
    {
        hybrid_web_view control;
        std::optional<std::string> seen;
        control.raw_message_received_event.connect(
            [&seen](const hybrid_web_view_raw_message_received_event_args& args) { seen = args.message; });
        control.raw_message_received("Hello from JS");
        ASSERT_TRUE(seen.has_value());
        EXPECT_EQ(seen.value_or(""), "Hello from JS");
    }

    TEST(hybrid_web_view, initialization_hooks_raise_events)
    {
        hybrid_web_view control;
        bool initializing = false;
        bool initialized = false;
        control.web_view_initializing.connect([&initializing] { initializing = true; });
        control.web_view_initialized.connect([&initialized] { initialized = true; });
        control.web_view_initialization_started();
        control.web_view_initialization_completed();
        EXPECT_TRUE(initializing);
        EXPECT_TRUE(initialized);
    }

    // WebResourceRequested raises the event and returns the subscriber-set handled flag.
    TEST(hybrid_web_view, web_resource_requested_returns_handled_flag)
    {
        hybrid_web_view control;
        std::string requested_url;
        control.web_resource_requested_event.connect([&requested_url](std::string_view url, bool& handled) {
            requested_url = std::string(url);
            handled = true;
        });
        EXPECT_TRUE(control.web_resource_requested("app://0.0.0.1/index.html"));
        EXPECT_EQ(requested_url, "app://0.0.0.1/index.html");
    }

    TEST(hybrid_web_view, web_resource_requested_unhandled_when_no_subscriber)
    {
        hybrid_web_view control;
        EXPECT_FALSE(control.web_resource_requested("app://0.0.0.1/missing.html"));
    }

    // Without a handler, invoke_js / evaluate_js complete the callback with nullopt (no faulted channel).
    TEST(hybrid_web_view, invoke_js_without_handler_completes_with_nullopt)
    {
        hybrid_web_view control;
        bool completed = false;
        std::optional<std::string> result{"sentinel"};
        control.invoke_js("DoThing", [&](std::optional<std::string> value) {
            completed = true;
            result = std::move(value);
        });
        EXPECT_TRUE(completed);
        EXPECT_FALSE(result.has_value());
    }

    TEST(hybrid_web_view, invoke_js_empty_method_name_is_nullopt)
    {
        hybrid_web_view control;
        std::optional<std::string> result{"sentinel"};
        control.invoke_js("", [&](std::optional<std::string> value) { result = std::move(value); });
        EXPECT_FALSE(result.has_value());
    }

    // ---- the handler seam (headless mirror of the WKWebView bridge) ----

    struct seam
    {
        hybrid_web_view control;
        std::shared_ptr<hybrid_web_view_handler> handler = std::make_shared<hybrid_web_view_handler>();
        maui::controls::hybrid_web_view_platform* platform = connect(control, handler);

        [[nodiscard]] static maui::controls::hybrid_web_view_platform* connect(
            hybrid_web_view& view, const std::shared_ptr<hybrid_web_view_handler>& target)
        {
            view.set_handler(target);
            return target->typed_platform_view();
        }
    };

    // send_raw_message builds the window.external.receiveMessage(<json>) script (native→JS).
    TEST(hybrid_web_view_handler_seam, send_raw_message_builds_receive_message_script)
    {
        seam s;
        s.control.send_raw_message("Hi there");
        ASSERT_EQ(s.platform->sent_raw_messages.size(), 1U);
        EXPECT_EQ(s.platform->sent_raw_messages[0], "Hi there");
        ASSERT_EQ(s.platform->evaluated_scripts.size(), 1U);
        EXPECT_EQ(s.platform->evaluated_scripts[0], R"(window.external.receiveMessage("Hi there"))");
    }

    // The send-message script JSON-quotes special chars (the C# JsonSerializer.Serialize of the string).
    TEST(hybrid_web_view_handler_seam, send_raw_message_json_quotes_special_chars)
    {
        seam s;
        s.control.send_raw_message(R"(Hybrid""'' {Test} with chars!)"); // the C# device-test message
        ASSERT_EQ(s.platform->evaluated_scripts.size(), 1U);
        EXPECT_EQ(s.platform->evaluated_scripts[0],
                  R"(window.external.receiveMessage("Hybrid\"\"'' {Test} with chars!"))");
    }

    // A "__RawMessage|..." script message reaches the control's RawMessageReceived (JS→native).
    TEST(hybrid_web_view_handler_seam, inbound_raw_message_reaches_control)
    {
        seam s;
        std::optional<std::string> seen;
        s.control.raw_message_received_event.connect(
            [&seen](const hybrid_web_view_raw_message_received_event_args& args) { seen = args.message; });

        s.handler->message_received("__RawMessage|You said: hello");

        ASSERT_TRUE(seen.has_value());
        EXPECT_EQ(seen.value_or(""), "You said: hello");
        ASSERT_EQ(s.platform->received_raw_messages.size(), 1U);
        EXPECT_EQ(s.platform->received_raw_messages[0], "You said: hello");
    }

    // The full SendRawMessage → page echoes → RawMessageReceived round trip (the C#
    // LoadsHtmlAndSendReceiveRawMessage device test, mirrored headlessly).
    TEST(hybrid_web_view_handler_seam, raw_message_round_trip_recording)
    {
        seam s;
        std::optional<std::string> last_received;
        s.control.raw_message_received_event.connect(
            [&last_received](const hybrid_web_view_raw_message_received_event_args& args) {
                last_received = args.message;
            });

        const std::string sent = "Hybrid Test";
        s.control.send_raw_message(sent);
        // The page "echoes" by posting a __RawMessage back through the bridge.
        s.handler->message_received("__RawMessage|You said: " + sent);

        ASSERT_TRUE(last_received.has_value());
        EXPECT_EQ(last_received.value_or(""), "You said: Hybrid Test");
    }

    // An invalid / unrecognized inbound message is dropped (no event, no fault).
    TEST(hybrid_web_view_handler_seam, invalid_inbound_message_is_dropped)
    {
        seam s;
        int received = 0;
        s.control.raw_message_received_event.connect(
            [&received](const hybrid_web_view_raw_message_received_event_args& /*args*/) { ++received; });
        s.handler->message_received("");                  // empty
        s.handler->message_received("no pipe here");      // no pipe
        s.handler->message_received("__Unknown|payload"); // unknown type
        EXPECT_EQ(received, 0);
    }

    // invoke_js builds the __InvokeJavaScript(taskId, method, [params]) script and completes when the
    // matching __InvokeJavaScriptCompleted message arrives.
    TEST(hybrid_web_view_handler_seam, invoke_js_round_trip)
    {
        seam s;
        bool completed = false;
        std::optional<std::string> result;
        s.control.invoke_js("EchoParameter", {"123"}, [&](std::optional<std::string> value) {
            completed = true;
            result = std::move(value);
        });

        ASSERT_EQ(s.platform->evaluated_scripts.size(), 1U);
        EXPECT_EQ(s.platform->evaluated_scripts[0], "window.HybridWebView.__InvokeJavaScript(1, EchoParameter, [123])");
        ASSERT_EQ(s.platform->pending_invokes.size(), 1U);
        EXPECT_FALSE(completed);

        // The page replies with the completion message for task id 1.
        s.handler->message_received("__InvokeJavaScriptCompleted|1|123");

        EXPECT_TRUE(completed);
        ASSERT_TRUE(result.has_value());
        EXPECT_EQ(result.value_or(""), "123");
        EXPECT_TRUE(s.platform->pending_invokes.empty());
    }

    TEST(hybrid_web_view_handler_seam, invoke_js_no_params_builds_empty_args)
    {
        seam s;
        s.control.invoke_js("DoThing", [](const std::optional<std::string>& /*value*/) {});
        ASSERT_EQ(s.platform->evaluated_scripts.size(), 1U);
        EXPECT_EQ(s.platform->evaluated_scripts[0], "window.HybridWebView.__InvokeJavaScript(1, DoThing, [])");
    }

    TEST(hybrid_web_view_handler_seam, invoke_js_multiple_params_are_comma_joined)
    {
        seam s;
        s.control.invoke_js("AddNumbers", {"123.456", "654.321"}, [](const std::optional<std::string>& /*value*/) {});
        ASSERT_EQ(s.platform->evaluated_scripts.size(), 1U);
        EXPECT_EQ(s.platform->evaluated_scripts[0],
                  "window.HybridWebView.__InvokeJavaScript(1, AddNumbers, [123.456, 654.321])");
    }

    // A null/undefined/void invoke result is nullopt (C#'s null Task result).
    TEST(hybrid_web_view_handler_seam, invoke_js_null_result_is_nullopt)
    {
        seam s;
        std::optional<std::string> result{"sentinel"};
        bool completed = false;
        s.control.invoke_js("VoidMethod", [&](std::optional<std::string> value) {
            completed = true;
            result = std::move(value);
        });
        s.handler->message_received("__InvokeJavaScriptCompleted|1|null");
        EXPECT_TRUE(completed);
        EXPECT_FALSE(result.has_value());
    }

    // A failed invoke (the __InvokeJavaScriptFailed channel) completes with nullopt.
    TEST(hybrid_web_view_handler_seam, invoke_js_failure_completes_with_nullopt)
    {
        seam s;
        std::optional<std::string> result{"sentinel"};
        bool completed = false;
        s.control.invoke_js("Throws", [&](std::optional<std::string> value) {
            completed = true;
            result = std::move(value);
        });
        s.handler->message_received(R"(__InvokeJavaScriptFailed|1|{"Message":"boom"})");
        EXPECT_TRUE(completed);
        EXPECT_FALSE(result.has_value());
    }

    // Two concurrent invocations get distinct task ids and complete independently.
    TEST(hybrid_web_view_handler_seam, concurrent_invokes_get_distinct_task_ids)
    {
        seam s;
        std::optional<std::string> first;
        std::optional<std::string> second;
        s.control.invoke_js("First", [&](std::optional<std::string> v) { first = std::move(v); });
        s.control.invoke_js("Second", [&](std::optional<std::string> v) { second = std::move(v); });
        ASSERT_EQ(s.platform->pending_invokes.size(), 2U);
        EXPECT_EQ(s.platform->evaluated_scripts[0], "window.HybridWebView.__InvokeJavaScript(1, First, [])");
        EXPECT_EQ(s.platform->evaluated_scripts[1], "window.HybridWebView.__InvokeJavaScript(2, Second, [])");

        s.handler->message_received("__InvokeJavaScriptCompleted|2|\"b\"");
        EXPECT_FALSE(first.has_value());
        ASSERT_TRUE(second.has_value());
        EXPECT_EQ(second.value_or(""), "\"b\"");

        s.handler->message_received("__InvokeJavaScriptCompleted|1|\"a\"");
        ASSERT_TRUE(first.has_value());
        EXPECT_EQ(first.value_or(""), "\"a\"");
    }

    // A completion for an unknown task id is silently dropped.
    TEST(hybrid_web_view_handler_seam, unknown_task_completion_is_dropped)
    {
        seam s;
        s.control.invoke_js("Method", [](const std::optional<std::string>& /*v*/) {});
        s.handler->message_received("__InvokeJavaScriptCompleted|999|\"x\""); // wrong id
        EXPECT_EQ(s.platform->pending_invokes.size(), 1U);                    // still pending
    }

    // ---- evaluate_js (the web_view escape/wrap/unquote pipeline) ----

    TEST(hybrid_web_view_handler_seam, evaluate_js_round_trip_delivers_answer)
    {
        seam s;
        s.platform->script_responder = [](const std::string& /*script*/) { return std::string("2"); };
        std::optional<std::string> result;
        s.control.evaluate_js("1+1", [&](std::optional<std::string> value) { result = std::move(value); });
        ASSERT_TRUE(result.has_value());
        EXPECT_EQ(result.value_or(""), "2");
        ASSERT_EQ(s.platform->evaluated_scripts.size(), 1U);
        EXPECT_EQ(s.platform->evaluated_scripts[0], "try{JSON.stringify(eval('1+1'))}catch(e){'null'};");
    }

    TEST(hybrid_web_view_handler_seam, evaluate_js_trims_json_stringify_quotes)
    {
        seam s;
        s.platform->script_responder = [](const std::string& /*script*/) { return std::string("\"Test\""); };
        std::optional<std::string> result;
        s.control.evaluate_js("test();", [&](std::optional<std::string> value) { result = std::move(value); });
        ASSERT_TRUE(result.has_value());
        EXPECT_EQ(result.value_or(""), "Test");
    }

    TEST(hybrid_web_view_handler_seam, evaluate_js_null_result_is_nullopt)
    {
        seam s; // script_responder unset => "null"
        std::optional<std::string> result{"sentinel"};
        s.control.evaluate_js("does.not.exist()", [&](std::optional<std::string> value) { result = std::move(value); });
        EXPECT_FALSE(result.has_value());
    }

    // ---- the protocol helpers ----

    TEST(hybrid_web_view_protocol, parse_raw_message)
    {
        const auto parsed = maui::core::parse_hybrid_message("__RawMessage|payload|with|pipes");
        EXPECT_EQ(parsed.type, hybrid_message_type::raw_message);
        // Only the first pipe splits the type; the rest is content (substring after the first pipe).
        EXPECT_EQ(parsed.content_or_task_id, "payload|with|pipes");
    }

    TEST(hybrid_web_view_protocol, parse_invoke_completed)
    {
        const auto parsed = maui::core::parse_hybrid_message("__InvokeJavaScriptCompleted|7|{\"x\":1}");
        EXPECT_EQ(parsed.type, hybrid_message_type::invoke_completed);
        EXPECT_EQ(parsed.content_or_task_id, "7");
        EXPECT_EQ(parsed.invoke_result, "{\"x\":1}");
    }

    TEST(hybrid_web_view_protocol, parse_invoke_failed)
    {
        const auto parsed = maui::core::parse_hybrid_message("__InvokeJavaScriptFailed|3|err");
        EXPECT_EQ(parsed.type, hybrid_message_type::invoke_failed);
        EXPECT_EQ(parsed.content_or_task_id, "3");
        EXPECT_EQ(parsed.invoke_result, "err");
    }

    TEST(hybrid_web_view_protocol, parse_invalid_messages)
    {
        EXPECT_EQ(maui::core::parse_hybrid_message("").type, hybrid_message_type::invalid);
        EXPECT_EQ(maui::core::parse_hybrid_message("no pipe").type, hybrid_message_type::invalid);
        EXPECT_EQ(maui::core::parse_hybrid_message("__Unknown|x").type, hybrid_message_type::invalid);
        // An invoke message missing the inner pipe is invalid.
        EXPECT_EQ(maui::core::parse_hybrid_message("__InvokeJavaScriptCompleted|nopipe").type,
                  hybrid_message_type::invalid);
    }

    TEST(hybrid_web_view_protocol, json_quote_escapes)
    {
        EXPECT_EQ(maui::core::json_quote("plain"), "\"plain\"");
        EXPECT_EQ(maui::core::json_quote("a\"b"), "\"a\\\"b\"");
        EXPECT_EQ(maui::core::json_quote("a\\b"), "\"a\\\\b\"");
        EXPECT_EQ(maui::core::json_quote("line\nbreak"), "\"line\\nbreak\"");
        EXPECT_EQ(maui::core::json_quote(std::string_view("\x01", 1)), "\"\\u0001\"");
    }

    TEST(hybrid_web_view_protocol, build_send_raw_message_script)
    {
        EXPECT_EQ(maui::core::build_send_raw_message_script("hi"), R"(window.external.receiveMessage("hi"))");
    }

    TEST(hybrid_web_view_protocol, build_invoke_java_script_script)
    {
        EXPECT_EQ(maui::core::build_invoke_java_script_script("5", "Method", {"1", "\"two\""}),
                  R"(window.HybridWebView.__InvokeJavaScript(5, Method, [1, "two"]))");
        EXPECT_EQ(maui::core::build_invoke_java_script_script("5", "NoArgs", {}),
                  "window.HybridWebView.__InvokeJavaScript(5, NoArgs, [])");
    }

    TEST(invoke_java_script_request, completes_exactly_once)
    {
        int calls = 0;
        std::optional<std::string> seen;
        bool seen_failed = true;
        maui::core::invoke_java_script_request request{
            "Method", {"1"}, [&](std::optional<std::string> result, bool failed) {
                ++calls;
                seen = std::move(result);
                seen_failed = failed;
            }};
        EXPECT_EQ(request.method_name(), "Method");
        ASSERT_EQ(request.param_values().size(), 1U);
        EXPECT_EQ(request.param_values()[0], "1");
        EXPECT_FALSE(request.completed());
        request.complete("first");
        request.complete("second");
        request.fail("late");
        EXPECT_EQ(calls, 1);
        ASSERT_TRUE(seen.has_value());
        EXPECT_EQ(seen.value_or(""), "first");
        EXPECT_FALSE(seen_failed);
        EXPECT_TRUE(request.completed());
    }

    // ---- sizing ----

    TEST(hybrid_web_view_handler_seam, desired_size_falls_back_to_minimum_under_unbounded_constraints)
    {
        const seam s;
        const auto size = s.handler->get_desired_size(std::numeric_limits<double>::infinity(),
                                                      std::numeric_limits<double>::infinity());
        EXPECT_DOUBLE_EQ(size.width, hybrid_web_view_handler::minimum_size);
        EXPECT_DOUBLE_EQ(size.height, hybrid_web_view_handler::minimum_size);
    }
} // namespace
