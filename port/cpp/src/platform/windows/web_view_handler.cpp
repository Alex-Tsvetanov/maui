// web_view_handler — Windows (WinUI 3) platform partial: a REAL Microsoft.UI.Xaml.Controls.WebView2
// held as one strong WinRT ref in web_view_platform::native. The windows twin of
// src/platform/apple_shared/web_view_handler.mm (the WKWebView recipe) / src/platform/android/
// web_view_handler.cpp (the android.webkit.WebView recipe) and the real-native sibling of the
// in-memory headless mirror (src/platform/headless/web_view_handler.cpp). Ported DIRECTLY from
// WebViewHandler.Windows.cs (the WebView2Proxy wiring) + Platform/Windows/MauiWebView.cs (the
// IWebViewDelegate.LoadHtml/LoadUrl recipe) + Platform/Windows/WebViewExtensions.cs.
//
// SCOPE (the android wave-22 twin): the STATIC HtmlWebViewSource is the target — map_source routes the
// source through the platform-as-i_web_view_delegate, and load_html drives WebView2.NavigateToString
// with MauiWebView.LoadHtml's <base>-tag insertion script prepended. NavigateToString requires a live
// CoreWebView2 (C# `await EnsureCoreWebView2Async()`), which the port bridges with the
// pending_source_load stash: subscribe CoreWebView2Initialized at connect, kick EnsureCoreWebView2Async,
// and replay the parked HTML from the last_html/last_base_url mirrors in the Initialized handler.
// load_url assigns WebView2.Source (which self-initializes the CoreWebView2). The real
// NavigationStarting/NavigationCompleted events ARE wired (the WebView2Proxy pipeline): send_navigating
// with cancel support (args.Cancel + the navigation_cancelled gate), send_navigated(success|failure),
// and UpdateCanGoBackForward off the control's own CanGoBack/CanGoForward.
//
// DOCUMENTED DEVIATIONS from the C# Windows oracle (each an infrastructure gap, NOT a behavior guess):
//   - SetVirtualHostNameToFolderMapping (MauiWebView's `appdir` virtual host → app folder, for
//     relative-resource resolution) is deferred: it needs the packaged/unpackaged application path
//     (AppInfoUtils.IsPackagedApp ? Package.InstalledLocation : AppContext.BaseDirectory). A fully
//     inline static HTML source resolves nothing relative, so the parity render needs none of it. The
//     <base href> insertion script itself IS ported (the LocalScheme base is still written).
//   - Cookie sync (SyncPlatformCookies/InitialCookiePreloadIfNecessary) is the header's documented
//     out-of-scope item, as on every backend. UpdateBackground (DefaultBackgroundColor +
//     PreferredColorScheme) keeps the base mirror (the header's first-cut list) and MapFlowDirection
//     deliberately no-ops in C# itself. CoreProcessFailed/HistoryChanged wiring and the window-Closed
//     auto-disconnect (WebView2Proxy.Connect(window)) are deferred with the same first-cut scope.
//   - map_evaluate_java_script completes through the headless canned-result seam; the native
//     ExecuteScriptAsync Completed-handler bridge is the deferred JS round-trip item (map_eval's
//     fire-and-forget ExecuteScriptAsync IS pushed natively).
//
// XAML-less degradation (the windows twin of the android VM-less fallback): the windows preset also
// runs the cross-platform suite on the host where NO XAML runtime exists; create_platform_view catches
// the construction failure (also the WebView2-Runtime-less box) and keeps native null, and on that host
// this partial REPLAYS the headless twin's synchronous navigation simulation (send_navigating → the
// in-memory back-forward list → send_navigated + UpdateCanGoBackForward) so that suite observes exactly
// the headless partial's behavior, while the mirrors (source kind / last_html / last_url / eval_scripts
// / the base IView mirrors) are ALWAYS maintained (the android dual-drive).

#include "maui/core/web_view_handler.hpp"

#include <any>
#include <cmath>
#include <cstddef>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Microsoft.Web.WebView2.Core.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/base.h>

#include "maui/core/dimension.hpp"
#include "maui/core/evaluate_java_script_request.hpp"
#include "maui/core/i_web_view.hpp"
#include "maui/core/i_web_view_source.hpp"
#include "maui/core/view_platform_base.hpp"
#include "maui/core/visibility.hpp"
#include "maui/core/web_navigation_event.hpp"
#include "maui/core/web_navigation_result.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"
#include "windows_native.hpp"

