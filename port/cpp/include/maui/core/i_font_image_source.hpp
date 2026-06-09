#pragma once
// maui::core::i_font_image_source  <=  Microsoft.Maui.IFontImageSource
//
// The virtual-view contract for an image rendered from a font glyph. Ported from
// src/Core/src/ImageSources/IFontImageSource.cs:
//     interface IFontImageSource : IImageSource { Color Color; Font Font; string Glyph; }
// Header-only abstract interface (no out-of-line state) — lives in maui_core with no .cpp.
//
// The font (family + size + auto-scaling) is carried by the maui::core::font value type, matching C#'s
// IFontImageSource.Font (a Microsoft.Maui.Font built from FontImageSource's FontFamily / Size /
// FontAutoScalingEnabled). The glyph is the character(s) to render; the color tints it.
//
// SIMPLIFICATION vs C#: C#'s FontImageSourceService resolves the glyph's typeface through an IFontManager
// (registered fonts by family name). The port has no font manager, so the service renders with the font
// value directly (apple maps font→NSFont via apple_conversions). Documented as a deviation in
// font_image_source_service.hpp.

#include <string_view>

#include "maui/core/font.hpp"
#include "maui/core/i_image_source.hpp"
#include "maui/graphics/color.hpp"

namespace maui::core
{
    // Inherits the virtual destructor + protected copy/move from i_image_source (the layered-interface
    // convention, like i_file_image_source / i_uri_image_source), adding Color / Font / Glyph.
    class i_font_image_source : public i_image_source
    {
    public:
        // The tint color of the rendered glyph (C# IFontImageSource.Color).
        [[nodiscard]] virtual maui::graphics::color color() const = 0;

        // The font the glyph is drawn from — family, size, auto-scaling (C# IFontImageSource.Font).
        [[nodiscard]] virtual maui::core::font font() const = 0;

        // The glyph character(s) to render (C# IFontImageSource.Glyph). The referent is owned by the
        // concrete source and stays valid for its lifetime.
        [[nodiscard]] virtual std::string_view glyph() const = 0;
    };
} // namespace maui::core
