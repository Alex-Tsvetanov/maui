// scroll_view_handler — Apple (AppKit / macOS) platform recipe: a real NSScrollView whose
// documentView IS the content child's native view. Translated from ScrollViewHandler.iOS.cs (UIKit's
// MauiScrollView), adapted to the AppKit scroller model:
//   - set_content assigns the content's native view as the documentView (C# UpdateContentView's
//     clear + AddSubview — AppKit's scroller has the dedicated documentView slot instead).
//   - Orientation + the two ScrollBarVisibility values funnel into ONE scroller refresh: an axis
//     scrolls only when the Orientation includes it, `never` hides that bar, `always` pins it
//     (autohidesScrollers off), `default` auto-hides — the AppKit analog of UpdateIsEnabled +
//     Update*ScrollBarVisibility.
//   - scroll_to ports MapRequestScrollTo: clamp the target to max(documentSize - viewport, 0), move
//     the clip view's bounds origin (reflectScrolledClipView), and acknowledge ScrollFinished. The
//     ANIMATED scroll completes synchronously here (the same animated-transition collapse the
//     navigation handler documents — AppKit has no setContentOffset:animated: + delegate-end pair).
//   - USER scrolls write back through an NSViewBoundsDidChangeNotification observer on the clip view
//     (the ScrollEventProxy.Scrolled twin): the virtual offsets follow the clip origin, raising the
//     control's Scrolled event.
// The scrollable EXTENT needs no extra push: the documentView is the content's native view, and the
// control's ArrangeContentUnbounded already frames it to the full (possibly overflowing) content size
// — the MauiScrollView LayoutSubviews → ContentSize update, realized structurally.
// Compiled as Objective-C++ with ARC for the `apple` backend.

#import <AppKit/AppKit.h>
#import <objc/runtime.h>

#include <algorithm>
#include <memory>
#include <string>
#include <string_view>

#include "apple_semantics_ops.hpp"
#include "apple_view_ops.hpp"
#include "apple_visual_ops.hpp"
#include "flipped_container.hpp"
#include "maui/core/i_scroll_view.hpp"
#include "maui/core/i_view.hpp"
#include "maui/core/i_view_handler.hpp"
#include "maui/core/scroll_view_handler.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/rect.hpp"

// Obj-C trampoline: forwards the clip view's bounds-change notifications (user/programmatic scrolls)
// to the C++ handler's virtual view — the ScrollEventProxy twin.
@interface MauiScrollViewProxy : NSObject
@property(nonatomic) maui::core::scroll_view_handler* handler;
- (void)boundsDidChange:(NSNotification*)notification;
@end

@implementation MauiScrollViewProxy
- (void)boundsDidChange:(NSNotification*)notification
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
    NSClipView* const clip = (NSClipView*)notification.object;
    const NSPoint origin = clip.bounds.origin;
    platform->offset_x = origin.x;
    platform->offset_y = origin.y;
    // The platform write-back (C# Scrolled → VirtualView.Horizontal/VerticalOffset = ContentOffset).
    view->set_horizontal_offset(origin.x);
    view->set_vertical_offset(origin.y);
}
@end

namespace
{
    // Key for the associated proxy kept alive by the NSScrollView (the entry-delegate pattern).
    const char k_proxy_key = 0;

    NSScrollView* as_scroller(void* native)
    {
        return (__bridge NSScrollView*)native;
    }

