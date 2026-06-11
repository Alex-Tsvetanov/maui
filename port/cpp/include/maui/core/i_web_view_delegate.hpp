#pragma once
// maui::core::i_web_view_delegate  <=  Microsoft.Maui.IWebViewDelegate
//
// The load sink a web-view SOURCE pushes its content into. Ported from
// src/Core/src/Core/IWebViewDelegate.cs. On every backend the web view's PLATFORM view implements this
// (C#'s MauiWKWebView : WKWebView, IWebViewDelegate): map_source hands the platform view to
// i_web_view_source::load, which calls back load_url / load_html with the content to display.
//
// C#'s `string?` parameters are modeled as string_views: the no-value case is the empty string (an empty
// html is not loaded; an empty base_url means "use the default base", mirroring the null checks in
// MauiWKWebView.LoadHtml / LoadUrl).

#include <string_view>

namespace maui::core
{
    class i_web_view_delegate
    {
    public:
        i_web_view_delegate() = default;
        virtual ~i_web_view_delegate() = default;
        i_web_view_delegate(const i_web_view_delegate&) = default;
        i_web_view_delegate(i_web_view_delegate&&) = default;
        i_web_view_delegate& operator=(const i_web_view_delegate&) = default;
        i_web_view_delegate& operator=(i_web_view_delegate&&) = default;

        virtual void load_html(std::string_view html, std::string_view base_url) = 0;
        virtual void load_url(std::string_view url) = 0;
    };
} // namespace maui::core
