// Apple (AppKit) backend tests for the hybrid_web_view bridge (G5) — the AppKit twin of
// hybrid_web_view_ios_tests.mm over the SAME shared WKWebView + WKScriptMessageHandler .mm partial: a real
// WKWebView with the bridge JS injected at document start, loading STATIC html (no network). The run loop
// is pumped until the page loads, then the raw-message echo round trip, an invoke_js return, and an
// evaluate_js("1+1") → "2" round trip are verified. Deadlines are GENEROUS (the WebContent process spawn
// dominates the first load). Compiled as Objective-C++ with ARC for the `apple` backend.
#import <AppKit/AppKit.h>
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

        bool load_page(const std::string& body)
        {
            WKWebView* const web_view = native_web_view(handler);
            NSString* const html = [NSString stringWithFormat:@"<html><body>%s</body></html>", body.c_str()];
            [web_view loadHTMLString:html baseURL:[NSURL URLWithString:@"https://demo.test/"]];
            return pump_until([&] { return web_view.loading == NO && web_view.URL != nil; });
        }
    };

    TEST(hybrid_web_view_apple, creates_a_wk_web_view)
    {
        seam s;
        EXPECT_TRUE([native_web_view(s.handler) isKindOfClass:[WKWebView class]]);
    }

    TEST(hybrid_web_view_apple, initialization_events_fire_on_create)
    {
        // The handler fires WebViewInitialization Started/Completed inside create_platform_view (before
        // set_handler returns), so the events must already have fired by the time the seam is built — but
        // they fire DURING set_handler, so subscribe before connecting via a fresh control.
        hybrid_web_view control;
        bool initializing = false;
        bool initialized = false;
        control.web_view_initializing.connect([&initializing] { initializing = true; });
        control.web_view_initialized.connect([&initialized] { initialized = true; });
        control.set_handler(std::make_shared<hybrid_web_view_handler>());
        EXPECT_TRUE(initializing);
        EXPECT_TRUE(initialized);
    }

    TEST(hybrid_web_view_apple, raw_message_round_trip)
    {
        seam s;
        ASSERT_TRUE(s.load_page("<script>window.addEventListener('HybridWebViewMessageReceived', function(e) {"
                                "  window.HybridWebView.SendRawMessage('You said: ' + e.detail.message);"
                                "});</script>"));

        const std::string sent = "Hybrid Test";
        s.control.send_raw_message(sent);

        ASSERT_TRUE(pump_until([&] { return !s.received.empty(); }));
        EXPECT_EQ(s.received.back(), "You said: " + sent);
    }

    TEST(hybrid_web_view_apple, invoke_js_returns_value)
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
        EXPECT_EQ(result.value_or(""), "123");
    }

    TEST(hybrid_web_view_apple, evaluate_js_round_trip)
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
