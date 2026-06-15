// web_view_handler — headless platform recipe. A testable stand-in for the WKWebView pipeline: the
// platform struct records the loaded source / navigation commands / evaluated scripts, keeps an
// in-memory back-forward list, and replays the MauiWebViewNavigationDelegate flow SYNCHRONOUSLY —
// DecidePolicy (send_navigating, cancel support) → mutate the back-forward list → DidFinishNavigation
// (send_navigated(success)) — with UpdateCanGoBackForward after both steps, exactly where the WK
// delegate runs it. The shared Apple .mm (src/platform/apple_shared/web_view_handler.mm) is the
// real-native twin.

#include "maui/core/web_view_handler.hpp"

#include <any>
#include <cmath>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "maui/core/evaluate_java_script_request.hpp"
#include "maui/core/i_web_view.hpp"
#include "maui/core/i_web_view_source.hpp"
#include "maui/core/web_navigation_event.hpp"
#include "maui/core/web_navigation_result.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

namespace maui::core
{
    namespace
    {
        // WebViewExtensions.UpdateCanGoBackForward over the simulated back-forward list.
        void update_can_go_back_forward(web_view_platform& platform)
        {
            if (platform.connected_view == nullptr)
            {
                return;
            }
            platform.connected_view->set_can_go_back(!platform.history.empty() && platform.history_index > 0);
            platform.connected_view->set_can_go_forward(!platform.history.empty() &&
                                                        platform.history_index + 1 < platform.history.size());
        }

        // A fresh content load (LoadRequest / LoadHtmlString): DecidePolicy reports NewPage for it; a
        // successful load truncates any forward entries then appends (the WKBackForwardList shape).
        void simulate_load(web_view_platform& platform, const std::string& url)
        {
            if (platform.connected_view == nullptr)
            {
                return;
            }
            const bool cancel = platform.connected_view->send_navigating(web_navigation_event::new_page, url);
            update_can_go_back_forward(platform);
            if (cancel)
            {
                return;
            }
            if (!platform.history.empty() && platform.history_index + 1 < platform.history.size())
            {
                platform.history.erase(
                    platform.history.begin() +
                        static_cast<std::vector<std::string>::difference_type>(platform.history_index + 1),
                    platform.history.end());
            }
            platform.history.push_back(url);
            platform.history_index = platform.history.size() - 1;
            platform.connected_view->send_navigated(web_navigation_event::new_page, url,
                                                    web_navigation_result::success);
            update_can_go_back_forward(platform);
        }
    } // namespace

    // Headless has no native view in the `native` slot, so destruction is trivial.
    web_view_platform::~web_view_platform() = default;

    // ---- i_web_view_delegate (the source's load sink; C# MauiWKWebView.LoadUrl/LoadHtml) ----
    void web_view_platform::load_url(std::string_view url)
    {
        last_source_kind = web_view_source_kind::url;
        last_url = std::string(url);
        simulate_load(*this, last_url);
    }

    void web_view_platform::load_html(std::string_view html, std::string_view base_url)
    {
        // C# LoadHtml ignores a null html; an html navigation's url is its base url (WKWebView reports
        // the baseURL — "about:blank" when none was supplied).
        if (html.empty())
        {
            return;
        }
        last_source_kind = web_view_source_kind::html;
        last_html = std::string(html);
        last_base_url = std::string(base_url);
        simulate_load(*this, last_base_url.empty() ? std::string("about:blank") : last_base_url);
    }

    std::unique_ptr<web_view_platform> web_view_handler::create_platform_view()
    {
        return std::make_unique<web_view_platform>();
    }

    void web_view_handler::on_connect_handler(web_view_platform& platform)
    {
        platform.connected_view = virtual_view();
    }

    void web_view_handler::on_disconnect_handler(web_view_platform& platform)
    {
        platform.connected_view = nullptr;
    }

    // WebViewHandler.MapSource: the platform view is the IWebViewDelegate the source loads into, then
    // UpdateCanGoBackForward (WebViewExtensions.UpdateSource).
    void web_view_handler::map_source(web_view_handler& handler, i_web_view& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        if (auto* source = view.source())
        {
            source->load(*platform);
        }
        update_can_go_back_forward(*platform);
    }

    // WebViewHandler.iOS.MapUserAgent + WebViewExtensions.UpdateUserAgent, headless stub: there is no
    // WKWebView to write CustomUserAgent into, so the synced value is mirrored in platform->user_agent
    // (the test-inspection seam). When the virtual view's UserAgent is unset, the platform has no default
    // to read back — C#'s `platformWebView.CustomUserAgent ?? userAgent` is genuinely platform state — so
    // the headless mirror records the empty string (no native sync). The apple_shared .mm is the
    // real-native twin that performs the bidirectional CustomUserAgent / `userAgent` KVC sync.
    void web_view_handler::map_user_agent(web_view_handler& handler, i_web_view& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->user_agent = std::string(view.user_agent());
    }

