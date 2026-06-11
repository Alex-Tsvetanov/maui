#pragma once
// maui::controls::web_navigation_event_args  <=  Microsoft.Maui.Controls.WebNavigationEventArgs
// maui::controls::web_navigating_event_args  <=  Microsoft.Maui.Controls.WebNavigatingEventArgs
// maui::controls::web_navigated_event_args   <=  Microsoft.Maui.Controls.WebNavigatedEventArgs
//
// The argument family of the web_view navigating/navigated events, ported from
// src/Controls/src/Core/WebView/WebNavigationEventArgs.cs + WebNavigatingEventArgs.cs +
// WebNavigatedEventArgs.cs (one header for the three — they are a single small hierarchy).
// `source` carries a url_web_view_source minted for the navigated url, exactly as C#'s
// IWebView.Navigating/Navigated implementations build `new UrlWebViewSource { Url = url }`.
//
// web_navigating_event_args is MUTABLE shared state across handlers — the web_view raises it as
// event<web_navigating_event_args&> so a subscriber can set `cancel` (C# WebNavigatingEventArgs.Cancel)
// and the platform aborts the navigation.

#include <memory>
#include <string>

#include "maui/core/web_navigation_event.hpp"
#include "maui/core/web_navigation_result.hpp"

namespace maui::controls
{
    class web_view_source;

    struct web_navigation_event_args
    {
        maui::core::web_navigation_event navigation_event = maui::core::web_navigation_event::new_page;
        std::shared_ptr<web_view_source> source;
        std::string url;
    };

    struct web_navigating_event_args : web_navigation_event_args
    {
        bool cancel = false;
    };

    struct web_navigated_event_args : web_navigation_event_args
    {
        maui::core::web_navigation_result result = maui::core::web_navigation_result::success;
    };
} // namespace maui::controls
