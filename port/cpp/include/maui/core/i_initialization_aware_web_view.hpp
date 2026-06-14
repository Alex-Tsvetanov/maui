#pragma once
// maui::core::i_initialization_aware_web_view  <=  Microsoft.Maui.IInitializationAwareWebView
//
// The mixin contract a web view implements to be notified around platform-view initialization. Ported
// from src/Core/src/Core/IInitializationAwareWebView.cs. C#'s two members —
// WebViewInitializationStarted(args) / WebViewInitializationCompleted(args) — are invoked by the handler
// inside CreatePlatformView, BEFORE and AFTER the WKWebView is built, to let the app configure the
// WKWebViewConfiguration (started) and inspect the finished view (completed); the control raises its
// WebViewInitializing / WebViewInitialized events.
//
// In the port the args objects (C#'s WebViewInitialization{Started,Completed}EventArgs wrapping the
// platform WKWebViewConfiguration / WKWebView) carry NO payload: the configuration object is deeply
// platform-specific and exposing it would leak WebKit types into the cross-platform contract. The hooks
// fire at the same points (around platform-view creation) so the control's lifecycle events are faithful;
// custom WKWebViewConfiguration tweaking through the args is a documented deviation.

namespace maui::core
{
    class i_initialization_aware_web_view
    {
    public:
        i_initialization_aware_web_view() = default;
        virtual ~i_initialization_aware_web_view() = default;
        i_initialization_aware_web_view(const i_initialization_aware_web_view&) = default;
        i_initialization_aware_web_view(i_initialization_aware_web_view&&) = default;
        i_initialization_aware_web_view& operator=(const i_initialization_aware_web_view&) = default;
        i_initialization_aware_web_view& operator=(i_initialization_aware_web_view&&) = default;

        // C# IInitializationAwareWebView.WebViewInitializationStarted: the platform view is about to be
        // created; the control raises WebViewInitializing.
        virtual void web_view_initialization_started() = 0;

        // C# IInitializationAwareWebView.WebViewInitializationCompleted: the platform view has been
        // created; the control raises WebViewInitialized.
        virtual void web_view_initialization_completed() = 0;
    };
} // namespace maui::core
