// maui::controls::image — out-of-line definitions: the shared bindable-property descriptors and the
// default-handler self-registration. See image.hpp. The aspect default is AspectFit (matches C#
// Image.Aspect's documented default); the source default is null (no source set). is_opaque /
// is_animation_playing / is_loading all default to false (C# ImageElement / Image.IsLoading defaults). The
// property NAMES match image_handler's mapper keys exactly so a property change drives the right MapXxx.

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

    const maui::core::bindable_property<bool>& image::is_opaque_property()
    {
        // C# ImageElement.IsOpaqueProperty default: false. Key matches image_handler's "is_opaque" mapper.
        static const maui::core::bindable_property<bool> descriptor{"is_opaque", false};
        return descriptor;
    }

    const maui::core::bindable_property<bool>& image::is_animation_playing_property()
    {
        // C# ImageElement.IsAnimationPlayingProperty default: false. Key matches "is_animation_playing".
        static const maui::core::bindable_property<bool> descriptor{"is_animation_playing", false};
        return descriptor;
    }

    const maui::core::bindable_property<bool>& image::is_loading_property()
    {
        // C# Image.IsLoadingProperty (read-only) default: false. Not mapped to the handler — it is the
        // control's own loading state, written by update_is_loading.
        static const maui::core::bindable_property<bool> descriptor{"is_loading", false};
        return descriptor;
    }
} // namespace maui::controls

// Self-register the default handler for image (opt-in, PROFILE §6).
MAUI_REGISTER_HANDLER(maui::controls::image, maui::core::image_handler)
