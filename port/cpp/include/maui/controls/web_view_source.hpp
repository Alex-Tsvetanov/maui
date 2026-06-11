#pragma once
// maui::controls::web_view_source  <=  Microsoft.Maui.Controls.WebViewSource
//
// The abstract base of the web-view source family — a BINDABLE source (UrlWebViewSource's Url and
// HtmlWebViewSource's Html/BaseUrl are bindable properties in C#, and the WebView propagates its
// BindingContext into the source via SetInheritedBindingContext). Ported from
// src/Controls/src/Core/WebView/WebViewSource.cs.
//
// source_changed mirrors C#'s internal SourceChanged event: a concrete source raises it (through
// on_source_changed) whenever its content property changes, and the owning web_view re-raises its own
// "source" property change so the handler re-runs map_source (WebView.OnSourceChanged →
// OnPropertyChanged(SourceProperty.PropertyName)).
//
// C#'s `implicit operator WebViewSource(string/Uri url)` is ported as web_view::set_source(string_view)
// minting a url_web_view_source (no implicit conversions in the port's API surface).
//
// DEVIATION (port mechanics, not surface): C#'s base is BindableObject; the port derives
// maui::controls::element because the runtime binding engine (set_binding + the context-driven reapply)
// lives at the element layer here (W1-02) — a source with a bound Url/Html needs it, exactly like the
// oracle's TestBindingContextPropagatesToSource.

#include "maui/controls/element.hpp"
#include "maui/core/event.hpp"
#include "maui/core/i_web_view_source.hpp"

namespace maui::controls
{
    class web_view_source : public element, public maui::core::i_web_view_source
    {
    public:
        // C# WebViewSource.SourceChanged (internal; the web_view subscribes while this is its Source).
        maui::core::event<> source_changed;

    protected:
        // C# WebViewSource.OnSourceChanged.
        void on_source_changed()
        {
            source_changed.raise();
        }
    };
} // namespace maui::controls
