#pragma once
// maui::controls::hybrid_web_view_handler  <=  Microsoft.Maui.Handlers.HybridWebViewHandler
//
// The handler for a view presenting local HTML content with a JS↔native message bridge. Ported from
// HybridWebViewHandler.cs (cross-platform mapper tables) + HybridWebViewHandler.iOS.cs + MauiHybridWebView.cs
// (the WKWebView recipe shared by BOTH Apple backends) + HybridWebViewHelper.cs (the message protocol).
//
// The control drives three handler COMMANDS (HybridWebViewHandler.CommandMapper):
//   - "send_raw_message"     (HybridWebViewRawMessage)            → MauiHybridWebView.SendRawMessage:
//                            evaluateJavaScript window.external.receiveMessage(<json message>);
//   - "invoke_java_script"   (shared_ptr<invoke_java_script_request>) → ProcessInvokeJavaScriptAsync:
//                            mint a task id, evaluateJavaScript window.HybridWebView.__InvokeJavaScript(...),
//                            and complete the request when the "__InvokeJavaScriptCompleted" raw message
//                            arrives (the callback-channel idiom — see invoke_java_script_request);
//   - "evaluate_java_script" (shared_ptr<evaluate_java_script_request>) → EvaluateJavaScript: the same
//                            escape/wrap/unquote pipeline web_view uses (reused verbatim).
// The INBOUND channel is the JS→native script-message handler ("webwindowinterop"): every body string is
// routed through message_received(), which parses the "<type>|<content>" protocol and either delivers a
// raw message to the virtual view (raw_message_received) or completes/fails the matching invoke task.
//
// Partial-class split (PROFILE §5): the mapper TABLES + ctor + the protocol-routing message_received() are
// cross-platform (hybrid_web_view_handler.cpp); the platform recipe (create/connect/disconnect/map_*/
// measure) is per backend. The apple and ios backends share ONE WKWebView .mm
// (src/platform/apple_shared/hybrid_web_view_handler.mm — WKScriptMessageHandler is identical on AppKit
// and UIKit, like the web_view handler); the headless partial
// (src/platform/headless/hybrid_web_view_handler.cpp) mirrors the bridge synchronously so the protocol +
// round trip are fully unit-testable.
//
// hybrid_web_view_platform is the single cross-platform platform-view struct: `native` holds the WKWebView*
// (retained in the .mm; unused headless). The headless mirrors record every sent native→JS script, the
// received JS→native raw messages, and the live invoke-task table; the script_responder seam lets a test
// drive a JS reply for an evaluated script (the headless analog of the page answering).
//
// OUT OF SCOPE (documented, not stubbed): SetInvokeJavaScriptTarget + InvokeDotNet (JS → .NET method
// invocation needs reflection — PROFILE §6); the app:// scheme handler serving local assets from
// HybridRoot/DefaultFile (a content-root file server, not the message bridge) — on apple/ios the page is
// loaded directly so the injected JS + bridge work; serving the asset tree is a later content-server unit.

#include <any>
#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "maui/core/command_mapper.hpp"
#include "maui/core/evaluate_java_script_request.hpp"
#include "maui/core/i_hybrid_web_view.hpp"
#include "maui/core/invoke_java_script_request.hpp"
#include "maui/core/move_only_function.hpp"
#include "maui/core/property_mapper.hpp"
#include "maui/core/view_handler.hpp"
#include "maui/core/view_platform_base.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

namespace maui::controls
{
    struct hybrid_web_view_platform : maui::core::view_platform_base
    {
        hybrid_web_view_platform() = default;
        ~hybrid_web_view_platform() override; // backend-defined: releases the retained WKWebView on apple/ios
        hybrid_web_view_platform(const hybrid_web_view_platform&) = delete;
        hybrid_web_view_platform(hybrid_web_view_platform&&) = delete;
        hybrid_web_view_platform& operator=(const hybrid_web_view_platform&) = delete;
        hybrid_web_view_platform& operator=(hybrid_web_view_platform&&) = delete;

        void* native = nullptr;

        // The connected virtual view (wired by on_connect_handler, cleared on disconnect) — the inbound
        // channel raw messages drive via raw_message_received.
        maui::core::i_hybrid_web_view* connected_view = nullptr;

        // The live InvokeJavaScript round trips, keyed by the task id the handler minted
        // (HybridWebViewTaskManager._asyncTaskCallbacks). A "__InvokeJavaScript{Completed,Failed}" raw
        // message removes + completes its entry.
        std::map<std::string, std::shared_ptr<maui::core::invoke_java_script_request>> pending_invokes;
        std::uint64_t last_task_id = 0; // HybridWebViewTaskManager._lastTaskId