    // WebViewHandler.MapGoBack + WebViewExtensions.UpdateGoBack, replayed over the simulated list: set
    // CurrentNavigationEvent = Back when a back entry exists, navigate only if CanGoBack, and let the
    // virtual view cancel through send_navigating (the DecidePolicy BackForward branch).
    void web_view_handler::map_go_back(web_view_handler& handler, i_web_view& view, const std::any& /*args*/)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        if (platform->history.empty() || platform->history_index == 0)
        {
            update_can_go_back_forward(*platform);
            return;
        }
        platform->current_navigation_event = web_navigation_event::back;
        const std::string target = platform->history[platform->history_index - 1];
        const bool cancel = view.send_navigating(platform->current_navigation_event, target);
        update_can_go_back_forward(*platform);
        if (cancel)
        {
            return;
        }
        --platform->history_index;
        view.send_navigated(platform->current_navigation_event, target, web_navigation_result::success);
        update_can_go_back_forward(*platform);
    }

    // WebViewHandler.MapGoForward + UpdateGoForward (the forward twin of map_go_back).
    void web_view_handler::map_go_forward(web_view_handler& handler, i_web_view& view, const std::any& /*args*/)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        if (platform->history.empty() || platform->history_index + 1 >= platform->history.size())
        {
            update_can_go_back_forward(*platform);
            return;
        }
        platform->current_navigation_event = web_navigation_event::forward;
        const std::string target = platform->history[platform->history_index + 1];
        const bool cancel = view.send_navigating(platform->current_navigation_event, target);
        update_can_go_back_forward(*platform);
        if (cancel)
        {
            return;
        }
        ++platform->history_index;
        view.send_navigated(platform->current_navigation_event, target, web_navigation_result::success);
        update_can_go_back_forward(*platform);
    }

    // WebViewHandler.MapReload (cookie sync out of scope) + UpdateReload: CurrentNavigationEvent =
    // Refresh, re-navigate the current entry in place (no back-forward mutation).
    void web_view_handler::map_reload(web_view_handler& handler, i_web_view& view, const std::any& /*args*/)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->current_navigation_event = web_navigation_event::refresh;
        if (platform->history.empty())
        {
            // A WKWebView with no content reloads to nothing — no navigation callbacks fire.
            return;
        }
        ++platform->reload_count;
        const std::string current = platform->history[platform->history_index];
        const bool cancel = view.send_navigating(platform->current_navigation_event, current);
        update_can_go_back_forward(*platform);
        if (cancel)
        {
            return;
        }
        view.send_navigated(platform->current_navigation_event, current, web_navigation_result::success);
        update_can_go_back_forward(*platform);
    }

    // WebViewHandler.MapEval: fire-and-forget script evaluation (the result is discarded).
    void web_view_handler::map_eval(web_view_handler& handler, i_web_view& /*view*/, const std::any& args)
    {
        auto* platform = handler.typed_platform_view();
        const auto* script = std::any_cast<std::string>(&args);
        if (platform == nullptr || script == nullptr)
        {
            return;
        }
        platform->eval_scripts.push_back(*script);
    }

    // WebViewHandler.MapEvaluateJavaScriptAsync: record the script and complete the request through the
    // canned-result seam (unset => "null", the WKWebView value for an errored/void script).
    void web_view_handler::map_evaluate_java_script(web_view_handler& handler, i_web_view& /*view*/,
                                                    const std::any& args)
    {
        const auto* request = std::any_cast<std::shared_ptr<evaluate_java_script_request>>(&args);
        if (request == nullptr || *request == nullptr)
        {
            return;
        }
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            // C# SetCanceled when the platform view is gone — surfaced as the "null" result (see
            // evaluate_java_script_request.hpp).
            (*request)->complete("null");
            return;
        }
        platform->eval_scripts.push_back((*request)->script());
        std::string result =
            platform->eval_result_provider ? platform->eval_result_provider((*request)->script()) : "null";
        (*request)->complete(std::move(result));
    }

    maui::graphics::size web_view_handler::get_desired_size(double width_constraint, double height_constraint) const
    {
        // WebViewHandler.iOS.GetDesiredSize: the platform measure (0 for the headless stand-in) falls
        // back to MinimumSize (44) per dimension when the constraint is unbounded or non-positive.
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

    void web_view_handler::platform_arrange(const maui::graphics::rect& /*frame*/)
    {
        // Headless: no native layout to apply.
    }
} // namespace maui::core
