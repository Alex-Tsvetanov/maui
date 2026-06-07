// image_handler — headless platform recipe. Mirrors the mapped aspect into image_platform so tests can
// observe it. The Apple twin is src/platform/apple/image_handler.mm.

#include "maui/core/image_handler.hpp"

#include <memory>

#include "maui/core/i_image.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

namespace maui::core
{
    // Backend-defined (the Apple twin CFReleases `native`). image_platform holds only trivially-
    // destructible members, so the headless body just clears the unused native slot — an explicit,
    // user-provided body (not `= default`) so the destructor mirrors the Apple RAII shape without
    // tripping performance-trivially-destructible on this trivial-member struct.
    image_platform::~image_platform()
    {
        native = nullptr;
    }

    std::unique_ptr<image_platform> image_handler::create_platform_view()
    {
        return std::make_unique<image_platform>();
    }

    void image_handler::map_aspect(image_handler& handler, i_image& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->image_aspect = view.aspect();
        }
    }

    maui::graphics::size image_handler::get_desired_size(double /*width_constraint*/,
                                                         double /*height_constraint*/) const
    {
        // No image bytes are loaded this cut, so there is no intrinsic content size to report.
        return {0, 0};
    }

    void image_handler::platform_arrange(const maui::graphics::rect& /*frame*/)
    {
        // Headless: no native layout to apply.
    }
} // namespace maui::core