    NSView* native_child(maui::core::i_view& child)
    {
        auto* handler = dynamic_cast<maui::core::i_view_handler*>(child.handler().get());
        if (handler == nullptr)
        {
            return nil;
        }
        return (__bridge NSView*)handler->native_view();
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
    // is derived from the Orientation (the C# iOS MapIsEnabled funnels there too).
    void scroll_view_platform::update_visibility(maui::core::visibility value)
    {
        as_scroller(native).hidden = value != maui::core::visibility::visible;
    }

    void scroll_view_platform::update_opacity(double value)
    {
        as_scroller(native).alphaValue = value;
    }

    void scroll_view_platform::update_automation_id(std::string_view value)
    {
        const std::string id(value);
        NSString* const raw = [NSString stringWithUTF8String:id.c_str()];
        as_scroller(native).accessibilityIdentifier = raw != nil ? raw : @"";
    }

    void scroll_view_platform::update_transform(const maui::core::transform_spec& value)
    {
        maui::platform::apple::apply_transform(native, value);
    }

    void scroll_view_platform::update_flow_direction(maui::core::flow_direction value)
    {
        maui::platform::apple::apply_flow_direction(native, value);
    }

    void scroll_view_platform::update_background(const maui::graphics::paint* value)
    {
        maui::platform::apple::apply_background(native, value);
    }

    void scroll_view_platform::update_shadow(const maui::core::i_shadow* value)
    {
        maui::platform::apple::apply_shadow(native, value);
    }

    void scroll_view_platform::update_clip(const maui::graphics::i_shape* value)
    {
        const NSRect bounds = as_scroller(native).bounds;
        maui::platform::apple::apply_clip(
            native, value,
            maui::graphics::rect{bounds.origin.x, bounds.origin.y, bounds.size.width, bounds.size.height});
    }

    void scroll_view_platform::update_semantics(const maui::core::semantics* value)
    {
        maui::platform::apple::apply_semantics((__bridge NSView*)native, value);
    }

    void scroll_view_platform::update_input_transparent(bool value)
    {
        maui::platform::apple::apply_input_transparent((__bridge NSView*)native, value);
    }

    std::unique_ptr<scroll_view_platform> scroll_view_handler::create_platform_view()
    {
        auto platform = std::make_unique<scroll_view_platform>();
        NSScrollView* const scroller = [[NSScrollView alloc] initWithFrame:NSMakeRect(0, 0, 0, 0)];
        scroller.drawsBackground = NO;
        scroller.hasVerticalScroller = YES; // the Vertical default; update_scrollers refines on connect
        scroller.autohidesScrollers = YES;
        platform->native = (__bridge_retained void*)scroller; // the void* slot owns one reference
        // A flipped clip view so the content lays out / scrolls top-down (top-left origin) regardless of
        // the document view's own flippedness — the UIScrollView top-left contentOffset analog.
        maui::platform::apple::install_flipped_clip_view(platform->native);
        return platform;
    }

    // Wire the scrolled write-back (the ScrollEventProxy.Connect twin): observe the clip view's
    // bounds changes through an associated proxy object.
    void scroll_view_handler::on_connect_handler(scroll_view_platform& platform)
    {
        NSScrollView* const scroller = as_scroller(platform.native);
        MauiScrollViewProxy* const proxy = [[MauiScrollViewProxy alloc] init];
        proxy.handler = this;
        objc_setAssociatedObject(scroller, &k_proxy_key, proxy, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
        scroller.contentView.postsBoundsChangedNotifications = YES;
        [[NSNotificationCenter defaultCenter] addObserver:proxy
                                                 selector:@selector(boundsDidChange:)
                                                     name:NSViewBoundsDidChangeNotification
                                                   object:scroller.contentView];
    }

    void scroll_view_handler::on_disconnect_handler(scroll_view_platform& platform)
    {
        NSScrollView* const scroller = as_scroller(platform.native);
        if (auto* const proxy = (MauiScrollViewProxy*)objc_getAssociatedObject(scroller, &k_proxy_key))
        {
            [[NSNotificationCenter defaultCenter] removeObserver:proxy
                                                            name:NSViewBoundsDidChangeNotification
                                                          object:scroller.contentView];
            proxy.handler = nullptr;
        }
        objc_setAssociatedObject(scroller, &k_proxy_key, nil, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
    }

    void scroll_view_handler::set_content()
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        NSScrollView* const scroller = as_scroller(platform->native);
        platform->hosted_content = virtual_view() != nullptr ? virtual_view()->content() : nullptr;
        if (platform->hosted_content == nullptr)
        {
            scroller.documentView = nil;
            return;
        }
        if (NSView* const subview = native_child(*platform->hosted_content))
        {
            [subview removeFromSuperview];
            scroller.documentView = subview;
        }
    }

    namespace
    {
        // The one scroller refresh Orientation + both ScrollBarVisibility pushes funnel into (see the
        // file comment). Reads the whole surface off the virtual view, like the C# Update* extensions.
        void update_scrollers(NSScrollView* scroller, const i_scroll_view& view)
        {
            const scroll_orientation direction = view.orientation();
            const bool h_axis = direction == scroll_orientation::horizontal || direction == scroll_orientation::both;
            const bool v_axis = direction == scroll_orientation::vertical || direction == scroll_orientation::both;

            const scroll_bar_visibility h_bar = view.horizontal_scroll_bar_visibility();
            const scroll_bar_visibility v_bar = view.vertical_scroll_bar_visibility();

            scroller.hasHorizontalScroller =
                h_bar == scroll_bar_visibility::never ? NO : (h_bar == scroll_bar_visibility::always || h_axis);
            scroller.hasVerticalScroller =
                v_bar == scroll_bar_visibility::never ? NO : (v_bar == scroll_bar_visibility::always || v_axis);
            scroller.autohidesScrollers =
                h_bar != scroll_bar_visibility::always && v_bar != scroll_bar_visibility::always;
        }
    } // namespace

    void scroll_view_handler::update_orientation()
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr || virtual_view() == nullptr)
        {
            return;
        }
        platform->orientation = virtual_view()->orientation();
        update_scrollers(as_scroller(platform->native), *virtual_view());
    }

