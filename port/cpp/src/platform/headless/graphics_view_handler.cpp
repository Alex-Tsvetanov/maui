// graphics_view_handler — headless platform recipe. The "native host" is the drawable borrow + the
// invalidation counter on graphics_view_platform; replay() stands in for the native drawRect (the
// golden-op tests replay the drawable into a recording_canvas). The Apple twin (a real NSView whose
// drawRect draws through coregraphics_canvas) is src/platform/apple/graphics_view_handler.mm.

#include "maui/core/graphics_view_handler.hpp"

#include <memory>

#include "maui/graphics/i_canvas.hpp"
#include "maui/graphics/i_drawable.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/rect_f.hpp"

namespace maui::core
{
    graphics_view_platform::~graphics_view_platform() = default;

    // The headless drawRect twin: replay the current drawable into the given canvas.
    void graphics_view_platform::replay(maui::graphics::i_canvas& canvas,
                                        const maui::graphics::rect_f& dirty_rect) const
    {
        if (drawable == nullptr)
        {
            return;
        }
        drawable->draw(canvas, dirty_rect);
    }

    std::unique_ptr<graphics_view_platform> graphics_view_handler::create_platform_view()
    {
        return std::make_unique<graphics_view_platform>();
    }

    // C# UpdateDrawable: point the host at VirtualView.Drawable (the PlatformGraphicsView.Drawable
    // setter also invalidates).
    void graphics_view_handler::update_drawable()
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->drawable = virtual_view() != nullptr ? virtual_view()->drawable() : nullptr;
        platform->invalidations++;
    }

    // C# InvalidateDrawable: count the redraw request.
    void graphics_view_handler::invalidate_drawable()
    {
        if (auto* platform = typed_platform_view())
        {
            platform->invalidations++;
        }
    }

    void graphics_view_handler::arrange_native(const maui::graphics::rect& /*frame*/)
    {
        // Headless: no native host to frame.
    }
} // namespace maui::core
