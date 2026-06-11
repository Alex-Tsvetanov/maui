// web_view_handler — the WKWebView platform recipe, ONE .mm SHARED by BOTH Apple backends (the
// gcd_dispatcher / coregraphics_canvas convention): WebKit's WKWebView API is identical on AppKit and
// UIKit — the analog of C#'s single WebViewHandler.iOS.cs covering iOS AND Mac Catalyst. Only the
// generic-IView pushes differ (NSView alphaValue vs UIView alpha, the apple ops helpers vs the ios
// four-fundamental convention), so those sit in small MAUI_PLATFORM_APPLE / MAUI_PLATFORM_IOS blocks
// (exactly one backend is ever compiled, so the branches are build-time disjoint).
//
// Ported from WebViewHandler.iOS.cs + MauiWKWebView.cs + MauiWebViewNavigationDelegate.cs +
// WebViewExtensions.cs (cookie sync / UserAgent / WKUIDelegate are out of scope — see the handler
// header):
//   - the platform struct IS the i_web_view_delegate (MauiWKWebView : IWebViewDelegate): load_html →
//     loadHTMLString:baseURL: (a missing base url falls back to the main bundle path, as C# does);
//     load_url → loadRequest: (an unparsable url falls back to a bundled-file load, C#'s LoadFile);
//   - an Obj-C MauiWebViewNavigationDelegate trampoline forwards decidePolicyForNavigationAction →
//     send_navigating (cancel support + the CurrentNavigationEvent kind for BackForward/Reload) and
//     didFinish/didFail[Provisional]Navigation → send_navigated(success/failure), refreshing the
//     handler-pushed CanGoBack/CanGoForward around each step (UpdateCanGoBackForward);
//   - the eval / evaluate_java_script commands run through -evaluateJavaScript:completionHandler:; the
//     completion (delivered by WebKit on the main thread — the dispatcher-marshalled-callback model)
//     converts the raw value exactly as HandleWKWebViewResult does and completes the request.
//     DEVIATION: an NSError from WebKit completes the request with "null" (C# faults the awaited task;
//     the port's callback channel has no faulted state — see evaluate_java_script_request.hpp).
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
#include <string>
#include <string_view>
#include <utility>

#include "maui/core/evaluate_java_script_request.hpp"
#include "maui/core/i_web_view.hpp"
#include "maui/core/i_web_view_source.hpp"
#include "maui/core/visibility.hpp"
#include "maui/core/web_navigation_event.hpp"
#include "maui/core/web_navigation_result.hpp"
#include "maui/core/web_view_handler.hpp"
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

    // -[NSString UTF8String] is nullable-annotated; guard the std::string construction.
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

    // MauiWebViewNavigationDelegate.GetCurrentUrl: PlatformView.Url?.AbsoluteUrl?.ToString() ?? "".
    std::string current_url(WKWebView* web_view)
    {
        NSURL* const url = web_view.URL.absoluteURL;
        return url != nil ? to_std_string(url.absoluteString) : std::string();
    }

    // WebViewExtensions.UpdateCanGoBackForward: push the platform's back/forward state into the
    // virtual view's handler-pushed read-onlys.
    void update_can_go_back_forward(maui::core::i_web_view& view, WKWebView* web_view)
    {
        view.set_can_go_back(web_view.canGoBack != NO);
        view.set_can_go_forward(web_view.canGoForward != NO);
    }

    // WebViewExtensions.HandleWKWebViewResult: nil/NSNull → "null"; strings/numbers verbatim; other
    // shapes (NSDictionary/NSArray) through JSON serialization; final fallback is -description.
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

    // Key for the associated navigation delegate kept alive by the WKWebView (navigationDelegate is weak).
    const char k_navigation_delegate_key = 0;
} // namespace

