#pragma once
// maui::graphics::image_paint  <=  Microsoft.Maui.Graphics.ImagePaint
//
// A drawing-canvas Paint whose fill is a drawing-layer image (IImage): ICanvas.SetFillPaint detects it
// and the Fill* methods tile the image across the shape (the CoreGraphics backend's FillWithImage, a
// CGPattern with the CGImage drawn in the tile callback). Ported from
// src/Graphics/src/Graphics/ImagePaint.cs — a Paint subclass carrying an `IImage Image` whose
// IsTransparent is hard-coded false (transparency depends on the image's pixels, which Paint can't know).
//
// NOTE: this is the *drawing-canvas* (ICanvas Fill*) image paint, distinct from the *view-background*
// core::image_source_paint (the IView.Background-as-image brush). They are separate C# types and serve
// separate paths.
//
// Inherits maui::graphics::paint (PROFILE §11 — runtime-polymorphic; the canvas SetFillPaint
// dynamic_casts to it). background_color() returns the base default (C# ImagePaint does not set
// BackgroundColor; an image paint has no flat fill color). is_transparent() is false, exactly as C#.
//
// Ownership: the image is held NON-OWNING (a raw borrow), the same convention as core::image_source_paint
// — the caller owns the image's lifetime; the paint references which image to tile, mirroring C#'s
// ImagePaint.Image reference property. The canvas reads it during the fill only.

#include "maui/graphics/color.hpp"
#include "maui/graphics/paint.hpp"

namespace maui::graphics
{
    class i_graphics_image;

    class image_paint final : public paint
    {
    public:
        image_paint() = default;
        explicit image_paint(i_graphics_image* image) : image_(image)
        {
        }

        // C# ImagePaint.Image — the image to tile as the fill (non-owning borrow; null when unset).
        [[nodiscard]] i_graphics_image* image() const noexcept
        {
            return image_;
        }
        void set_image(i_graphics_image* value) noexcept
        {
            image_ = value;
        }

        // ---- paint ----
        // C# ImagePaint does not override BackgroundColor — it stays the base default.
        [[nodiscard]] maui::graphics::color background_color() const override
        {
            return maui::graphics::color{};
        }
        // C# ImagePaint.IsTransparent => false (always; the actual transparency is in the image content).
        [[nodiscard]] bool is_transparent() const override
        {
            return false;
        }

    private:
        i_graphics_image* image_ = nullptr; // non-owning (the caller owns the image)
    };
} // namespace maui::graphics
