#pragma once
// maui::core::web_view_handler  <=  Microsoft.Maui.Handlers.WebViewHandler
//
// The handler for a view presenting HTML content. The Source property flows virtual→native through the
// property mapper (map_source hands the platform view — which implements i_web_view_delegate, like C#'s
// MauiWKWebView — to i_web_view_source::load); the navigation operations and JavaScript evaluation are
// COMMANDS ("go_back"/"go_forward"/"reload"/"eval"/"evaluate_java_script", mirroring
// WebViewHandler.CommandMapper); navigation state flows native→virtual via send_navigating/send_navigated
// + set_can_go_back/set_can_go_forward (the MauiWebViewNavigationDelegate +
// WebViewExtensions.UpdateCanGoBackForward pipeline). Ported from WebViewHandler.cs (cross-platform) +
// WebViewHandler.iOS.cs (the WKWebView recipe, shared by BOTH Apple backends).
//
// Partial-class split (PROFILE §5): the mapper TABLES + ctor are cross-platform (web_view_handler.cpp);
// the platform recipe — create/connect/disconnect/map_*/measure — is per backend. The apple and ios
// backends share ONE WKWebView .mm (src/platform/apple_shared/web_view_handler.mm — WebKit's API is the
// same on AppKit and UIKit, the analog of C#'s single WebViewHandler.iOS.cs covering iOS + Mac Catalyst);
// the headless partial (src/platform/headless/web_view_handler.cpp) mirrors the WKWebView pipeline
// synchronously over an in-memory back-forward list so the control logic is fully oracle-testable.
//
// web_view_platform is the single cross-platform platform-view struct: `native` holds the WKWebView*
// (retained in the .mm; unused headless), and it IS the i_web_view_delegate the source loads into
// (C# MauiWKWebView : WKWebView, IWebViewDelegate — load_url/load_html are backend-defined). The
// headless mirrors record the loaded source, the simulated back-forward list, the navigation-command
// counters and every evaluated script; eval_result_provider is the canned-result seam headless eval
// round trips complete through.
//
// UserAgent flows through map_user_agent (WebViewHandler.iOS.MapUserAgent → WebViewExtensions
// UpdateUserAgent): bidirectional — write WKWebView.CustomUserAgent when the virtual view has a value,
// else read the platform's CustomUserAgent / default `userAgent` back into the virtual view. The
// apple_shared .mm also installs a WKUIDelegate that presents the native JS alert / confirm / prompt
// dialogs (MauiWebViewUIDelegate); both are no-ops headless (the user_agent mirror is stored for tests).
//
// OUT OF SCOPE (documented, not stubbed): cookie sync (the bulk of WebViewHandler.iOS.cs) and the
// Android/Windows client mappings.

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "maui/core/command_mapper.hpp"
#include "maui/core/i_web_view.hpp"
#include "maui/core/i_web_view_delegate.hpp"
#include "maui/core/move_only_function.hpp"
#include "maui/core/property_mapper.hpp"
#include "maui/core/view_handler.hpp"
#include "maui/core/view_platform_base.hpp"
#include "maui/core/visibility.hpp"
#include "maui/core/web_navigation_event.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

namespace maui::core
{
    // Which delegate channel the last source load used (headless mirror).
    enum class web_view_source_kind : std::uint8_t
    {
        none = 0,
        url,
        html
    };

    struct web_view_platform : view_platform_base, i_web_view_delegate
    {
        web_view_platform() = default;
        ~web_view_platform() override; // backend-defined: releases the retained WKWebView on apple/ios
        web_view_platform(const web_view_platform&) = delete;
        web_view_platform(web_view_platform&&) = delete;
        web_view_platform& operator=(const web_view_platform&) = delete;
        web_view_platform& operator=(web_view_platform&&) = delete;

        void* native = nullptr;

        // ---- i_web_view_delegate (backend-defined) ----
        // The platform view doubles as the source's load sink (C# MauiWKWebView : IWebViewDelegate):
        // apple/ios forward to WKWebView loadHTMLString:/loadRequest:; headless records the content and
        // runs the synchronous navigation simulation against the in-memory back-forward list.
        void load_html(std::string_view html, std::string_view base_url) override;
        void load_url(std::string_view url) override;

