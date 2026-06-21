// refresh_view_handler — iOS (UIKit) platform recipe: a UIView host for the scrollable Content plus a
// real UIRefreshControl. Ported from RefreshViewHandler.iOS.cs + MauiRefreshView.cs:
//   - the host owns a UIRefreshControl; its ValueChanged (the MauiRefreshViewProxy.OnRefresh twin) sets
//     IRefreshView.IsRefreshing = true, which re-enters the control's coercion (Refreshing + command).
//   - MapIsRefreshing drives the UIRefreshControl's beginRefreshing/endRefreshing
//     (RefreshControl.IsRefreshing).
//   - MapRefreshColor sets the UIRefreshControl.tintColor (UpdateRefreshColor).
//   - MapIsRefreshEnabled enables/disables the gesture (the refresh control's userInteractionEnabled).
// If the hosted content is a UIScrollView the refresh control attaches to it (the C# native parenting);
// otherwise it sits on the host (the headless / non-scroll content path). Compiled as Objective-C++ with
// ARC only for the `ios` backend.

#import <UIKit/UIKit.h>
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
} // namespace

namespace maui::core
{
    refresh_view_platform::~refresh_view_platform()
    {
        if (native != nullptr)
        {
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

        // C# native parenting: a UIScrollView content owns the refresh control directly (pull-to-refresh
        // on the scroller). A UIRefreshControl is ONLY valid on a scroll view — UIKit asserts if it is
        // added to a plain UIView — so non-scrollable content leaves the control detached (the documented
        // no-native-pull fallback; the refreshing flag is still mirrored and request_refresh still works).
        UIRefreshControl* const control = refresh_control(host);
        if ([subview isKindOfClass:[UIScrollView class]])
        {
            ((UIScrollView*)subview).refreshControl = control;
        }
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
