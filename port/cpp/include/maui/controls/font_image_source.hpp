#pragma once
// maui::controls::font_image_source  <=  Microsoft.Maui.Controls.FontImageSource
//
// A concrete image source that renders a glyph from a font. Ported from
// src/Controls/src/Core/FontImageSource.cs (the Color / FontFamily / Glyph / Size / FontAutoScalingEnabled
// properties + IsEmpty => string.IsNullOrEmpty(Glyph)) and the IFontImageSource contract. It owns the glyph
// string, the tint color, and a maui::core::font assembled from FontFamily + Size + FontAutoScalingEnabled.
//
// DEFAULTS (match C#): Size = 30 (FontImageSource.SizeProperty default), FontAutoScalingEnabled = false,
// Color = the default (transparent) color, Glyph / FontFamily empty. IsEmpty is true while the glyph is
// empty (C# string.IsNullOrEmpty(Glyph)).
//
// Unlike the bindable FontImageSource control in C# (a BindableObject), this is a plain value-ish source
// object (like file_image_source / uri_image_source) — the image control owns it as a shared_ptr and a
// source change re-runs the load. Minted via image_source::from_font(...) (the factory lives alongside
// from_file in file_image_source.hpp). The glyph is rasterized ASYNCHRONOUSLY by font_image_source_service
// through the image_source_loader (apple draws an NSAttributedString into an NSImage; headless mirrors it).

#include <string>
#include <string_view>
#include <utility>

#include "maui/core/font.hpp"
#include "maui/core/i_font_image_source.hpp"
#include "maui/graphics/color.hpp"

namespace maui::controls
{
    class font_image_source : public maui::core::i_font_image_source
    {
    public:
        // C# FontImageSource.SizeProperty default (30 device-independent units).
        static constexpr double default_size = 30.0;

        // glyph: the character(s) to render. font: the typeface (built via maui::core::font, carrying family
        // + size + auto-scaling). color: the tint.
        font_image_source(std::string glyph, maui::core::font font, maui::graphics::color color)
            : glyph_(std::move(glyph)), font_(std::move(font)), color_(color)
        {
        }

        // C# FontImageSource.IsEmpty => string.IsNullOrEmpty(Glyph).
        [[nodiscard]] bool is_empty() const override
        {
            return glyph_.empty();
        }
        [[nodiscard]] maui::graphics::color color() const override
        {
            return color_;
        }
        [[nodiscard]] maui::core::font font() const override
        {
            return font_;
        }
        [[nodiscard]] std::string_view glyph() const override
        {
            return glyph_;
        }

    private:
        std::string glyph_;
        maui::core::font font_;
        maui::graphics::color color_;
    };
} // namespace maui::controls
