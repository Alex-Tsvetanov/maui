// hybrid_web_view_handler — WinUI 3 platform partial: a REAL Microsoft.UI.Xaml.Controls.WebView2, the
// same native type HybridWebViewHandler.Windows.cs creates (`ViewHandler<IHybridWebView, WebView2>`, :16).
// The windows twin of src/platform/android/hybrid_web_view_handler.cpp (android.webkit.WebView) and
// src/platform/apple_shared/hybrid_web_view_handler.mm (WKWebView); it REPLACES the headless mirror
// (src/platform/headless/hybrid_web_view_handler.cpp), which had no native view and rendered nothing.
//
// WHAT THE BOARD ACTUALLY SHOWS — MEASURED, and it is the whole design of this file:
//   docs/comparison/captures/windows/maui/hybrid_web_view_{light,dark}.png render WebView2's OWN built-in
//   error page — a torn-page glyph, "This 0.0.0.1 page can't be found", "No webpage was found for the web
//   address: https://0.0.0.1/", a small "HTTP ERROR 404" label and a Refresh button — and render it
//   IDENTICALLY in both themes (cell region [252:780, 20:1004] mean 242.17 in light AND in dark; a browser
//   error page is chrome, not app content, so the OS theme never touches it).
//   The cause is on the MAUI side: maui-reference/pages/hybrid_web_view.xaml:28 sets
//   HybridRoot="HybridSamplePage", and NO such asset folder exists — the reference app's Resources/Raw
//   holds only welcome.html + AboutAssets.txt, which is all MauiReference.csproj:84's
//   <MauiAsset Include="Resources\Raw\**"> glob can pick up. So MAUI's own GetResponseStreamAsync misses
//   on the FIRST request and takes its 404 branch (HybridWebViewHandler.Windows.cs:262).
//
//   => PARITY HERE IS A REAL WebView2 THAT 404s, NOT AN ASSET FILE SERVER. Serving HybridRoot would be
//   dead code (there is nothing to serve, on either side, and maui_add_app.cmake copies RESOURCES flat
//   with no directory support) AND a divergence: the port would render content MAUI does not.
//   The android partial already made this exact ruling — see its SCOPE note.
//
//   THE ERROR PAGE ITSELF IS THE TARGET, not merely a painted cell. Scored against the frozen reference
//   with tools/parity/pixel_score.py: MAUI's own 404 rect composited into the port frame reads 0.21%
//   light / 0.19% dark, while a plain white cell reads 2.26% / 2.24% — RED on both. Only the identical
//   error page passes, which is why the intercept below is not optional.
//
// WHY THE INTERCEPT: it is what the ORACLE does (:325 AddWebResourceRequestedFilter, :147-151
// CreateWebResourceResponse, :262 the 404), so it is ported for fidelity. A no-intercept shortcut
// (navigate and let the connection fail) would additionally risk Chromium's CONNECTION-failure page
// rather than the SERVER-404 page — but that prediction is unverified and is not what this rests on.
//
// DOCUMENTED DEVIATIONS from the C# Windows oracle, each an infrastructure gap, not a behaviour guess:
//   - No asset serving (see above): the AppOrigin branch answers 404 unconditionally instead of walking
//     HybridRoot/DefaultFile. Also no `_framework/hybridwebview.js` and no `__hwvInvokeDotNet` endpoint —
//     the latter is the port-wide no-reflection deferral (PROFILE §6; i_hybrid_web_view.hpp).
//   - No deferral. C#'s GetDeferral()/Complete() pair (:130, :154) exists only to bridge its AWAITED asset
//     read. This response is produced inline, so there is nothing to defer.
//   - The three command maps keep their HEADLESS-MIRROR bodies (copied verbatim from
//     src/platform/headless/hybrid_web_view_handler.cpp), exactly as the android partial does: the page is
//     a 404, so there is no document to script against and `window.HybridWebView.__InvokeJavaScript` is
//     never defined. The mirrors keep message_received / create_invoke_task and the cross-platform
//     round-trip suite observing the recipe. THE UPGRADE PATH IS NOT UNIFORM: the day assets are deployed,
//     map_send_raw_message becomes `CoreWebView2.PostWebMessageAsString` (MauiHybridWebView.cs:24-26 — a
//     web message, NOT a script evaluation), while invoke/evaluate become ExecuteScriptAsync.
//   - No custom CoreWebView2Environment. The oracle's :307-320 CreateWithOptionsAsync +
//     CreateCoreWebView2ControllerOptions + WebViewInitializationStartedEventArgs path is POST-10.0.71
//     `src/` (parity ruling 11: the local src/ is newer than the shipped MauiVersion 10.0.71 the board
//     renders). The bare EnsureCoreWebView2Async() used here is both the safer bet and render-identical.
//     Same for :119's WebRequestInterceptingWebView.TryInterceptResponseStream app-override hook.
//   - Generic-IView pushes are NOT overridden: hybrid_web_view_platform declares those overrides only
//     under MAUI_PLATFORM_APPLE / MAUI_PLATFORM_IOS, so view_platform_base's mirrors stand — the same
//     state the android partial documents, and exact for this page (the cell is visible, opaque,
//     untransformed, and paints its own error page over its own background).
//   - MauiHybridWebView is not subclassed. It carries WebViewReadyTask/RunAfterInitialize and implements
//     IHybridPlatformWebView.SendRawMessage, but adds no RENDERING behaviour, so a plain WebView2 is the
//     faithful native type for a parity render.

