// web_view_handler — WinUI 3 platform partial: a REAL Microsoft.UI.Xaml.Controls.WebView2, the same
// native type WebViewHandler.Windows.cs creates (`ViewHandler<IWebView, WebView2>`, whose
// CreatePlatformView is `new MauiWebView(this)` — a WebView2 subclass, :19). Ported from
// WebViewHandler.Windows.cs + Platform/Windows/WebViewExtensions.cs + Platform/Windows/MauiWebView.cs.
// The windows twin of src/platform/android/web_view_handler.cpp (a real android.webkit.WebView) and
// src/platform/apple_shared/web_view_handler.mm (a real WKWebView); src/platform/headless/
// web_view_handler.cpp keeps the in-memory back-forward simulation for the VM-less suite.
//
// WHY THIS FILE EXISTS: without it the windows build kept the HEADLESS partial, which creates no native
// view at all — so the page's 1008x240 WebView cell (rows 32..271) was never painted and the page
// background showed through. That one rectangle is 29.5% of the frame and WAS the entire dark score.
//
// MauiWebView is NOT subclassed: it carries only IWebViewDelegate + NavigationStarting virtual-host
// bookkeeping, and in this port web_view_platform ALREADY is the i_web_view_delegate. A plain WebView2
// is the same control.
//
// READINESS SEAM (the C++ analogue of MauiWebView's `await EnsureCoreWebView2Async()`): the oracle's own
// hook, WebView2Proxy.Connect's `platformView.CoreWebView2Initialized` (WebViewHandler.Windows.cs:346,
// :385-402) — a XAML event raised ON THE UI THREAD. Using it instead of the IAsyncAction completion avoids
// a DispatcherQueue hop entirely (an IAsyncAction completion arrives on a pool thread, and every XAML call
// here is UI-affine — see image_source_services.cpp's hop and the Microsoft.UI-vs-Windows::System
// DispatcherQueue trap it has to dodge). EnsureCoreWebView2Async() is still called to KICK initialization
// off rather than waiting for the implicit Loaded-time init: this control is a Canvas child the port
// arranges by hand, and WebView2 init spawns msedgewebview2.exe, which the parity capture's settle races.
//
// DOCUMENTED DEVIATIONS from the C# Windows oracle (each an infrastructure gap or a render-wins ruling,
// never a behavior guess):
//   - NAVIGATION EVENTS: WebView2Proxy also hooks NavigationStarting / NavigationCompleted /
//     HistoryChanged / ProcessFailed and forwards them into IWebView.Navigating/Navigated
//     (WebViewHandler.Windows.cs:64-85, :141-175). NOT wired here — the same deferral the android partial
//     documents for MauiWebViewClient. No docs/comparison scenario drives web_view navigation, so nothing
//     on the board observes them; the headless partial still drives the simulated channel for the unit
//     suite. VISIBLE CONSEQUENCE: the code-first gallery page binds `navigated` to its status label
//     (examples/gallery/pages/web_view_page.hpp:45-47), so with the events unwired that label keeps its
//     authored resting text — which is what the shared twin (maui-reference/pages/web_view.xaml:14, a
//     STATIC "No navigation yet") shows too. That alignment is a side effect, not the reason.
//     Do NOT "fix" it by wiring NavigationCompleted alone: LoadHtml navigates via NavigateToString, whose
//     reported Source is about:blank or a base64 data: URI, either of which would put an ugly or enormous
//     string into the label. Wire it together with a real url source.
//   - BASE-TAG INSERTION SCRIPT: MauiWebView.LoadHtml (:47-70) prepends `<script>…injects <base href>…
//     </script>\n` to the html before NavigateToString. NOT ported, for two reasons: (1) anything before
//     `<!DOCTYPE html>` puts the document in QUIRKS mode, while the MAUI ground truth for this page
//     arrives through LoadUrl (Source="welcome.html") with the DOCTYPE first, i.e. STANDARDS mode —
//     porting the script would render the cpp column in quirks against a standards reference,
//     re-introducing exactly the font-metric diff examples/gallery/pages/web_view_page.hpp:155 documents;
//     (2) the <base href> only matters for RELATIVE resources inside the html, and no gallery page ships
//     an HtmlWebViewSource with any. Revisit when one does.
//   - load_html DROPS base_url ENTIRELY, and with it MauiWebView.LoadHtml's mapBaseDirectory branch
//     (:49-54 + :58-64): C# treats an EMPTY baseUrl as "serve from the app directory" — it substitutes
//     LocalScheme and calls SetVirtualHostNameToFolderMapping — while a NON-empty one only feeds the base
//     tag above. Neither half is reachable from any gallery page (the code-first twin always passes a
//     non-empty absolute base_url, so C# would take neither branch), and the base_url is still mirrored
//     into last_base_url for the cross-platform suite. Port both halves together with the base tag.
//   - go_back / go_forward GUARD THE WebView2 WRAPPER property, where WebViewExtensions.cs:74/:85 guards
//     `CoreWebView2.CanGoBack`. The oracle reads the wrapper only for its CurrentNavigationEvent set
//     (WebViewHandler.Windows.cs:107/:115). Both read the same underlying state once CoreWebView2 exists;
//     before it does, the wrapper reads false (which is why this is safe pre-init) whereas the oracle
//     would NullReference. map_reload adds the same pre-init CoreWebView2 guard, which UpdateReload
//     (:91-94, a bare `platformWebView?.Reload()`) does not have.
//   - COOKIES: SyncPlatformCookies / the CookieManager round trip (WebViewHandler.Windows.cs:177-300) is
//     out of scope for the whole port, on every backend.
//   - Profile.PreferredColorScheme (WebViewExtensions.UpdateBackground:44-55) is NOT pushed — see
//     update_background below for the src-vs-shipped flag.
//   - Eval's DispatcherQueue.TryEnqueue wrapper (WebViewExtensions.cs:102-108) is skipped: every command
//     mapper in this port runs on the UI thread by construction, so the hop would be a no-op. The
//     EvaluateJavaScript RESULT hop is NOT skipped — that one is real.
//
// PARITY: WebView2 initialization is asynchronous and spawns a browser process, so both columns race the
// capture settle. MEASURED against the frozen reference (composite + tools/parity/pixel_score.py): a cell
// painted with the real welcome.html content scores 0.47%/SSIM 0.984 light and 0.15%/0.995 dark, and a
// cell that comes up blank white scores 0.14%/0.995 light and 0.48%/0.984 dark. Both are inside the
// board's green band, so the race is not a parity hazard here — only NOT painting the cell is.

