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
#include "maui/controls/scroll_view.hpp"
#include "maui/core/i_scroll_view.hpp"
#include "maui/core/i_view.hpp"
#include "maui/core/i_view_handler.hpp"
#include "maui/core/scroll_view_handler.hpp"
#include "maui/core/thickness.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/rect.hpp"

// Obj-C delegate trampoline: forwards the scroll events to the C++ handler's virtual view — the
// ScrollEventProxy twin (Scrolled + ScrollAnimationEnded).
// The MauiScrollView twin, cut down to its safe-area job. C# MauiScrollView is a UIScrollView subclass
// that overrides SafeAreaInsetsDidChange + AdjustedContentInsetDidChange and re-runs its layout; the port
// keeps its cross-platform arrange, so this subclass only PUSHES the two platform facts the control needs
// (scroll_view::effective_safe_area) and asks for a re-layout. Both are needed because iOS sets
// AdjustedContentInset only when the content OVERFLOWS: when it fits, that stays zero and the control has
// to inset its own content (MauiScrollView.cs:383-386).
@interface MauiIosScrollView : UIScrollView
@property(nonatomic) maui::core::scroll_view_handler* mauiHandler;
@end

@interface MauiScrollViewDelegate : NSObject <UIScrollViewDelegate>
@property(nonatomic) maui::core::scroll_view_handler* handler;
@end

@implementation MauiIosScrollView
{
    maui::core::thickness _lastPushedSafeArea;
    maui::core::thickness _lastPushedSystemInset;
    BOOL _inPush;      // we are inside our own arrange (UIKit's inset callbacks re-enter it)
    BOOL _pendingPush; // …and a re-entrant call arrived while we were, so run another pass
}

// Push the two platform facts to the control and re-drive the content arrange if either MOVED.
// C# MauiScrollView does this from SafeAreaInsetsDidChange / AdjustedContentInsetDidChange (which call
// InvalidateMeasure + InvalidateAncestorsMeasures); the port re-arranges in place instead, because its
// arrange is cross-platform and re-entering it with the same frame is idempotent.
- (void)mauiPushSafeAreaState
{
    // A re-entrant call must be DEFERRED, never dropped. Our own arrange sets contentSize, and UIKit
    // answers by calling adjustedContentInsetDidChange RIGHT BACK, synchronously, from inside it — and
    // that callback carries the very value that decides which CrossPlatformArrange branch is correct.
    // An early `return` here swallows it: the control stays on the manual branch (content already at the
    // inset origin) while UIKit ALSO offsets by its contentInset, double-insetting to 82pt. That is not
    // hypothetical — it put slider/layout_is_enabled at +32px (too LOW) on a full board sweep, the exact
    // mirror of the -32px this slice set out to fix, while a --only run of the same pages passed on
    // timing luck. So: latch, re-run, and let the loop converge.
    if (_inPush)
    {
        _pendingPush = YES;
        return;
    }
    if (self.mauiHandler == nullptr || self.mauiHandler->virtual_view() == nullptr)
    {
        return;
    }
    // The concrete control, not an interface: MAUI parks this whole decision on the PLATFORM view
    // (MauiScrollView), so there is no ISafeAreaView2-style seam for the system inset. The port moved the
    // decision into the control, so the platform pushes into it directly (other iOS handlers likewise
    // reach for controls/ types).
    auto* const scroller = dynamic_cast<maui::controls::scroll_view*>(self.mauiHandler->virtual_view());
    if (scroller == nullptr)
    {
        return;
    }
    _inPush = YES;
    // Bounded: the anti-flip-flop clamp in platform_arrange keeps UIKit's decision stable, so this
    // settles in about two passes (manual -> UIKit adds its inset -> system -> values repeat). The cap is
    // a backstop against a pathological oscillation hanging the UI, which is what an unbounded version
    // did on iOS.
    for (int pass = 0; pass < 4; ++pass)
    {
        _pendingPush = NO;
        const UIEdgeInsets safe = self.safeAreaInsets;
        const UIEdgeInsets system = self.adjustedContentInset;
        const maui::core::thickness safe_area{safe.left, safe.top, safe.right, safe.bottom};
        const maui::core::thickness system_inset{system.left, system.top, system.right, system.bottom};
        if (safe_area == _lastPushedSafeArea && system_inset == _lastPushedSystemInset)
        {
            break; // converged — nothing moved since the last arrange
        }
        _lastPushedSafeArea = safe_area;
        _lastPushedSystemInset = system_inset;
        scroller->set_safe_area_insets(safe_area);
        scroller->set_system_adjusted_content_inset(system_inset);
        // Re-arrange in place: same frame, so platform_arrange's setFrame is a no-op and only the CONTENT
        // moves (to the other branch of MauiScrollView.CrossPlatformArrange).
        scroller->arrange(scroller->frame());
        if (!_pendingPush)
        {
            break;
        }
    }
    _inPush = NO;
}

