// iOS (UIKit, on-simulator) backend tests for the web_view seam — the UIKit twin of
// web_view_apple_tests.mm over the SAME shared WKWebView .mm partial: a real WKWebView loading STATIC
// content (no network), the run loop pumped until the navigation delegate reports navigated (success),
// a second load flipping the handler-pushed CanGoBack read-only over two static file:// pages, go_back
// navigating back with the Back kind, and the eval_js "1+1" → "2" round trip. Deadlines are GENEROUS
// (the WebContent process spawn dominates the first load) and every wait has a positive completion
// condition, keeping the on-simulator suite non-flaky. Compiled as Objective-C++ with ARC for the `ios`
// backend; run ON the booted simulator via tools/ios-sim-run.sh.
#import <UIKit/UIKit.h>
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
        const std::filesystem::path dir = std::filesystem::temp_directory_path() / "maui_web_view_ios_tests";
        std::filesystem::create_directories(dir);
        const std::filesystem::path file = dir / name;
        std::ofstream stream(file);
        stream << "<html><body>" << body << "</body></html>";
        stream.close();
        return "file://" + file.string();
    }

    TEST(web_view_ios, creates_a_wk_web_view_with_no_history)
    {
        seam s;
        EXPECT_TRUE([native_web_view(s.handler) isKindOfClass:[WKWebView class]]);
        EXPECT_FALSE(s.control.can_go_back());
        EXPECT_FALSE(s.control.can_go_forward());
    }

    TEST(web_view_ios, static_html_load_fires_navigated_success)
    {
        seam s;
        ASSERT_TRUE(s.load_html_and_wait("<html><body><p>Hello WebView</p></body></html>", "https://demo.test/a"));
        EXPECT_EQ(s.navigated.back().result, web_navigation_result::success);
        EXPECT_EQ(s.navigated.back().navigation_event, web_navigation_event::new_page);
        EXPECT_FALSE(s.control.can_go_forward());
    }

    TEST(web_view_ios, second_load_enables_can_go_back_and_go_back_navigates)
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

    TEST(web_view_ios, eval_js_round_trip_evaluates_arithmetic)
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

    TEST(web_view_ios, eval_js_string_result_is_unquoted)
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

    TEST(web_view_ios, eval_js_undefined_result_is_null)
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
} // namespace
