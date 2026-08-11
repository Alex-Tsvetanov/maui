// refresh_view_handler — iOS (UIKit) platform recipe: a UIView host for the scrollable Content plus a
// real UIRefreshControl. Ported from RefreshViewHandler.iOS.cs + MauiRefreshView.cs:
//   - the host owns a UIRefreshControl; its ValueChanged (the MauiRefreshViewProxy.OnRefresh twin) sets
//     IRefreshView.IsRefreshing = true, which re-enters the control's coercion (Refreshing + command).
//   - MapIsRefreshing drives the UIRefreshControl's beginRefreshing/endRefreshing
//     (RefreshControl.IsRefreshing).
//   - MapRefreshColor sets the UIRefreshControl.tintColor (UpdateRefreshColor).
//   - MapIsRefreshEnabled enables/disables the gesture (the refresh control's userInteractionEnabled).
// The refresh control is attached by RECURSING the hosted subtree for a scroller, exactly as
// MauiRefreshView.TryInsertRefresh does (see try_insert_refresh below) — a RefreshView whose ScrollView
// sits under an intermediate wrapper still gets one. When the subtree holds no scroller the control stays
// detached (the refreshing flag is still mirrored and request_refresh() still works). Compiled as
// Objective-C++ with ARC only for the `ios` backend.

#import <UIKit/UIKit.h>
#import <WebKit/WebKit.h>
#import <objc/runtime.h>

#include <memory>
#include <string>
#include <string_view>

#include "ios_conversions.hpp"
#include "ios_semantics_ops.hpp"
#include "ios_visual_ops.hpp"
#include "maui/core/i_refresh_view.hpp"
#include "maui/core/i_view.hpp"
#include "maui/core/i_view_handler.hpp"
#include "maui/core/refresh_view_handler.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/color.hpp"
#include "maui/graphics/paint.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

// The target-action trampoline for the UIRefreshControl's ValueChanged (ScrollEventProxy twin).
@interface MauiRefreshTarget : NSObject
@property(nonatomic) maui::core::refresh_view_handler* handler;
- (void)onRefresh:(id)sender;
@end

@implementation MauiRefreshTarget
- (void)onRefresh:(id)sender
{
    (void)sender;
    if (self.handler != nullptr)
    {
        self.handler->request_refresh(); // -> IRefreshView.IsRefreshing = true
    }
}
@end

namespace
{
    UIView* as_host(void* native)
    {
        return (__bridge UIView*)native;
    }

    UIView* native_child(maui::core::i_view& child)
    {
        auto* handler = dynamic_cast<maui::core::i_view_handler*>(child.handler().get());
        if (handler == nullptr)
        {
            return nil;
        }
        return (__bridge UIView*)handler->native_view();
    }

    // The associated-object key for the refresh control + its target (kept alive by the host UIView).
    const char k_refresh_control_key = 0;
    const char k_refresh_target_key = 0;

    UIRefreshControl* refresh_control(UIView* host)
    {
        return (UIRefreshControl*)objc_getAssociatedObject(host, &k_refresh_control_key);
    }

    // The depth cap on the scroller search below. A UIView tree cannot cycle (a view has exactly one
    // superview), so this only bounds pathological nesting; the C# original has no cap.
    const int k_max_subview_depth = 64;