- (void)safeAreaInsetsDidChange
{
    [super safeAreaInsetsDidChange];
    [self mauiPushSafeAreaState];
}

- (void)adjustedContentInsetDidChange
{
    [super adjustedContentInsetDidChange];
    [self mauiPushSafeAreaState];
}
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
        MauiIosScrollView* const scroller = [[MauiIosScrollView alloc] initWithFrame:CGRectMake(0, 0, 0, 0)];
        platform->native = (__bridge_retained void*)scroller; // the void* slot owns one reference
        return platform;
    }

    // The ScrollEventProxy.Connect twin: hook the delegate (kept alive by an associated reference —
    // UIScrollView.delegate is weak).
    void scroll_view_handler::on_connect_handler(scroll_view_platform& platform)
    {
        UIScrollView* const scroller = as_scroller(platform.native);
        if ([scroller isKindOfClass:[MauiIosScrollView class]])
        {
            ((MauiIosScrollView*)scroller).mauiHandler = this; // non-owning backref; cleared on disconnect
        }
        MauiScrollViewDelegate* const delegate = [[MauiScrollViewDelegate alloc] init];
        delegate.handler = this;
        scroller.delegate = delegate;
        objc_setAssociatedObject(scroller, &k_delegate_key, delegate, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
    }

    void scroll_view_handler::on_disconnect_handler(scroll_view_platform& platform)
    {
        UIScrollView* const scroller = as_scroller(platform.native);
        if ([scroller isKindOfClass:[MauiIosScrollView class]])
        {
            ((MauiIosScrollView*)scroller).mauiHandler = nullptr; // drop the backref (no ownership)
        }
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
        // C# MauiScrollView.cs:474-490 — the anti-flip-flop clamp, and the reason this whole thing is
        // stable. With ContentInsetAdjustmentBehavior.Automatic, UIKit decides whether to inset the SCROLL
        // VIEW (AdjustedContentInset, when the content overflows) or to push the safe area into the CHILD
        // (when it fits). Content sized between "fits alone" and "fits with the safe area" makes it flip
        // between the two, and since our contentSize depends on which branch arranged the content, the two
        // chase each other forever — a hang, not a wobble (measured: the iOS gallery stopped responding on
        // exactly the ScrollView-rooted pages). MAUI's fix, ported verbatim: when the content is nearly
        // large enough to scroll, force contentSize PAST the bounds so UIKit stays in scrollable mode and
        // keeps the inset at the scroll-view level. The width/height asymmetry (+= vs =) is C#'s own.
        if (scroller.contentInsetAdjustmentBehavior == UIScrollViewContentInsetAdjustmentAutomatic)
        {
            maui::core::thickness safe_area;
            if (auto* const control = dynamic_cast<maui::controls::scroll_view*>(virtual_view()))
            {
                safe_area = control->effective_safe_area();
            }
            if (extent.width <= frame.width && (safe_area.horizontal_thickness() + extent.width) >= frame.width)
            {
                extent.width += frame.width + 1;
            }
            // DOCUMENTED DEVIATION: C# uses `>` here; the port uses `>=`. With `>`, content that fills the
            // window EXACTLY never converges: the manual branch yields extent 1039 (clamped to 1040 ->
            // UIKit adds its inset) but the system branch then arranges into the reduced 998, and
            // 41 + 998 > 1039 is false BY ONE BOUNDARY, so contentSize drops back under the threshold,
            // UIKit removes the inset, and the two branches trade places forever (measured: sysTop
            // 0/41/0/41 on every pass, leaving whichever branch the pass cap happened to stop on — which
            // is why this page measured 0, then +32, then -32 across three runs). MAUI never notices
            // because its LayoutSubviews only re-arranges when the FRAME changed, so a pure inset flip
            // does not re-enter its arrange; the port drives layout itself and does. `>=` pins UIKit in
            // scrollable mode for the exactly-fits case, which converges in two passes (verified).
            if (extent.height <= frame.height && (safe_area.vertical_thickness() + extent.height) >= frame.height)
            {
                extent.height = frame.height + 1;
            }
        }
        if (!CGSizeEqualToSize(scroller.contentSize, extent))
        {
            scroller.contentSize = extent; // ScrollViewExtensions.UpdateContentSize's change guard
        }
    }

    // Render transform pushed to the native UIView via the shared ios apply_transform helper
    // (the generic-IView ViewMapper widening). `native` is this struct's UIView handle.
    void scroll_view_platform::update_transform(const maui::core::transform_spec& value)
    {
        maui::platform::ios::apply_transform(native, value);
    }

} // namespace maui::core