        // ---- headless mirrors (the apple/ios build pushes to `native` instead) ----
        // Every native→JS script the handler evaluated (send_raw_message + invoke_java_script +
        // evaluate_java_script all funnel through here in order).
        std::vector<std::string> evaluated_scripts;
        // The raw message content delivered to the virtual view (the "__RawMessage" payloads received).
        std::vector<std::string> received_raw_messages;
        // The raw messages the host SENT to the page (the json-quoted argument of receiveMessage), in
        // builder form — the script is also in evaluated_scripts.
        std::vector<std::string> sent_raw_messages;
        // The headless answer seam: given an evaluate_java_script script, return its raw result (the page
        // answering). Unset => "null" (an errored/void script), matching the WKWebView value.
        maui::core::move_only_function<std::string(const std::string& script)> script_responder;

#ifdef MAUI_PLATFORM_APPLE
        // Apple backend: the generic IView pushes onto the AppKit WKWebView (an NSView). is_enabled keeps
        // the base mirror — a WKWebView is not an NSControl (the web_view_platform precedent).
        void update_visibility(maui::core::visibility value) override;
        void update_opacity(double value) override;
        void update_automation_id(std::string_view value) override;
        void update_transform(const maui::core::transform_spec& value) override;
        void update_flow_direction(maui::core::flow_direction value) override;
        void update_background(const maui::graphics::paint* value) override;
        void update_shadow(const maui::core::i_shadow* value) override;
        void update_clip(const maui::graphics::i_shape* value) override;
        void update_semantics(const maui::core::semantics* value) override;
        void update_input_transparent(bool value) override;
#endif

#ifdef MAUI_PLATFORM_IOS
        // iOS backend (M6 fan-out convention): the four fundamental IView pushes onto the UIKit WKWebView.
        void update_visibility(maui::core::visibility value) override;
        void update_opacity(double value) override;
        void update_is_enabled(bool value) override;
        void update_automation_id(std::string_view value) override;
#endif
    };

    class hybrid_web_view_handler
        : public maui::core::view_handler<hybrid_web_view_handler, maui::core::i_hybrid_web_view,
                                          hybrid_web_view_platform>
    {
    public:
        hybrid_web_view_handler();

        static maui::core::property_mapper<maui::core::i_hybrid_web_view, hybrid_web_view_handler>& mapper();
        static maui::core::command_mapper<maui::core::i_hybrid_web_view, hybrid_web_view_handler>& command_mapper();

        // Platform recipe (per backend). create_platform_view is NON-static (C#'s instance
        // CreatePlatformView): the .mm fires the IInitializationAwareWebView hooks around creation and
        // wires the script-message handler's back-reference, both of which need the virtual view / this.
        std::unique_ptr<hybrid_web_view_platform> create_platform_view();
        void on_connect_handler(hybrid_web_view_platform& platform);
        static void on_disconnect_handler(hybrid_web_view_platform& platform);

        [[nodiscard]] maui::graphics::size get_desired_size(double width_constraint,
                                                            double height_constraint) const override;
        void platform_arrange(const maui::graphics::rect& frame) override;

        // C# HybridWebViewHandler/WebViewHandler.MinimumSize fallback (the web_view precedent).
        static constexpr double minimum_size = 44.0;

        // Command map (platform recipe) — HybridWebViewHandler.CommandMapper's entries.
        // args: a std::string — the raw message (C# MapSendRawMessage with HybridWebViewRawMessage).
        static void map_send_raw_message(hybrid_web_view_handler& handler, maui::core::i_hybrid_web_view& view,
                                         const std::any& args);
        // args: a std::shared_ptr<invoke_java_script_request> (C# MapInvokeJavaScriptAsync).
        static void map_invoke_java_script(hybrid_web_view_handler& handler, maui::core::i_hybrid_web_view& view,
                                           const std::any& args);
        // args: a std::shared_ptr<evaluate_java_script_request> (C# MapEvaluateJavaScriptAsync — the same
        // pipeline as web_view).
        static void map_evaluate_java_script(hybrid_web_view_handler& handler, maui::core::i_hybrid_web_view& view,
                                             const std::any& args);

        // The JS→native ingestion point (C# HybridWebViewHandler.MessageReceived →
        // HybridWebViewHelper.ProcessRawMessage). The script-message handler / the headless mirror call
        // it with every body string; it parses the protocol and routes raw messages to the virtual view
        // or completes/fails the matching invoke task. Public so the per-backend .mm bridge can reach it.
        void message_received(std::string_view raw_message);

    private:
        // HybridWebViewTaskManager.CreateTask: mint the next task id and register the request.
        [[nodiscard]] std::string create_invoke_task(
            const std::shared_ptr<maui::core::invoke_java_script_request>& request);
    };
} // namespace maui::controls