    void scroll_view_handler::update_horizontal_scroll_bar_visibility()
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr || virtual_view() == nullptr)
        {
            return;
        }
        platform->horizontal_bar_visibility = virtual_view()->horizontal_scroll_bar_visibility();
        update_scrollers(as_scroller(platform->native), *virtual_view());
    }

    void scroll_view_handler::update_vertical_scroll_bar_visibility()
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr || virtual_view() == nullptr)
        {
            return;
        }
        platform->vertical_bar_visibility = virtual_view()->vertical_scroll_bar_visibility();
        update_scrollers(as_scroller(platform->native), *virtual_view());
    }

    // C# MapRequestScrollTo: clamp the target to the available scroll range, move the native offset
    // (the clip origin), and acknowledge ScrollFinished. The bounds-change notification fires the
    // offsets back through the proxy; the completion is synchronous for instant AND animated (the
    // documented AppKit collapse).
    void scroll_view_handler::scroll_to(const scroll_to_request& request)
    {
        auto* platform = typed_platform_view();
        auto* view = virtual_view();
        if (platform == nullptr || platform->native == nullptr || view == nullptr)
        {
            return;
        }
        platform->scroll_requests.push_back(request);

        NSScrollView* const scroller = as_scroller(platform->native);
        NSClipView* const clip = scroller.contentView;
        const NSSize document_size = scroller.documentView != nil ? scroller.documentView.frame.size : NSMakeSize(0, 0);
        const double available_x = std::max(document_size.width - clip.bounds.size.width, 0.0);
        const double available_y = std::max(document_size.height - clip.bounds.size.height, 0.0);
        const double target_x = std::clamp(request.horizontal_offset, 0.0, available_x);
        const double target_y = std::clamp(request.vertical_offset, 0.0, available_y);

        const bool already_at_target = clip.bounds.origin.x == target_x && clip.bounds.origin.y == target_y;
        if (!already_at_target)
        {
            [clip setBoundsOrigin:NSMakePoint(target_x, target_y)];
            [scroller reflectScrolledClipView:clip];
        }
        view->scroll_finished();
    }

    void scroll_view_handler::platform_arrange(const maui::graphics::rect& frame)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        [as_scroller(platform->native) setFrame:NSMakeRect(frame.x, frame.y, frame.width, frame.height)];
    }
} // namespace maui::core