        // The connected virtual view (wired by on_connect_handler, cleared on disconnect) — the inbound
        // channel the (simulated or real) navigation drives via send_navigating/send_navigated +
        // set_can_go_back/set_can_go_forward.
        i_web_view* connected_view = nullptr;

        // MauiWebViewNavigationDelegate.CurrentNavigationEvent: which event kind the next BackForward /
        // Reload navigation reports (map_go_back/map_go_forward/map_reload set it before navigating).
        web_navigation_event current_navigation_event = web_navigation_event::new_page;

        // ---- headless mirrors (the apple/ios build pushes to `native` instead) ----
        // The user agent map_user_agent last synced (the headless stub stores it for test inspection;
        // the apple/ios build pushes it to WKWebView.CustomUserAgent and reads the default back here).
        std::string user_agent;
        web_view_source_kind last_source_kind = web_view_source_kind::none;
        std::string last_url;
        std::string last_html;
        std::string last_base_url;
        // The simulated WKBackForwardList: the visited urls + the current position (valid when
        // !history.empty()). can_go_back/can_go_forward derive from it after every navigation.
        std::vector<std::string> history;
        std::size_t history_index = 0;
        int reload_count = 0;
        // Every script handed to "eval" / "evaluate_java_script" (the wrapped form the control built).
        std::vector<std::string> eval_scripts;
        // The canned-result seam an "evaluate_java_script" command completes through on headless
        // (unset => "null", the platform's errored/void-script value).
        move_only_function<std::string(const std::string& script)> eval_result_provider;

#ifdef MAUI_PLATFORM_APPLE
        // Apple backend: push the generic IView properties to the WKWebView (defined in
        // src/platform/apple_shared/web_view_handler.mm). is_enabled keeps the base mirror — an AppKit
        // WKWebView is not an NSControl and has no enabled state (the layout_platform precedent).
        void update_visibility(maui::core::visibility value) override;
        void update_opacity(double value) override;
        void update_automation_id(std::string_view value) override;
        void update_transform(const maui::core::transform_spec& value) override;
        void update_flow_direction(maui::core::flow_direction value) override;
        void update_background(const maui::graphics::paint* value) override;
        void update_shadow(const maui::core::i_shadow* value) override;
        void update_clip(const maui::graphics::i_shape* value) override;
        void update_semantics(const maui::core::semantics* value) override;
        void update_input_transparent(bool value) override;
#endif

#ifdef MAUI_PLATFORM_IOS
        // iOS backend (M6 fan-out convention): the four fundamental IView pushes onto the UIKit
        // WKWebView; the remaining generic-IView pushes keep the view_platform_base mirrors until the
        // shared ios op helpers land (see port/STATUS.md).
        void update_visibility(maui::core::visibility value) override;
        void update_opacity(double value) override;
        void update_is_enabled(bool value) override;
        void update_automation_id(std::string_view value) override;
#endif

#ifdef MAUI_PLATFORM_ANDROID
        // Android backend (the M-android per-control fan-out): push the generic IView properties to the
        // real android.webkit.WebView (defined in src/platform/android/web_view_handler.cpp). visibility/
        // opacity/automation_id map onto the View surface; transform/flow_direction/background/semantics
        // route through the shared android ops. is_enabled keeps the base mirror (a WebView's interactivity
        // is governed by its content; ViewExtensions has no MapIsEnabled for it — the apple-twin precedent),
        // and shadow/clip/input_transparent have no plain-View analog (WrapperView-only in C#), so they too
        // keep ONLY the headless mirror (the progress_bar/image partial precedent).
        void update_visibility(maui::core::visibility value) override;
        void update_opacity(double value) override;
        void update_automation_id(std::string_view value) override;
        void update_transform(const maui::core::transform_spec& value) override;
        void update_flow_direction(maui::core::flow_direction value) override;
        void update_background(const maui::graphics::paint* value) override;
        void update_semantics(const maui::core::semantics* value) override;
#endif

#ifdef MAUI_PLATFORM_WINDOWS
        // Windows (WinUI 3) backend (src/platform/windows/web_view_handler.cpp): `native` holds a real
        // Microsoft.UI.Xaml.Controls.WebView2 (WebViewHandler.Windows.cs + Platform/Windows/
        // MauiWebView.cs). The generic-IView pushes call the view_platform_base body FIRST — the windows
        // preset also runs the cross-platform suite on the host WITHOUT a XAML runtime
        // (create_platform_view degrades to a null native there), and on that host the partial ALSO
        // replays the headless twin's synchronous navigation simulation so the suite observes exactly
        // the headless behavior — then push to the control when one exists. is_enabled keeps the base
        // mirror (a WebView2 is a FrameworkElement, not a Control); transform / flow_direction /
        // background / semantics / shadow / clip / input_transparent keep the base mirrors in this
        // first cut (deferred; C#'s Windows MapFlowDirection deliberately no-ops anyway).
        void update_visibility(maui::core::visibility value) override;
        void update_opacity(double value) override;
        void update_automation_id(std::string_view value) override;
        // Native event wiring state (NO winrt types in shared headers — opaque ints only): the
        // NavigationStarting / NavigationCompleted / CoreWebView2Initialized winrt::event_token values
        // (the WebView2Proxy wiring of WebViewHandler.Windows.cs), revoked in on_disconnect_handler
        // (the dtor releases the control, dropping any remainder with it).
        std::int64_t navigation_starting_token = 0;
        std::int64_t navigation_completed_token = 0;
        std::int64_t core_initialized_token = 0;
        // C#'s _navigationResult = WebNavigationResult.Cancel gate: NavigationStarting sets it when the
        // virtual view cancelled; NavigationCompleted then skips send_navigated (the proxy's
        // `is not WebNavigationResult.Cancel` check).
        bool navigation_cancelled = false;
        // MauiWebView.LoadHtml's `await EnsureCoreWebView2Async()` gap: NavigateToString needs a live
        // CoreWebView2, so an HTML load issued before initialization parks here and the
        // CoreWebView2Initialized handler replays it from the last_html/last_base_url mirrors.
        bool pending_source_load = false;
#endif
    };