    // MauiRefreshView.TryInsertRefresh (src/Core/src/Platform/iOS/MauiRefreshView.cs:132-180). The node
    // itself is tested FIRST, then its children in index order, first hit winning:
    //   :141  a UIScrollView takes the control — a UIRefreshControl is ONLY valid on a scroller (UIKit
    //         asserts if it is added to a plain UIView),
    //   :163  a WKWebView hands it to its own scrollView, by INSERTION rather than the property,
    //   :169-177  otherwise recurse Subviews (pre-order, index order),
    //   :179  false when the subtree holds neither — the control stays detached, which is the documented
    //         no-native-pull fallback (the refreshing flag is still mirrored, request_refresh still works).
    // Not ported, deliberately: the :134 ShouldAllowRefreshGesture gate (the port keeps the control
    // attached and drives UIRefreshControl.enabled from MapIsRefreshEnabled instead); the :143-146
    // CanUseRefreshControlProperty() fork, whose predicate is
    // `navigationController?.navigationBar.prefersLargeTitles ?? true` and which degenerates to the `??
    // true` property path here because the port hosts pages without a UINavigationController; the :149-153
    // bounds nudge, which is `-contentOffset.Y` and so a no-op at attach time; and :157-158, which only
    // feed TryOffsetRefresh (:75-103, the programmatic-IsRefreshing scroll offset the port does not run).
    bool try_insert_refresh(UIView* view, UIRefreshControl* control, NSInteger index, int depth)
    {
        if (view == nil || control == nil || depth > k_max_subview_depth)
        {
            return false;
        }
        if ([view isKindOfClass:[UIScrollView class]])
        {
            UIScrollView* const scroller = (UIScrollView*)view;
            scroller.refreshControl = control; // :143-144
            // :155 — without this a scroller whose content is shorter than its frame refuses to overscroll,
            // so the pull can never reach the control and the spinner never runs.
            scroller.alwaysBounceVertical = YES;
            return true;
        }
        if ([view isKindOfClass:[WKWebView class]])
        {
            [((WKWebView*)view).scrollView insertSubview:control atIndex:index]; // :165
            return true;
        }
        NSArray<UIView*>* const children = view.subviews;
        for (NSUInteger i = 0; i < children.count; ++i)
        {
            if (try_insert_refresh(children[i], control, static_cast<NSInteger>(i), depth + 1)) // :172-176
            {
                return true;
            }
        }
        return false; // :179
    }
} // namespace

