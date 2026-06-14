// indicator_view_handler — Apple (AppKit / macOS) platform recipe. AppKit has NO UIPageControl, so the
// managed platform view is an NSStackView of dot subviews (the C# MauiPageControl twin assembled by
// hand — the Android handler's ResetIndicators recipe is the closest cross-platform precedent). Each
// dot is a layer-backed NSView with a circular/square corner radius; the selected dot takes the
// selected color, the rest the indicator color; the row holds GetMaximumVisible dots. Recreated on any
// count/size/shape/color/position change (the C# UpdateIndicatorCount / ResetIndicators). The
// cross-platform mirror (dot_count / current_page / shape / colors / size) is kept in sync so the
// apple suite can assert both the real subview row and the recorded state.
//
// Compiled as Objective-C++ with ARC only for the `apple` backend. Translated from
// IndicatorViewHandler.cs/.Android.cs (the dot-assembly recipe) + IndicatorViewExtensions.cs
// (GetMaximumVisible) + MauiPageControl.cs (the size/clamp behavior).
//
// DOCUMENTED DEVIATION: AppKit has no native page control; the dots are drawn by hand. No native tap
// write-back (the dots are inert NSViews — UIPageControl's ValueChanged has no AppKit analog here), so
// the apple indicator Position is virtual→native only (set_position still works programmatically).

#import <AppKit/AppKit.h>

#include <algorithm>
#include <memory>
#include <string>
#include <string_view>

#include "apple_conversions.hpp"
#include "maui/core/i_indicator_view.hpp"
#include "maui/core/indicator_view_handler.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

namespace
{
    NSStackView* as_stack(void* native)
    {
        return (__bridge NSStackView*)native;
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
        as_stack(native).hidden = value != maui::core::visibility::visible;
    }

    void indicator_view_platform::update_opacity(double value)
    {
        as_stack(native).alphaValue = value;
    }

    void indicator_view_platform::update_automation_id(std::string_view value)
    {
        const std::string id(value);
        NSString* const raw = [NSString stringWithUTF8String:id.c_str()];
        as_stack(native).accessibilityIdentifier = raw != nil ? raw : @"";
    }

    std::unique_ptr<indicator_view_platform> indicator_view_handler::create_platform_view()
    {
        auto platform = std::make_unique<indicator_view_platform>();
        NSStackView* const stack = [[NSStackView alloc] initWithFrame:NSMakeRect(0, 0, 0, 0)];
        stack.orientation = NSUserInterfaceLayoutOrientationHorizontal;
        stack.spacing = 4; // the C# DefaultPadding
        stack.wantsLayer = YES;
        platform->native = (__bridge_retained void*)stack; // the void* slot owns one reference
        return platform;
    }

    void indicator_view_handler::on_connect_handler(indicator_view_platform& /*platform*/)
    {
        // C# ConnectHandler: SetIndicatorView + UpdateIndicator. The dot row is built by the mapper pass
        // (which runs right after connect), so no extra work here.
    }

    void indicator_view_handler::on_disconnect_handler(indicator_view_platform& /*platform*/)
    {
    }

    namespace
    {
        // Rebuild the dot row: GetMaximumVisible dots sized indicator_size, the current page tinted with
        // the selected color, the rest the indicator color. The C# UpdateIndicatorCount + ResetIndicators
        // collapsed (AppKit rebuilds the whole row on any change — cheap, a handful of small views).
        void rebuild_dots(maui::core::indicator_view_handler& handler, maui::core::i_indicator_view& view)
        {
            auto* platform = handler.typed_platform_view();
            if (platform == nullptr)
            {
                return;
            }
            const int dots = maui::core::max_visible_indicators(view);
            const int position = view.position();
            const int current = dots > 0 ? std::min(position, dots - 1) : -1;
            const double size = view.indicator_size();
            const bool square = view.indicators_shape() == maui::controls::indicator_shape::square;

            // Mirror first (the oracle record), then materialize the row.
            platform->dot_count = dots;
            platform->current_page = current;
            platform->indicator_size = size;
            platform->shape = view.indicators_shape();
            platform->indicator_color = view.indicator_color();
            platform->selected_indicator_color = view.selected_indicator_color();

            NSStackView* const stack = as_stack(platform->native);
            for (NSView* const child in [stack.arrangedSubviews copy])
            {
                [stack removeArrangedSubview:child];
                [child removeFromSuperview];
            }
            for (int index = 0; index < dots; ++index)
            {
                NSView* const dot = [[NSView alloc] initWithFrame:NSMakeRect(0, 0, size, size)];
                dot.wantsLayer = YES;
                dot.layer.cornerRadius = square ? 0.0 : size / 2.0;
                const maui::graphics::color tint =
                    index == current ? view.selected_indicator_color() : view.indicator_color();
                dot.layer.backgroundColor = maui::platform::apple::to_ns_color(tint).CGColor;
                [dot.widthAnchor constraintEqualToConstant:size].active = YES;
                [dot.heightAnchor constraintEqualToConstant:size].active = YES;
                [stack addArrangedSubview:dot];
            }
        }
    } // namespace

    void indicator_view_handler::map_count(indicator_view_handler& handler, i_indicator_view& view)
    {
        rebuild_dots(handler, view);
    }

    void indicator_view_handler::map_maximum_visible(indicator_view_handler& handler, i_indicator_view& view)
    {
        rebuild_dots(handler, view);
    }

    void indicator_view_handler::map_hide_single(indicator_view_handler& handler, i_indicator_view& view)
    {
        rebuild_dots(handler, view);
    }

    void indicator_view_handler::map_position(indicator_view_handler& handler, i_indicator_view& view)
    {
        rebuild_dots(handler, view); // re-tint the selected dot
    }

    void indicator_view_handler::map_indicator_size(indicator_view_handler& handler, i_indicator_view& view)
    {
        rebuild_dots(handler, view);
    }

    void indicator_view_handler::map_indicator_color(indicator_view_handler& handler, i_indicator_view& view)
    {
        rebuild_dots(handler, view);
    }

    void indicator_view_handler::map_selected_indicator_color(indicator_view_handler& handler, i_indicator_view& view)
    {
        rebuild_dots(handler, view);
    }

    void indicator_view_handler::map_indicator_shape(indicator_view_handler& handler, i_indicator_view& view)
    {
        rebuild_dots(handler, view);
    }

    maui::graphics::size indicator_view_handler::get_desired_size(double /*width_constraint*/,
                                                                  double /*height_constraint*/) const
    {
        const auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return {0, 0};
        }
        const NSSize fitting = [as_stack(platform->native) fittingSize];
        return {fitting.width, fitting.height};
    }

    void indicator_view_handler::platform_arrange(const maui::graphics::rect& frame)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        [as_stack(platform->native) setFrame:NSMakeRect(frame.x, frame.y, frame.width, frame.height)];
    }
} // namespace maui::core