namespace
{
    namespace muxc = winrt::Microsoft::UI::Xaml::Controls;
    namespace wv2core = winrt::Microsoft::Web::WebView2::Core;
    namespace wnative = maui::platform::win;

    using maui::core::web_navigation_event;
    using maui::core::web_navigation_result;
    using maui::core::web_view_platform;
    using maui::core::web_view_source_kind;

    // MauiWebView.LocalHostName / LocalScheme: the arbitrary local host a base-url-less html source (and
    // a relative url) is rebased under.
    constexpr const char* k_local_scheme = "https://appdir/";

    // MauiWebView.BaseInsertionScript (verbatim, whitespace and all): inserts a <base> tag into the
    // document head when none exists; "baseTag" is the replace token GetBaseTagInsertionScript fills.
    constexpr const char* k_base_insertion_script =
        "\n\t\t\tvar head = document.getElementsByTagName('head')[0];"
        "\n\t\t\tvar bases = head.getElementsByTagName('base');"
        "\n\t\t\tif(bases.length == 0) {"
        "\n\t\t\t\thead.innerHTML = 'baseTag' + head.innerHTML;"
        "\n\t\t\t}";

    // string.Replace(token, replacement) — every occurrence, left to right.
    [[nodiscard]] std::string replace_all(std::string text, std::string_view token, std::string_view replacement)
    {
        std::size_t position = 0;
        while ((position = text.find(token, position)) != std::string::npos)
        {
            text.replace(position, token.size(), replacement);
            position += replacement.size();
        }
        return text;
    }

    // MauiWebView.GetBaseTagInsertionScript: <script>{BaseInsertionScript with "baseTag" → the literal
    // <base href="..."></base>}</script>.
    [[nodiscard]] std::string base_tag_insertion_script(std::string_view base_url)
    {
        const std::string base_tag = "<base href=\"" + std::string(base_url) + "\"></base>";
        return "<script>" + replace_all(k_base_insertion_script, "baseTag", base_tag) + "</script>";
    }

    // The stored WebView2 control (null on the XAML-less host).
    [[nodiscard]] muxc::WebView2 web_view_of(const web_view_platform& platform)
    {
        return wnative::borrow<muxc::WebView2>(platform.native);
    }

    // The initialized CoreWebView2, or null before EnsureCoreWebView2Async completed. The try/catch is
    // WebViewExtensions.IsValid's disposed/invalid guard (Close() leaves the control throwing here).
    [[nodiscard]] wv2core::CoreWebView2 core_of(muxc::WebView2 const& web_view)
    {
        if (web_view == nullptr)
        {
            return nullptr;
        }
        try
        {
            return web_view.CoreWebView2();
        }
        catch (const winrt::hresult_error&)
        {
            return nullptr;
        }
    }

    // WebViewExtensions.UpdateCanGoBackForward: webView.CanGoBack/CanGoForward = platformWebView's. With
    // a live CoreWebView2 the control's own properties are the source; the XAML-less host (and the
    // pre-init window) derives from the simulated back-forward list exactly like the headless twin.
    void update_can_go_back_forward(web_view_platform& platform)
    {
        if (platform.connected_view == nullptr)
        {
            return;
        }
        const auto web_view = web_view_of(platform);
        if (core_of(web_view) != nullptr)
        {
            try
            {
                platform.connected_view->set_can_go_back(web_view.CanGoBack());
                platform.connected_view->set_can_go_forward(web_view.CanGoForward());
                return;
            }
            catch (const winrt::hresult_error&)
            {
                // fall through to the mirror derivation
            }
        }
        platform.connected_view->set_can_go_back(!platform.history.empty() && platform.history_index > 0);
        platform.connected_view->set_can_go_forward(!platform.history.empty() &&
                                                    platform.history_index + 1 < platform.history.size());
    }

    // The headless twin's fresh-content load, verbatim (see src/platform/headless/web_view_handler.cpp):
    // DecidePolicy(NewPage, cancellable) → truncate-forward + append → DidFinish(success), with
    // UpdateCanGoBackForward after both steps. Runs ONLY on the XAML-less host (header note) — with a
    // real control the wired NavigationStarting/NavigationCompleted report the real events instead.
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

