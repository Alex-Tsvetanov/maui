// hybrid_web_view_handler — the WKWebView + WKScriptMessageHandler bridge, ONE .mm SHARED by BOTH Apple
// backends (the web_view_handler convention): WebKit's WKWebView / WKScriptMessageHandler API is identical
// on AppKit and UIKit — the analog of C#'s single HybridWebViewHandler.iOS.cs covering iOS AND Mac
// Catalyst. Only the generic-IView pushes differ (NSView alphaValue vs UIView alpha, the apple ops helpers
// vs the ios four-fundamental convention), so those sit in small MAUI_PLATFORM_APPLE / MAUI_PLATFORM_IOS
// blocks (exactly one backend is ever compiled, so the branches are build-time disjoint).
//
// Ported from HybridWebViewHandler.iOS.cs + MauiHybridWebView.cs + HybridWebViewHelper.cs:
//   - CreatePlatformView: a WKWebViewConfiguration with AllowsContentJavaScript, the media-playback
//     defaults (config-time only), the "webwindowinterop" WKScriptMessageHandler, and — DEVIATION (see
//     the handler header) — the HybridWebView.js bridge injected as a document-start WKUserScript instead
//     of being served by the app:// scheme handler (the port does not host the asset tree). The
//     WebViewInitialization Started/Completed hooks fire around creation exactly as C# does.
//   - the WKScriptMessageHandler forwards every body string to handler->message_received() (C#'s
//     HybridWebViewHandler.MessageReceived → ProcessRawMessage);
//   - MapSendRawMessage / MapInvokeJavaScriptAsync / MapEvaluateJavaScriptAsync drive
//     -evaluateJavaScript:completionHandler:; the invoke-completion arrives later as a script message
//     (the "__InvokeJavaScriptCompleted" channel routed through message_received), while the evaluate
//     command converts the raw value (HandleWKWebViewResult, reused from the web_view recipe) and
//     completes its request from WebKit's main-thread callback.
// Compiled as Objective-C++ with ARC for the `apple` and `ios` backends.

#ifdef MAUI_PLATFORM_APPLE
    #import <AppKit/AppKit.h>
#endif
#ifdef MAUI_PLATFORM_IOS
    #import <UIKit/UIKit.h>
#endif
#import <WebKit/WebKit.h>
#import <objc/runtime.h>

#include <cmath>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

#include "maui/controls/hybrid_web_view_handler.hpp"
#include "maui/core/evaluate_java_script_request.hpp"
#include "maui/core/hybrid_web_view_protocol.hpp"
#include "maui/core/i_hybrid_web_view.hpp"
#include "maui/core/invoke_java_script_request.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

#ifdef MAUI_PLATFORM_APPLE
    #include "../apple/apple_semantics_ops.hpp"
    #include "../apple/apple_view_ops.hpp"
    #include "../apple/apple_visual_ops.hpp"
#endif

namespace
{
    WKWebView* as_web_view(void* native)
    {
        return (__bridge WKWebView*)native;
    }

    std::string to_std_string(NSString* value)
    {
        const char* const utf8 = value.UTF8String;
        return utf8 != nullptr ? std::string(utf8) : std::string();
    }

    NSString* to_ns_string(std::string_view value)
    {
        const std::string copy(value);
        NSString* const result = [NSString stringWithUTF8String:copy.c_str()];
        return result != nil ? result : @"";
    }

    // WebViewExtensions.HandleWKWebViewResult (reused verbatim from the web_view recipe): nil/NSNull →
    // "null"; strings/numbers verbatim; other shapes through JSON; final fallback is -description.
    std::string handle_wk_web_view_result(id result)
    {
        if (result == nil || [result isKindOfClass:[NSNull class]])
        {
            return "null";
        }
        if ([result isKindOfClass:[NSString class]])
        {
            return to_std_string((NSString*)result);
        }
        if ([result isKindOfClass:[NSNumber class]])
        {
            return to_std_string(((NSNumber*)result).stringValue);
        }
        if ([NSJSONSerialization isValidJSONObject:result])
        {
            NSError* error = nil;
            NSData* const json = [NSJSONSerialization dataWithJSONObject:result
                                                                 options:NSJSONWritingPrettyPrinted
                                                                   error:&error];
            if (error == nil && json != nil)
            {
                NSString* const text = [[NSString alloc] initWithData:json encoding:NSUTF8StringEncoding];
                if (text != nil)
                {
                    return to_std_string(text);
                }
            }
        }
        NSString* const description = [result description];
        return description != nil ? to_std_string(description) : "null";
    }

    // The "webwindowinterop" script message handler name (HybridWebViewHandler.ScriptMessageHandlerName).
    NSString* const k_script_message_handler_name = @"webwindowinterop";

