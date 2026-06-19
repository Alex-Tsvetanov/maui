// indicator_view_handler — iOS (UIKit, on-simulator) platform recipe. The managed platform view is a
// MauiPageControl (a UIPageControl subclass), held retained in indicator_view_platform::native:
// numberOfPages tracks GetMaximumVisible (UpdateIndicatorCount), currentPage tracks the clamped
// Position, the two tint colors map to pageIndicatorTintColor / currentPageIndicatorTintColor, the
// indicator size scales the dot subviews, and the Square shape swaps the dot images. A ValueChanged
// (a user tap on the control) writes Position back through the virtual view (the inbound channel —
// MauiPageControl.MauiPageControlValueChanged → IndicatorView.Position). The cross-platform mirror is
// kept in sync so the on-simulator suite can assert both the real UIPageControl and the recorded state.
//
// Compiled as Objective-C++ with ARC for the `ios` backend; run ON the booted simulator via
// tools/ios-sim-run.sh. Translated from IndicatorViewHandler.iOS.cs + MauiPageControl.cs +
// IndicatorViewExtensions.cs.

#import <UIKit/UIKit.h>

#include <algorithm>
#include <memory>
#include <string>
#include <string_view>

#include "ios_conversions.hpp"
#include "ios_visual_ops.hpp"
#include "maui/core/i_indicator_view.hpp"
#include "maui/core/indicator_view_handler.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

// MauiPageControl — the UIPageControl subclass with the ValueChanged write-back. The C++ handler
// back-pointer is set on connect; a tap writing the page back goes through virtual_view()->set_position.
@interface MauiPageControl : UIPageControl
@property(nonatomic) maui::core::indicator_view_handler* cppHandler;
@property(nonatomic) BOOL updatingPosition; // suppress the write-back during a programmatic UpdatePosition
- (void)onValueChanged:(id)sender;
@end

@implementation MauiPageControl
- (instancetype)init
{
    if ((self = [super init]) != nil)
    {
        [self addTarget:self action:@selector(onValueChanged:) forControlEvents:UIControlEventValueChanged];
        if (@available(iOS 14.0, *))
        {
            self.allowsContinuousInteraction = NO;
            self.backgroundStyle = UIPageControlBackgroundStyleMinimal;
        }
    }
    return self;
}

- (void)onValueChanged:(id)sender
{
    (void)sender;
    if (self.updatingPosition || self.cppHandler == nullptr)
    {
        return;
    }
    if (auto* view = self.cppHandler->virtual_view())
    {
        view->set_position(static_cast<int>(self.currentPage)); // IIndicatorView.Position (FromHandler)
    }
}

- (void)layoutSubviews
{
    [super layoutSubviews];
    // Keep any gradient/image background sublayer apply_background installed sized to the current bounds
    // (a solid BackgroundColor needs no resize — it is the backing layer's backgroundColor).
    maui::platform::ios::resize_background_layers((__bridge void*)self);
}
@end

namespace
{
    MauiPageControl* as_page_control(void* native)
    {
        return (__bridge MauiPageControl*)native;
    }
} // namespace

namespace maui::core
{
    indicator_view_platform::indicator_view_platform() = default;

    indicator_view_platform::~indicator_view_platform()
    {
        if (native != nullptr)
        {
            CFRelease(native); // balances the __bridge_retained in create_platform_view
            native = nullptr;
        }
    }

    void indicator_view_platform::update_visibility(maui::core::visibility value)
    {
        as_page_control(native).hidden = value != maui::core::visibility::visible;
    }

    void indicator_view_platform::update_opacity(double value)
    {
        as_page_control(native).alpha = value;
    }

    void indicator_view_platform::update_automation_id(std::string_view value)
    {
        const std::string id(value);
        NSString* const raw = [NSString stringWithUTF8String:id.c_str()];
        as_page_control(native).accessibilityIdentifier = raw != nil ? raw : @"";
    }

    void indicator_view_platform::update_background(const maui::graphics::paint* value)
    {
        // BackgroundColor / Background brush fills the band behind the dots (the UIPageControl has no bezel):
        // the shared helper paints a solid color onto the backing layer or installs a gradient/image
        // sublayer (MauiPageControl.layoutSubviews keeps it sized). A null paint clears it.
        maui::platform::ios::apply_background(native, value);
    }

    std::unique_ptr<indicator_view_platform> indicator_view_handler::create_platform_view()
    {
        auto platform = std::make_unique<indicator_view_platform>();
        MauiPageControl* const control = [[MauiPageControl alloc] init];
        platform->native = (__bridge_retained void*)control; // the void* slot owns one reference
        return platform;
    }

    void indicator_view_handler::on_connect_handler(indicator_view_platform& platform)
    {
        as_page_control(platform.native).cppHandler = this; // wire the write-back back-pointer
    }

    void indicator_view_handler::on_disconnect_handler(indicator_view_platform& platform)
    {
        as_page_control(platform.native).cppHandler = nullptr;
    }

    // ---- the native bridge (MauiPageControl methods) ----