    // MauiWebView.LoadHtml's post-EnsureCoreWebView2Async tail: prepend the <base>-tag insertion script
    // (base = the source's BaseUrl, else LocalScheme) and NavigateToString. Reads the html/base from the
    // dual-drive mirrors so the CoreWebView2Initialized replay needs no extra stash.
    // deferred: CoreWebView2.SetVirtualHostNameToFolderMapping("appdir", ApplicationPath, Allow) — the
    // mapBaseDirectory branch (MauiWebView.LoadHtml) needs the packaged/unpackaged app path; a fully
    // inline static html source resolves no relative resources (header note).
    void navigate_to_pending_html(web_view_platform& platform, muxc::WebView2 const& web_view)
    {
        const std::string base_url = platform.last_base_url.empty() ? std::string(k_local_scheme)
                                                                    : platform.last_base_url;
        const std::string html_with_script = base_tag_insertion_script(base_url) + "\n" + platform.last_html;
        try
        {
            web_view.NavigateToString(wnative::to_hstring_utf8(html_with_script));
        }
        catch (const winrt::hresult_error&)
        {
            // NavigateToString on a torn-down control — quiet degradation (the mirrors already hold the
            // content, the headless-shape story of every backend).
        }
    }

    // WebViewExtensions.UpdateUserAgent: CoreWebView2 == null → return (the CoreWebView2Initialized
    // replay re-runs it); a set virtual value writes CoreWebView2.Settings.UserAgent, an unset one reads
    // the platform default back into the virtual view.
    void apply_user_agent(web_view_platform& platform, maui::core::i_web_view& view)
    {
        const auto web_view = web_view_of(platform);
        const auto core = core_of(web_view);
        if (core == nullptr)
        {
            return;
        }
        try
        {
            auto settings = core.Settings();
            if (!view.user_agent().empty())
            {
                settings.UserAgent(wnative::to_hstring_utf8(view.user_agent()));
                return;
            }
            const std::string resolved = wnative::to_utf8(settings.UserAgent());
            platform.user_agent = resolved;
            view.set_user_agent(resolved); // re-enters map_user_agent on the set branch (terminates)
        }
        catch (const winrt::hresult_error&)
        {
            // Settings on a torn-down CoreWebView2 — keep the mirror.
        }
    }
} // namespace

namespace maui::core
{
    // Releases the one strong ref pinning the WebView2 (the wnative shape of the pimpl-owned-native
    // doctrine; the android twin deletes its JNI global ref here). Any handler token not revoked by
    // on_disconnect_handler drops with the control.
    web_view_platform::~web_view_platform()
    {
        wnative::release(native);
    }

    // ---- i_web_view_delegate (the source's load sink; MauiWebView.IWebViewDelegate.LoadHtml/LoadUrl) --

    void web_view_platform::load_html(std::string_view html, std::string_view base_url)
    {
        if (native == nullptr)
        {
            // XAML-less host: the headless twin verbatim — C# WK LoadHtml ignores a null html; an html
            // navigation's url is its base url ("about:blank" when none was supplied).
            if (html.empty())
            {
                return;
            }
            last_source_kind = web_view_source_kind::html;
            last_html = std::string(html);
            last_base_url = std::string(base_url);
            simulate_load(*this, last_base_url.empty() ? std::string("about:blank") : last_base_url);
            return;
        }
        // Real control: mirrors first (dual-drive), then MauiWebView.LoadHtml — which does NOT early-out
        // on an empty html (`$"{script}\n{html}"` navigates regardless, unlike the WK twin).
        last_source_kind = web_view_source_kind::html;
        last_html = std::string(html);
        last_base_url = std::string(base_url);
        const auto web_view = web_view_of(*this);
        if (web_view == nullptr)
        {
            return;
        }
        if (core_of(web_view) != nullptr)
        {
            pending_source_load = false;
            navigate_to_pending_html(*this, web_view);
            return;
        }
        // C# `await EnsureCoreWebView2Async()`: park the load and continue in CoreWebView2Initialized
        // (wired at connect); the mirrors above are the stash.
        pending_source_load = true;
        try
        {
            [[maybe_unused]] const auto ensure = web_view.EnsureCoreWebView2Async(); // fire-and-forget
        }
        catch (const winrt::hresult_error&)
        {
            // No WebView2 Runtime / no dispatcher — the pending load quietly never replays (the mirrors
            // keep the content observable, the standard degradation).
        }
    }