#include "maui/controls/hybrid_web_view_handler.hpp"

// C++/WinRT include rule (winui_interop.hpp): the FULL header for every namespace whose MEMBERS are
// called, or the call fails with C3779, which names neither the header nor the concept.
#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Microsoft.Web.WebView2.Core.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Storage.Streams.h>

// AFTER the winrt block (the GetCurrentTime macro trap image_source_services.cpp documents); it is what
// supplies FAILED() below.
#include <windows.h>

#include <algorithm>
#include <any>
#include <cmath>
#include <limits>
#include <memory>
#include <optional>
#include <string>
#include <utility>

#include "maui/core/evaluate_java_script_request.hpp"
#include "maui/core/hybrid_web_view_protocol.hpp"
#include "maui/core/i_hybrid_web_view.hpp"
#include "maui/core/invoke_java_script_request.hpp"
#include "maui/core/view_chrome_ops.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"
#include "winui_interop.hpp"

namespace
{
    // Named `winui`, NOT `xaml` — see progress_bar_handler.cpp's identical note (the port's own maui::xaml
    // XAML-loader namespace would shadow a file-scope `xaml` alias inside namespace maui::*).
    namespace winui = winrt::Microsoft::UI::Xaml;
    namespace webview2 = winrt::Microsoft::Web::WebView2::Core;
    using web_view_control = winui::Controls::WebView2;

    // HybridWebViewHandler.cs:48-61 — AppHostAddress "0.0.0.1" + AppHostScheme "https" (the "app" scheme
    // is #if IOS || MACCATALYST only), i.e. AppOrigin == "https://0.0.0.1/". ONE spelling of this oracle
    // constant so the navigate and the intercept below can never drift apart.
    constexpr std::string_view k_app_origin = "https://0.0.0.1/";

    web_view_control as_web_view(void* native)
    {
        return maui::platform::windows::ref<winui::UIElement>(native).as<web_view_control>();
    }
} // namespace

namespace maui::controls
{
    hybrid_web_view_platform::~hybrid_web_view_platform()
    {
        if (native == nullptr)
        {
            return;
        }
        // Revoke even if disconnect never ran — the WebMessageReceived lambda captures the HANDLER, and
        // button_handler.hpp's click_token note is the house rule this follows.
        if (web_message_token != 0)
        {
            as_web_view(native).WebMessageReceived(winrt::event_token{web_message_token});
            web_message_token = 0;
        }
        maui::platform::windows::drop<winui::UIElement>(native);
    }

