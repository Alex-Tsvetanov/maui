// layout_handler — Apple (AppKit / macOS) platform recipe: a plain NSView container that hosts the
// arranged children. The real-native twin of the headless partial. Translated from LayoutHandler.iOS.cs
// (UIKit's LayoutView → a plain AppKit NSView): the panel only HOSTS — each child is positioned by the
// layout_manager via the child's own platform_arrange; the panel just manages the subview list. AppKit's
// origin is lower-left, which the manager's rects already assume (the headless geometry is shared).
// Compiled as Objective-C++ with ARC for the `apple` backend.

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
#include "maui/core/i_element_handler.hpp"
#include "maui/core/i_view.hpp"
#include "maui/core/i_view_handler.hpp"
#include "maui/core/layout_handler.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

namespace
{
    NSView* as_panel(void* native)
    {
        return (__bridge NSView*)native;
    }

    // The child's native NSView, via its view-handler's native_view() (null if the child is unattached or
    // its handler has no native view). native_view() returns the real NSView the pimpl owns — not the
    // pimpl pointer that platform_view() returns.
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
        NSView* const panel = [[NSView alloc] initWithFrame:NSMakeRect(0, 0, 0, 0)];
        platform->native = (__bridge_retained void*)panel; // the void* slot owns one reference
        return platform;
    }

    void layout_handler::add(i_view& child)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        if (NSView* const subview = native_child(child))
        {
            [as_panel(platform->native) addSubview:subview];
            platform->children.push_back(&child);
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

    void layout_handler::insert(int index, i_view& child)
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
        // AppKit has no insert-at-index; positioning within the subview list is by relative ordering.
        const NSInteger count = static_cast<NSInteger>(panel.subviews.count);
        if (index <= 0 || count == 0)
        {
            [panel addSubview:subview positioned:NSWindowBelow relativeTo:panel.subviews.firstObject];
        }
        else if (index >= count)
        {
            [panel addSubview:subview positioned:NSWindowAbove relativeTo:panel.subviews.lastObject];
        }
        else
        {
            [panel addSubview:subview
                   positioned:NSWindowBelow
                   relativeTo:panel.subviews[static_cast<NSUInteger>(index)]];
        }
        auto& children = platform->children;
        const auto position = std::min(static_cast<std::size_t>(std::max(index, 0)), children.size());
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

    void layout_handler::update_z_index(i_view& /*child*/)
    {
        // Re-ordering to honor z-index is deferred (the M3 managers do not yet read z_index); the panel's
        // subview order follows the logical child order maintained by add/insert.
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
} // namespace maui::core
