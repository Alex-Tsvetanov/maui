// scroll_view_handler — iOS (UIKit) platform recipe: a real UIScrollView hosting the content child's
// native view as its (tagged) scrollable subview. Ported DIRECTLY from ScrollViewHandler.iOS.cs +
// MauiScrollView.cs + ScrollViewExtensions.cs:
//   - set_content ports UpdateContentView: remove the tagged current content, tag + add the new
//     content's native view (MauiScrollView.ContentTag = 0x845fed).
//   - update_orientation ports MapOrientation → UpdateIsEnabled (ScrollEnabled = Orientation !=
//     Neither && IsEnabled); the bar pushes port Update*ScrollBarVisibility (ShowsXxxScrollIndicator =
//     visibility is Always or Default).
//   - scroll_to ports MapRequestScrollTo VERBATIM: clamp the target to max(ContentSize - Frame, 0),
//     SetContentOffset(target, animated = !instant), and ScrollFinished when instant or already at
//     the target; an ANIMATED scroll completes through the delegate's scrollViewDidEndScrollingAnimation
//     (the ScrollAnimationEnded proxy event).
//   - USER scrolls write back through scrollViewDidScroll (the ScrollEventProxy.Scrolled twin).
//   - platform_arrange frames the scroller and pushes the scrollable extent: ContentSize derives from
//     the hosted content's (unbounded-arranged) frame + the view Padding — the MauiScrollView
//     LayoutSubviews → CrossPlatformArrange → ContentSize update, realized from the already-arranged
//     native content.
// Compiled as Objective-C++ with ARC only for the `ios` backend.

#import <UIKit/UIKit.h>
#import <objc/runtime.h>

#include <algorithm>
#include <memory>
#include <string>
#include <string_view>

#include "ios_semantics_ops.hpp"
#include "ios_visual_ops.hpp"
#include "maui/core/i_scroll_view.hpp"
#include "maui/core/i_view.hpp"
#include "maui/core/i_view_handler.hpp"
#include "maui/core/scroll_view_handler.hpp"
#include "maui/core/thickness.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/rect.hpp"

// Obj-C delegate trampoline: forwards the scroll events to the C++ handler's virtual view — the
// ScrollEventProxy twin (Scrolled + ScrollAnimationEnded).
@interface MauiScrollViewDelegate : NSObject <UIScrollViewDelegate>
@property(nonatomic) maui::core::scroll_view_handler* handler;
@end

@implementation MauiScrollViewDelegate
- (void)scrollViewDidScroll:(UIScrollView*)scrollView
{
    if (self.handler == nullptr)
    {
        return;
    }
    auto* const platform = self.handler->typed_platform_view();
    auto* const view = self.handler->virtual_view();
    if (platform == nullptr || view == nullptr)
    {
        return;
    }
    const CGPoint offset = scrollView.contentOffset;
    platform->offset_x = offset.x;
    platform->offset_y = offset.y;
    // C# Scrolled: VirtualView.HorizontalOffset/VerticalOffset = platformView.ContentOffset.
    view->set_horizontal_offset(offset.x);
    view->set_vertical_offset(offset.y);
}

- (void)scrollViewDidEndScrollingAnimation:(UIScrollView*)scrollView
{
    if (self.handler == nullptr)
    {
        return;
    }
    if (auto* const view = self.handler->virtual_view())
    {
        view->scroll_finished(); // C# ScrollAnimationEnded -> VirtualView.ScrollFinished()
    }
}
@end

namespace
{
    // C# MauiScrollView.ContentTag — tags the hosted content subview so it can be found/replaced.
    constexpr NSInteger k_content_tag = 0x845fed;

    // Key for the associated delegate kept alive by the UIScrollView (its `delegate` is weak).
    const char k_delegate_key = 0;

