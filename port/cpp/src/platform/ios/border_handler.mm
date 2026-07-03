// border_handler — iOS (UIKit) platform recipe: a plain UIView host that holds the single content
// child as a subview and carries the border on its layer — the shape mask plus the CAShapeLayer
// stroke built by ios_border_ops.hpp (the MauiCALayer recipe over stock layers; the ops header
// documents the adaptation). Ported from BorderHandler.iOS.cs (CreatePlatformView → the ContentView
// host; UpdateContent → ClearSubviews + re-parent the content's native view) + the StrokeExtensions
// funnel (every stroke map → one UpdateMauiCALayer refresh → update_border here). Compiled as
// Objective-C++ with ARC only for the `ios` backend.

#import <UIKit/UIKit.h>

#include <memory>
#include <string>
#include <string_view>

#include "ios_border_ops.hpp"
#include "ios_semantics_ops.hpp"
#include "ios_visual_ops.hpp"
#include "maui/core/border_handler.hpp"
#include "maui/core/i_border_view.hpp"
#include "maui/core/i_view.hpp"
#include "maui/core/i_view_handler.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/rect.hpp"

// MauiIosBorder — the UIView host the border handler presents. Its layoutSubviews override does the
// bounds-dependent re-sync the handler's property/arrange path cannot do on its own: (1) re-sizes any
// gradient/image Background sublayer apply_background installed (a solid BackgroundColor needs no resize
// — it is the backing layer's backgroundColor), and (2) re-runs the border-stroke + shape-mask refresh
// against the CURRENT bounds. apply_background and the stroke are first applied at property-sync time,
// before the host has been arranged, when layer.bounds is still zero — so without this hook a
// gradient/image fill would stay zero-sized (invisible) and a UIKit-driven re-layout (autoresize /
// rotation) that bypasses the handler's platform_arrange would leave the mask clipping to the old size.
// The same resize_background_layers pattern the value-control MauiIos* subclasses use, plus the
// border_refresh block the handler installs in update_border so layoutSubviews can re-stroke without a
// back-reference to the C++ handler. (No C# MauiIosBorder analog — C# iOS hosts the border in a
// MauiCALayer-backed ContentView that re-draws in DrawInContext; the port rebuilds the equivalent from
// stock layers, so this layoutSubviews is the stock-layer stand-in for that redraw.)
@interface MauiIosBorder : UIView
// Re-applies the bounds-dependent border stroke + shape mask; set by border_handler::update_border to
// capture the current border spec. nil before the first stroke push (no stroke → nothing to re-apply).
@property(nonatomic, copy) void (^borderRefresh)(void);
@end

@implementation MauiIosBorder
- (void)layoutSubviews
{
    [super layoutSubviews];
    // Re-size the gradient/image Background sublayer to the new bounds (the invisible-gradient fix), then
    // re-stroke + re-mask the shape against those bounds (keeps the gradient clipped after a UIKit-driven
    // resize that did not route through the handler's platform_arrange / update_border).
    maui::platform::ios::resize_background_layers((__bridge void*)self);
    if (self.borderRefresh != nil)
    {
        self.borderRefresh();
    }
}
@end

namespace
{
    UIView* as_host(void* native)
    {
        return (__bridge UIView*)native;
    }

    // The child's native UIView via its view-handler's native_view() (the content_page helper twin).
    UIView* native_child(maui::core::i_view& child)
    {
        auto* handler = dynamic_cast<maui::core::i_view_handler*>(child.handler().get());
        if (handler == nullptr)
        {
            return nil;
        }
        return (__bridge UIView*)handler->native_view();
    }
} // namespace

namespace maui::core
{
    border_platform::~border_platform()
    {
        if (native != nullptr)
        {
            // Remove the unmasked shadow sibling from the host's superlayer (it outlives the host otherwise).
            // The null shadow removes it regardless of the spec/fill args.
            maui::platform::ios::apply_border_shadow(native, nullptr, border, false, maui::graphics::rect{});
            CFRelease(native); // balances the __bridge_retained in create_platform_view
            native = nullptr;
        }
    }

    // The generic-IView property pushes. is_enabled, transform and flow_direction keep the base
    // mirrors (the content_page partial's scope); update_clip stays with the base mirror too — the
    // border shape owns the layer mask (border_handler.hpp).
    void border_platform::update_visibility(maui::core::visibility value)
    {
        as_host(native).hidden = value != maui::core::visibility::visible;
    }

    void border_platform::update_opacity(double value)
    {
        as_host(native).alpha = value;
    }

    void border_platform::update_automation_id(std::string_view value)
    {
        const std::string id(value);
        NSString* const raw = [NSString stringWithUTF8String:id.c_str()];
        as_host(native).accessibilityIdentifier = raw != nil ? raw : @"";
    }

    void border_platform::update_background(const maui::graphics::paint* value)
    {
        // The container layer's fill; the border shape mask bounds it to the shape (the C# MauiCALayer
        // draws the background into the same clipped layer).
        background = value;
        maui::platform::ios::apply_background(native, value);
        // The fill-vs-stroke-only state picks the shadow SILHOUETTE (border_shadow_silhouette_path), so a
        // Background change must re-cast the shadow. shadow may still be null here (property-push order) —
        // then this is a no-op and update_shadow casts it later with the now-stored background.
        const CGRect b = as_host(native).bounds;
        maui::platform::ios::apply_border_shadow(
            native, shadow, border, value != nullptr,
            maui::graphics::rect{b.origin.x, b.origin.y, b.size.width, b.size.height});
    }