#include "maui/core/web_view_handler.hpp"

// C++/WinRT include rule (winui_interop.hpp): the FULL header for every namespace whose MEMBERS are
// called, or the call fails with C3779, which names neither the header nor the concept.
#include <winrt/Microsoft.UI.Dispatching.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Microsoft.Web.WebView2.Core.h>
#include <winrt/Windows.Foundation.h>

// AFTER the winrt block, exactly like image_source_services.cpp — <windows.h> defines GetCurrentTime as a
// function-like macro that eats the argument list of the identically-named C++/WinRT member if a winrt
// header is parsed afterwards (host_run.cpp documents the same trap). It is what supplies FAILED() below.
#include <windows.h>

#include <algorithm>
#include <any>
#include <cmath>
#include <functional>
#include <limits>
#include <memory>
#include <string>
#include <string_view>
#include <utility>

#include "maui/core/evaluate_java_script_request.hpp"
#include "maui/core/i_web_view.hpp"
#include "maui/core/i_web_view_source.hpp"
#include "maui/core/view_chrome_ops.hpp"
#include "maui/core/view_platform_base.hpp"
#include "maui/core/visibility.hpp"
#include "maui/core/web_navigation_event.hpp"
#include "maui/graphics/color.hpp"
#include "maui/graphics/gradient_paint.hpp"
#include "maui/graphics/paint.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"
#include "maui/graphics/solid_paint.hpp"
#include "winui_interop.hpp"
#include "winui_visual_ops.hpp"

