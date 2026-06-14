#pragma once
// maui::core::i_hybrid_web_view  <=  Microsoft.Maui.IHybridWebView
//
// The virtual-view contract for a view that presents LOCAL HTML content and lets JavaScript and the host
// communicate through raw messages and method invocation. Ported from src/Core/src/Core/IHybridWebView.cs
// (IHybridWebView : IView, IWebRequestInterceptingWebView, IInitializationAwareWebView).
//
// Channels (mirroring the C# member roles):
//   - default_file() / hybrid_root() are the content-root knobs the handler reads when serving local
//     assets (DefaultFile within HybridRoot). They are outbound config, not mapped per-change.
//   - raw_message_received(raw_message) is the INBOUND raw-message channel (C# IHybridWebView
//     .RawMessageReceived): the handler calls it when a "__RawMessage" arrives from the web view, and the
//     control raises its raw_message_received event.
//   - send_raw_message / invoke_js / evaluate_js are NOT contract methods here: in the port the control
//     drives them as handler COMMANDS ("send_raw_message" / "invoke_java_script" / "evaluate_java_script"
//     — HybridWebViewHandler.CommandMapper's keys), so the i_* surface carries only what the handler reads
//     or pushes.
//
// The C# variant bases — IWebRequestInterceptingWebView (WebResourceRequested) and
// IInitializationAwareWebView (WebViewInitialization Started/Completed) — are ported as the separate
// contracts i_web_request_intercepting_web_view / i_initialization_aware_web_view, which this interface
// inherits exactly as C# does. See those headers for the per-backend feasibility notes.
//
// OUT OF SCOPE (documented, not stubbed — the no-reflection consequence, PROFILE §6): SetInvokeJavaScriptTarget
// + InvokeDotNet (JavaScript → .NET method invocation). C# resolves the target method by reflection
// (Type.GetMethod) and JSON-deserializes the parameters to the .NET parameter types; C++23 has no
// reflection, so the `window.HybridWebView.InvokeDotNet` path and the InvokeJavaScriptTarget/Type members
// are not ported. The native→JS / JS→native RAW message channel and InvokeJavaScript (host → JS) ARE
// fully ported.

#include <string>
#include <string_view>

#include "maui/core/i_initialization_aware_web_view.hpp"
#include "maui/core/i_view.hpp"
#include "maui/core/i_web_request_intercepting_web_view.hpp"

namespace maui::core
{
    class i_hybrid_web_view : public i_view,
                              public i_web_request_intercepting_web_view,
                              public i_initialization_aware_web_view
    {
    public:
        // C# IHybridWebView.DefaultFile — the file served as the default within hybrid_root (default
        // "index.html"). The empty string models C#'s null.
        [[nodiscard]] virtual std::string default_file() const = 0;

        // C# IHybridWebView.HybridRoot — the app "Raw" asset folder holding the web app's contents
        // (default "wwwroot"). The empty string models C#'s null.
        [[nodiscard]] virtual std::string hybrid_root() const = 0;

        // C# IHybridWebView.RawMessageReceived: raised by the platform when a raw message arrives from the
        // web view ("__RawMessage|<content>"); the control raises its raw_message_received event.
        virtual void raw_message_received(std::string_view raw_message) = 0;
    };
} // namespace maui::core
