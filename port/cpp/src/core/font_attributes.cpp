// maui::core::font_attributes — out-of-line WithAttributes / GetFontAttributes (FontExtensions.cs).

#include "maui/core/font_attributes.hpp"

#include "maui/core/font.hpp"

namespace maui::core
{
    font with_attributes(const font& base, font_attributes attributes)
    {
        const bool bold = (attributes & font_attributes::bold) != font_attributes::none;
        const bool italic = (attributes & font_attributes::italic) != font_attributes::none;
        return base.with_weight(bold ? font_weight::bold : font_weight::regular,
                                italic ? font_slant::italic : font_slant::normal);
    }

    font_attributes attributes_of(const font& value)
    {
        font_attributes attributes =
            value.weight() == font_weight::bold ? font_attributes::bold : font_attributes::none;
        if (value.slant() != font_slant::normal)
        {
            attributes |= font_attributes::italic;
        }
        return attributes;
    }
} // namespace maui::core