namespace
{
    // Named `winui`, NOT `xaml` — see progress_bar_handler.cpp's identical note (the port's own maui::xaml
    // XAML-loader namespace would shadow a file-scope `xaml` alias inside namespace maui::*).
    namespace winui = winrt::Microsoft::UI::Xaml;
    namespace webview2 = winrt::Microsoft::Web::WebView2::Core;
    using web_view_control = winui::Controls::WebView2;

    // MauiWebView.cs:31-32 — the arbitrary local host name the virtual-folder mapping uses, and the
    // scheme a RELATIVE Source is rebased onto.
    constexpr std::wstring_view k_local_host = L"appdir";
    constexpr std::string_view k_local_scheme = "https://appdir/";

    web_view_control as_web_view(void* native)
    {
        return maui::platform::windows::ref<winui::UIElement>(native).as<web_view_control>();
    }

    // MauiWebView.ApplicationPath (:43-45) for an UNPACKAGED app: AppContext.BaseDirectory, i.e. the
    // running exe's own directory — which is where examples/cmake/maui_add_app.cmake's windows branch
    // copies an example's RESOURCES flat. The gallery is unpackaged (it ships the WinAppSDK bootstrap DLL
    // beside the exe), so the packaged branch never applies.
    // ponytail: byte-identical to image_source_services.cpp's exe_directory(); duplicated rather than
    // promoted into winui_interop.hpp to keep this change to one new file. Promote it on a third caller.
    std::wstring application_path()
    {
        std::wstring buffer(MAX_PATH, L'\0');
        for (;;)
        {
            const DWORD written = ::GetModuleFileNameW(nullptr, buffer.data(), static_cast<DWORD>(buffer.size()));
            if (written == 0)
            {
                return {};
            }
            if (written < buffer.size())
            {
                buffer.resize(written);
                break;
            }
            buffer.resize(buffer.size() * 2);
        }
        const auto slash = buffer.find_last_of(L"\\/");
        return slash == std::wstring::npos ? std::wstring{} : buffer.substr(0, slash);
    }

    // Run `navigate` once the CoreWebView2 exists — see this file's READINESS SEAM note.
    //
    // The delegate captures ONLY its payload by value and takes the control from the `sender` argument, so
    // it can never dangle on a web_view_platform that died while WebView2 was still initializing.
    // ponytail: no revoker is stored. CoreWebView2Initialized fires at most once per control, and a source
    // set twice before init simply registers a second handler whose (later) navigation wins — the correct
    // last-writer-wins outcome. Store a revoker only if a page starts swapping sources pre-init.
    void navigate_when_ready(const web_view_control& web, std::function<void(const web_view_control&)> navigate)
    {
        if (web.CoreWebView2() != nullptr)
        {
            navigate(web);
            return;
        }
        web.CoreWebView2Initialized(
            [navigate = std::move(navigate)](const web_view_control& sender,
                                             const winui::Controls::CoreWebView2InitializedEventArgs& args) {
                // Exception() is a winrt::hresult (S_OK on success), NOT a nullable reference — comparing
                // it to nullptr does not compile. A failed init leaves CoreWebView2() null, and every call
                // `navigate` makes would then be a null-vtable dereference: an ACCESS VIOLATION, not a
                // catchable hresult_error. Both guards are load-bearing.
                if (FAILED(args.Exception()) || sender.CoreWebView2() == nullptr)
                {
                    return;
                }
                navigate(sender);
            });
        try
        {
            [[maybe_unused]] const auto init = web.EnsureCoreWebView2Async();
        }
        catch (const winrt::hresult_error&)
        {
            // No WebView2 Runtime on the machine. MauiWebView swallows the analogous failure around
            // `Source = uri` (:95-101); the page then renders without web content instead of taking the
            // app down — the same degradation shape the android partial uses when WebView.<init> throws.
        }
    }