// Obj-C trampoline: forwards the WKNavigationDelegate callbacks to the C++ handler's virtual view —
// the port of MauiWebViewNavigationDelegate.cs (which C# attaches in the MauiWKWebView ctor; the port
// attaches it in on_connect_handler so it can carry the handler back-reference).
@interface MauiCppWebViewNavigationDelegate : NSObject <WKNavigationDelegate>
@property(nonatomic) maui::core::web_view_handler* handler;
// C# _lastEvent: the kind decidePolicy resolved, reported by the completion callbacks.
@property(nonatomic) maui::core::web_navigation_event lastEvent;
@end

@implementation MauiCppWebViewNavigationDelegate

- (instancetype)init
{
    self = [super init];
    if (self != nil)
    {
        _lastEvent = maui::core::web_navigation_event::new_page;
    }
    return self;
}

// MauiWebViewNavigationDelegate.DecidePolicy.
- (void)webView:(WKWebView*)webView
    decidePolicyForNavigationAction:(WKNavigationAction*)navigationAction
                    decisionHandler:(void (^)(WKNavigationActionPolicy))decisionHandler
{
    maui::core::web_view_handler* const handler = self.handler;
    auto* const virtual_view = handler != nullptr ? handler->virtual_view() : nullptr;
    auto* const platform = handler != nullptr ? handler->typed_platform_view() : nullptr;
    if (virtual_view == nullptr || platform == nullptr)
    {
        decisionHandler(WKNavigationActionPolicyCancel);
        return;
    }

    auto nav_event = maui::core::web_navigation_event::new_page;
    switch (navigationAction.navigationType)
    {
        case WKNavigationTypeBackForward:
            nav_event = platform->current_navigation_event;
            break;
        case WKNavigationTypeReload:
            nav_event = maui::core::web_navigation_event::refresh;
            break;
        case WKNavigationTypeLinkActivated:
        case WKNavigationTypeFormSubmitted:
        case WKNavigationTypeFormResubmitted:
        case WKNavigationTypeOther:
        default:
            nav_event = maui::core::web_navigation_event::new_page;
            break;
    }
    self.lastEvent = nav_event;

    const std::string last_url = to_std_string(navigationAction.request.URL.absoluteString);
    const bool cancel = virtual_view->send_navigating(nav_event, last_url);
    update_can_go_back_forward(*virtual_view, webView);

    // target="_blank" (no target frame): load the request in this web view ourselves and always cancel
    // the original navigation.
    if (navigationAction.targetFrame == nil)
    {
        if (!cancel)
        {
            [webView loadRequest:navigationAction.request];
        }
        decisionHandler(WKNavigationActionPolicyCancel);
        return;
    }
    decisionHandler(cancel ? WKNavigationActionPolicyCancel : WKNavigationActionPolicyAllow);
}

// MauiWebViewNavigationDelegate.DidFinishNavigation (the cookie re-sync of ProcessNavigatedAsync is out
// of scope; its trailing UpdateCanGoBackForward is kept).
- (void)webView:(WKWebView*)webView didFinishNavigation:(WKNavigation*)navigation
{
    maui::core::web_view_handler* const handler = self.handler;
    auto* const virtual_view = handler != nullptr ? handler->virtual_view() : nullptr;
    if (virtual_view == nullptr)
    {
        return;
    }
    update_can_go_back_forward(*virtual_view, webView);
    if (webView.loading)
    {
        return;
    }
    virtual_view->send_navigated(self.lastEvent, current_url(webView), maui::core::web_navigation_result::success);
    update_can_go_back_forward(*virtual_view, webView);
}

// MauiWebViewNavigationDelegate.DidFailNavigation.
- (void)webView:(WKWebView*)webView didFailNavigation:(WKNavigation*)navigation withError:(NSError*)error
{
    [self reportFailure:webView];
}

// MauiWebViewNavigationDelegate.DidFailProvisionalNavigation.
- (void)webView:(WKWebView*)webView didFailProvisionalNavigation:(WKNavigation*)navigation withError:(NSError*)error
{
    [self reportFailure:webView];
}