    // The bridge JavaScript injected at document start. Since the port does not host the app:// asset tree,
    // the HybridWebView.js the C# scheme handler would serve is provided here directly — defining the
    // window.external.receiveMessage sink (native→JS raw messages), window.HybridWebView.SendRawMessage /
    // __InvokeJavaScript (JS→native), keyed off window.webkit.messageHandlers.webwindowinterop, exactly as
    // HybridWebView.js does for the WKWebView branch. A faithful subset (InvokeDotNet is out of scope —
    // PROFILE §6 no-reflection).
    NSString* const k_bridge_script = @R"JS((() => {
  let sendMessageFunction = null;
  function dispatchHybridWebViewMessage(message) {
    const event = new CustomEvent('HybridWebViewMessageReceived', { detail: { message: message } });
    window.dispatchEvent(event);
  }
  window.external = { receiveMessage: (message) => { dispatchHybridWebViewMessage(message); } };
  sendMessageFunction = msg => window.webkit.messageHandlers.webwindowinterop.postMessage(msg);
  function sendMessageToDotNet(type, message) {
    const messageToSend = type + '|' + message;
    if (sendMessageFunction) { sendMessageFunction(messageToSend); }
  }
  function invokeJavaScriptCallbackInDotNet(taskId, result) {
    const json = JSON.stringify(result);
    sendMessageToDotNet('__InvokeJavaScriptCompleted', taskId + '|' + json);
  }
  function invokeJavaScriptFailedInDotNet(taskId, error) {
    let errorObj;
    if (!error) { errorObj = { Message: 'Unknown error', StackTrace: Error().stack }; }
    else if (error instanceof Error) { errorObj = { Name: error.name, Message: error.message, StackTrace: error.stack }; }
    else if (typeof error === 'string') { errorObj = { Message: error, StackTrace: Error().stack }; }
    else { errorObj = { Message: JSON.stringify(error), StackTrace: Error().stack }; }
    sendMessageToDotNet('__InvokeJavaScriptFailed', taskId + '|' + JSON.stringify(errorObj));
  }
  function sendRawMessage(message) { sendMessageToDotNet('__RawMessage', message); }
  async function invokeJavaScript(taskId, methodName, args) {
    try { const result = await methodName(...args); invokeJavaScriptCallbackInDotNet(taskId, result); }
    catch (ex) { invokeJavaScriptFailedInDotNet(taskId, ex); }
  }
  window['HybridWebView'] = { SendRawMessage: sendRawMessage, __InvokeJavaScript: invokeJavaScript };
})();)JS";

    // Key for the associated script-message handler kept alive by the WKWebView (the content controller
    // retains it, but the handler back-reference is cleared on disconnect through this object).
    const char k_script_handler_key = 0;
} // namespace

// Obj-C trampoline: forwards every "webwindowinterop" script message to the C++ handler — the port of
// HybridWebViewHandler.WebViewScriptMessageHandler.
@interface MauiCppHybridScriptMessageHandler : NSObject <WKScriptMessageHandler>
@property(nonatomic) maui::controls::hybrid_web_view_handler* handler;
@end

@implementation MauiCppHybridScriptMessageHandler

- (void)userContentController:(WKUserContentController*)userContentController
      didReceiveScriptMessage:(WKScriptMessage*)message
{
    maui::controls::hybrid_web_view_handler* const handler = self.handler;
    if (handler == nullptr || message == nil)
    {
        return;
    }
    if ([message.body isKindOfClass:[NSString class]])
    {
        handler->message_received(to_std_string((NSString*)message.body));
    }
}

@end

namespace maui::controls
{
    hybrid_web_view_platform::~hybrid_web_view_platform()
    {
        if (native != nullptr)
        {
            CFRelease(native); // balances the __bridge_retained in create_platform_view
            native = nullptr;
        }
    }

#ifdef MAUI_PLATFORM_APPLE
    void hybrid_web_view_platform::update_visibility(maui::core::visibility value)
    {
        as_web_view(native).hidden = value != maui::core::visibility::visible;
    }

    void hybrid_web_view_platform::update_opacity(double value)
    {
        as_web_view(native).alphaValue = value;
    }

    void hybrid_web_view_platform::update_automation_id(std::string_view value)
    {
        as_web_view(native).accessibilityIdentifier = to_ns_string(value);
    }

    void hybrid_web_view_platform::update_transform(const maui::core::transform_spec& value)
    {
        maui::platform::apple::apply_transform(native, value);
    }

    void hybrid_web_view_platform::update_flow_direction(maui::core::flow_direction value)
    {
        maui::platform::apple::apply_flow_direction(native, value);
    }

    void hybrid_web_view_platform::update_background(const maui::graphics::paint* value)
    {
        maui::platform::apple::apply_background(native, value);
    }

