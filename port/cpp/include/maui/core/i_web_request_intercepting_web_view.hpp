#pragma once
// maui::core::i_web_request_intercepting_web_view  <=  Microsoft.Maui.IWebRequestInterceptingWebView
//
// The mixin contract a web view implements to let the host intercept resource requests. Ported from
// src/Core/src/Core/IWebRequestInterceptingWebView.cs. C#'s single member is
// `bool WebResourceRequested(WebResourceRequestedEventArgs args)` — the platform's URL-scheme handler
// asks the virtual view whether the app wants to override a request, and the control raises its
// WebResourceRequested event, returning the args' Handled flag.
//
// In the port the args object (C#'s WebViewWebResourceRequestedEventArgs wrapping a platform request +
// response) is reduced to the requested url + a handled-out flag: the full request/response object is
// deeply platform-specific (NSUrlRequest / IWKUrlSchemeTask) and the custom-response path
// (SetResponse(status, contentType, headers, stream)) is not yet modeled — see web_resource_requested.
//
// DEVIATION (documented, not stubbed): the headless backend exposes web_resource_requested so the
// inbound channel is unit-testable; on apple/ios the WKWebView app:// scheme handler that would drive it
// is itself a deviation (see hybrid_web_view_handler), so the interception hook is reached only where the
// scheme handler is ported.

#include <string_view>

namespace maui::core
{
    class i_web_request_intercepting_web_view
    {
    public:
        i_web_request_intercepting_web_view() = default;
        virtual ~i_web_request_intercepting_web_view() = default;
        i_web_request_intercepting_web_view(const i_web_request_intercepting_web_view&) = default;
        i_web_request_intercepting_web_view(i_web_request_intercepting_web_view&&) = default;
        i_web_request_intercepting_web_view& operator=(const i_web_request_intercepting_web_view&) = default;
        i_web_request_intercepting_web_view& operator=(i_web_request_intercepting_web_view&&) = default;

        // C# IWebRequestInterceptingWebView.WebResourceRequested: raised by the platform when a resource
        // is requested; returns true when a subscriber set Handled (so the platform stops its own
        // resolution). The url is the requested absolute url.
        virtual bool web_resource_requested(std::string_view url) = 0;
    };
} // namespace maui::core