- (void)reportFailure:(WKWebView*)webView
{
    maui::core::web_view_handler* const handler = self.handler;
    auto* const virtual_view = handler != nullptr ? handler->virtual_view() : nullptr;
    if (virtual_view == nullptr)
    {
        return;
    }
    virtual_view->send_navigated(self.lastEvent, current_url(webView), maui::core::web_navigation_result::failure);
    update_can_go_back_forward(*virtual_view, webView);
}

@end

namespace maui::core
{
    web_view_platform::~web_view_platform()
    {
        if (native != nullptr)
        {
            CFRelease(native); // balances the __bridge_retained in create_platform_view
            native = nullptr;
        }
    }

    // ---- i_web_view_delegate (the source's load sink; MauiWKWebView.LoadHtml/LoadUrl) ----
    void web_view_platform::load_html(std::string_view html, std::string_view base_url)
    {
        if (html.empty())
        {
            return; // C# LoadHtml: only a non-null html loads
        }
        last_source_kind = web_view_source_kind::html;
        last_html = std::string(html);
        last_base_url = std::string(base_url);
        // C#: a null baseUrl falls back to the main bundle path (so relative resources resolve).
        NSURL* const base = base_url.empty() ? [NSURL fileURLWithPath:NSBundle.mainBundle.bundlePath isDirectory:YES]
                                             : [NSURL URLWithString:to_ns_string(base_url)];
        [as_web_view(native) loadHTMLString:to_ns_string(html) baseURL:base];
    }

    void web_view_platform::load_url(std::string_view url)
    {
        last_source_kind = web_view_source_kind::url;
        last_url = std::string(url);
        NSString* const text = to_ns_string(url);
        NSURL* const ns_url = [NSURL URLWithString:text];
        if (ns_url != nil && ns_url.fileURL)
        {
            // A file:// url goes through the sandboxed-read entry point (the C# LoadFileUrl channel),
            // granting read access to its directory so sibling resources resolve.
            [as_web_view(native) loadFileURL:ns_url allowingReadAccessToURL:ns_url.URLByDeletingLastPathComponent];
            return;
        }
        if (ns_url != nil && ns_url.scheme != nil)
        {
            [as_web_view(native) loadRequest:[NSURLRequest requestWithURL:ns_url]];
            return;
        }
        // C# LoadUrlAsync's UriFormatException fallback (LoadFile): try the url as a bundled resource.
        NSString* const file = [text.lastPathComponent stringByDeletingPathExtension];
        NSString* const extension = text.pathExtension;
        NSURL* const resource = [NSBundle.mainBundle URLForResource:file withExtension:extension];
        if (resource != nil)
        {
            [as_web_view(native) loadFileURL:resource allowingReadAccessToURL:resource];
        }
    }

#ifdef MAUI_PLATFORM_APPLE
    // The generic-IView property pushes onto the AppKit WKWebView (an NSView). is_enabled keeps the
    // view_platform_base mirror — a WKWebView is not an NSControl (the layout_platform precedent).
    void web_view_platform::update_visibility(maui::core::visibility value)
    {
        as_web_view(native).hidden = value != maui::core::visibility::visible;
    }

    void web_view_platform::update_opacity(double value)
    {
        as_web_view(native).alphaValue = value;
    }

    void web_view_platform::update_automation_id(std::string_view value)
    {
        as_web_view(native).accessibilityIdentifier = to_ns_string(value);
    }

    void web_view_platform::update_transform(const maui::core::transform_spec& value)
    {
        maui::platform::apple::apply_transform(native, value);
    }

    void web_view_platform::update_flow_direction(maui::core::flow_direction value)
    {
        maui::platform::apple::apply_flow_direction(native, value);
    }

    void web_view_platform::update_background(const maui::graphics::paint* value)
    {
        maui::platform::apple::apply_background(native, value);
    }

    void web_view_platform::update_shadow(const maui::core::i_shadow* value)
    {
        maui::platform::apple::apply_shadow(native, value);
    }