    UIScrollView* as_scroller(void* native)
    {
        return (__bridge UIScrollView*)native;
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

    // C# ScrollViewExtensions.GetContentView: the subview carrying the content tag, or nil.
    UIView* tagged_content(UIScrollView* scroller)
    {
        NSArray<UIView*>* const subviews = scroller.subviews;
        for (NSUInteger i = 0; i < subviews.count; i++)
        {
            if (subviews[i].tag == k_content_tag)
            {
                return subviews[i];
            }
        }
        return nil;
    }
} // namespace

namespace maui::core
{
    scroll_view_platform::~scroll_view_platform()
    {
        if (native != nullptr)
        {
            CFRelease(native); // balances the __bridge_retained in create_platform_view
            native = nullptr;
        }
    }

    // The generic-IView property pushes. is_enabled keeps the base mirror — the native scroll-ability
    // funnels through update_orientation (C#'s iOS MapIsEnabled does the same).
    void scroll_view_platform::update_visibility(maui::core::visibility value)
    {
        as_scroller(native).hidden = value != maui::core::visibility::visible;
    }

    void scroll_view_platform::update_opacity(double value)
    {
        as_scroller(native).alpha = value;
    }

    void scroll_view_platform::update_automation_id(std::string_view value)
    {
        const std::string id(value);
        NSString* const raw = [NSString stringWithUTF8String:id.c_str()];
        as_scroller(native).accessibilityIdentifier = raw != nil ? raw : @"";
    }

    void scroll_view_platform::update_background(const maui::graphics::paint* value)
    {
        maui::platform::ios::apply_background(native, value);
    }

    void scroll_view_platform::update_shadow(const maui::core::i_shadow* value)
    {
        maui::platform::ios::apply_shadow(native, value);
    }

    void scroll_view_platform::update_clip(const maui::graphics::i_shape* value)
    {
        const CGRect bounds = as_scroller(native).bounds;
        maui::platform::ios::apply_clip(
            native, value,
            maui::graphics::rect{bounds.origin.x, bounds.origin.y, bounds.size.width, bounds.size.height});
    }

    void scroll_view_platform::update_semantics(const maui::core::semantics* value)
    {
        maui::platform::ios::apply_semantics((__bridge UIView*)native, value);
    }

    void scroll_view_platform::update_input_transparent(bool value)
    {
        maui::platform::ios::apply_input_transparent((__bridge UIView*)native, value);
    }

    std::unique_ptr<scroll_view_platform> scroll_view_handler::create_platform_view()
    {
        auto platform = std::make_unique<scroll_view_platform>();
        UIScrollView* const scroller = [[UIScrollView alloc] initWithFrame:CGRectMake(0, 0, 0, 0)];
        platform->native = (__bridge_retained void*)scroller; // the void* slot owns one reference
        return platform;
    }