    class web_view_handler : public view_handler<web_view_handler, i_web_view, web_view_platform>
    {
    public:
        web_view_handler();

        static property_mapper<i_web_view, web_view_handler>& mapper();
        static command_mapper<i_web_view, web_view_handler>& command_mapper();

        // Platform recipe (per backend).
        static std::unique_ptr<web_view_platform> create_platform_view();
        void on_connect_handler(web_view_platform& platform);
        static void on_disconnect_handler(web_view_platform& platform);

        [[nodiscard]] maui::graphics::size get_desired_size(double width_constraint,
                                                            double height_constraint) const override;
        void platform_arrange(const maui::graphics::rect& frame) override;

        // C# WebViewHandler.MinimumSize: the fallback edge used when the platform measures 0 under an
        // unbounded constraint (GetDesiredSize's `width = MinimumSize` branches).
        static constexpr double minimum_size = 44.0;

        // Property map (platform recipe): Source?.Load(platform-as-delegate) + UpdateCanGoBackForward.
        static void map_source(web_view_handler& handler, i_web_view& view);
        // WebViewHandler.iOS.MapUserAgent: bidirectional CustomUserAgent sync (WebViewExtensions
        // UpdateUserAgent). Headless stores the value in platform->user_agent (no native sync).
        static void map_user_agent(web_view_handler& handler, i_web_view& view);

        // Command map (platform recipe) — WebViewHandler.CommandMapper's entries.
        static void map_go_back(web_view_handler& handler, i_web_view& view, const std::any& args);
        static void map_go_forward(web_view_handler& handler, i_web_view& view, const std::any& args);
        static void map_reload(web_view_handler& handler, i_web_view& view, const std::any& args);
        // args: a std::string — the script (fire-and-forget; C# MapEval).
        static void map_eval(web_view_handler& handler, i_web_view& view, const std::any& args);
        // args: a std::shared_ptr<evaluate_java_script_request> (C# MapEvaluateJavaScriptAsync).
        static void map_evaluate_java_script(web_view_handler& handler, i_web_view& view, const std::any& args);
    };
} // namespace maui::core