    // WebViewExtensions.UpdateCanGoBackForward (:96-100): `webView.CanGoBack = platformWebView.CanGoBack`.
    // These are the WebView2 WRAPPER properties, which read false before CoreWebView2 exists — so this is
    // safe pre-init and needs no guard, exactly as in C#.
    void update_can_go_back_forward(const web_view_control& web, maui::core::i_web_view& view)
    {
        view.set_can_go_back(web.CanGoBack());
        view.set_can_go_forward(web.CanGoForward());
    }
} // namespace

namespace maui::core
{
    web_view_platform::~web_view_platform()
    {
        if (native != nullptr)
        {
            maui::platform::windows::drop<winui::UIElement>(native);
        }
    }

    // ---- i_web_view_delegate (the source's load sink; C# MauiWebView : IWebViewDelegate) --------------

    void web_view_platform::load_html(std::string_view html, std::string_view base_url)
    {
        // Mirrors FIRST (the cross-platform suite reads these), then the native navigate — the android
        // partial's ordering.
        last_source_kind = web_view_source_kind::html;
        last_html = std::string(html);
        last_base_url = std::string(base_url);
        if (native == nullptr)
        {
            return;
        }
        // MauiWebView.LoadHtml (:47-70) minus the base-tag script and the mapBaseDirectory branch — see
        // this file's DEVIATIONS note.
        navigate_when_ready(as_web_view(native), [html = last_html](const web_view_control& web) {
            web.NavigateToString(maui::platform::windows::to_hstring(html));
        });
    }

    void web_view_platform::load_url(std::string_view url)
    {
        last_source_kind = web_view_source_kind::url;
        last_url = std::string(url);
        if (native == nullptr)
        {
            return;
        }
        // MauiWebView.LoadUrl (:73-101): a RELATIVE url — or one already on the local scheme — is served
        // out of the app's own directory through a virtual host and rebased onto https://appdir/. This is
        // the load-bearing path for the shared twin's `Source="welcome.html"`.
        //
        // The absolute test is `find("://")`, the same heuristic the android partial uses rather than a
        // full Uri parse; C# uses UriKind.RelativeOrAbsolute, which additionally treats schemes like
        // `mailto:` as absolute — no gallery page uses one.
        const bool absolute = last_url.find("://") != std::string::npos;
        const bool local = last_url.starts_with(k_local_scheme);
        const bool map_folder = !absolute || local;
        const std::string target = absolute ? last_url : std::string(k_local_scheme) + last_url;
        navigate_when_ready(as_web_view(native), [target, map_folder](const web_view_control& web) {
            if (map_folder)
            {
                try
                {
                    web.CoreWebView2().SetVirtualHostNameToFolderMapping(
                        winrt::hstring{k_local_host}, winrt::hstring{application_path()},
                        webview2::CoreWebView2HostResourceAccessKind::Allow);
                }
                catch (const winrt::hresult_error&)
                {
                }
            }
            try
            {
                web.Source(winrt::Windows::Foundation::Uri{maui::platform::windows::to_hstring(target)});
            }
            catch (const winrt::hresult_error&)
            {
                // MauiWebView.cs:98-101 catches and logs the same failure rather than crashing.
            }
        });
    }

    // ---- platform recipe ---------------------------------------------------------------------------