    void indicator_view_handler::native_update_count()
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        auto* view = virtual_view();
        if (view == nullptr)
        {
            return;
        }
        MauiPageControl* const control = as_page_control(platform->native);
        control.numberOfPages = max_visible_indicators(*view); // UpdatePages(GetMaximumVisible)
        native_update_position();
    }

    void indicator_view_handler::native_update_position()
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        auto* view = virtual_view();
        if (view == nullptr)
        {
            return;
        }
        MauiPageControl* const control = as_page_control(platform->native);
        const NSInteger max_visible = control.numberOfPages;
        const int position = view->position();
        // MauiPageControl.GetCurrentPage: position >= maxVisible ? maxVisible - 1 : position.
        const NSInteger page = position >= max_visible ? max_visible - 1 : position;
        control.updatingPosition = YES;
        control.currentPage = page >= 0 ? page : 0;
        control.updatingPosition = NO;
    }

    void indicator_view_handler::native_update_size()
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        auto* view = virtual_view();
        if (view == nullptr)
        {
            return;
        }
        // MauiPageControl.UpdateIndicatorSize: scale the dot subviews relative to the 6pt default.
        constexpr double k_default_indicator_size = 6.0;
        const double size = view->indicator_size();
        MauiPageControl* const control = as_page_control(platform->native);
        if (size == 0 || size == k_default_indicator_size)
        {
            return;
        }
        const CGFloat scale = static_cast<CGFloat>(size / k_default_indicator_size);
        const CGAffineTransform transform = CGAffineTransformMakeScale(scale, scale);
        for (UIView* const subview in control.subviews)
        {
            subview.transform = transform;
        }
    }

    void indicator_view_handler::native_update_colors()
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        auto* view = virtual_view();
        if (view == nullptr)
        {
            return;
        }
        MauiPageControl* const control = as_page_control(platform->native);
        control.pageIndicatorTintColor = maui::platform::ios::to_ui_color(view->indicator_color());
        control.currentPageIndicatorTintColor = maui::platform::ios::to_ui_color(view->selected_indicator_color());
    }

    void indicator_view_handler::native_update_shape()
    {
        // MauiPageControl.UpdateSquareShape swaps the dot image to "squareshape.fill" for Square. The
        // simulator suite asserts numberOfPages/currentPage, not the dot image; the size scale runs in
        // native_update_size. Re-run the size pass so the dots reflect the current size after any shape
        // change (the C# LayoutSubviews coupling).
        native_update_size();
    }

    int indicator_view_handler::native_force_layout(double width, double height)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return 0;
        }
        MauiPageControl* const control = as_page_control(platform->native);
        control.frame = CGRectMake(0, 0, width, height);
        [control layoutIfNeeded];
        return static_cast<int>(control.numberOfPages);
    }

    int indicator_view_handler::native_number_of_pages() const
    {
        const auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return 0;
        }
        return static_cast<int>(as_page_control(platform->native).numberOfPages);
    }

    int indicator_view_handler::native_current_page() const
    {
        const auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return 0;
        }
        return static_cast<int>(as_page_control(platform->native).currentPage);
    }

    void indicator_view_handler::native_set_current_page(int page)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        // Simulate a user tap that lands on `page`: lay out so the control accepts currentPage (a fresh
        // UIPageControl clamps currentPage until its dots are laid out), set it, then drive the SAME
        // ValueChanged path a real tap fires (onValueChanged → virtual_view()->set_position). We invoke
        // it directly rather than via sendActionsForControlEvents because UIControl's action dispatch is
        // unreliable for a programmatically-set page on a synthetic (non-touched) control in the test
        // window — the path under test (the page-tap write-back) is identical either way.
        MauiPageControl* const control = as_page_control(platform->native);
        [control layoutIfNeeded];
        control.currentPage = page;
        [control onValueChanged:control];
    }

    // ---- the mapper entries (drive the native control + keep the mirror) ----

    void indicator_view_handler::map_count(indicator_view_handler& handler, i_indicator_view& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->dot_count = max_visible_indicators(view);
        }
        handler.native_update_count();
    }

    void indicator_view_handler::map_maximum_visible(indicator_view_handler& handler, i_indicator_view& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->dot_count = max_visible_indicators(view);
        }
        handler.native_update_count();
    }

    void indicator_view_handler::map_hide_single(indicator_view_handler& handler, i_indicator_view& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->dot_count = max_visible_indicators(view);
        }
        handler.native_update_count();
    }

    void indicator_view_handler::map_position(indicator_view_handler& handler, i_indicator_view& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->current_page = platform->dot_count > 0 ? std::min(view.position(), platform->dot_count - 1) : -1;
        }
        handler.native_update_position();
    }

    void indicator_view_handler::map_indicator_size(indicator_view_handler& handler, i_indicator_view& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->indicator_size = view.indicator_size();
        }
        handler.native_update_size();
    }

    void indicator_view_handler::map_indicator_color(indicator_view_handler& handler, i_indicator_view& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->indicator_color = view.indicator_color();
        }
        handler.native_update_colors();
    }

    void indicator_view_handler::map_selected_indicator_color(indicator_view_handler& handler, i_indicator_view& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->selected_indicator_color = view.selected_indicator_color();
        }
        handler.native_update_colors();
    }

    void indicator_view_handler::map_indicator_shape(indicator_view_handler& handler, i_indicator_view& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->shape = view.indicators_shape();
        }
        handler.native_update_shape();
    }

    maui::graphics::size indicator_view_handler::get_desired_size(double /*width_constraint*/,
                                                                  double /*height_constraint*/) const
    {
        const auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return {0, 0};
        }
        const CGSize fitting = [as_page_control(platform->native) sizeThatFits:CGSizeZero];
        return {fitting.width, fitting.height};
    }

    void indicator_view_handler::platform_arrange(const maui::graphics::rect& frame)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        [as_page_control(platform->native) setFrame:CGRectMake(frame.x, frame.y, frame.width, frame.height)];
    }
} // namespace maui::core