    void web_view_platform::load_url(std::string_view url)
    {
        if (native == nullptr)
        {
            // XAML-less host: the headless twin verbatim.
            last_source_kind = web_view_source_kind::url;
            last_url = std::string(url);
            simulate_load(*this, last_url);
            return;
        }
        last_source_kind = web_view_source_kind::url;
        last_url = std::string(url);
        const auto web_view = web_view_of(*this);
        if (web_view == nullptr)
        {
            return;
        }
        // MauiWebView.IWebViewDelegate.LoadUrl: a non-absolute uri is rebased under the appdir local
        // scheme (the folder mapping itself is the deferred item — header note); cookie sync is out of
        // scope. Assigning WebView2.Source implicitly initializes the CoreWebView2, so no pending stash
        // is needed on the url channel.
        std::string resolved(url);
        const bool has_scheme = resolved.find("://") != std::string::npos;
        if (!has_scheme)
        {
            resolved = std::string(k_local_scheme) + resolved;
        }
        try
        {
            web_view.Source(winrt::Windows::Foundation::Uri{wnative::to_hstring_utf8(resolved)});
        }
        catch (const winrt::hresult_error&)
        {
            // MauiWebView.LoadUrl's catch: Debug.WriteLine($"Failed to load: {uri} {exc}") — quiet
            // degradation on a malformed uri.
        }
    }

    // The generic-IView pushes (the shared view_mapper calls these through view_platform_base). Each
    // calls the base body FIRST — the headless mirrors must stay live for the XAML-less cross-platform
    // suite (header note) — then pushes to the real control when one exists.

    void web_view_platform::update_visibility(maui::core::visibility value)
    {
        view_platform_base::update_visibility(value);
        // ViewExtensions.UpdateVisibility (Windows): Hidden rides Opacity 0, Collapsed collapses.
        wnative::apply_visibility(native, value, alpha);
    }

    void web_view_platform::update_opacity(double value)
    {
        view_platform_base::update_opacity(value);
        // ViewExtensions.UpdateOpacity: the Hidden state pins opacity 0 (apply_visibility restores it).
        if (!hidden)
        {
            wnative::apply_opacity(native, value);
        }
    }

    void web_view_platform::update_automation_id(std::string_view value)
    {
        view_platform_base::update_automation_id(value);
        // ViewExtensions.UpdateAutomationId: AutomationProperties.SetAutomationId.
        wnative::apply_automation_id(native, value);
    }

    std::unique_ptr<web_view_platform> web_view_handler::create_platform_view()
    {
        auto platform = std::make_unique<web_view_platform>();
        try
        {
            // WebViewHandler.Windows CreatePlatformView: new MauiWebView(handler) — a
            // Microsoft.UI.Xaml.Controls.WebView2. MauiWebView's ctor NavigationStarting hook (the
            // appdir virtual-host auto map/unmap) rides the deferred folder mapping (header note).
            const muxc::WebView2 web_view;
            platform->native = wnative::store(web_view); // released in ~web_view_platform
        }
        catch (const winrt::hresult_error&)
        {
            platform->native = nullptr; // XAML-less degradation (header note)
        }
        return platform;
    }

