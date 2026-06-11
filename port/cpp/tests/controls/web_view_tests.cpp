// Tests for the web_view control + its headless handler seam. The control half ports
// src/Controls/tests/Core.UnitTests/WebViewUnitTests.cs (source conversion / SourceChanged propagation /
// source disconnect / BindingContext propagation); the seam half characterizes the WKWebView pipeline the
// headless partial mirrors synchronously (MauiWebViewNavigationDelegate + WebViewExtensions: navigating
// → navigated ordering, the back-forward list driving CanGoBack/CanGoForward, the go_back/go_forward/
// reload command kinds, navigation cancel, and the EvaluateJavaScriptAsync escape/wrap/unquote round
// trip). The shared Apple .mm is the real-native twin verified on both Apple backends.
#include "maui/controls/web_view.hpp"

#include <limits>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "maui/controls/bindings/binding.hpp"
#include "maui/controls/html_web_view_source.hpp"
#include "maui/controls/url_web_view_source.hpp"
#include "maui/controls/web_navigation_event_args.hpp"
#include "maui/controls/web_view_source.hpp"
#include "maui/core/bindable_object.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/evaluate_java_script_request.hpp"
#include "maui/core/property.hpp"
#include "maui/core/web_navigation_event.hpp"
#include "maui/core/web_navigation_result.hpp"
#include "maui/core/web_view_handler.hpp"
#include "maui/core/web_view_helper.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::controls::html_web_view_source;
    using maui::controls::url_web_view_source;
    using maui::controls::web_navigated_event_args;
    using maui::controls::web_navigating_event_args;
    using maui::controls::web_view;
    using maui::core::escape_js_string;
    using maui::core::evaluate_java_script_request;
    using maui::core::web_navigation_event;
    using maui::core::web_navigation_result;
    using maui::core::web_view_handler;
    using maui::core::web_view_source_kind;

    // ---- the control in isolation (WebViewUnitTests.cs) ----

    // TestSourceImplicitConversion: a string source becomes a UrlWebViewSource with that url.
    TEST(web_view, source_implicit_conversion)
    {
        web_view control;
        EXPECT_EQ(control.source(), nullptr);
        control.set_source("http://www.google.com");
        ASSERT_NE(control.source(), nullptr);
        auto* url_source = dynamic_cast<url_web_view_source*>(control.source());
        ASSERT_NE(url_source, nullptr);
        EXPECT_EQ(url_source->url(), "http://www.google.com");
    }

    // TestSourceChangedPropagation: changing the CURRENT source's url re-raises the control's "source"
    // property change.
    TEST(web_view, source_changed_propagation)
    {
        auto source = std::make_shared<url_web_view_source>("http://www.google.com");
        web_view control;
        control.set_source(source);

        bool signaled = false;
        control.property_changed.connect([&signaled](const std::string_view& name) {
            if (name == "source")
            {
                signaled = true;
            }
        });
        EXPECT_FALSE(signaled);
        source->set_url("http://www.xamarin.com");
        EXPECT_TRUE(signaled);
    }

    // TestSourceDisconnected: a REPLACED source no longer propagates its changes.
    TEST(web_view, source_disconnected)
    {
        auto source = std::make_shared<url_web_view_source>("http://www.google.com");
        web_view control;
        control.set_source(source);
        control.set_source(std::make_shared<url_web_view_source>("Foo"));

        bool signaled = false;
        control.property_changed.connect([&signaled](const std::string_view& name) {
            if (name == "source")
            {
                signaled = true;
            }
        });
        EXPECT_FALSE(signaled);
        source->set_url("http://www.xamarin.com");
        EXPECT_FALSE(signaled);
    }

    // TestBindingContextPropagatesToSource: a source's bindings resolve against the web_view's
    // BindingContext (SetInheritedBindingContext flows the VM into the source).
    namespace vm_props
    {
        const maui::core::bindable_property<std::string>& html_prop()
        {
            static const maui::core::bindable_property<std::string> descriptor{
                "html_value", std::string{"<html><body><p>This is a WebView!</p></body></html>"}};
            return descriptor;
        }
        const maui::core::bindable_property<std::string>& url_prop()
        {
            static const maui::core::bindable_property<std::string> descriptor{"url_value",
                                                                               std::string{"http://xamarin.com"}};
            return descriptor;
        }
    } // namespace vm_props

    struct web_view_view_model : maui::core::bindable_object
    {
        maui::core::property<std::string> html{*this, vm_props::html_prop()};
        maui::core::property<std::string> url{*this, vm_props::url_prop()};
    };

    TEST(web_view, binding_context_propagates_to_source)
    {
        web_view html_web_view;
        web_view url_web_view;

        auto html_source = std::make_shared<html_web_view_source>();
        html_source->set_binding("html", std::make_shared<maui::controls::binding>("html_value"));
        html_web_view.set_source(html_source);

        auto url_source = std::make_shared<url_web_view_source>();
        url_source->set_binding("url", std::make_shared<maui::controls::binding>("url_value"));
        url_web_view.set_source(url_source);

        auto view_model = std::make_shared<web_view_view_model>();
        html_web_view.set_binding_context(view_model);
        url_web_view.set_binding_context(view_model);

        EXPECT_EQ(html_source->html(), "<html><body><p>This is a WebView!</p></body></html>");
        EXPECT_EQ(url_source->url(), "http://xamarin.com");
    }

    // The context also reaches a source set AFTER the context (C#'s propertyChanged hook).
    TEST(web_view, binding_context_reaches_source_set_after_context)
    {
        web_view control;
        auto view_model = std::make_shared<web_view_view_model>();
        control.set_binding_context(view_model);

        auto url_source = std::make_shared<url_web_view_source>();
        url_source->set_binding("url", std::make_shared<maui::controls::binding>("url_value"));
        control.set_source(url_source);

        EXPECT_EQ(url_source->url(), "http://xamarin.com");
    }

    // ---- the handler seam (headless mirror of the WKWebView pipeline) ----

    struct seam
    {
        web_view control;
        std::shared_ptr<web_view_handler> handler = std::make_shared<web_view_handler>();
        maui::core::web_view_platform* platform = nullptr;

        seam()
        {
            control.set_handler(handler);
            platform = handler->typed_platform_view();
        }
    };

    TEST(web_view_handler_seam, url_source_maps_to_platform)
    {
        seam s;
        EXPECT_EQ(s.platform->last_source_kind, web_view_source_kind::none);
        s.control.set_source("https://example.test/page1");
        EXPECT_EQ(s.platform->last_source_kind, web_view_source_kind::url);
        EXPECT_EQ(s.platform->last_url, "https://example.test/page1");
        ASSERT_EQ(s.platform->history.size(), 1U);
        EXPECT_FALSE(s.control.can_go_back());
        EXPECT_FALSE(s.control.can_go_forward());
    }

    TEST(web_view_handler_seam, html_source_maps_to_platform)
    {
        seam s;
        auto source = std::make_shared<html_web_view_source>("<html><body>Hi</body></html>", "https://base.test/");
        s.control.set_source(source);
        EXPECT_EQ(s.platform->last_source_kind, web_view_source_kind::html);
        EXPECT_EQ(s.platform->last_html, "<html><body>Hi</body></html>");
        EXPECT_EQ(s.platform->last_base_url, "https://base.test/");
        ASSERT_EQ(s.platform->history.size(), 1U);
        EXPECT_EQ(s.platform->history[0], "https://base.test/");
    }

    TEST(web_view_handler_seam, html_source_without_base_url_navigates_about_blank)
    {
        seam s;
        s.control.set_source(std::make_shared<html_web_view_source>("<p>x</p>"));
        ASSERT_EQ(s.platform->history.size(), 1U);
        EXPECT_EQ(s.platform->history[0], "about:blank");
    }

    TEST(web_view_handler_seam, load_fires_navigating_then_navigated_with_new_page_success)
    {
        seam s;
        std::vector<std::string> order;
        web_navigation_event navigating_kind{};
        web_navigation_event navigated_kind{};
        web_navigation_result navigated_result{};
        std::string navigated_url;
        s.control.navigating.connect([&](const web_navigating_event_args& args) {
            order.emplace_back("navigating");
            navigating_kind = args.navigation_event;
        });
        s.control.navigated.connect([&](const web_navigated_event_args& args) {
            order.emplace_back("navigated");
            navigated_kind = args.navigation_event;
            navigated_result = args.result;
            navigated_url = args.url;
        });

        s.control.set_source("https://example.test/page1");

        ASSERT_EQ(order.size(), 2U);
        EXPECT_EQ(order[0], "navigating");
        EXPECT_EQ(order[1], "navigated");
        EXPECT_EQ(navigating_kind, web_navigation_event::new_page);
        EXPECT_EQ(navigated_kind, web_navigation_event::new_page);
        EXPECT_EQ(navigated_result, web_navigation_result::success);
        EXPECT_EQ(navigated_url, "https://example.test/page1");
    }

    // The navigated args carry a url_web_view_source minted for the navigated url (the C#
    // `new UrlWebViewSource { Url = url }` in IWebView.Navigated).
    TEST(web_view_handler_seam, navigated_args_source_is_url_source_for_url)
    {
        seam s;
        std::shared_ptr<maui::controls::web_view_source> args_source;
        s.control.navigated.connect([&](const web_navigated_event_args& args) { args_source = args.source; });
        s.control.set_source("https://example.test/page1");
        auto* url_source = dynamic_cast<url_web_view_source*>(args_source.get());
        ASSERT_NE(url_source, nullptr);
        EXPECT_EQ(url_source->url(), "https://example.test/page1");
    }

    TEST(web_view_handler_seam, second_load_enables_can_go_back)
    {
        seam s;
        s.control.set_source("https://example.test/page1");
        EXPECT_FALSE(s.control.can_go_back());
        s.control.set_source("https://example.test/page2");
        EXPECT_TRUE(s.control.can_go_back());
        EXPECT_FALSE(s.control.can_go_forward());
        ASSERT_EQ(s.platform->history.size(), 2U);
        EXPECT_EQ(s.platform->history_index, 1U);
    }

    TEST(web_view_handler_seam, go_back_navigates_back_with_back_kind)
    {
        seam s;
        s.control.set_source("https://example.test/page1");
        s.control.set_source("https://example.test/page2");

        std::vector<web_navigation_event> kinds;
        std::string last_navigated_url;
        s.control.navigated.connect([&](const web_navigated_event_args& args) {
            kinds.push_back(args.navigation_event);
            last_navigated_url = args.url;
        });

        s.control.go_back();

        ASSERT_EQ(kinds.size(), 1U);
        EXPECT_EQ(kinds[0], web_navigation_event::back);
        EXPECT_EQ(last_navigated_url, "https://example.test/page1");
        EXPECT_FALSE(s.control.can_go_back());
        EXPECT_TRUE(s.control.can_go_forward());
        EXPECT_EQ(s.platform->history_index, 0U);
    }

    TEST(web_view_handler_seam, go_forward_navigates_forward_with_forward_kind)
    {
        seam s;
        s.control.set_source("https://example.test/page1");
        s.control.set_source("https://example.test/page2");
        s.control.go_back();

        std::vector<web_navigation_event> kinds;
        s.control.navigated.connect(
            [&](const web_navigated_event_args& args) { kinds.push_back(args.navigation_event); });

        s.control.go_forward();

        ASSERT_EQ(kinds.size(), 1U);
        EXPECT_EQ(kinds[0], web_navigation_event::forward);
        EXPECT_TRUE(s.control.can_go_back());
        EXPECT_FALSE(s.control.can_go_forward());
        EXPECT_EQ(s.platform->history_index, 1U);
    }

    TEST(web_view_handler_seam, go_back_without_history_is_a_noop)
    {
        seam s;
        int navigated_count = 0;
        s.control.navigated.connect([&](const web_navigated_event_args& /*args*/) { ++navigated_count; });
        s.control.go_back();
        s.control.go_forward();
        EXPECT_EQ(navigated_count, 0);
        EXPECT_FALSE(s.control.can_go_back());
        EXPECT_FALSE(s.control.can_go_forward());
    }

    TEST(web_view_handler_seam, reload_fires_refresh_and_keeps_history)
    {
        seam s;
        s.control.set_source("https://example.test/page1");

        std::vector<web_navigation_event> kinds;
        std::string navigated_url;
        s.control.navigated.connect([&](const web_navigated_event_args& args) {
            kinds.push_back(args.navigation_event);
            navigated_url = args.url;
        });

        s.control.reload();

        ASSERT_EQ(kinds.size(), 1U);
        EXPECT_EQ(kinds[0], web_navigation_event::refresh);
        EXPECT_EQ(navigated_url, "https://example.test/page1");
        EXPECT_EQ(s.platform->reload_count, 1);
        EXPECT_EQ(s.platform->history.size(), 1U);
        EXPECT_FALSE(s.control.can_go_back());
    }

    TEST(web_view_handler_seam, reload_without_content_fires_nothing)
    {
        seam s;
        int navigated_count = 0;
        s.control.navigated.connect([&](const web_navigated_event_args& /*args*/) { ++navigated_count; });
        s.control.reload();
        EXPECT_EQ(navigated_count, 0);
        EXPECT_EQ(s.platform->reload_count, 0);
    }

    TEST(web_view_handler_seam, navigating_cancel_prevents_navigation)
    {
        seam s;
        s.control.set_source("https://example.test/page1");

        s.control.navigating.connect([](web_navigating_event_args& args) { args.cancel = true; });
        int navigated_count = 0;
        s.control.navigated.connect([&](const web_navigated_event_args& /*args*/) { ++navigated_count; });

        s.control.set_source("https://example.test/page2");

        EXPECT_EQ(navigated_count, 0);
        EXPECT_EQ(s.platform->history.size(), 1U);
        EXPECT_FALSE(s.control.can_go_back());
    }

    TEST(web_view_handler_seam, navigating_cancel_prevents_go_back)
    {
        seam s;
        s.control.set_source("https://example.test/page1");
        s.control.set_source("https://example.test/page2");

        s.control.navigating.connect([](web_navigating_event_args& args) { args.cancel = true; });
        s.control.go_back();

        EXPECT_EQ(s.platform->history_index, 1U);
        EXPECT_TRUE(s.control.can_go_back());
        EXPECT_FALSE(s.control.can_go_forward());
    }

    // A fresh load truncates the forward entries (the WKBackForwardList shape).
    TEST(web_view_handler_seam, fresh_load_truncates_forward_history)
    {
        seam s;
        s.control.set_source("https://example.test/page1");
        s.control.set_source("https://example.test/page2");
        s.control.go_back();
        EXPECT_TRUE(s.control.can_go_forward());

        s.control.set_source("https://example.test/page3");

        EXPECT_FALSE(s.control.can_go_forward());
        EXPECT_TRUE(s.control.can_go_back());
        ASSERT_EQ(s.platform->history.size(), 2U);
        EXPECT_EQ(s.platform->history[1], "https://example.test/page3");
    }

    // Changing the CURRENT source's url re-runs map_source (the SourceChanged → OnPropertyChanged
    // pipeline) and performs a fresh load.
    TEST(web_view_handler_seam, source_changed_reloads_platform)
    {
        seam s;
        auto source = std::make_shared<url_web_view_source>("https://example.test/page1");
        s.control.set_source(source);
        ASSERT_EQ(s.platform->history.size(), 1U);

        source->set_url("https://example.test/page2");

        EXPECT_EQ(s.platform->last_url, "https://example.test/page2");
        ASSERT_EQ(s.platform->history.size(), 2U);
        EXPECT_TRUE(s.control.can_go_back());
    }

    // ---- scripting ----

    TEST(web_view_handler_seam, eval_records_script)
    {
        seam s;
        s.control.eval("document.title = 'x';");
        ASSERT_EQ(s.platform->eval_scripts.size(), 1U);
        EXPECT_EQ(s.platform->eval_scripts[0], "document.title = 'x';");
    }

    TEST(web_view_handler_seam, eval_js_round_trip_delivers_canned_result)
    {
        seam s;
        s.platform->eval_result_provider = [](const std::string& /*script*/) { return std::string("2"); };

        std::optional<std::string> result;
        bool completed = false;
        s.control.eval_js("1+1", [&](const std::optional<std::string>& value) {
            completed = true;
            result = value;
        });

        ASSERT_TRUE(completed);
        ASSERT_TRUE(result.has_value());
        EXPECT_EQ(*result, "2");
        // The platform received the ESCAPED + WRAPPED script (the EvaluateJavaScriptAsync transform).
        ASSERT_EQ(s.platform->eval_scripts.size(), 1U);
        EXPECT_EQ(s.platform->eval_scripts[0], "try{JSON.stringify(eval('1+1'))}catch(e){'null'};");
    }

    TEST(web_view_handler_seam, eval_js_null_result_is_nullopt)
    {
        seam s; // eval_result_provider unset => "null" (an errored/void script)
        std::optional<std::string> result{"sentinel"};
        s.control.eval_js("does.not.exist()", [&](const std::optional<std::string>& value) { result = value; });
        EXPECT_FALSE(result.has_value());
    }

    TEST(web_view_handler_seam, eval_js_trims_json_stringify_quotes)
    {
        seam s;
        s.platform->eval_result_provider = [](const std::string& /*script*/) { return std::string("\"Test\""); };
        std::optional<std::string> result;
        s.control.eval_js("test();", [&](const std::optional<std::string>& value) { result = value; });
        ASSERT_TRUE(result.has_value());
        EXPECT_EQ(*result, "Test");
    }

    TEST(web_view_handler_seam, eval_js_escapes_the_script_for_the_eval_literal)
    {
        seam s;
        s.control.eval_js("alert('hi')\n", [](const std::optional<std::string>& /*value*/) {});
        ASSERT_EQ(s.platform->eval_scripts.size(), 1U);
        EXPECT_EQ(s.platform->eval_scripts[0], "try{JSON.stringify(eval('alert(\\'hi\\')\\n'))}catch(e){'null'};");
    }

    TEST(web_view, eval_js_without_handler_completes_with_nullopt)
    {
        web_view control;
        std::optional<std::string> result{"sentinel"};
        bool completed = false;
        control.eval_js("1+1", [&](const std::optional<std::string>& value) {
            completed = true;
            result = value;
        });
        ASSERT_TRUE(completed);
        EXPECT_FALSE(result.has_value());
    }

    // ---- the helpers ----

    TEST(web_view_helper, escape_js_string_matches_the_oracle)
    {
        // WebViewHelper.EscapeJsString: nothing special => unchanged.
        EXPECT_EQ(escape_js_string("plain text 123"), "plain text 123");
        EXPECT_EQ(escape_js_string("back\\slash"), "back\\\\slash");
        EXPECT_EQ(escape_js_string("it's"), "it\\'s");
        EXPECT_EQ(escape_js_string("line\nbreak"), "line\\nbreak");
        EXPECT_EQ(escape_js_string("carriage\rreturn"), "carriage\\rreturn");
        EXPECT_EQ(escape_js_string("ls\xE2\x80\xA8ps\xE2\x80\xA9"), "ls\\u2028ps\\u2029");
        EXPECT_EQ(escape_js_string(""), "");
    }

    TEST(evaluate_java_script_request, completes_exactly_once)
    {
        int calls = 0;
        std::string seen;
        evaluate_java_script_request request{"script", [&](std::string result) {
                                                 ++calls;
                                                 seen = std::move(result);
                                             }};
        EXPECT_EQ(request.script(), "script");
        EXPECT_FALSE(request.completed());
        request.complete("first");
        request.complete("second");
        EXPECT_EQ(calls, 1);
        EXPECT_EQ(seen, "first");
        EXPECT_TRUE(request.completed());
    }

    // ---- sizing ----

    TEST(web_view_handler_seam, desired_size_falls_back_to_minimum_under_unbounded_constraints)
    {
        seam s;
        const auto size = s.handler->get_desired_size(std::numeric_limits<double>::infinity(),
                                                      std::numeric_limits<double>::infinity());
        EXPECT_DOUBLE_EQ(size.width, web_view_handler::minimum_size);
        EXPECT_DOUBLE_EQ(size.height, web_view_handler::minimum_size);
    }
} // namespace
