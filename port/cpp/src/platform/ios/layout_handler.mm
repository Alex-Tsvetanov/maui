// layout_handler — iOS (UIKit) platform recipe: a plain UIView container that hosts the arranged
// children. The real-native twin of the headless partial, ported DIRECTLY from LayoutHandler.iOS.cs
// (the same oracle the AppKit twin in src/platform/apple/layout_handler.mm was adapted from — UIKit
// needs no adaptation: InsertSubview:atIndex: exists natively, where AppKit had to emulate it with
// relative positioning). The panel only HOSTS — each child is positioned by the layout_manager via the
// child's own platform_arrange; the panel just manages the subview list, z-ordered by
// GetLayoutHandlerIndex. C#'s LayoutView (a MauiView subclass carrying CrossPlatformLayout) collapses
// to a plain UIView here because the port's measure/arrange flows through the cross-platform
// layout_manager, not UIKit's layoutSubviews — the same simplification as the AppKit twin; for the same
// reason C#'s InvalidateAncestorsMeasures / SetNeedsLayout calls have no analog. Compiled as
// Objective-C++ with ARC only for the `ios` backend.

#import <UIKit/UIKit.h>

#include <algorithm>
#include <cstddef>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "ios_semantics_ops.hpp"
#include "ios_visual_ops.hpp"
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
    UIView* as_panel(void* native)
    {
        return (__bridge UIView*)native;
    }

    // The child's native UIView to host, via its view-handler's native_view() (nil if the child is
    // unattached or its handler has no native view). native_view() is C#'s ToPlatform() =
    // ContainerView ?? PlatformView, so a NeedsContainer child (e.g. the switch) hands back its CONTAINER
    // UIView here — not the bare native, and not the pimpl pointer that platform_view() returns. Mirrors
    // the apple twin's native_child helper.
    UIView* native_child(maui::core::i_view& child)
    {
        auto* handler = dynamic_cast<maui::core::i_view_handler*>(child.handler().get());
        if (handler == nullptr)
        {
            return nil;
        }
        return (__bridge UIView*)handler->native_view();
    }

    // Insert `subview` at `target_index` in `panel`'s subview list — C#'s PlatformView.InsertSubview(
    // child, targetIndex), with the index clamped to the valid [0, count] range (UIKit throws on an
    // out-of-range index; a negative index — an unfound child — drops to the bottom, like the AppKit
    // twin's place_subview_at and the headless insert_at). atIndex:0 is the back-most slot.
    void insert_subview_at(UIView* panel, UIView* subview, NSInteger target_index)
    {
        const auto count = static_cast<NSInteger>(panel.subviews.count);
        [panel insertSubview:subview atIndex:std::clamp<NSInteger>(target_index, 0, count)];
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
    // panel is a plain UIView; is_enabled has no UIView equivalent, so it is left to the base mirror.
    void layout_platform::update_visibility(maui::core::visibility value)
    {
        as_panel(native).hidden = value != maui::core::visibility::visible;
    }

    void layout_platform::update_opacity(double value)
    {
        as_panel(native).alpha = value;
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
        UIView* const panel = [[UIView alloc] initWithFrame:CGRectMake(0, 0, 0, 0)];
        platform->native = (__bridge_retained void*)panel; // the void* slot owns one reference
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
        if (UIView* const subview = native_child(child))
        {
            const int target = virtual_view() != nullptr ? get_layout_handler_index(*virtual_view(), child) : -1;
            insert_subview_at(as_panel(platform->native), subview, static_cast<NSInteger>(target));
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
        if (UIView* const subview = native_child(child);
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
        UIView* const panel = as_panel(platform->native);
        // C# ClearSubviews. Snapshot the subviews (removeFromSuperview mutates the live array) and tear
        // them down without an Obj-C fast-enumeration loop (which clang-tidy's init-variables check
        // misreads as uninitialized).
        NSArray<UIView*>* const snapshot = [panel.subviews copy];
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
        UIView* const panel = as_panel(platform->native);
        UIView* const subview = native_child(child);
        if (subview == nil)
        {
            return;
        }
        const int target = virtual_view() != nullptr ? get_layout_handler_index(*virtual_view(), child) : -1;
        insert_subview_at(panel, subview, static_cast<NSInteger>(target));
        auto& children = platform->children;
        const auto position = std::min(static_cast<std::size_t>(std::max(target, 0)), children.size());
        children.insert(children.begin() + static_cast<std::ptrdiff_t>(position), &child);
    }

    // C# LayoutHandler.Update: remove the existing subview at `index`, then insert the new child's at
    // GetLayoutHandlerIndex (the z-ordered slot, like Add/Insert). The children mirror replaces in place
    // (count unchanged), matching the headless twin the tests observe.
    void layout_handler::update(int index, i_view& child)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        UIView* const panel = as_panel(platform->native);
        UIView* const subview = native_child(child);
        if (subview == nil)
        {
            return;
        }
        // index >= 0 is checked first, so the bound compares in the unsigned domain (NSUInteger count).
        if (index >= 0 && static_cast<NSUInteger>(index) < panel.subviews.count)
        {
            [panel.subviews[static_cast<NSUInteger>(index)] removeFromSuperview];
            const int target = virtual_view() != nullptr ? get_layout_handler_index(*virtual_view(), child) : index;
            insert_subview_at(panel, subview, static_cast<NSInteger>(target));
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
        UIView* const panel = as_panel(platform->native);
        if (panel.subviews.count == 0)
        {
            return;
        }
        UIView* const subview = native_child(child);
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
        insert_subview_at(panel, subview, target);
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
        [as_panel(platform->native) setFrame:CGRectMake(frame.x, frame.y, frame.width, frame.height)];
    }

    // Background / shadow / clip pushed to the native view's layer via the shared ios_visual_ops helpers
    // (the direct PaintExtensions / ShadowExtensions / WrapperView.SetClip ports). `native` is this
    // struct's UIView handle.
    void layout_platform::update_background(const maui::graphics::paint* value)
    {
        maui::platform::ios::apply_background(native, value);
    }

    void layout_platform::update_shadow(const maui::core::i_shadow* value)
    {
        maui::platform::ios::apply_shadow(native, value);
    }

    void layout_platform::update_clip(const maui::graphics::i_shape* value)
    {
        // The clip mask is sized to the view's current bounds (WrapperView.SetClip uses the view frame).
        const CGRect bounds = ((__bridge UIView*)native).bounds;
        maui::platform::ios::apply_clip(
            native, value,
            maui::graphics::rect{bounds.origin.x, bounds.origin.y, bounds.size.width, bounds.size.height});
    }

    // Accessibility metadata + the input-transparent flag pushed to the native panel via the shared
    // ios_semantics_ops helpers (SemanticExtensions.UpdateSemantics / ViewExtensions.
    // UpdateInputTransparent). `native` is this struct's UIView handle.
    void layout_platform::update_semantics(const maui::core::semantics* value)
    {
        maui::platform::ios::apply_semantics((__bridge UIView*)native, value);
    }

    void layout_platform::update_input_transparent(bool value)
    {
        maui::platform::ios::apply_input_transparent((__bridge UIView*)native, value);
    }

    // ILayout.ClipsToBounds → UIView.ClipsToBounds, the REAL UIKit property (C# LayoutViewExtensions.
    // UpdateClipsToBounds: layoutView.ClipsToBounds = layout.ClipsToBounds — no layer dance needed,
    // unlike the AppKit twin's layer.masksToBounds).
    void layout_platform::update_clips_to_bounds(bool value)
    {
        clips_to_bounds = value; // keep the mirror in sync with the base
        as_panel(native).clipsToBounds = static_cast<BOOL>(value);
    }
} // namespace maui::core