    void web_view_platform::update_clip(const maui::graphics::i_shape* value)
    {
        // The clip mask is sized to the view's current bounds (WrapperView.SetClip uses the view frame).
        const NSRect bounds = ((__bridge NSView*)native).bounds;
        maui::platform::apple::apply_clip(
            native, value,
            maui::graphics::rect{bounds.origin.x, bounds.origin.y, bounds.size.width, bounds.size.height});
    }

    void web_view_platform::update_semantics(const maui::core::semantics* value)
    {
        maui::platform::apple::apply_semantics((__bridge NSView*)native, value);
    }

    void web_view_platform::update_input_transparent(bool value)
    {
        maui::platform::apple::apply_input_transparent((__bridge NSView*)native, value);
    }
#endif

#ifdef MAUI_PLATFORM_IOS
    // The four fundamental IView pushes onto the UIKit WKWebView (the M6 fan-out convention).
    void web_view_platform::update_visibility(maui::core::visibility value)
    {
        as_web_view(native).hidden = value != maui::core::visibility::visible;
    }

    void web_view_platform::update_opacity(double value)
    {
        as_web_view(native).alpha = value;
    }

    void web_view_platform::update_is_enabled(bool value)
    {
        // ViewExtensions.UpdateIsEnabled's non-UIControl branch: the interaction toggle only.
        as_web_view(native).userInteractionEnabled = static_cast<BOOL>(value);
    }

    void web_view_platform::update_automation_id(std::string_view value)
    {
        as_web_view(native).accessibilityIdentifier = to_ns_string(value);
    }
#endif

    std::unique_ptr<web_view_platform> web_view_handler::create_platform_view()
    {
        auto platform = std::make_unique<web_view_platform>();
        // MauiWKWebView.CreateConfiguration: the media-playback defaults must be set at creation time,
        // and the process pool is SHARED across instances so cookies synchronize.
        WKWebViewConfiguration* const configuration = [[WKWebViewConfiguration alloc] init];
#ifdef MAUI_PLATFORM_IOS
        configuration.allowsPictureInPictureMediaPlayback = YES;
        configuration.allowsInlineMediaPlayback = YES;
#endif
        configuration.mediaTypesRequiringUserActionForPlayback = WKAudiovisualMediaTypeNone;
        static WKProcessPool* shared_pool = nil; // C# MauiWKWebView.SharedPool
        if (shared_pool == nil)
        {
            shared_pool = configuration.processPool;
        }
        else
        {
            configuration.processPool = shared_pool;
        }
        WKWebView* const web_view = [[WKWebView alloc] initWithFrame:CGRectZero configuration:configuration];
#ifdef MAUI_PLATFORM_IOS
        // MauiWKWebView ctor: BackgroundColor = Clear, AutosizesSubviews = true (UIKit-only surface).
        web_view.backgroundColor = UIColor.clearColor;
        web_view.autoresizesSubviews = YES;
#endif
        platform->native = (__bridge_retained void*)web_view; // the void* slot owns one reference
        return platform;
    }

