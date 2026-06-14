// iOS (UIKit, on-simulator) backend tests for the hybrid_web_view bridge (G5) — the real JS↔native round
// trip over the SHARED WKWebView + WKScriptMessageHandler .mm partial. The handler injects the bridge JS
// (window.HybridWebView / window.external) at document start, so a STATIC page (no network) can use it
// immediately. The run loop is pumped until the page loads, then:
//   - a JS handler that echoes a raw message back is registered; send_raw_message → the page posts a
//     "__RawMessage" back through window.HybridWebView.SendRawMessage → the control's
//     raw_message_received event fires (JS→native);
//   - invoke_js calls a global JS function and asserts the returned value arrives through the
//     "__InvokeJavaScriptCompleted" channel (native→JS→native);
//   - evaluate_js("1+1") returns "2" through -evaluateJavaScript:completionHandler:.
// Deadlines are GENEROUS (the WebContent process spawn dominates the first load) and every wait has a
// positive completion condition. Compiled as Objective-C++ with ARC for the `ios` backend; run ON the
// booted simulator via tools/ios-sim-run.sh. tests/controls/hybrid_web_view_apple_tests.mm is the AppKit
// twin.
#import <UIKit/UIKit.h>
#import <WebKit/WebKit.h>

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "maui/controls/hybrid_web_view.hpp"
#include "maui/controls/hybrid_web_view_handler.hpp"
#include "maui/controls/hybrid_web_view_raw_message_received_event_args.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::controls::hybrid_web_view;
    using maui::controls::hybrid_web_view_handler;
    using maui::controls::hybrid_web_view_raw_message_received_event_args;

    constexpr NSTimeInterval k_deadline_seconds = 30.0;

    template <typename Predicate> bool pump_until(Predicate done)
    {
        NSDate* const deadline = [NSDate dateWithTimeIntervalSinceNow:k_deadline_seconds];
        while (!done() && deadline.timeIntervalSinceNow > 0)
        {
            [[NSRunLoop currentRunLoop] runMode:NSDefaultRunLoopMode
                                     beforeDate:[NSDate dateWithTimeIntervalSinceNow:0.05]];
        }
        return done();
    }

    WKWebView* native_web_view(const std::shared_ptr<hybrid_web_view_handler>& handler)
    {
        return (__bridge WKWebView*)handler->typed_platform_view()->native;
    }

    struct seam
    {
        hybrid_web_view control;
        std::shared_ptr<hybrid_web_view_handler> handler = std::make_shared<hybrid_web_view_handler>();
        std::vector<std::string> received;

        seam()
        {
            control.raw_message_received_event.connect(
                [this](const hybrid_web_view_raw_message_received_event_args& args) {
                    received.push_back(args.message.value_or(""));
                });
            control.set_handler(handler);
        }

        // Load STATIC html (the injected bridge is available) and pump until the page finishes loading.
        bool load_page(const std::string& body)
        {
            WKWebView* const web_view = native_web_view(handler);
            NSString* const html = [NSString stringWithFormat:@"<html><body>%s</body></html>", body.c_str()];
            [web_view loadHTMLString:html baseURL:[NSURL URLWithString:@"https://demo.test/"]];
            return pump_until([&] { return web_view.loading == NO && web_view.URL != nil; });
        }
    };

    TEST(hybrid_web_view_ios, creates_a_wk_web_view_with_the_message_handler)
    {
        seam s;
        EXPECT_TRUE([native_web_view(s.handler) isKindOfClass:[WKWebView class]]);
    }

    // JS→native: the page echoes a raw message back through window.HybridWebView.SendRawMessage; the
    // control's raw_message_received event fires with the echoed content.
    TEST(hybrid_web_view_ios, raw_message_round_trip)
    {
        seam s;
        // The page listens for native→JS raw messages (window.external.receiveMessage, defined by the
        // injected bridge) and echoes "You said: <msg>" straight back via SendRawMessage.
        ASSERT_TRUE(s.load_page("<script>window.addEventListener('HybridWebViewMessageReceived', function(e) {"
                                "  window.HybridWebView.SendRawMessage('You said: ' + e.detail.message);"
                                "});</script>"));

        const std::string sent = R"(Hybrid Test with chars!)";
        s.control.send_raw_message(sent);

        ASSERT_TRUE(pump_until([&] { return !s.received.empty(); }));
        EXPECT_EQ(s.received.back(), "You said: " + sent);
    }

    // native→JS→native: invoke a global JS function and assert its return value arrives.
    TEST(hybrid_web_view_ios, invoke_js_returns_value)
    {
        seam s;
        ASSERT_TRUE(s.load_page("<script>function EchoParameter(x) { return x; }</script>"));

        bool completed = false;
        std::optional<std::string> result;
        s.control.invoke_js("EchoParameter", {"123"}, [&](std::optional<std::string> value) {
            completed = true;
            result = std::move(value);
        });

        ASSERT_TRUE(pump_until([&] { return completed; }));
        ASSERT_TRUE(result.has_value());
        EXPECT_EQ(result.value_or(""), "123"); // the JSON-encoded return (a number)
    }

    TEST(hybrid_web_view_ios, invoke_js_string_return)
    {
        seam s;
        ASSERT_TRUE(s.load_page("<script>function Concat(a, b) { return a + b; }</script>"));

        bool completed = false;
        std::optional<std::string> result;
        s.control.invoke_js("Concat", {"\"abc\"", "\"def\""}, [&](std::optional<std::string> value) {
            completed = true;
            result = std::move(value);
        });

        ASSERT_TRUE(pump_until([&] { return completed; }));
        ASSERT_TRUE(result.has_value());
        EXPECT_EQ(result.value_or(""), "\"abcdef\""); // the raw JSON-encoded string
    }

    TEST(hybrid_web_view_ios, evaluate_js_round_trip)
    {
        seam s;
        ASSERT_TRUE(s.load_page("<p>eval host</p>"));

        bool completed = false;
        std::optional<std::string> result;
        s.control.evaluate_js("1+1", [&](std::optional<std::string> value) {
            completed = true;
            result = std::move(value);
        });
        ASSERT_TRUE(pump_until([&] { return completed; }));
        ASSERT_TRUE(result.has_value());
        EXPECT_EQ(result.value_or(""), "2");
    }
} // namespace