    std::unique_ptr<web_view_platform> web_view_handler::create_platform_view()
    {
        auto platform = std::make_unique<web_view_platform>();
        try
        {
            // WebViewHandler.Windows.CreatePlatformView (:19). Activating WebView2 needs the projected
            // Microsoft.Web.WebView2.Core surface + the WebView2 Runtime; if either is missing the
            // activation throws, and the handler degrades to the mirror-only behaviour rather than taking
            // the page down (the same shape image_source_services uses for a missing Win2D).
            const web_view_control web;
            platform->native = maui::platform::windows::take<winui::UIElement>(web);
        }
        catch (const winrt::hresult_error&)
        {
        }
        return platform;
    }

    void web_view_handler::on_connect_handler(web_view_platform& platform)
    {
        // WebViewHandler.Windows.ConnectHandler (:26-38) also installs the WebView2Proxy; only its
        // CoreWebView2Initialized hook is used here, lazily, by navigate_when_ready — see DEVIATIONS.
        platform.connected_view = virtual_view();
    }

    void web_view_handler::on_disconnect_handler(web_view_platform& platform)
    {
        platform.connected_view = nullptr;
        if (platform.native == nullptr)
        {
            return;
        }
        // WebViewHandler.Windows.Disconnect (:50-58): `if (CoreWebView2 is not null) platformView.Close()`
        // — the browser process is torn down with the handler, not left to finalization.
        const web_view_control web = as_web_view(platform.native);
        if (web.CoreWebView2() != nullptr)
        {
            web.Close();
        }
    }

