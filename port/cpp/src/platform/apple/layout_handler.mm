// layout_handler — Apple (AppKit / macOS) platform recipe: a plain NSView container that hosts the
// arranged children. The real-native twin of the headless partial. Translated from LayoutHandler.iOS.cs
// (UIKit's LayoutView → an AppKit NSView): the panel only HOSTS — each child is positioned by the
// layout_manager via the child's own platform_arrange; the panel just manages the subview list. The
// layout_manager's arrange rects are TOP-DOWN (the shared headless geometry, the UIKit convention), so
// the panel is a FLIPPED NSView (create_flipped_host: isFlipped=YES, top-left origin) — otherwise the
// children would render bottom-up / inverted vs iOS. Compiled as Objective-C++ with ARC for the `apple`
// backend.

#import <AppKit/AppKit.h>

#include <algorithm>
#include <cstddef>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "apple_semantics_ops.hpp"
#include "apple_view_ops.hpp"
#include "apple_visual_ops.hpp"
#include "flipped_container.hpp"
#include "maui/core/i_element_handler.hpp"
#include "maui/core/i_layout.hpp"
#include "maui/core/i_view.hpp"
#include "maui/core/i_view_handler.hpp"
#include "maui/core/layout_handler.hpp"
#include "maui/core/layout_z_order.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

namespace
{
    NSView* as_panel(void* native)
    {
        return (__bridge NSView*)native;
    }

    // The child's native NSView to host, via its view-handler's native_view() (null if the child is
    // unattached or its handler has no native view). native_view() is C#'s ToPlatform() =
    // ContainerView ?? PlatformView, so a NeedsContainer child (e.g. the switch) hands back its CONTAINER
    // NSView here — not the bare native, and not the pimpl pointer that platform_view() returns.
    NSView* native_child(maui::core::i_view& child)
    {
        auto* handler = dynamic_cast<maui::core::i_view_handler*>(child.handler().get());
        if (handler == nullptr)
        {
            return nil;
        }
        return (__bridge NSView*)handler->native_view();
    }

    // Place `subview` at `target_index` in `panel`'s subview list (the AppKit analog of UIKit's
    // InsertSubview:atIndex:). AppKit has no insert-at-index — subview order is set by relative positioning,
    // so position the view below the subview currently at target_index (or above the last for an
    // end/overflow index). A negative or zero index drops it to the bottom (NSWindowBelow the first).
    void place_subview_at(NSView* panel, NSView* subview, NSInteger target_index)
    {
        const NSInteger count = static_cast<NSInteger>(panel.subviews.count);
        if (target_index <= 0 || count == 0)
        {
            [panel addSubview:subview positioned:NSWindowBelow relativeTo:panel.subviews.firstObject];
        }
        else if (target_index >= count)
        {
            [panel addSubview:subview positioned:NSWindowAbove relativeTo:panel.subviews.lastObject];
        }
        else
        {
            [panel addSubview:subview
                   positioned:NSWindowBelow
                   relativeTo:panel.subviews[static_cast<NSUInteger>(target_index)]];
        }
    }
} // namespace

namespace maui::core
{
    layout_platform::~layout_platform()
    {
        if (native != nullptr)
        {
            CFRelease(native); // balances the __bridge_retained in create_platform_view
            native = nullptr;
        }
    }

    // The generic-IView property pushes (the shared view_mapper calls these via view_platform_base). The
    // panel is a plain NSView; is_enabled has no NSView equivalent, so it is left to the base mirror.
    void layout_platform::update_visibility(maui::core::visibility value)
    {
        as_panel(native).hidden = value != maui::core::visibility::visible;
    }

    void layout_platform::update_opacity(double value)
    {
        as_panel(native).alphaValue = value;
    }

    void layout_platform::update_automation_id(std::string_view value)
    {
        const std::string id(value);
        NSString* const raw = [NSString stringWithUTF8String:id.c_str()];
        as_panel(native).accessibilityIdentifier = raw != nil ? raw : @"";
    }

