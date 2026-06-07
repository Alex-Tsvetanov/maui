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
#include <vector>

#include "maui/core/i_element_handler.hpp"
#include "maui/core/i_view.hpp"
#include "maui/core/i_view_handler.hpp"
#include "maui/core/layout_handler.hpp"
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
        for (NSView* const subview in [panel.subviews copy])
        {
            [subview removeFromSuperview];
        }
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
        // Swap the existing subview at `index` for the new child's, in place.
        if (index >= 0 && index < static_cast<int>(panel.subviews.count))
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
} // namespace maui::core