    void hybrid_web_view_platform::update_shadow(const maui::core::i_shadow* value)
    {
        maui::platform::apple::apply_shadow(native, value);
    }

    void hybrid_web_view_platform::update_clip(const maui::graphics::i_shape* value)
    {
        const NSRect bounds = ((__bridge NSView*)native).bounds;
        maui::platform::apple::apply_clip(
            native, value,
            maui::graphics::rect{bounds.origin.x, bounds.origin.y, bounds.size.width, bounds.size.height});
    }

    void hybrid_web_view_platform::update_semantics(const maui::core::semantics* value)
    {
        maui::platform::apple::apply_semantics((__bridge NSView*)native, value);
    }

    void hybrid_web_view_platform::update_input_transparent(bool value)
    {
        maui::platform::apple::apply_input_transparent((__bridge NSView*)native, value);
    }
#endif

#ifdef MAUI_PLATFORM_IOS
    void hybrid_web_view_platform::update_visibility(maui::core::visibility value)
    {
        as_web_view(native).hidden = value != maui::core::visibility::visible;
    }

    void hybrid_web_view_platform::update_opacity(double value)
    {
        as_web_view(native).alpha = value;
    }

    void hybrid_web_view_platform::update_is_enabled(bool value)
    {
        as_web_view(native).userInteractionEnabled = static_cast<BOOL>(value);
    }

    void hybrid_web_view_platform::update_automation_id(std::string_view value)
    {
        as_web_view(native).accessibilityIdentifier = to_ns_string(value);
    }
#endif

    std::unique_ptr<hybrid_web_view_platform> hybrid_web_view_handler::create_platform_view()
    {
        // HybridWebViewHandler.CreatePlatformView: fire WebViewInitializationStarted (the config is the
        // platform args — not exposed; see i_initialization_aware_web_view), build the configuration with
        // the media-playback defaults + AllowsContentJavaScript + the "webwindowinterop" message handler +
        // the bridge user script, then fire WebViewInitializationCompleted.
        if (auto* view = virtual_view())
        {
            view->web_view_initialization_started();
        }

        auto platform = std::make_unique<hybrid_web_view_platform>();

        WKWebViewConfiguration* const configuration = [[WKWebViewConfiguration alloc] init];
#ifdef MAUI_PLATFORM_IOS
        configuration.allowsPictureInPictureMediaPlayback = YES;
        configuration.allowsInlineMediaPlayback = YES;
#endif
        configuration.mediaTypesRequiringUserActionForPlayback = WKAudiovisualMediaTypeNone;
        configuration.defaultWebpagePreferences.allowsContentJavaScript = YES;

        MauiCppHybridScriptMessageHandler* const script_handler = [[MauiCppHybridScriptMessageHandler alloc] init];
        script_handler.handler = this;
        [configuration.userContentController addScriptMessageHandler:script_handler name:k_script_message_handler_name];

        // DEVIATION: inject the bridge JS directly (the port does not serve HybridWebView.js via the app://
        // scheme handler). Document-start so the page sees window.HybridWebView / window.external from the
        // first script it runs.
        WKUserScript* const bridge = [[WKUserScript alloc] initWithSource:k_bridge_script
                                                            injectionTime:WKUserScriptInjectionTimeAtDocumentStart
                                                         forMainFrameOnly:NO];
        [configuration.userContentController addUserScript:bridge];

        WKWebView* const web_view = [[WKWebView alloc] initWithFrame:CGRectZero configuration:configuration];
#ifdef MAUI_PLATFORM_IOS
        web_view.backgroundColor = UIColor.clearColor;
        web_view.autoresizesSubviews = YES;
#endif
        platform->native = (__bridge_retained void*)web_view; // the void* slot owns one reference
        // Keep the script handler alive for the web view's lifetime + reachable for disconnect cleanup.
        objc_setAssociatedObject(web_view, &k_script_handler_key, script_handler, OBJC_ASSOCIATION_RETAIN_NONATOMIC);

        if (auto* view = virtual_view())
        {
            view->web_view_initialization_completed();
        }
        return platform;
    }

    void hybrid_web_view_handler::on_connect_handler(hybrid_web_view_platform& platform)
    {
        platform.connected_view = virtual_view();
    }

