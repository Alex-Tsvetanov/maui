#pragma once
// maui::core::i_web_view  <=  Microsoft.Maui.IWebView
//
// The virtual-view contract for a view that presents HTML content. Ported from
// src/Core/src/Core/IWebView.cs (IWebView : IView).
//
// Channels (mirroring the C# member roles):
//   - source() is the outbound content the handler's map_source pushes into the platform view
//     (Source?.Load(webViewDelegate) — see i_web_view_source / i_web_view_delegate).
//   - can_go_back / can_go_forward are HANDLER-PUSHED read-onlys: the platform partial writes them via
//     set_can_go_back/set_can_go_forward after every navigation (WebViewExtensions.UpdateCanGoBackForward
//     `webView.CanGoBack = platformWebView.CanGoBack`); the control exposes them read-only.
//   - send_navigating / send_navigated are the inbound navigation channel (C# IWebView.Navigating /
//     Navigated, renamed to the port's send_* convention like i_button::send_clicked): the platform's
//     navigation delegate calls them, the control raises its navigating/navigated events.
//     send_navigating returns the args' Cancel flag so the platform can abort the navigation.
//   - GoBack/GoForward/Reload/Eval/EvaluateJavaScriptAsync are NOT contract methods here: in the port the
//     control drives them as handler COMMANDS ("go_back"/"go_forward"/"reload"/"eval"/
//     "evaluate_java_script" — WebViewHandler.CommandMapper's keys), so the i_* surface carries only what
//     the handler reads or pushes.
//
//   - user_agent() / set_user_agent() are the BIDIRECTIONAL UserAgent channel (C# IWebView.UserAgent
//     { get; set; }): the developer or a binding can set it (virtual→native: the handler writes
//     WKWebView.CustomUserAgent), and when it is UNSET the handler READS BACK the platform's
//     CustomUserAgent / default `userAgent` and stores it into the virtual view (WebViewExtensions.iOS
//     UpdateUserAgent). C#'s `string?` is modeled as std::string — empty means "unset" (the null branch).
//
// OUT OF SCOPE (documented, not stubbed): Cookies (CookieContainer sync) and ProcessTerminated (the
// WebContent-process crash notification).

#include <string_view>

#include "maui/core/i_view.hpp"
#include "maui/core/web_navigation_event.hpp"
#include "maui/core/web_navigation_result.hpp"

namespace maui::core
{
    class i_web_view_source;

    class i_web_view : public i_view
    {
    public:
        // C# IWebView.Source. Non-owning view of the control's source (null when no source is set).
        [[nodiscard]] virtual i_web_view_source* source() const = 0;

        // C# IWebView.CanGoBack / CanGoForward — readable by anyone, written ONLY by the handler's
        // platform partial after a navigation (UpdateCanGoBackForward).
        [[nodiscard]] virtual bool can_go_back() const = 0;
        virtual void set_can_go_back(bool value) = 0;
        [[nodiscard]] virtual bool can_go_forward() const = 0;
        virtual void set_can_go_forward(bool value) = 0;

        // C# IWebView.UserAgent { get; set; }: the bidirectional user-agent slot. The getter returns a
        // view over the stored value (empty == C#'s null, "unset"); the setter triggers a property change
        // so the handler's map_user_agent runs (WebViewExtensions.iOS UpdateUserAgent — write
        // CustomUserAgent when set, else read the platform default back into the virtual view).
        [[nodiscard]] virtual std::string_view user_agent() const = 0;
        virtual void set_user_agent(std::string value) = 0;

        // C# IWebView.Navigating: raised by the platform BEFORE a navigation; returns true when a
        // subscriber cancelled it (WebNavigatingEventArgs.Cancel).
        virtual bool send_navigating(web_navigation_event navigation_event, std::string_view url) = 0;
        // C# IWebView.Navigated: raised by the platform AFTER a navigation completes (or fails).
        virtual void send_navigated(web_navigation_event navigation_event, std::string_view url,
                                    web_navigation_result result) = 0;
    };
} // namespace maui::core
