// shape_view_handler — headless platform recipe. The "native host" is the shape_drawable + the
// invalidation counter on shape_view_platform; replay() stands in for the native drawRect (the
// golden-op tests replay the shape into a recording_canvas). The Apple twin (the shared NSView
// drawing host) is src/platform/apple/shape_view_handler.mm.

#include "maui/core/shape_view_handler.hpp"

#include <memory>

#include "maui/graphics/i_canvas.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/rect_f.hpp"

namespace maui::core
{
    shape_view_platform::~shape_view_platform() = default;

    // The headless drawRect twin: draw the shape through the host's drawable.
    void shape_view_platform::replay(maui::graphics::i_canvas& canvas, const maui::graphics::rect_f& dirty_rect)
    {
        drawable.draw(canvas, dirty_rect);
    }

    std::unique_ptr<shape_view_platform> shape_view_handler::create_platform_view()
    {
        return std::make_unique<shape_view_platform>();
    }

    // C# UpdateShape (ShapeViewExtensions): the host's drawable renders this shape view. The fresh
    // ShapeDrawable allocation collapses onto an in-place re-point (header note); the winding /
    // render-transform pushes re-read with it.
    void shape_view_handler::update_shape()
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->drawable.update_shape_view(virtual_view());
        refresh_drawable_state();
        platform->invalidations++;
    }

    // C# InvalidateShape: refresh the drawable pushes + count the redraw request.
    void shape_view_handler::invalidate_shape()
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        refresh_drawable_state();
        platform->invalidations++;
    }

    void shape_view_handler::arrange_native(const maui::graphics::rect& /*frame*/)
    {
        // Headless: no native host to frame.
    }
} // namespace maui::core