    // The ScrollEventProxy.Connect twin: hook the delegate (kept alive by an associated reference —
    // UIScrollView.delegate is weak).
    void scroll_view_handler::on_connect_handler(scroll_view_platform& platform)
    {
        UIScrollView* const scroller = as_scroller(platform.native);
        MauiScrollViewDelegate* const delegate = [[MauiScrollViewDelegate alloc] init];
        delegate.handler = this;
        scroller.delegate = delegate;
        objc_setAssociatedObject(scroller, &k_delegate_key, delegate, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
    }

    void scroll_view_handler::on_disconnect_handler(scroll_view_platform& platform)
    {
        UIScrollView* const scroller = as_scroller(platform.native);
        if (auto* const delegate = (MauiScrollViewDelegate*)objc_getAssociatedObject(scroller, &k_delegate_key))
        {
            delegate.handler = nullptr;
        }
        scroller.delegate = nil;
        objc_setAssociatedObject(scroller, &k_delegate_key, nil, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
    }

    // C# UpdateContentView: remove the tagged current content, then tag + add the new one.
    void scroll_view_handler::set_content()
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        UIScrollView* const scroller = as_scroller(platform->native);
        if (UIView* const current = tagged_content(scroller))
        {
            [current removeFromSuperview];
        }
        platform->hosted_content = virtual_view() != nullptr ? virtual_view()->content() : nullptr;
        if (platform->hosted_content == nullptr)
        {
            return;
        }
        if (UIView* const subview = native_child(*platform->hosted_content))
        {
            [subview removeFromSuperview];
            subview.tag = k_content_tag;
            [scroller addSubview:subview];
        }
    }

    // C# MapOrientation → UpdateIsEnabled: Neither disables scrolling, otherwise IsEnabled rules.
    void scroll_view_handler::update_orientation()
    {
        auto* platform = typed_platform_view();
        auto* view = virtual_view();
        if (platform == nullptr || platform->native == nullptr || view == nullptr)
        {
            return;
        }
        platform->orientation = view->orientation();
        as_scroller(platform->native).scrollEnabled =
            view->orientation() != scroll_orientation::neither && view->is_enabled();
    }

    // C# Update*ScrollBarVisibility: the indicator shows for Always and Default, hides for Never.
    void scroll_view_handler::update_horizontal_scroll_bar_visibility()
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr || virtual_view() == nullptr)
        {
            return;
        }
        const scroll_bar_visibility value = virtual_view()->horizontal_scroll_bar_visibility();
        platform->horizontal_bar_visibility = value;
        as_scroller(platform->native).showsHorizontalScrollIndicator = value != scroll_bar_visibility::never;
    }

    void scroll_view_handler::update_vertical_scroll_bar_visibility()
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr || virtual_view() == nullptr)
        {
            return;
        }
        const scroll_bar_visibility value = virtual_view()->vertical_scroll_bar_visibility();
        platform->vertical_bar_visibility = value;
        as_scroller(platform->native).showsVerticalScrollIndicator = value != scroll_bar_visibility::never;
    }

    // C# MapRequestScrollTo, verbatim (see the file comment).
    void scroll_view_handler::scroll_to(const scroll_to_request& request)
    {
        auto* platform = typed_platform_view();
        auto* view = virtual_view();
        if (platform == nullptr || platform->native == nullptr || view == nullptr)
        {
            return;
        }
        platform->scroll_requests.push_back(request);

        UIScrollView* const scroller = as_scroller(platform->native);
        const double available_x = std::max(scroller.contentSize.width - scroller.frame.size.width, 0.0);
        const double available_y = std::max(scroller.contentSize.height - scroller.frame.size.height, 0.0);
        const double target_x = std::clamp(request.horizontal_offset, 0.0, available_x);
        const double target_y = std::clamp(request.vertical_offset, 0.0, available_y);

        const bool already_at_target = scroller.contentOffset.x == target_x && scroller.contentOffset.y == target_y;
        if (!already_at_target)
        {
            [scroller setContentOffset:CGPointMake(target_x, target_y) animated:!request.instant];
        }
        if (request.instant || already_at_target)
        {
            view->scroll_finished();
        }
    }

    // Frame the scroller and push the scrollable extent (MauiScrollView.LayoutSubviews → ContentSize):
    // the hosted content was already arranged unbounded by the control, so its native frame IS the
    // content extent; the view Padding pads the far edge.
    void scroll_view_handler::platform_arrange(const maui::graphics::rect& frame)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        UIScrollView* const scroller = as_scroller(platform->native);
        [scroller setFrame:CGRectMake(frame.x, frame.y, frame.width, frame.height)];

        CGSize extent = CGSizeZero;
        if (UIView* const content = tagged_content(scroller))
        {
            maui::core::thickness inset;
            if (virtual_view() != nullptr)
            {
                inset = virtual_view()->padding();
            }
            extent =
                CGSizeMake(CGRectGetMaxX(content.frame) + inset.right, CGRectGetMaxY(content.frame) + inset.bottom);
        }
        if (!CGSizeEqualToSize(scroller.contentSize, extent))
        {
            scroller.contentSize = extent; // ScrollViewExtensions.UpdateContentSize's change guard
        }
    }
} // namespace maui::core
