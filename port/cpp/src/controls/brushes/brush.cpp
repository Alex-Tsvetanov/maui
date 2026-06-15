// maui::controls::brush + immutable_brush out-of-line definitions (the named statics + has_transparency +
// the immutable parent refusal). Headers: brushes/brush.hpp, brushes/immutable_brush.hpp.

#include "maui/controls/brushes/brush.hpp"

#include <stdexcept>

#include "maui/controls/brushes/gradient_brush.hpp"
#include "maui/controls/brushes/immutable_brush.hpp"
#include "maui/controls/brushes/solid_color_brush.hpp"
#include "maui/controls/element.hpp"
#include "maui/graphics/color.hpp"
#include "maui/graphics/colors.hpp" // IWYU pragma: keep  (colors:: used via the named_brushes.inc X-macro)

namespace maui::controls
{
    bool brush::has_transparency(const brush* background)
    {
        // C# Brush.HasTransparency: a SolidColorBrush is transparent if Color.Alpha < 1 (null Color is NOT
        // transparent — null?.Alpha < 1 is false); a GradientBrush is transparent if ANY stop's color has
        // alpha < 1 (a null/absent stop color does not count). Other / null brushes are opaque.
        if (background == nullptr)
        {
            return false;
        }
        if (const auto* solid = dynamic_cast<const solid_color_brush*>(background))
        {
            const auto& c = solid->color();
            return c.has_value() && c->alpha < 1.0F;
        }
        if (const auto* gradient = dynamic_cast<const gradient_brush*>(background))
        {
            for (const auto& stop : gradient->gradient_stops().items())
            {
                if (stop)
                {
                    const auto& c = stop->color();
                    if (c.has_value() && c->alpha < 1.0F)
                    {
                        return true;
                    }
                }
            }
        }
        return false;
    }

    // ---- immutable_brush parent refusal ----
    void immutable_brush::on_logical_parent_changing(element* new_parent)
    {
        // C# ImmutableBrush.OnParentChangingCore — a shared system brush cannot be parented. A null parent
        // (detach) is permitted so teardown never throws.
        if (new_parent != nullptr)
        {
            throw std::logic_error("Parent cannot be set on this Brush.");
        }
    }

    // ---- named statics (Brush.AliceBlue …) ----
    // Each is a function-local immutable_brush singleton seeded with the matching graphics::colors constant
    // (C#: `static ImmutableBrush x; public static SolidColorBrush X => x ??= new(Colors.X);`). Returned by
    // reference as solid_color_brush& (immutable_brush is-a solid_color_brush). Generated off the shared
    // color table so the statics can never drift from the colors.
#define MAUI_CONTROLS_NAMED_BRUSH(name, str, argb)                                                                     \
    solid_color_brush& brush::name()                                                                                   \
    {                                                                                                                  \
        static immutable_brush instance{::maui::graphics::colors::name};                                               \
        return instance;                                                                                               \
    }
#include "maui/controls/brushes/named_brushes.inc"
#undef MAUI_CONTROLS_NAMED_BRUSH
} // namespace maui::controls