    void web_view_handler::on_connect_handler(web_view_platform& platform)
    {
        platform.connected_view = virtual_view();
        // C# ConnectHandler: _navigationResult = WebNavigationResult.Success.
        platform.navigation_cancelled = false;
        const auto web_view = web_view_of(platform);
        if (web_view == nullptr)
        {
            return; // XAML-less host: the simulated navigation channel is the whole story
        }
        // The native events route through the platform callbacks (the peer is the platform struct,
        // whose heap address is stable until disconnect revokes these handlers — the button precedent).
        auto* peer = &platform;

        // WebView2Proxy.Connect: platformView.CoreWebView2Initialized += OnCoreWebView2Initialized. The
        // core-level events (HistoryChanged/NavigationStarting/NavigationCompleted/ProcessFailed) that
        // C# hooks inside it are wired below on the CONTROL's own forwarding events instead (same args
        // types, one revocation seam); HistoryChanged/ProcessFailed are the deferred remainder (header).
        const winrt::event_token init_token = web_view.CoreWebView2Initialized(
            [peer](muxc::WebView2 const& sender, muxc::CoreWebView2InitializedEventArgs const&) {
                if (core_of(sender) == nullptr)
                {
                    return; // initialization failed (args.Exception) — stay on the mirrors
                }
                // OnCoreWebView2Initialized tail: sender.UpdateUserAgent(VirtualView) — then the parked
                // LoadHtml replay (the port's `await EnsureCoreWebView2Async()` bridge).
                // deferred: UpdateBackground (DefaultBackgroundColor) + SyncPlatformCookies.
                if (peer->connected_view != nullptr)
                {
                    apply_user_agent(*peer, *peer->connected_view);
                }
                if (peer->pending_source_load && peer->last_source_kind == web_view_source_kind::html)
                {
                    navigate_to_pending_html(*peer, sender);
                }
                peer->pending_source_load = false;
            });
        platform.core_initialized_token = init_token.value;

        // WebViewHandler.OnNavigationStarting: only an absolute Uri reaches Navigating (WebView2 reports
        // absolute uris; the empty guard is the port's TryCreate stand-in); args.Cancel follows the
        // virtual view's verdict, and a cancel resets the event state + arms the NavigationCompleted
        // skip (_navigationResult = Cancel).
        const winrt::event_token starting_token = web_view.NavigationStarting(
            [peer](muxc::WebView2 const&, wv2core::CoreWebView2NavigationStartingEventArgs const& args) {
                const std::string uri = wnative::to_utf8(args.Uri());
                if (peer->connected_view == nullptr || uri.empty())
                {
                    return;
                }
                const bool cancel = peer->connected_view->send_navigating(peer->current_navigation_event, uri);
                args.Cancel(cancel);
                if (cancel)
                {
                    peer->current_navigation_event = web_navigation_event::new_page;
                    peer->navigation_cancelled = true;
                }
                else
                {
                    peer->navigation_cancelled = false;
                }
            });
        platform.navigation_starting_token = starting_token.value;

        // WebView2Proxy.OnNavigationCompleted → NavigationSucceeded / NavigationFailed → SendNavigated
        // (cookie sync out of scope): skipped entirely after a cancelled start (C#'s
        // `_navigationResult is not WebNavigationResult.Cancel`), then UpdateCanGoBackForward and the
        // CurrentNavigationEvent = NewPage reset (SendNavigated's tail).
        const winrt::event_token completed_token = web_view.NavigationCompleted(
            [peer](muxc::WebView2 const& sender, wv2core::CoreWebView2NavigationCompletedEventArgs const& args) {
                if (peer->navigation_cancelled || peer->connected_view == nullptr)
                {
                    return;
                }
                std::string uri;
                try
                {
                    // C# reads sender.Source (the CoreWebView2's current source) — the control's Source
                    // mirrors it.
                    if (const auto source = sender.Source())
                    {
                        uri = wnative::to_utf8(source.AbsoluteUri());
                    }
                }
                catch (const winrt::hresult_error&)
                {
                    // torn-down control — fall through with the empty uri (no Navigated, like C#)
                }
                if (args.IsSuccess())
                {
                    // NavigationSucceeded: SendNavigated(Success) when the source resolves; the
                    // UpdateCanGoBackForward runs either way.
                    if (!uri.empty())
                    {
                        peer->connected_view->send_navigated(peer->current_navigation_event, uri,
                                                             web_navigation_result::success);
                    }
                    update_can_go_back_forward(*peer);
                }
                else if (!uri.empty())
                {
                    // NavigationFailed: SendNavigated(Failure).
                    peer->connected_view->send_navigated(peer->current_navigation_event, uri,
                                                         web_navigation_result::failure);
                    update_can_go_back_forward(*peer);
                }
                peer->current_navigation_event = web_navigation_event::new_page;
            });
        platform.navigation_completed_token = completed_token.value;
        // deferred: the OnLoaded window wiring (WebView2Proxy.Connect(window) → window.Closed →
        // Disconnect) — the port's page teardown drives on_disconnect_handler directly.
    }

