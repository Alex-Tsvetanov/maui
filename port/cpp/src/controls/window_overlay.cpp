// maui::controls::window_overlay — the concrete window overlay: the element list + Draw + the
// initialize/invalidate/deinitialize drawing lifecycle through the owned graphics_view. See
// window_overlay.hpp. Ported from WindowOverlay.cs (+ the .Standard / .iOS lifecycle partials).
//
// The drawing seam mirrors C#'s OverlayGraphicsView(this): the owned graphics_view's drawable is a
// self-adapter forwarding to window_overlay::draw, so the overlay renders through the SAME
// graphics_view → graphics_view_handler → native graphics host (W2-23) as any GraphicsView. The
// handler is resolved from the default registry on initialize() (backend-correct) and detached on
// deinitialize().

#include "maui/controls/window_overlay.hpp"

#include <algorithm>
#include <memory>
#include <utility>
#include <vector>

#include "maui/core/handler_registry.hpp"
#include "maui/core/i_element_handler.hpp"
#include "maui/core/i_window_overlay_element.hpp"
#include "maui/graphics/i_canvas.hpp"
#include "maui/graphics/point.hpp"
#include "maui/graphics/rect_f.hpp"

namespace maui::controls
{
    window_overlay::window_overlay(maui::core::i_window* window) : window_(window)
    {
        // C#'s OverlayGraphicsView is constructed with `this` as the drawable; the port wires the
        // self-adapter as the graphics_view's drawable so the overlay's elements render through it.
        self_drawable_ = std::make_shared<overlay_drawable>(*this);
        graphics_view_.set_drawable(self_drawable_);
    }

    window_overlay::~window_overlay() = default;

    void window_overlay::set_is_visible(bool value)
    {
        // C# WindowOverlay.IsVisible setter: store, and invalidate when the platform layer is up.
        is_visible_ = value;
        if (is_platform_view_initialized_)
        {
            invalidate();
        }
    }

    void window_overlay::draw(maui::graphics::i_canvas& canvas, const maui::graphics::rect_f& dirty_rect)
    {
        // C# WindowOverlay.Draw: nothing while hidden; else replay every window element.
        if (!is_visible_)
        {
            return;
        }
        for (maui::core::i_window_overlay_element* const element : elements_)
        {
            element->draw(canvas, dirty_rect);
        }
    }

    void window_overlay::invalidate()
    {
        // C# WindowOverlay.Invalidate (.iOS): _graphicsView?.InvalidateDrawable(). Drive the owned
        // graphics_view's invalidate command (→ native setNeedsDisplay / the headless redraw count).
        graphics_view_.invalidate();
    }

    bool window_overlay::add_window_element(maui::core::i_window_overlay_element& element)
    {
        // C# WindowOverlay.AddWindowElement: HashSet.Add (false if already present), then Invalidate.
        if (std::ranges::find(elements_, &element) != elements_.end())
        {
            invalidate();
            return false;
        }
        elements_.push_back(&element);
        invalidate();
        return true;
    }

    bool window_overlay::remove_window_element(maui::core::i_window_overlay_element& element)
    {
        // C# WindowOverlay.RemoveWindowElement: HashSet.Remove (false if absent), then Invalidate.
        const auto it = std::ranges::find(elements_, &element);
        if (it == elements_.end())
        {
            invalidate();
            return false;
        }
        elements_.erase(it);
        invalidate();
        return true;
    }

    void window_overlay::remove_window_elements()
    {
        // C# WindowOverlay.RemoveWindowElements: Clear, then Invalidate.
        elements_.clear();
        invalidate();
    }

    bool window_overlay::initialize()
    {
        // C# WindowOverlay.Initialize (.Standard returns true; .iOS builds the OverlayGraphicsView).
        // The port attaches the graphics_view's handler from the default registry (backend-correct) so
        // the draw layer exists, then marks itself initialized. Idempotent (already-initialized is true).
        if (is_platform_view_initialized_)
        {
            return true;
        }
        if (!graphics_view_.handler())
        {
            auto handler = maui::core::default_handler_registry().create_handler<graphics_view>();
            if (!handler)
            {
                return false; // no graphics_view handler registered — cannot wire the draw layer
            }
            graphics_view_.set_handler(std::shared_ptr<maui::core::i_element_handler>(std::move(handler)));
        }
        is_platform_view_initialized_ = true;
        return true;
    }

    bool window_overlay::deinitialize()
    {
        // C# WindowOverlay.Deinitialize → DeinitializePlatformDependencies: tear the platform layer
        // down (the port detaches the graphics_view's handler) and clear the initialized flag.
        graphics_view_.set_handler(nullptr);
        is_platform_view_initialized_ = false;
        return true;
    }

    void window_overlay::on_tapped_internal(const maui::graphics::point& point)
    {
        // C# WindowOverlay.OnTappedInternal: collect the drawable elements containing the point (when
        // EnableDrawableTouchHandling), then raise Tapped. The DisableUITouchEventPassthrough branch
        // (visual-tree GetVisualTreeElements) is deferred — the port has no visual-tree hit test.
        std::vector<const maui::core::i_window_overlay_element*> hit;
        if (enable_drawable_touch_handling_)
        {
            for (const maui::core::i_window_overlay_element* const element : elements_)
            {
                if (element->contains(point))
                {
                    hit.push_back(element);
                }
            }
        }
        tapped.raise(point, hit);
    }
} // namespace maui::controls
