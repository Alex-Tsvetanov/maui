// maui::graphics::system_background_paint — out-of-line definitions. See system_background_paint.hpp.
// The static fallback color is opaque white (the light-mode value of UIColor.systemBackground); the
// Apple backends override this to the dynamic system background via a dynamic_cast to this type.

#include "maui/graphics/system_background_paint.hpp"

#include "maui/graphics/color.hpp"

namespace maui::graphics
{
    system_background_paint::system_background_paint()
        : solid_paint(maui::graphics::color(1.0F, 1.0F, 1.0F, 1.0F)) // opaque white light-mode fallback
    {
    }

    void system_background_paint::anchor()
    {
    }
} // namespace maui::graphics