    void web_view_handler::on_disconnect_handler(web_view_platform& platform)
    {
        platform.connected_view = nullptr;
        platform.pending_source_load = false;
        const auto web_view = web_view_of(platform);
        if (web_view != nullptr)
        {
            // WebView2Proxy.Disconnect: CoreWebView2Initialized -= …; the navigation handlers unhook
            // (wired on the control here — connect note); then WebViewHandler.Disconnect closes the
            // control when a CoreWebView2 exists (Close subsumes the proxy's webView2.Stop()).
            if (platform.core_initialized_token != 0)
            {
                web_view.CoreWebView2Initialized(winrt::event_token{platform.core_initialized_token});
            }
            if (platform.navigation_starting_token != 0)
            {
                web_view.NavigationStarting(winrt::event_token{platform.navigation_starting_token});
            }
            if (platform.navigation_completed_token != 0)
            {
                web_view.NavigationCompleted(winrt::event_token{platform.navigation_completed_token});
            }
            try
            {
                if (core_of(web_view) != nullptr)
                {
                    web_view.Close();
                }
            }
            catch (const winrt::hresult_error&)
            {
                // already closed/torn down — idempotent teardown
            }
        }
        platform.core_initialized_token = 0;
        platform.navigation_starting_token = 0;
        platform.navigation_completed_token = 0;
    }

    // WebViewHandler.MapSource + WebViewExtensions.UpdateSource: the platform view is the
    // i_web_view_delegate the source loads into, then UpdateCanGoBackForward.
    void web_view_handler::map_source(web_view_handler& handler, i_web_view& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        if (auto* source = view.source())
        {
            source->load(*platform); // → load_html / load_url on the platform (real WebView2 + mirror)
        }
        update_can_go_back_forward(*platform);
    }

    // WebViewHandler.MapUserAgent + WebViewExtensions.UpdateUserAgent: bidirectional
    // CoreWebView2.Settings.UserAgent sync; C# early-returns while CoreWebView2 is null and the
    // CoreWebView2Initialized replay covers the gap. The user_agent mirror is always kept.
    void web_view_handler::map_user_agent(web_view_handler& handler, i_web_view& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->user_agent = std::string(view.user_agent()); // headless mirror first
        apply_user_agent(*platform, view);
    }

    // ---- navigation commands (WebViewHandler.CommandMapper) ----
    // With a live CoreWebView2 the native calls run and the wired events report Navigating/Navigated;
    // otherwise (the XAML-less host / pre-init) the headless twin's synchronous simulation replays so
    // the cross-platform suite observes exactly the headless behavior (header note).

    // WebViewHandler.MapGoBack (CurrentNavigationEvent = Back when CanGoBack) +
    // WebViewExtensions.UpdateGoBack (CanGoBack → GoBack, then UpdateCanGoBackForward).
    void web_view_handler::map_go_back(web_view_handler& handler, i_web_view& view, const std::any& /*args*/)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        const auto web_view = web_view_of(*platform);
        if (core_of(web_view) != nullptr)
        {
            try
            {
                if (web_view.CanGoBack())
                {
                    platform->current_navigation_event = web_navigation_event::back;
                    web_view.GoBack(); // the wired NavigationStarting/Completed report the events
                }
            }
            catch (const winrt::hresult_error&)
            {
                // torn-down control — nothing to navigate
            }
            update_can_go_back_forward(*platform);
            return;
        }
        // Headless simulation (the headless twin verbatim).
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

    // WebViewHandler.MapGoForward + WebViewExtensions.UpdateGoForward (the forward twin of map_go_back).
    void web_view_handler::map_go_forward(web_view_handler& handler, i_web_view& view, const std::any& /*args*/)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        const auto web_view = web_view_of(*platform);
        if (core_of(web_view) != nullptr)
        {
            try
            {
                if (web_view.CanGoForward())
                {
                    platform->current_navigation_event = web_navigation_event::forward;
                    web_view.GoForward();
                }
            }
            catch (const winrt::hresult_error&)
            {
                // torn-down control — nothing to navigate
            }
            update_can_go_back_forward(*platform);
            return;
        }
        // Headless simulation (the headless twin verbatim).
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

