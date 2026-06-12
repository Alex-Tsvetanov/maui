// maui::controls::shapes::path — out-of-line definitions: the Data/RenderTransform descriptors +
// the default-handler self-registration (the shared shape_view_handler). See path.hpp.

#include "maui/controls/shapes/path.hpp"

#include <memory>

#include "maui/controls/shapes/geometry.hpp"
#include "maui/controls/shapes/transform.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/handler_registry.hpp"
#include "maui/core/shape_view_handler.hpp"

namespace maui::controls::shapes
{
    const maui::core::bindable_property<std::shared_ptr<geometry>>& path::data_property()
    {
        // C# Path.DataProperty default: null. The key matches the handler's absorbed sub-handler
        // entry ("data" → InvalidateShape).
        static const maui::core::bindable_property<std::shared_ptr<geometry>> descriptor{"data"};
        return descriptor;
    }

    const maui::core::bindable_property<std::shared_ptr<transform>>& path::render_transform_property()
    {
        // C# Path.RenderTransformProperty default: null.
        static const maui::core::bindable_property<std::shared_ptr<transform>> descriptor{"render_transform"};
        return descriptor;
    }
} // namespace maui::controls::shapes

// Self-register the shared shape handler for path (opt-in, PROFILE §6).
MAUI_REGISTER_HANDLER(maui::controls::shapes::path, maui::core::shape_view_handler)