    void hybrid_web_view_handler::on_disconnect_handler(hybrid_web_view_platform& platform)
    {
        platform.connected_view = nullptr;
        WKWebView* const web_view = as_web_view(platform.native);
        // HybridWebViewHandler.DisconnectHandler: remove the script message handler so the retained
        // trampoline drops its handler back-reference and stops routing.
        auto* const script_handler =
            (MauiCppHybridScriptMessageHandler*)objc_getAssociatedObject(web_view, &k_script_handler_key);
        if (script_handler != nil)
        {
            script_handler.handler = nullptr;
        }
        [web_view.configuration.userContentController removeScriptMessageHandlerForName:k_script_message_handler_name];
        objc_setAssociatedObject(web_view, &k_script_handler_key, nil, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
    }

    // HybridWebViewHandler.MapSendRawMessage → MauiHybridWebView.SendRawMessage: evaluate
    // window.external.receiveMessage(<json message>).
    void hybrid_web_view_handler::map_send_raw_message(hybrid_web_view_handler& handler,
                                                       maui::core::i_hybrid_web_view& /*view*/, const std::any& args)
    {
        auto* platform = handler.typed_platform_view();
        const auto* message = std::any_cast<std::string>(&args);
        if (platform == nullptr || platform->native == nullptr || message == nullptr)
        {
            return;
        }
        const std::string script = maui::core::build_send_raw_message_script(*message);
        [as_web_view(platform->native) evaluateJavaScript:to_ns_string(script) completionHandler:nil];
    }

    // HybridWebViewHandler.MapInvokeJavaScriptAsync → ProcessInvokeJavaScriptAsync: mint a task id,
    // evaluate window.HybridWebView.__InvokeJavaScript(...). The completion arrives later as a
    // "__InvokeJavaScriptCompleted" script message routed through message_received. The Promise the JS
    // returns is intentionally NOT awaited here (iOS can't bridge a Promise to a string).
    void hybrid_web_view_handler::map_invoke_java_script(hybrid_web_view_handler& handler,
                                                         maui::core::i_hybrid_web_view& /*view*/, const std::any& args)
    {
        const auto* request_ptr = std::any_cast<std::shared_ptr<maui::core::invoke_java_script_request>>(&args);
        if (request_ptr == nullptr || *request_ptr == nullptr)
        {
            return;
        }
        const std::shared_ptr<maui::core::invoke_java_script_request> request = *request_ptr;
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            request->complete(std::nullopt); // C# SetCanceled when the platform view is gone.
            return;
        }
        const std::string task_id = handler.create_invoke_task(request);
        const std::string script =
            maui::core::build_invoke_java_script_script(task_id, request->method_name(), request->param_values());
        [as_web_view(platform->native) evaluateJavaScript:to_ns_string(script) completionHandler:nil];
    }

    // HybridWebViewHandler.MapEvaluateJavaScriptAsync: run the (already escaped+wrapped) script and complete
    // the request from WebKit's main-thread completion callback. DEVIATION: an NSError completes with
    // "null" (the web_view recipe — the callback channel has no faulted state).
    void hybrid_web_view_handler::map_evaluate_java_script(hybrid_web_view_handler& handler,
                                                           maui::core::i_hybrid_web_view& /*view*/,
                                                           const std::any& args)
    {
        const auto* request_ptr = std::any_cast<std::shared_ptr<maui::core::evaluate_java_script_request>>(&args);
        if (request_ptr == nullptr || *request_ptr == nullptr)
        {
            return;
        }
        const std::shared_ptr<maui::core::evaluate_java_script_request> request = *request_ptr;
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            request->complete("null");
            return;
        }
        [as_web_view(platform->native)
            evaluateJavaScript:to_ns_string(request->script())
             completionHandler:^(id result, NSError* error) {
               request->complete(error != nil ? std::string("null") : handle_wk_web_view_result(result));
             }];
    }

    maui::graphics::size hybrid_web_view_handler::get_desired_size(double width_constraint,
                                                                   double height_constraint) const
    {
        const auto* platform = typed_platform_view();
        double width = 0;
        double height = 0;
        if (platform != nullptr && platform->native != nullptr)
        {
#ifdef MAUI_PLATFORM_IOS
            const CGFloat fit_width =
                std::isfinite(width_constraint) ? static_cast<CGFloat>(width_constraint) : CGFLOAT_MAX;
            const CGFloat fit_height =
                std::isfinite(height_constraint) ? static_cast<CGFloat>(height_constraint) : CGFLOAT_MAX;
            const CGSize fitting = [as_web_view(platform->native) sizeThatFits:CGSizeMake(fit_width, fit_height)];
            width = fitting.width;
            height = fitting.height;
#else
            const NSSize fitting = as_web_view(platform->native).fittingSize;
            width = fitting.width;
            height = fitting.height;
#endif
        }
        if (width == 0 && (width_constraint <= 0 || !std::isfinite(width_constraint)))
        {
            width = minimum_size;
        }
        if (height == 0 && (height_constraint <= 0 || !std::isfinite(height_constraint)))
        {
            height = minimum_size;
        }
        return {width, height};
    }

    void hybrid_web_view_handler::platform_arrange(const maui::graphics::rect& frame)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        [as_web_view(platform->native) setFrame:CGRectMake(frame.x, frame.y, frame.width, frame.height)];
    }
} // namespace maui::controls