    // WebViewHandler.MapReload (CurrentNavigationEvent = Refresh; cookie sync out of scope) +
    // WebViewExtensions.UpdateReload: platformWebView.Reload().
    void web_view_handler::map_reload(web_view_handler& handler, i_web_view& view, const std::any& /*args*/)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        const auto web_view = web_view_of(*platform);
        if (core_of(web_view) != nullptr)
        {
            platform->current_navigation_event = web_navigation_event::refresh;
            ++platform->reload_count; // dual-drive mirror
            try
            {
                web_view.Reload();
            }
            catch (const winrt::hresult_error&)
            {
                // torn-down control — nothing to reload
            }
            return;
        }
        // Headless simulation (the headless twin verbatim): re-navigate the current entry in place.
        platform->current_navigation_event = web_navigation_event::refresh;
        if (platform->history.empty())
        {
            return; // an empty web view reloads to nothing — no navigation callbacks fire
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

    // WebViewHandler.MapEval + WebViewExtensions.Eval: fire-and-forget ExecuteScriptAsync (C# hops
    // through DispatcherQueue.TryEnqueue; the port's mapper flow already runs on the UI thread).
    void web_view_handler::map_eval(web_view_handler& handler, i_web_view& /*view*/, const std::any& args)
    {
        auto* platform = handler.typed_platform_view();
        const auto* script = std::any_cast<std::string>(&args);
        if (platform == nullptr || script == nullptr)
        {
            return;
        }
        platform->eval_scripts.push_back(*script); // headless mirror first
        const auto web_view = web_view_of(*platform);
        if (core_of(web_view) == nullptr)
        {
            return; // pre-init / XAML-less: the mirror is the whole story (C#'s awaited call would throw)
        }
        try
        {
            [[maybe_unused]] const auto operation =
                web_view.ExecuteScriptAsync(wnative::to_hstring_utf8(*script)); // result discarded (MapEval)
        }
        catch (const winrt::hresult_error&)
        {
            // torn-down control — the mirror keeps the script observable
        }
    }

    // WebViewHandler.MapEvaluateJavaScriptAsync: record the script and complete the request through the
    // canned-result seam (unset => "null"), like the android twin.
    // deferred: the native round trip — WebViewExtensions.EvaluateJavaScript's
    // request.RunAndReport(ExecuteScriptAsync(script)) needs the IAsyncOperation Completed-handler
    // bridge into evaluate_java_script_request.
    void web_view_handler::map_evaluate_java_script(web_view_handler& handler, i_web_view& /*view*/,
                                                    const std::any& args)
    {
        const auto* request_ptr = std::any_cast<std::shared_ptr<evaluate_java_script_request>>(&args);
        if (request_ptr == nullptr || *request_ptr == nullptr)
        {
            return;
        }
        const std::shared_ptr<evaluate_java_script_request> request = *request_ptr;
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            // C# SetCanceled when the platform view is gone — surfaced as the "null" result (see
            // evaluate_java_script_request.hpp).
            request->complete("null");
            return;
        }
        platform->eval_scripts.push_back(request->script());
        std::string result =
            platform->eval_result_provider ? platform->eval_result_provider(request->script()) : "null";
        request->complete(std::move(result));
    }

    maui::graphics::size web_view_handler::get_desired_size(double width_constraint, double height_constraint) const
    {
        const auto* platform = typed_platform_view();
        if (platform == nullptr)
        {
            return {0, 0};
        }
        if (platform->native == nullptr)
        {
            // XAML-less degradation: the headless MinimumSize (44) fallback per dimension when the
            // constraint is unbounded or non-positive (the WebViewHandler.iOS.GetDesiredSize shape the
            // headless twin mirrors), so the host suite observes the headless numbers.
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
        // ViewHandlerExtensions.Windows.cs GetDesiredSizeFromHandler: FrameworkElement.Measure +
        // DesiredSize with the AdjustForExplicitSize clamp fed from the virtual view's explicit
        // width()/height() (see measure_native). C#'s Windows partial has NO MinimumSize override — the
        // 44 floor is the iOS/Tizen partials' own, so the native path takes the raw measure.
        const double explicit_width = virtual_view() != nullptr ? virtual_view()->width() : dimension::unset;
        const double explicit_height = virtual_view() != nullptr ? virtual_view()->height() : dimension::unset;
        return wnative::measure_native(platform->native, width_constraint, height_constraint, explicit_width,
                                       explicit_height);
    }

    void web_view_handler::platform_arrange(const maui::graphics::rect& frame)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return; // XAML-less: no native layout to apply
        }
        // The shared Canvas recipe: Canvas.SetLeft/SetTop + explicit Width/Height pin the control to the
        // frame (ViewHandlerExtensions.Windows.cs PlatformArrangeHandler on the Canvas layout model).
        wnative::arrange_native(platform->native, frame);
    }
} // namespace maui::core
