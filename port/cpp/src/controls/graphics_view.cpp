// maui::controls::graphics_view — out-of-line definitions: the drawable descriptor + the
// default-handler self-registration. See graphics_view.hpp. The property NAME matches
// graphics_view_handler's "drawable" mapper key.

#include "maui/controls/graphics_view.hpp"

#include <memory>

#include "maui/core/bindable_property.hpp"
#include "maui/core/graphics_view_handler.hpp"
#include "maui/core/handler_registry.hpp"
#include "maui/graphics/i_drawable.hpp"

namespace maui::controls
{
    const maui::core::bindable_property<std::shared_ptr<maui::graphics::i_drawable>>& graphics_view::drawable_property()
    {
        // C# GraphicsView.DrawableProperty default: null (nothing to draw).
        static const maui::core::bindable_property<std::shared_ptr<maui::graphics::i_drawable>> descriptor{"drawable"};
        return descriptor;
    }
} // namespace maui::controls

// Self-register the default handler for graphics_view (opt-in, PROFILE §6).
MAUI_REGISTER_HANDLER(maui::controls::graphics_view, maui::core::graphics_view_handler)
