// maui::controls::image — out-of-line definitions: the shared bindable-property descriptors and the
// default-handler self-registration. See image.hpp. The aspect default is AspectFit (matches C#
// Image.Aspect's documented default); the source default is null (no source set).

#include "maui/controls/image.hpp"

#include <memory>

#include "maui/core/aspect.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/handler_registry.hpp"
#include "maui/core/i_image_source.hpp"
#include "maui/core/image_handler.hpp"

namespace maui::controls
{
    const maui::core::bindable_property<maui::core::aspect>& image::aspect_property()
    {
        static const maui::core::bindable_property<maui::core::aspect> descriptor{"aspect",
                                                                                  maui::core::aspect::aspect_fit};
        return descriptor;
    }

    const maui::core::bindable_property<std::shared_ptr<maui::core::i_image_source>>& image::source_property()
    {
        // Default is a null source (the "source" key matches image_handler's map_source entry).
        static const maui::core::bindable_property<std::shared_ptr<maui::core::i_image_source>> descriptor{"source"};
        return descriptor;
    }
} // namespace maui::controls

// Self-register the default handler for image (opt-in, PROFILE §6).
MAUI_REGISTER_HANDLER(maui::controls::image, maui::core::image_handler)