    // WebViewHandler.MapSource -> WebViewExtensions.UpdateSource (:15-22): the platform view IS the
    // IWebViewDelegate the source loads into, then UpdateCanGoBackForward.
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
        if (platform->native != nullptr)
        {
            update_can_go_back_forward(as_web_view(platform->native), view);
        }
    }

    // WebViewHandler.MapUserAgent -> WebViewExtensions.UpdateUserAgent (:57-66).
    void web_view_handler::map_user_agent(web_view_handler& handler, i_web_view& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->user_agent = std::string(view.user_agent()); // mirror first
        if (platform->native == nullptr)
        {
            return;
        }
        const auto core = as_web_view(platform->native).CoreWebView2();
        if (core == nullptr)
        {
            // The oracle's own early-out (:59-60). C# re-runs this from OnCoreWebView2Initialized; that
            // re-run is part of the deferred WebView2Proxy — no gallery page sets UserAgent.
            return;
        }
        if (!view.user_agent().empty())
        {
            core.Settings().UserAgent(maui::platform::windows::to_hstring(view.user_agent()));
            return;
        }
        const std::string resolved = maui::platform::windows::to_utf8(core.Settings().UserAgent());
        platform->user_agent = resolved;
        view.set_user_agent(resolved); // re-enters on the set branch (terminates)
    }

    // ---- navigation commands (WebViewHandler.CommandMapper) -----------------------------------------

    void web_view_handler::map_go_back(web_view_handler& handler, i_web_view& view, const std::any& /*args*/)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        const web_view_control web = as_web_view(platform->native);
        // MapGoBack (:105-111): CurrentNavigationEvent = Back only when the platform CAN go back; then
        // UpdateGoBack (:69-78) navigates and re-reads. See DEVIATIONS for the wrapper-vs-core guard.
        if (web.CanGoBack())
        {
            platform->current_navigation_event = web_navigation_event::back;
            if (web.CoreWebView2() != nullptr)
            {
                web.GoBack();
            }
        }
        update_can_go_back_forward(web, view);
    }

    void web_view_handler::map_go_forward(web_view_handler& handler, i_web_view& view, const std::any& /*args*/)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        const web_view_control web = as_web_view(platform->native);
        if (web.CanGoForward()) // MapGoForward (:113-119) + UpdateGoForward (:80-89)
        {
            platform->current_navigation_event = web_navigation_event::forward;
            if (web.CoreWebView2() != nullptr)
            {
                web.GoForward();
            }
        }
        update_can_go_back_forward(web, view);
    }

    void web_view_handler::map_reload(web_view_handler& handler, i_web_view& /*view*/, const std::any& /*args*/)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        // MapReload (:121-127) sets Refresh UNCONDITIONALLY (no CanGo guard), then UpdateReload (:91-94).
        platform->current_navigation_event = web_navigation_event::refresh;
        ++platform->reload_count; // mirror
        if (platform->native == nullptr)
        {
            return;
        }
        const web_view_control web = as_web_view(platform->native);
        if (web.CoreWebView2() != nullptr)
        {
            web.Reload();
        }
    }

    // WebViewHandler.MapEval (:129-136) -> WebViewExtensions.Eval (:102-108): fire-and-forget. The
    // oracle's DispatcherQueue.TryEnqueue wrapper is skipped — see DEVIATIONS.
    void web_view_handler::map_eval(web_view_handler& handler, i_web_view& /*view*/, const std::any& args)
    {
        auto* platform = handler.typed_platform_view();
        const auto* script = std::any_cast<std::string>(&args);
        if (platform == nullptr || script == nullptr)
        {
            return;
        }
        platform->eval_scripts.push_back(*script); // mirror first
        if (platform->native == nullptr)
        {
            return;
        }
        const web_view_control web = as_web_view(platform->native);
        if (web.CoreWebView2() == nullptr)
        {
            return;
        }
        [[maybe_unused]] const auto op = web.ExecuteScriptAsync(maui::platform::windows::to_hstring(*script));
    }

    // WebViewHandler.MapEvaluateJavaScriptAsync (:315-327) -> WebViewExtensions.EvaluateJavaScript
    // (:111-114): `request.RunAndReport(ExecuteScriptAsync(...))`. C# SetCanceled's the request when the
    // platform view is gone — surfaced here as the "null" result (see evaluate_java_script_request.hpp,
    // and the headless/android partials, which resolve it the same way).
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
            request->complete("null");
            return;
        }
        platform->eval_scripts.push_back(request->script());
        if (platform->native == nullptr)
        {
            request->complete("null");
            return;
        }
        const web_view_control web = as_web_view(platform->native);
        if (web.CoreWebView2() == nullptr)
        {
            request->complete("null");
            return;
        }
        // The completion lands on a POOL thread and complete() runs developer code that touches controls —
        // hop back to the UI queue, exactly like image_source_services.cpp. Microsoft::UI::Dispatching,
        // NOT the identically-named UWP Windows::System queue.
        const auto ui_queue = web.DispatcherQueue();
        web.ExecuteScriptAsync(maui::platform::windows::to_hstring(request->script()))
            .Completed([request, ui_queue](const winrt::Windows::Foundation::IAsyncOperation<winrt::hstring>& op,
                                           winrt::Windows::Foundation::AsyncStatus status) {
                std::string result = "null"; // the WebView2 value for an errored/void script
                if (status == winrt::Windows::Foundation::AsyncStatus::Completed)
                {
                    result = maui::platform::windows::to_utf8(op.GetResults());
                }
                if (ui_queue == nullptr)
                {
                    return;
                }
                ui_queue.TryEnqueue(
                    [request, result = std::move(result)]() mutable { request->complete(std::move(result)); });
            });
    }

    // ---- measure / arrange --------------------------------------------------------------------------
    // The Windows recipe is ViewHandlerExtensions.Windows.cs GetDesiredSizeFromHandler + AdjustForExplicit
    // Size — NOT the iOS partial's MinimumSize(44) fallback (WebViewHandler has no Windows GetDesiredSize
    // override at all, and MinimumSize is declared only on iOS and Tizen). Identical to
    // progress_bar_handler.cpp's block, and load-bearing here: the demo page pins HeightRequest=240 and a
    // WebView2 measures 0 on its own.

    maui::graphics::size web_view_handler::get_desired_size(double width_constraint, double height_constraint) const
    {
        const auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return {0, 0};
        }
        if (width_constraint < 0 || height_constraint < 0)
        {
            return {0, 0}; // XAML's Measure THROWS on a negative Size — a crash guard, not a formality
        }
        const web_view_control web = as_web_view(platform->native);
        // ARRANGE/EXPLICIT-SIZE FIX (progress_bar_handler.cpp's identical block, commit a2444f94ba): pin
        // Width/Height to the view's own explicit request instead of clearing to NaN, then only WIDEN the
        // incoming constraint at measure time. platform_arrange's OWN stamp (below) is UNTOUCHED.
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

    void web_view_handler::platform_arrange(const maui::graphics::rect& frame)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        // PlatformArrangeHandler's guard, WIDENED to non-finite — see progress_bar_handler.cpp for why
        // (an unrecoverable stowed exception, 0xC000027B, otherwise).
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
        if (const auto* view = virtual_view(); view != nullptr)
        {
            apply_native_clip(platform->native, view->clip());
        }
    }

    // ---- generic-IView property pushes --------------------------------------------------------------
    // WebView2 IS a Control, so IsEnabled/Visibility/Opacity/AutomationId all reach it directly through
    // the shared winui_visual_ops helpers (the progress_bar precedent). FlowDirection is deliberately NOT
    // overridden: WebViewHandler.Windows.MapFlowDirection (:329-334) is an EXPLICIT no-op, and
    // view_platform_base's mirror-only body is exactly that.

    void web_view_platform::update_visibility(maui::core::visibility value)
    {
        maui::platform::windows::apply_visibility(native, value);
    }

    void web_view_platform::update_opacity(double value)
    {
        maui::platform::windows::apply_opacity(native, value);
    }

    void web_view_platform::update_is_enabled(bool value)
    {
        maui::platform::windows::apply_is_enabled(native, value);
    }

    void web_view_platform::update_automation_id(std::string_view value)
    {
        maui::platform::windows::apply_automation_id(native, value);
    }

    // WebViewHandler.cs:29 remaps `[nameof(IView.Background)] = MapBackground` on WINDOWS ONLY, so the
    // generic UIElement/Control.Background push must NOT run for a WebView2 — that would be invented.
    // WebViewExtensions.UpdateBackground (:24-55): a SolidPaint's Color (or a GradientPaint's StartColor,
    // the oracle's own "best-effort approximation") becomes WebView2.DefaultBackgroundColor; a null
    // Background changes nothing (there is no clearing branch).
    //
    // SRC-vs-SHIPPED FLAG (parity rule 4 (RENDER-BREAKS-TIES)): the oracle also sets CoreWebView2.Profile.
    // PreferredColorScheme (Light when a background is set, Auto otherwise). That whole method is
    // `internal` and carries `//TODO: Make it public in .NET 11` (WebViewHandler.Windows.cs:99) — direct
    // evidence it is post-10.0 churn that shipped 10.0.71 (the MauiVersion the board renders,
    // MauiReference.csproj:18) may not have at all. Skipped here. Zero render risk on the board: Auto is
    // WebView2's default, no gallery page sets a WebView Background, and welcome.html pins
    // `<meta name="color-scheme">` either way.
    void web_view_platform::update_background(const maui::graphics::paint* value)
    {
        view_platform_base::update_background(value); // keep the mirror
        if (native == nullptr || value == nullptr)
        {
            return;
        }
        maui::graphics::color chosen{};
        if (const auto* gradient = dynamic_cast<const maui::graphics::gradient_paint*>(value))
        {
            chosen = gradient->start_color();
        }
        else if (dynamic_cast<const maui::graphics::solid_paint*>(value) != nullptr)
        {
            chosen = value->background_color();
        }
        else
        {
            return; // C# tests only for SolidPaint / GradientPaint; anything else leaves the default
        }
        as_web_view(native).DefaultBackgroundColor(maui::platform::windows::to_ui_color(chosen));
    }
} // namespace maui::core