    std::unique_ptr<hybrid_web_view_platform> hybrid_web_view_handler::create_platform_view()
    {
        // HybridWebViewHandler.CreatePlatformView (:20-23). The IInitializationAwareWebView hooks fire
        // around creation exactly as the headless and apple twins do.
        if (auto* view = virtual_view())
        {
            view->web_view_initialization_started();
        }
        auto platform = std::make_unique<hybrid_web_view_platform>();
        try
        {
            // Activating WebView2 needs the projected Microsoft.Web.WebView2.Core surface + the WebView2
            // Runtime; if either is missing the activation throws and the handler degrades to the
            // mirror-only behaviour instead of taking the page down (the sibling web_view partial's shape).
            const web_view_control web;
            platform->native = maui::platform::windows::take<winui::UIElement>(web);
        }
        catch (const winrt::hresult_error&)
        {
        }
        if (auto* view = virtual_view())
        {
            view->web_view_initialization_completed();
        }
        return platform;
    }

    void hybrid_web_view_handler::on_connect_handler(hybrid_web_view_platform& platform)
    {
        platform.connected_view = virtual_view();
        if (platform.native == nullptr)
        {
            return;
        }
        const web_view_control web = as_web_view(platform.native);

        // ---- JS -> native ---------------------------------------------------------------------------
        // HybridWebViewHandler.Windows.cs:331 subscribes WebMessageReceived; :99-102 funnels
        // TryGetWebMessageAsString into MessageReceived (-> HybridWebViewHelper.ProcessRawMessage, which
        // this port spells hybrid_web_view_handler::message_received). Native on WebView2, unlike android,
        // whose partial had to defer this for want of a dev.mauicpp @JavascriptInterface trampoline.
        // The lambda captures the HANDLER, so the token is stored and revoked in on_disconnect_handler and
        // ~hybrid_web_view_platform — button_platform::click_token's discipline verbatim.
        auto* const owner = this;
        platform.web_message_token =
            web.WebMessageReceived(
                   [owner](const web_view_control&, const webview2::CoreWebView2WebMessageReceivedEventArgs& args) {
                       owner->message_received(maui::platform::windows::to_utf8(args.TryGetWebMessageAsString()));
                   })
                .value;

        // ---- initialization -------------------------------------------------------------------------
        // The oracle starts EnsureCoreWebView2Async from ConnectHandler (:301 -> :304-335), i.e. BEFORE
        // the control is Loaded (:36-38 hooks Loaded only to grab the Window for the Closed teardown),
        // then sets Source once it completes (:40-48). So initializing an UNPARENTED WebView2 is proven
        // safe by the oracle, not assumed here — and setting Source INSIDE the completion (rather than
        // before it) removes any question of the filter racing the first navigation.
        //
        // Completion is observed through the XAML control's CoreWebView2Initialized event rather than the
        // IAsyncAction: it is a XAML event and is therefore raised ON THE UI THREAD, so no
        // DispatcherQueue.TryEnqueue hop is needed. (Contrast image_source_services.cpp, whose hop exists
        // precisely because an HttpClient completion lands on a threadpool thread.) One-shot and
        // self-revoking through check_box_handler.cpp's shared_ptr<event_token> idiom, so it needs no
        // field on the platform struct.
        auto init_token = std::make_shared<winrt::event_token>();
        *init_token =
            web.CoreWebView2Initialized([init_token](const web_view_control& sender,
                                                     const winui::Controls::CoreWebView2InitializedEventArgs& args) {
                sender.CoreWebView2Initialized(*init_token);
                // Exception() is a winrt::hresult (S_OK on success), NOT a nullable reference — comparing
                // it to nullptr does not compile. On failure (no WebView2 Runtime, a locked or read-only
                // user-data folder) CoreWebView2() is null and every call below would be a null-vtable
                // dereference, so bail: the cell stays empty — today's behaviour, not a new crash.
                if (FAILED(args.Exception()))
                {
                    return;
                }
                const auto core = sender.CoreWebView2();
                if (core == nullptr)
                {
                    return;
                }
                // :323. (:322 AreDevToolsEnabled follows HybridWebViewDeveloperTools, which defaults to
                // disabled — the WinUI default — so it is deliberately not pushed.)
                core.Settings().IsWebMessageEnabled(true);
                // :325 — the filter that makes AppOrigin interceptable at all.
                core.AddWebResourceRequestedFilter(L"*", webview2::CoreWebView2WebResourceContext::All);
                core.WebResourceRequested([](const webview2::CoreWebView2& sender_core,
                                             const webview2::CoreWebView2WebResourceRequestedEventArgs& e) {
                    // :125 — only AppOrigin requests are ours; anything else is left to WebView2's normal
                    // networking (:158-162).
                    const std::string url = maui::platform::windows::to_utf8(e.Request().Uri());
                    if (!url.starts_with(k_app_origin))
                    {
                        return;
                    }
                    // :233-257 would resolve the relative path under HybridRoot and stream the asset back.
                    // There is NOTHING to stream — and neither has MAUI on this board (see the file-top
                    // note) — so this takes MAUI's own :262 fallback unconditionally: a bodiless 404 with
                    // the empty header block C# builds for a null stream and null content type (:136-145),
                    // which is exactly what makes WebView2 paint the "HTTP ERROR 404" page the reference
                    // column shows. Environment() lives on ICoreWebView2_2, which the CoreWebView2 runtime
                    // class flattens, so this is a direct member call.
                    e.Response(sender_core.Environment().CreateWebResourceResponse(nullptr, 404, L"Not Found", L""));
                });
                // :46 `PlatformView.Source = new Uri(new Uri(AppOriginUri, "/").ToString())`.
                sender.Source(winrt::Windows::Foundation::Uri{maui::platform::windows::to_hstring(k_app_origin)});
            });

        // Kick initialization off. The IAsyncAction's own completion is observed with an empty handler
        // deliberately: the event above is the signal this file acts on, and leaving the action UNOBSERVED
        // would turn an activation failure into an unhandled winrt error instead of an empty cell.
        try
        {
            web.EnsureCoreWebView2Async().Completed(
                [](const winrt::Windows::Foundation::IAsyncAction&, winrt::Windows::Foundation::AsyncStatus) {});
        }
        catch (const winrt::hresult_error&)
        {
        }
    }