    std::unique_ptr<layout_platform> layout_handler::create_platform_view()
    {
        auto platform = std::make_unique<layout_platform>();
        // A flipped (top-left origin) panel so the layout_manager's top-down child frames render top-down.
        platform->native = maui::platform::apple::create_flipped_host(); // retained — the void* slot owns one ref
        return platform;
    }

    // C# LayoutHandler.Add inserts the subview at GetLayoutHandlerIndex (the child's z-ordered position),
    // so the panel stays front-to-back by z-index. The child is already in the layout's logical list.
    void layout_handler::add(i_view& child)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        if (NSView* const subview = native_child(child))
        {
            const int target = virtual_view() != nullptr ? get_layout_handler_index(*virtual_view(), child) : -1;
            place_subview_at(as_panel(platform->native), subview, static_cast<NSInteger>(target));
            auto& children = platform->children;
            const auto position = std::min(static_cast<std::size_t>(std::max(target, 0)), children.size());
            children.insert(children.begin() + static_cast<std::ptrdiff_t>(position), &child);
        }
    }

    void layout_handler::remove(i_view& child)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        if (NSView* const subview = native_child(child);
            subview != nil && subview.superview == as_panel(platform->native))
        {
            [subview removeFromSuperview];
        }
        std::erase(platform->children, &child);
    }

    void layout_handler::clear()
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        NSView* const panel = as_panel(platform->native);
        // Snapshot the subviews (removeFromSuperview mutates the live array) and tear them down without an
        // Obj-C fast-enumeration loop (which clang-tidy's init-variables check misreads as uninitialized).
        NSArray<NSView*>* const snapshot = [panel.subviews copy];
        [snapshot makeObjectsPerformSelector:@selector(removeFromSuperview)];
        platform->children.clear();
    }

    // C# LayoutHandler.Insert also positions the subview at GetLayoutHandlerIndex (the z-ordered slot), not
    // the logical `index` — the panel's subview order is z-index-driven. The child is in the logical list.
    void layout_handler::insert(int /*index*/, i_view& child)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        NSView* const panel = as_panel(platform->native);
        NSView* const subview = native_child(child);
        if (subview == nil)
        {
            return;
        }
        const int target = virtual_view() != nullptr ? get_layout_handler_index(*virtual_view(), child) : -1;
        place_subview_at(panel, subview, static_cast<NSInteger>(target));
        auto& children = platform->children;
        const auto position = std::min(static_cast<std::size_t>(std::max(target, 0)), children.size());
        children.insert(children.begin() + static_cast<std::ptrdiff_t>(position), &child);
    }

    void layout_handler::update(int index, i_view& child)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        NSView* const panel = as_panel(platform->native);
        NSView* const subview = native_child(child);
        if (subview == nil)
        {
            return;
        }
        // Swap the existing subview at `index` for the new child's, in place. index >= 0 is checked first,
        // so the bound compares in the unsigned domain (matching NSUInteger count).
        if (index >= 0 && static_cast<NSUInteger>(index) < panel.subviews.count)
        {
            [panel.subviews[static_cast<NSUInteger>(index)] removeFromSuperview];
            [panel addSubview:subview
                   positioned:(static_cast<NSUInteger>(index) < panel.subviews.count ? NSWindowBelow : NSWindowAbove)
                   relativeTo:(static_cast<NSUInteger>(index) < panel.subviews.count
                                   ? panel.subviews[static_cast<NSUInteger>(index)]
                                   : panel.subviews.lastObject)];
        }
        auto& children = platform->children;
        if (index >= 0 && static_cast<std::size_t>(index) < children.size())
        {
            children[static_cast<std::size_t>(index)] = &child; // replace-in-place: count is unchanged
        }
    }

    // C# LayoutHandler.EnsureZIndexOrder: if `child`'s subview is not already at its z-ordered index, move
    // it there (remove + reinsert at the target). The children mirror is re-synced to match.
    void layout_handler::update_z_index(i_view& child)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr || virtual_view() == nullptr)
        {
            return;
        }
        NSView* const panel = as_panel(platform->native);
        if (panel.subviews.count == 0)
        {
            return;
        }
        NSView* const subview = native_child(child);
        if (subview == nil)
        {
            return;
        }
        const NSUInteger current_index = [panel.subviews indexOfObject:subview];
        if (current_index == NSNotFound)
        {
            return;
        }
        // A found index is a real subview position, so it fits in the signed NSInteger used for target.
        const auto current = static_cast<NSInteger>(current_index);
        const auto target = static_cast<NSInteger>(get_layout_handler_index(*virtual_view(), child));
        if (target < 0 || current == target)
        {
            return; // not found, or already at its z-ordered slot
        }
        [subview removeFromSuperview];
        place_subview_at(panel, subview, target);
        auto& children = platform->children;
        std::erase(children, &child);
        const auto position = std::min(static_cast<std::size_t>(std::max<NSInteger>(target, 0)), children.size());
        children.insert(children.begin() + static_cast<std::ptrdiff_t>(position), &child);
    }

    maui::graphics::size layout_handler::get_desired_size(double /*width_constraint*/,
                                                          double /*height_constraint*/) const
    {
        // A layout computes its own size through its layout_manager (the control overrides measure to
        // delegate to the manager, not the handler), so the handler reports nothing here.
        return {0, 0};
    }

    void layout_handler::platform_arrange(const maui::graphics::rect& frame)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        [as_panel(platform->native) setFrame:NSMakeRect(frame.x, frame.y, frame.width, frame.height)];
    }

    // Render transform + flow direction pushed to the native view via the shared apple_view_ops helpers
    // (M4c: the generic-IView ViewMapper widening). `native` is this struct's NSView handle.
    void layout_platform::update_transform(const maui::core::transform_spec& value)
    {
        maui::platform::apple::apply_transform(native, value);
    }

    void layout_platform::update_flow_direction(maui::core::flow_direction value)
    {
        maui::platform::apple::apply_flow_direction(native, value);
    }

    // Background / shadow / clip pushed to the native view's layer via the shared apple_visual_ops helpers
    // (M4d ViewMapper visuals). `native` is this struct's NSView handle.
    void layout_platform::update_background(const maui::graphics::paint* value)
    {
        maui::platform::apple::apply_background(native, value);
    }

    void layout_platform::update_shadow(const maui::core::i_shadow* value)
    {
        maui::platform::apple::apply_shadow(native, value);
    }

    void layout_platform::update_clip(const maui::graphics::i_shape* value)
    {
        // The clip mask is sized to the view's current bounds (WrapperView.SetClip uses the view frame).
        const NSRect bounds = ((__bridge NSView*)native).bounds;
        maui::platform::apple::apply_clip(
            native, value,
            maui::graphics::rect{bounds.origin.x, bounds.origin.y, bounds.size.width, bounds.size.height});
    }

    // Accessibility metadata + the input-transparent flag pushed to the native panel via the shared
    // apple_semantics_ops helpers (M5d native a11y / hit-test). `native` is this struct's NSView handle;
    // the -hitTest: gate drops the panel from hit-testing when transparent (MAUI LayoutView.HitTest).
    void layout_platform::update_semantics(const maui::core::semantics* value)
    {
        maui::platform::apple::apply_semantics((__bridge NSView*)native, value);
    }

    void layout_platform::update_input_transparent(bool value)
    {
        maui::platform::apple::apply_input_transparent((__bridge NSView*)native, value);
    }

    // ILayout.ClipsToBounds → the panel layer's masksToBounds (C# iOS sets PlatformView.ClipsToBounds,
    // which maps to CALayer.masksToBounds). A layer-backed NSView is required, so request one first.
    void layout_platform::update_clips_to_bounds(bool value)
    {
        clips_to_bounds = value; // keep the mirror in sync with the base
        NSView* const panel = as_panel(native);
        panel.wantsLayer = YES;
        panel.layer.masksToBounds = value ? YES : NO;
    }
} // namespace maui::core