    void border_platform::update_shadow(const maui::core::i_shadow* value)
    {
        // The host layer is masked to the shape (apply_clip), and a masked CALayer can't cast a shadow, so
        // apply_shadow on it is dead. Drive an unmasked sibling shadow layer instead (created once the host
        // has a superlayer — arrange_native re-applies from the stored borrow so the deferred create lands).
        // The silhouette follows the Background: an opaque fill casts the whole shape, a stroke-only Border
        // only its ring (border_shadow_silhouette_path).
        shadow = value;
        const CGRect b = as_host(native).bounds;
        maui::platform::ios::apply_border_shadow(
            native, value, border, background != nullptr,
            maui::graphics::rect{b.origin.x, b.origin.y, b.size.width, b.size.height});
    }

    void border_platform::update_semantics(const maui::core::semantics* value)
    {
        maui::platform::ios::apply_semantics((__bridge UIView*)native, value);
    }

    void border_platform::update_input_transparent(bool value)
    {
        maui::platform::ios::apply_input_transparent((__bridge UIView*)native, value);
    }

    std::unique_ptr<border_platform> border_handler::create_platform_view()
    {
        auto platform = std::make_unique<border_platform>();
        // A MauiIosBorder (not a plain UIView) so layoutSubviews can re-sync the bounds-dependent
        // gradient/image Background fill + the stroke/mask after layout (see the subclass doc-comment).
        MauiIosBorder* const host = [[MauiIosBorder alloc] initWithFrame:CGRectMake(0, 0, 0, 0)];
        platform->native = (__bridge_retained void*)host; // the void* slot owns one reference
        return platform;
    }

    void border_handler::set_content()
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        UIView* const host = as_host(platform->native);

        // C# UpdateContent: ClearSubviews, then re-parent the content's native view.
        NSArray<UIView*>* const snapshot = [host.subviews copy];
        [snapshot makeObjectsPerformSelector:@selector(removeFromSuperview)];

        platform->hosted_content = virtual_view() != nullptr ? virtual_view()->content() : nullptr;
        if (platform->hosted_content == nullptr)
        {
            return;
        }
        if (UIView* const subview = native_child(*platform->hosted_content))
        {
            [subview removeFromSuperview];
            [host addSubview:subview];
        }
    }

    void border_handler::update_border()
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr || virtual_view() == nullptr)
        {
            return;
        }
        // Mirror the resolved stroke surface, then push it onto the host's layer (the bounds-dependent
        // stroke path uses the view's LOCAL bounds, like the update_clip callers).
        platform->border = make_border_stroke_spec(*virtual_view());
        void* const native = platform->native;
        const CGRect bounds = as_host(native).bounds;
        maui::platform::ios::apply_border_stroke(
            native, platform->border,
            maui::graphics::rect{bounds.origin.x, bounds.origin.y, bounds.size.width, bounds.size.height});

        // Install the layoutSubviews re-stroke: a UIKit-driven re-layout (autoresize / rotation) that does
        // NOT route through the handler's platform_arrange must still re-apply the stroke + shape mask to
        // the new bounds, so the gradient background stays clipped to the shape. Capture the just-resolved
        // spec by value; the shape is a non-owning borrow the control keeps alive (re-read every
        // update_border, so the block is always current).
        if ([as_host(native) isKindOfClass:[MauiIosBorder class]])
        {
            const maui::core::border_stroke_spec spec = platform->border;
            const bool has_fill = platform->background != nullptr;
            ((MauiIosBorder*)as_host(native)).borderRefresh = ^{
              const CGRect b = as_host(native).bounds;
              const maui::graphics::rect r{b.origin.x, b.origin.y, b.size.width, b.size.height};
              maui::platform::ios::apply_border_stroke(native, spec, r);
              // Track the shadow sibling to the new bounds too (a UIKit-driven resize that bypasses arrange).
              maui::platform::ios::reframe_border_shadow(native, spec, has_fill, r);
            };
        }
    }

    void border_handler::arrange_native(const maui::graphics::rect& frame)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        [as_host(platform->native) setFrame:CGRectMake(frame.x, frame.y, frame.width, frame.height)];
        // Re-apply the shadow sibling now the host has a frame AND a superlayer (the deferred create from
        // update_shadow lands here on the first arrange, and it re-frames on every subsequent arrange).
        const CGRect b = as_host(platform->native).bounds;
        maui::platform::ios::apply_border_shadow(
            platform->native, platform->shadow, platform->border, platform->background != nullptr,
            maui::graphics::rect{b.origin.x, b.origin.y, b.size.width, b.size.height});
    }

    // Render transform pushed to the native UIView via the shared ios apply_transform helper
    // (the generic-IView ViewMapper widening). `native` is this struct's UIView handle.
    void border_platform::update_transform(const maui::core::transform_spec& value)
    {
        maui::platform::ios::apply_transform(native, value);
    }

} // namespace maui::core
