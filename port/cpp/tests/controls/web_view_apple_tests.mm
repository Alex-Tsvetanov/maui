// Apple (AppKit) backend tests for the web_view seam — a real WKWebView loading STATIC html (no
// network): the run loop is pumped until the navigation delegate reports navigated (success), a second
// load flips the handler-pushed CanGoBack read-only, go_back navigates back with the Back kind, and an
// eval_js round trip evaluates "1+1" → "2" through -evaluateJavaScript:completionHandler:. Deadlines are
// GENEROUS (the WebContent process spawn dominates the first load) and every wait has a positive
// completion condition, keeping the suite non-flaky. Compiled as Objective-C++ with ARC for the `apple`
// backend; tests/controls/web_view_ios_tests.mm is the on-simulator UIKit twin.
#import <WebKit/WebKit.h>

#include <filesystem>
#include <fstream>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "maui/controls/html_web_view_source.hpp"
#include "maui/controls/web_navigation_event_args.hpp"
#include "maui/controls/web_view.hpp"
#include "maui/core/web_navigation_event.hpp"
#include "maui/core/web_navigation_result.hpp"
#include "maui/core/web_view_handler.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::controls::html_web_view_source;
    using maui::controls::web_navigated_event_args;
    using maui::controls::web_view;
    using maui::core::web_navigation_event;
    using maui::core::web_navigation_result;
    using maui::core::web_view_handler;

    // The generous per-wait deadline (the C# device tests' pageLoadTimeout ballpark).
    constexpr NSTimeInterval k_deadline_seconds = 30.0;

    // Pump the main run loop until `done()` or the deadline; returns done()'s final value.
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

    WKWebView* native_web_view(const std::shared_ptr<web_view_handler>& handler)
    {
        return (__bridge WKWebView*)handler->typed_platform_view()->native;
    }

    struct seam
    {
        web_view control;
        std::shared_ptr<web_view_handler> handler = std::make_shared<web_view_handler>();
        std::vector<web_navigated_event_args> navigated;

        seam()
        {
            control.navigated.connect([this](const web_navigated_event_args& args) { navigated.push_back(args); });
            control.set_handler(handler);
        }

        // Load a static html source and pump until ITS navigated callback lands.
        bool load_html_and_wait(const std::string& html, const std::string& base_url)
        {
            const std::size_t before = navigated.size();
            control.set_source(std::make_shared<html_web_view_source>(html, base_url));
            return pump_until([&] { return navigated.size() > before; });
        }

        // Load a static file:// url and pump until ITS navigated callback lands.
        bool load_url_and_wait(const std::string& url)
        {
            const std::size_t before = navigated.size();
            control.set_source(url);
            return pump_until([&] { return navigated.size() > before; });
        }
    };

    // Write a STATIC html file under a unique temp dir and return its file:// url (no network —
    // WKWebView's loadHTMLString never enters the back-forward list, so the history-transition test
    // navigates between two static files instead).
    std::string write_static_page(const std::string& name, const std::string& body)
    {
        const std::filesystem::path dir = std::filesystem::temp_directory_path() / "maui_web_view_apple_tests";
        std::filesystem::create_directories(dir);
        const std::filesystem::path file = dir / name;
        std::ofstream stream(file);
        stream << "<html><body>" << body << "</body></html>";
        stream.close();
        return "file://" + file.string();
    }

    TEST(web_view_apple, creates_a_wk_web_view_with_no_history)
    {
        seam s;
        EXPECT_TRUE([native_web_view(s.handler) isKindOfClass:[WKWebView class]]);
        EXPECT_FALSE(s.control.can_go_back());
        EXPECT_FALSE(s.control.can_go_forward());
    }

    TEST(web_view_apple, static_html_load_fires_navigated_success)
    {
        seam s;
        ASSERT_TRUE(s.load_html_and_wait("<html><body><p>Hello WebView</p></body></html>", "https://demo.test/a"));
        EXPECT_EQ(s.navigated.back().result, web_navigation_result::success);
        EXPECT_EQ(s.navigated.back().navigation_event, web_navigation_event::new_page);
        EXPECT_FALSE(s.control.can_go_forward());
    }

    TEST(web_view_apple, second_load_enables_can_go_back_and_go_back_navigates)
    {
        // Two STATIC local pages (loadHTMLString navigations never enter WKWebView's back-forward
        // list, so the history transition is proven over file:// urls — still no network).
        const std::string page_a = write_static_page("page_a.html", "<h1>Page A</h1>");
        const std::string page_b = write_static_page("page_b.html", "<h1>Page B</h1>");

        seam s;
        ASSERT_TRUE(s.load_url_and_wait(page_a));
        EXPECT_FALSE(s.control.can_go_back());

        ASSERT_TRUE(s.load_url_and_wait(page_b));
        // The handler-pushed read-only flips once the second navigation lands.
        ASSERT_TRUE(pump_until([&] { return s.control.can_go_back(); }));

        // go_back → the Back kind navigated callback + can_go_forward flips.
        const std::size_t before = s.navigated.size();
        s.control.go_back();
        ASSERT_TRUE(pump_until([&] { return s.navigated.size() > before; }));
        EXPECT_EQ(s.navigated.back().navigation_event, web_navigation_event::back);
        EXPECT_EQ(s.navigated.back().result, web_navigation_result::success);
        ASSERT_TRUE(pump_until([&] { return s.control.can_go_forward(); }));
        EXPECT_FALSE(s.control.can_go_back());
    }

    TEST(web_view_apple, eval_js_round_trip_evaluates_arithmetic)
    {
        seam s;
        ASSERT_TRUE(s.load_html_and_wait("<html><body><p>eval host</p></body></html>", "https://demo.test/eval"));

        bool completed = false;
        std::optional<std::string> result;
        s.control.eval_js("1+1", [&](const std::optional<std::string>& value) {
            completed = true;
            result = value;
        });
        ASSERT_TRUE(pump_until([&] { return completed; }));
        ASSERT_TRUE(result.has_value());
        EXPECT_EQ(*result, "2");
    }

    TEST(web_view_apple, eval_js_string_result_is_unquoted)
    {
        seam s;
        ASSERT_TRUE(s.load_html_and_wait("<html><body><script>function test(){return 'Test';}</script></body></html>",
                                         "https://demo.test/fn"));

        bool completed = false;
        std::optional<std::string> result;
        s.control.eval_js("test();", [&](const std::optional<std::string>& value) {
            completed = true;
            result = value;
        });
        ASSERT_TRUE(pump_until([&] { return completed; }));
        ASSERT_TRUE(result.has_value());
        EXPECT_EQ(*result, "Test"); // the JSON.stringify quotes are trimmed (the C# device-test oracle)
    }

    TEST(web_view_apple, eval_js_undefined_result_is_null)
    {
        seam s;
        ASSERT_TRUE(s.load_html_and_wait("<html><body></body></html>", "https://demo.test/null"));

        bool completed = false;
        std::optional<std::string> result{"sentinel"};
        s.control.eval_js("does.not.exist()", [&](const std::optional<std::string>& value) {
            completed = true;
            result = value;
        });
        ASSERT_TRUE(pump_until([&] { return completed; }));
        EXPECT_FALSE(result.has_value());
    }

    // ---- user agent (WebViewHandler.iOS.MapUserAgent + WebViewExtensions.UpdateUserAgent) ----

    // Setting UserAgent flows virtual→native: WKWebView.customUserAgent reflects the value.
    TEST(web_view_apple, set_user_agent_updates_custom_user_agent)
    {
        seam s;
        s.control.set_user_agent("MauiUA/1.0");
        EXPECT_EQ(std::string(native_web_view(s.handler).customUserAgent.UTF8String), "MauiUA/1.0");
    }

    // Leaving UserAgent unset reads the platform default back into the virtual view (the bidirectional
    // branch): WKWebView reports a non-empty default `userAgent`, which lands in the control.
    TEST(web_view_apple, unset_user_agent_reads_platform_default_back)
    {
        seam s; // the full mapper ran at connect with the unset value → read-back populated the control
        EXPECT_FALSE(s.control.user_agent().empty());
    }

    // ---- WKUIDelegate JS dialogs (NSAlert analog) ----

    // MapWKUIDelegate installs our WKUIDelegate trampoline on the WKWebView.
    TEST(web_view_apple, installs_ui_delegate)
    {
        seam s;
        EXPECT_NE(native_web_view(s.handler).UIDelegate, nil);
    }

    // JS confirm() routes through the WKUIDelegate. With no anchoring window in the test process the panel
    // auto-completes with Cancel (false) — proving the completion handler is invoked (no hang / no blocking
    // modal) and the result reaches JS.
    TEST(web_view_apple, js_confirm_completes_with_false_without_window)
    {
        seam s;
        ASSERT_TRUE(s.load_html_and_wait("<html><body><p>confirm host</p></body></html>", "https://demo.test/c"));

        bool completed = false;
        std::optional<std::string> result;
        s.control.eval_js("confirm('proceed?')", [&](const std::optional<std::string>& value) {
            completed = true;
            result = value;
        });
        ASSERT_TRUE(pump_until([&] { return completed; }));
        ASSERT_TRUE(result.has_value());
        EXPECT_EQ(*result, "false"); // JSON.stringify(false)
    }

    // JS alert() routes through the WKUIDelegate and completes (returns undefined → "null").
    TEST(web_view_apple, js_alert_completes_and_script_continues)
    {
        seam s;
        ASSERT_TRUE(s.load_html_and_wait("<html><body><p>alert host</p></body></html>", "https://demo.test/a"));

        bool completed = false;
        std::optional<std::string> result{"sentinel"};
        s.control.eval_js("alert('hi')", [&](const std::optional<std::string>& value) {
            completed = true;
            result = value;
        });
        ASSERT_TRUE(pump_until([&] { return completed; }));
        EXPECT_FALSE(result.has_value());
    }

    // JS prompt() routes through the WKUIDelegate. With no window the panel auto-completes with Cancel
    // (nil → JS null → "null").
    TEST(web_view_apple, js_prompt_completes_with_null_without_window)
    {
        seam s;
        ASSERT_TRUE(s.load_html_and_wait("<html><body><p>prompt host</p></body></html>", "https://demo.test/p"));

        bool completed = false;
        std::optional<std::string> result{"sentinel"};
        s.control.eval_js("prompt('name?','default')", [&](const std::optional<std::string>& value) {
            completed = true;
            result = value;
        });
        ASSERT_TRUE(pump_until([&] { return completed; }));
        EXPECT_FALSE(result.has_value());
    }

    // Destroying a web_view must leave NOTHING attached that still holds the freed web_view_handler*.
    // on_connect_handler sets `delegate.handler = this` and keeps the MauiCppWebViewNavigationDelegate
    // alive on the WKWebView via an associated object; only on_disconnect_handler used to undo that, and
    // NOTHING calls disconnect_handler() when a handler is merely destroyed (there is no ~view_handler
    // doing it). ~web_view_platform CFReleased the native view and stopped — so a WKWebView that outlives
    // the handler (in any real app a superview retains it; here the ARC local does) kept a live delegate
    // pointing at freed memory, and the next WebKit navigation callback dereferenced it.
    // This is the entry_handler defect (1e7812c243) three files over; the fix is the same
    // detach-from-the-dtor-too, not a null check.
    TEST(web_view_apple, destroying_a_web_view_detaches_the_navigation_delegate)
    {
        WKWebView* web = nil;
        {
            seam s;
            web = native_web_view(s.handler); // an ARC strong local: the web view outlives the handler
            ASSERT_NE(web.navigationDelegate, nil);
        } // ~web_view_handler -> ~web_view_platform; disconnect_handler() never runs

        EXPECT_EQ(web.navigationDelegate, nil);
        EXPECT_EQ(web.UIDelegate, nil);

        // Drive the callback WebKit would. Pinning the delegate here is harmless — the freed object is
        // the C++ handler behind `self.handler`, not the delegate — so this cannot false-clean.
        id<WKNavigationDelegate> const nav = web.navigationDelegate;
        if (nav != nil)
        {
            [nav webView:web didFinishNavigation:nil]; // pre-fix: heap-use-after-free on the handler
        }
        // Reaching here without an ASan report IS the assertion.
    }
} // namespace
