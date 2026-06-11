#pragma once
// maui::core::i_web_view_source  <=  Microsoft.Maui.IWebViewSource
//
// The data provider for a web view: a source knows how to push its content (a url or an html string)
// into the platform view through the i_web_view_delegate. Ported from src/Core/src/Core/IWebViewSource.cs.
// The concrete sources live in the Controls layer (maui::controls::url_web_view_source /
// html_web_view_source ⇐ UrlWebViewSource / HtmlWebViewSource), mirroring C#'s split.

namespace maui::core
{
    class i_web_view_delegate;

    class i_web_view_source
    {
    public:
        i_web_view_source() = default;
        virtual ~i_web_view_source() = default;
        i_web_view_source(const i_web_view_source&) = default;
        i_web_view_source(i_web_view_source&&) = default;
        i_web_view_source& operator=(const i_web_view_source&) = default;
        i_web_view_source& operator=(i_web_view_source&&) = default;

        // C# IWebViewSource.Load: push this source's content into the platform view.
        virtual void load(i_web_view_delegate& web_view_delegate) = 0;
    };
} // namespace maui::core