    void web_view_handler::on_connect_handler(web_view_platform& platform)
    {
        platform.connected_view = virtual_view();
        WKWebView* const web_view = as_web_view(platform.native);
        MauiCppWebViewNavigationDelegate* const delegate = [[MauiCppWebViewNavigationDelegate alloc] init];
        delegate.handler = this;
        web_view.navigationDelegate = delegate; // WKWebView holds the delegate weakly...
        // ...so keep it alive for the web view's lifetime via an associated object.
        objc_setAssociatedObject(web_view, &k_navigation_delegate_key, delegate, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
    }

    void web_view_handler::on_disconnect_handler(web_view_platform& platform)
    {
        platform.connected_view = nullptr;
        WKWebView* const web_view = as_web_view(platform.native);
        auto* const delegate =
            (MauiCppWebViewNavigationDelegate*)objc_getAssociatedObject(web_view, &k_navigation_delegate_key);
        if (delegate != nil)
        {
            delegate.handler = nullptr;
        }
        web_view.navigationDelegate = nil;
        objc_setAssociatedObject(web_view, &k_navigation_delegate_key, nil, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
    }

    // WebViewHandler.MapSource + WebViewExtensions.UpdateSource: the platform view is the
    // IWebViewDelegate the source loads into, then UpdateCanGoBackForward.
    void web_view_handler::map_source(web_view_handler& handler, i_web_view& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        if (auto* source = view.source())
        {
            source->load(*platform);
        }
        update_can_go_back_forward(view, as_web_view(platform->native));
    }

    // WebViewHandler.MapGoBack + WebViewExtensions.UpdateGoBack.
    void web_view_handler::map_go_back(web_view_handler& handler, i_web_view& view, const std::any& /*args*/)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        WKWebView* const web_view = as_web_view(platform->native);
        if (web_view.canGoBack)
        {
            platform->current_navigation_event = web_navigation_event::back;
            [web_view goBack];
        }
        update_can_go_back_forward(view, web_view);
    }

    // WebViewHandler.MapGoForward + WebViewExtensions.UpdateGoForward.
    void web_view_handler::map_go_forward(web_view_handler& handler, i_web_view& view, const std::any& /*args*/)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        WKWebView* const web_view = as_web_view(platform->native);
        if (web_view.canGoForward)
        {
            platform->current_navigation_event = web_navigation_event::forward;
            [web_view goForward];
        }
        update_can_go_back_forward(view, web_view);
    }

    // WebViewHandler.MapReload (the cookie re-sync is out of scope) + WebViewExtensions.UpdateReload.
    void web_view_handler::map_reload(web_view_handler& handler, i_web_view& /*view*/, const std::any& /*args*/)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        platform->current_navigation_event = web_navigation_event::refresh;
        ++platform->reload_count;
        [as_web_view(platform->native) reload];
    }

    // WebViewHandler.MapEval + WebViewExtensions.Eval: fire-and-forget evaluation.
    void web_view_handler::map_eval(web_view_handler& handler, i_web_view& /*view*/, const std::any& args)
    {
        auto* platform = handler.typed_platform_view();
        const auto* script = std::any_cast<std::string>(&args);
        if (platform == nullptr || platform->native == nullptr || script == nullptr)
        {
            return;
        }
        platform->eval_scripts.push_back(*script);
        [as_web_view(platform->native) evaluateJavaScript:to_ns_string(*script) completionHandler:nil];
    }

    // WebViewHandler.MapEvaluateJavaScriptAsync + WebViewExtensions.EvaluateJavaScript: run the script
    // and complete the request from WebKit's main-thread completion callback.
    void web_view_handler::map_evaluate_java_script(web_view_handler& handler, i_web_view& /*view*/,
                                                    const std::any& args)
    {
        const auto* request_ptr = std::any_cast<std::shared_ptr<evaluate_java_script_request>>(&args);
        if (request_ptr == nullptr || *request_ptr == nullptr)
        {
            return;
        }
        const std::shared_ptr<evaluate_java_script_request> request = *request_ptr;
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            // C# SetCanceled when the platform view is gone — surfaced as the "null" result.
            request->complete("null");
            return;
        }
        platform->eval_scripts.push_back(request->script());
        [as_web_view(platform->native)
            evaluateJavaScript:to_ns_string(request->script())
             completionHandler:^(id result, NSError* error) {
               request->complete(error != nil ? std::string("null") : handle_wk_web_view_result(result));
             }];
    }

    maui::graphics::size web_view_handler::get_desired_size(double width_constraint, double height_constraint) const
    {
        // WebViewHandler.iOS.GetDesiredSize: the platform measure, with the MinimumSize (44) fallback
        // per dimension when it measured 0 under an unbounded/non-positive constraint.
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
            // AppKit has no sizeThatFits; the auto-layout fittingSize is the NSView measuring analog.
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

    void web_view_handler::platform_arrange(const maui::graphics::rect& frame)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        [as_web_view(platform->native) setFrame:CGRectMake(frame.x, frame.y, frame.width, frame.height)];
    }
} // namespace maui::core