namespace maui::core
{
    refresh_view_platform::~refresh_view_platform()
    {
        // The UIRefreshControl (and its target, re-armed on every content set) outlives the handler
        // whenever a superview retains the host — null the raw back-pointer before the release.
        if (native != nullptr)
        {
            UIView* const host = as_host(native);
            if (auto* const target = (MauiRefreshTarget*)objc_getAssociatedObject(host, &k_refresh_target_key))
            {
                target.handler = nullptr; // the back-pointer live_view re-reads after user code
            }
            objc_setAssociatedObject(host, &k_refresh_target_key, nil, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
            CFRelease(native); // balances the __bridge_retained in create_platform_view
            native = nullptr;
        }
    }

    void refresh_view_platform::update_visibility(maui::core::visibility value)
    {
        as_host(native).hidden = value != maui::core::visibility::visible;
    }

    void refresh_view_platform::update_opacity(double value)
    {
        as_host(native).alpha = value;
    }

    void refresh_view_platform::update_automation_id(std::string_view value)
    {
        const std::string id(value);
        NSString* const raw = [NSString stringWithUTF8String:id.c_str()];
        as_host(native).accessibilityIdentifier = raw != nil ? raw : @"";
    }

    void refresh_view_platform::update_background(const maui::graphics::paint* value)
    {
        maui::platform::ios::apply_background(native, value);
    }

    void refresh_view_platform::update_shadow(const maui::core::i_shadow* value)
    {
        maui::platform::ios::apply_shadow(native, value);
    }

    void refresh_view_platform::update_clip(const maui::graphics::i_shape* value)
    {
        const CGRect bounds = as_host(native).bounds;
        maui::platform::ios::apply_clip(
            native, value,
            maui::graphics::rect{bounds.origin.x, bounds.origin.y, bounds.size.width, bounds.size.height});
    }

    void refresh_view_platform::update_semantics(const maui::core::semantics* value)
    {
        maui::platform::ios::apply_semantics((__bridge UIView*)native, value);
    }

    void refresh_view_platform::update_input_transparent(bool value)
    {
        maui::platform::ios::apply_input_transparent((__bridge UIView*)native, value);
    }

    std::unique_ptr<refresh_view_platform> refresh_view_handler::create_platform_view()
    {
        auto platform = std::make_unique<refresh_view_platform>();
        UIView* const host = [[UIView alloc] initWithFrame:CGRectMake(0, 0, 0, 0)];

        // The real UIRefreshControl + its ValueChanged target, kept alive by the host (associated objects).
        UIRefreshControl* const control = [[UIRefreshControl alloc] init];
        MauiRefreshTarget* const target = [[MauiRefreshTarget alloc] init];
        [control addTarget:target action:@selector(onRefresh:) forControlEvents:UIControlEventValueChanged];
        objc_setAssociatedObject(host, &k_refresh_control_key, control, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
        objc_setAssociatedObject(host, &k_refresh_target_key, target, OBJC_ASSOCIATION_RETAIN_NONATOMIC);

        platform->native = (__bridge_retained void*)host; // the void* slot owns one reference
        return platform;
    }

    void refresh_view_handler::set_content()
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        UIView* const host = as_host(platform->native);
        MauiRefreshTarget* const target = (MauiRefreshTarget*)objc_getAssociatedObject(host, &k_refresh_target_key);
        target.handler = this; // wire the proxy to this handler (re-armed on each content set)

        NSArray<UIView*>* const snapshot = [host.subviews copy];
        [snapshot makeObjectsPerformSelector:@selector(removeFromSuperview)];

        platform->hosted_content = virtual_view() != nullptr ? virtual_view()->content() : nullptr;
        if (platform->hosted_content == nullptr)
        {
            return;
        }
        UIView* const subview = native_child(*platform->hosted_content);
        if (subview == nil)
        {
            return;
        }
        [subview removeFromSuperview];
        [host addSubview:subview];

        // C# native parenting (UpdateContent:71 → TryInsertRefresh): walk the hosted subtree for the
        // scroller that takes the control, so a scroller under an intermediate wrapper is found too. The
        // bool is ignored here exactly as it is in UpdateContent — "no scroller anywhere" is the
        // documented detached fallback, not an error.
        try_insert_refresh(subview, refresh_control(host), 0, 0);
    }

    void refresh_view_handler::update_is_refreshing()
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr || virtual_view() == nullptr)
        {
            return;
        }
        const bool refreshing = virtual_view()->is_refreshing();
        platform->refreshing = refreshing;
        UIRefreshControl* const control = refresh_control(as_host(platform->native));
        // -beginRefreshing/-endRefreshing are only valid while the control is hosted by a UIScrollView
        // (UIKit asserts otherwise). When the content isn't scrollable the flag is still mirrored, which is
        // the documented no-native-spinner fallback (C#'s MauiRefreshView always wraps a scroll view).
        if (control.superview != nil && [control.superview isKindOfClass:[UIScrollView class]])
        {
            if (refreshing)
            {
                [control beginRefreshing];
            }
            else
            {
                [control endRefreshing];
            }
        }
    }

    void refresh_view_handler::update_refresh_color()
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr || virtual_view() == nullptr)
        {
            return;
        }
        const maui::graphics::paint* const paint = virtual_view()->refresh_color();
        UIRefreshControl* const control = refresh_control(as_host(platform->native));
        if (paint != nullptr)
        {
            const maui::graphics::color color = paint->background_color();
            platform->has_refresh_color = true;
            platform->refresh_color_argb = color.to_uint();
            control.tintColor = maui::platform::ios::to_ui_color(color);
        }
        else
        {
            platform->has_refresh_color = false;
            platform->refresh_color_argb = 0;
        }
    }

    void refresh_view_handler::update_is_refresh_enabled()
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr || virtual_view() == nullptr)
        {
            return;
        }
        const bool enabled = virtual_view()->is_refresh_enabled();
        platform->refresh_enabled = enabled;
        refresh_control(as_host(platform->native)).enabled = enabled;
    }

    void refresh_view_handler::request_refresh()
    {
        if (auto* view = virtual_view())
        {
            view->set_is_refreshing(true);
        }
    }

    void refresh_view_handler::platform_arrange(const maui::graphics::rect& frame)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        [as_host(platform->native) setFrame:CGRectMake(frame.x, frame.y, frame.width, frame.height)];
    }

    // Render transform pushed to the native UIView via the shared ios apply_transform helper
    // (the generic-IView ViewMapper widening). `native` is this struct's UIView handle.
    void refresh_view_platform::update_transform(const maui::core::transform_spec& value)
    {
        maui::platform::ios::apply_transform(native, value);
    }

} // namespace maui::core