    void hybrid_web_view_handler::on_disconnect_handler(hybrid_web_view_platform& platform)
    {
        platform.connected_view = nullptr;
        if (platform.native == nullptr)
        {
            return;
        }
        const web_view_control web = as_web_view(platform.native);
        // Revoke EXACTLY what on_connect_handler registered, before the handler its lambda captures can
        // die (button_handler.cpp's discipline).
        if (platform.web_message_token != 0)
        {
            web.WebMessageReceived(winrt::event_token{platform.web_message_token});
            platform.web_message_token = 0;
        }
        // :63-71 — Close() only once a CoreWebView2 exists, mirroring the oracle's guard. It tears down
        // the browser process this view owns; without it a page swap leaks an msedgewebview2.exe per visit.
        if (web.CoreWebView2() != nullptr)
        {
            web.Close();
        }
    }

    // ---- command maps: the headless-mirror bodies, copied VERBATIM from
    // src/platform/headless/hybrid_web_view_handler.cpp. Required at link time (the swap removes that TU)
    // and correct on this board: the page is a 404, so there is no document to script against. See the
    // file-top DOCUMENTED DEVIATIONS for the (non-uniform) upgrade path.

    void hybrid_web_view_handler::map_send_raw_message(hybrid_web_view_handler& handler,
                                                       maui::core::i_hybrid_web_view& /*view*/, const std::any& args)
    {
        auto* platform = handler.typed_platform_view();
        const auto* message = std::any_cast<std::string>(&args);
        if (platform == nullptr || message == nullptr)
        {
            return;
        }
        platform->sent_raw_messages.push_back(*message);
        platform->evaluated_scripts.push_back(maui::core::build_send_raw_message_script(*message));
    }

    void hybrid_web_view_handler::map_invoke_java_script(hybrid_web_view_handler& handler,
                                                         maui::core::i_hybrid_web_view& /*view*/, const std::any& args)
    {
        const auto* request_ptr = std::any_cast<std::shared_ptr<maui::core::invoke_java_script_request>>(&args);
        if (request_ptr == nullptr || *request_ptr == nullptr)
        {
            return;
        }
        const std::shared_ptr<maui::core::invoke_java_script_request> request = *request_ptr;
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            request->complete(std::nullopt);
            return;
        }
        const std::string task_id = handler.create_invoke_task(request);
        platform->evaluated_scripts.push_back(
            maui::core::build_invoke_java_script_script(task_id, request->method_name(), request->param_values()));
    }

    void hybrid_web_view_handler::map_evaluate_java_script(hybrid_web_view_handler& handler,
                                                           maui::core::i_hybrid_web_view& /*view*/,
                                                           const std::any& args)
    {
        const auto* request = std::any_cast<std::shared_ptr<maui::core::evaluate_java_script_request>>(&args);
        if (request == nullptr || *request == nullptr)
        {
            return;
        }
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            (*request)->complete("null");
            return;
        }
        platform->evaluated_scripts.push_back((*request)->script());
        std::string result = platform->script_responder ? platform->script_responder((*request)->script()) : "null";
        (*request)->complete(std::move(result));
    }

    maui::graphics::size hybrid_web_view_handler::get_desired_size(double width_constraint,
                                                                   double height_constraint) const
    {
        const auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return {0, 0};
        }
        // GetDesiredSizeFromHandler's first guard: a negative constraint measures to nothing. XAML's
        // Measure THROWS on a negative Size, so this is a crash guard, not a formality.
        if (width_constraint < 0 || height_constraint < 0)
        {
            return {0, 0};
        }
        // NO 44 floor here. MinimumSize is declared ONLY on iOS (WebViewHandler.iOS.cs:18) and Tizen
        // (WebViewHandler.Tizen.cs:5) — there is no Windows MinimumSize at all, so this is the plain
        // progress_bar_handler.cpp ARRANGE/EXPLICIT-SIZE pattern. (The port header's `minimum_size = 44.0`
        // stays for the headless/apple partials; it does not apply on this backend.)
        const web_view_control web = as_web_view(platform->native);
        const auto* view = virtual_view();
        const double explicit_width = (view != nullptr) ? view->width() : std::numeric_limits<double>::quiet_NaN();
        const double explicit_height = (view != nullptr) ? view->height() : std::numeric_limits<double>::quiet_NaN();
        web.Width(explicit_width);
        web.Height(explicit_height);
        const double adjusted_width_constraint =
            std::isnan(explicit_width) ? width_constraint : std::max(width_constraint, explicit_width);
        const double adjusted_height_constraint =
            std::isnan(explicit_height) ? height_constraint : std::max(height_constraint, explicit_height);
        web.Measure(
            winrt::Windows::Foundation::Size{maui::platform::windows::measure_constraint(adjusted_width_constraint),
                                             maui::platform::windows::measure_constraint(adjusted_height_constraint)});
        const auto desired = web.DesiredSize();
        return {desired.Width, desired.Height};
    }

    void hybrid_web_view_handler::platform_arrange(const maui::graphics::rect& frame)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        // PlatformArrangeHandler's guard, WIDENED to non-finite — see progress_bar_handler.cpp (an
        // unrecoverable stowed exception, 0xC000027B, otherwise).
        if (!std::isfinite(frame.x) || !std::isfinite(frame.y) || !std::isfinite(frame.width) ||
            !std::isfinite(frame.height) || frame.width < 0 || frame.height < 0)
        {
            return;
        }
        const web_view_control web = as_web_view(platform->native);
        winui::Controls::Canvas::SetLeft(web, frame.x);
        winui::Controls::Canvas::SetTop(web, frame.y);
        web.Width(frame.width);
        web.Height(frame.height);
        // QUALIFIED, unlike the maui::core handlers' identical call: this file is `namespace
        // maui::controls` while apply_native_clip is declared in `maui::core`, and ADL cannot bridge them
        // (the arguments are void* and maui::graphics::i_shape*). Unqualified gives error C3861, which
        // reads like a missing include. Same note as src/platform/windows/collection_view_handler.cpp.
        if (const auto* view = virtual_view(); view != nullptr)
        {
            maui::core::apply_native_clip(platform->native, view->clip());
        }
    }
} // namespace maui::controls
